#include <Arduino.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "common/config.h"

// ---------------------------------------------------------------------------
// VANE_OFFSET — set after calibration:
//   Point the buoy bow directly into the wind, read vane_raw from serial,
//   then set that value here and reflash.  After calibration, vane_relative
//   reads 0° when the bow is pointing directly into the wind.
// ---------------------------------------------------------------------------
#define VANE_OFFSET 0.0f

// Compass calibration — replace after running compass_test in CALIBRATION_MODE 1
#define CAL_X_MIN  -1500
#define CAL_X_MAX   1500
#define CAL_Y_MIN  -1500
#define CAL_Y_MAX   1500
#define CAL_Z_MIN  -1500
#define CAL_Z_MAX   1500

#define COMPASS_ADDR   0x0D
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1

QMC5883LCompass  compass;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool oledOk = false;

// ---------------------------------------------------------------------------
// Wind speed — ISR + 1-second pulse integration (unchanged from .ino)
// ---------------------------------------------------------------------------
volatile uint32_t windSpeedPulseCount = 0;
volatile uint32_t lastWindSpeedPulse  = 0;
uint32_t          lastWindSpeedCalc   = 0;
float             windSpeed           = 0.0f;

void IRAM_ATTR windSpeedISR() {
    uint32_t now = millis();
    if (now - lastWindSpeedPulse > 10) {   // 10 ms debounce
        windSpeedPulseCount++;
        lastWindSpeedPulse = now;
    }
}

float updateWindSpeed() {
    uint32_t now     = millis();
    uint32_t elapsed = now - lastWindSpeedCalc;
    if (elapsed >= 1000) {
        float pps  = (float)windSpeedPulseCount / (elapsed / 1000.0f);
        windSpeed  = pps * 1.609344f;   // Davis: 1 pulse/s = 1 mph → km/h
        windSpeedPulseCount = 0;
        lastWindSpeedCalc   = now;
    }
    return windSpeed;
}

// ---------------------------------------------------------------------------
// Wind vane — raw potentiometer reading mapped to 0–360°
// ---------------------------------------------------------------------------
float readVaneRaw() {
    return (analogRead(WIND_DIR_PIN) / 4095.0f) * 360.0f;
}

// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) delay(10);

    Serial.println("Wind + Compass Sensor Fusion Test — Module 6");
    Serial.println("vane_raw\tvane_rel\tcompass\tabs_wind\terror\tspeed_kmh");

    pinMode(WIND_SPEED_PIN, INPUT_PULLUP);
    pinMode(WIND_DIR_PIN,   INPUT);
    attachInterrupt(digitalPinToInterrupt(WIND_SPEED_PIN), windSpeedISR, FALLING);
    lastWindSpeedCalc = millis();

    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    // I2C scan — confirm both devices are present
    Serial.println("I2C scan:");
    bool compassFound = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("  0x"); Serial.print(addr, HEX);
            if (addr == COMPASS_ADDR) { Serial.print("  <- QMC5883L"); compassFound = true; }
            if (addr == OLED_ADDRESS)   Serial.print("  <- OLED");
            Serial.println();
        }
    }

    if (!compassFound) {
        Serial.println("ERROR: QMC5883L not found at 0x0D — check wiring");
        while (1) delay(1000);
    }

    // OLED is optional — continue without it
    oledOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (!oledOk) {
        Serial.println("OLED not found — serial output only");
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("= WIND+COMPASS TEST =");
        display.println("Initialising...");
        display.display();
    }

    compass.setADDR(COMPASS_ADDR);
    compass.init();
    compass.setSmoothing(5, true);
    compass.setCalibration(CAL_X_MIN, CAL_X_MAX, CAL_Y_MIN, CAL_Y_MAX, CAL_Z_MIN, CAL_Z_MAX);

    Serial.println("Ready.");
    Serial.println("VANE_OFFSET calibration: point bow into wind, note vane_raw,");
    Serial.println("  set #define VANE_OFFSET to that value, then reflash.");
}

// ---------------------------------------------------------------------------
void loop() {
    float speed = updateWindSpeed();

    // --- Compass ---
    compass.read();
    int compass_heading = compass.getAzimuth();   // 0–359°, magnetic north = 0°

    // --- Sensor fusion ---
    //
    // vane_raw      : raw potentiometer angle (0–360°)
    // vane_relative : offset-corrected angle — 0° = wind from bow
    // abs_wind_dir  : absolute magnetic direction wind is coming FROM
    // heading_error : control signal — positive = wind from starboard (turn right)
    //                                  negative = wind from port     (turn left)
    //
    float vane_raw      = readVaneRaw();
    float vane_relative = fmod(vane_raw - VANE_OFFSET + 360.0f, 360.0f);
    float abs_wind_dir  = fmod((float)compass_heading + vane_relative, 360.0f);
    float heading_error = vane_relative;
    if (heading_error > 180.0f) heading_error -= 360.0f;

    // Serial — tab-separated, one line per sample
    Serial.print(vane_raw, 1);      Serial.print("\t");
    Serial.print(vane_relative, 1); Serial.print("\t");
    Serial.print(compass_heading);  Serial.print("\t");
    Serial.print(abs_wind_dir, 1);  Serial.print("\t");
    Serial.print(heading_error, 1); Serial.print("\t");
    Serial.println(speed, 2);

    // OLED
    if (oledOk) {
        display.clearDisplay();
        display.setTextSize(1);

        display.setCursor(0, 0);
        display.println("= WIND+COMPASS =");

        display.setCursor(0, 12);
        display.print("Vane:");
        display.print(vane_relative, 0);
        display.print((char)247);   // degree symbol
        display.print(" Err:");
        display.print(heading_error, 0);
        display.print((char)247);

        display.setCursor(0, 24);
        display.print("Cmpss:");
        display.print(compass_heading);
        display.print((char)247);

        display.setCursor(0, 36);
        display.print("Wind:");
        display.print(abs_wind_dir, 0);
        display.print((char)247);
        display.print(" (abs)");

        display.setCursor(0, 50);
        display.print(speed, 1);
        display.print("km/h ");
        display.print(speed / 1.852f, 1);
        display.print("kt");

        display.display();
    }

    delay(200);
}
