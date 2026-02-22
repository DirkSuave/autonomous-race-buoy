#include <Arduino.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "common/config.h"

// -----------------------------------------------------------------------
// Set CALIBRATION_MODE 1 to find your axis min/max values:
//   - Flash with CALIBRATION_MODE 1
//   - Slowly rotate the board through all orientations for ~30 seconds
//   - Note the X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX printed
//   - Paste those values into the setCalibration() call below
//   - Reflash with CALIBRATION_MODE 0 for normal operation
// -----------------------------------------------------------------------
#define CALIBRATION_MODE 0

// Calibration values — replace after running CALIBRATION_MODE 1
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

QMC5883LCompass compass;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#if CALIBRATION_MODE
int16_t xMin, xMax, yMin, yMax, zMin, zMax;
#endif

bool oledOk = false;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) delay(10);

    Serial.println("Compass Test — QMC5883L");

    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    // I2C scan
    Serial.println("I2C scan:");
    bool compassFound = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("  0x");
            Serial.print(addr, HEX);
            if (addr == COMPASS_ADDR) { Serial.print("  <- QMC5883L"); compassFound = true; }
            if (addr == OLED_ADDRESS) Serial.print("  <- OLED");
            Serial.println();
        }
    }

    if (!compassFound) {
        Serial.println("ERROR: QMC5883L not found at 0x0D — check wiring");
        while (1) delay(1000);
    }

    // OLED init (optional — continue if absent)
    oledOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (!oledOk) {
        Serial.println("OLED not found — serial output only");
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("== COMPASS TEST ==");
        display.println("Initialising...");
        display.display();
    }

    // Compass init
    compass.setADDR(COMPASS_ADDR);
    compass.init();
    compass.setSmoothing(5, true);

#if CALIBRATION_MODE
    Serial.println("CALIBRATION MODE — rotate board slowly through all axes");
    Serial.println("Watch for min/max values to stabilise, then note them.");
    xMin = xMax = yMin = yMax = zMin = zMax = 0;
#else
    compass.setCalibration(CAL_X_MIN, CAL_X_MAX, CAL_Y_MIN, CAL_Y_MAX, CAL_Z_MIN, CAL_Z_MAX);
    Serial.println("Running — rotate board to verify heading changes.");
    Serial.println("Heading   X       Y       Z");
#endif
}

void loop() {
    compass.read();

    int16_t x = compass.getX();
    int16_t y = compass.getY();
    int16_t z = compass.getZ();
    int     az = compass.getAzimuth();

    char dir[3];
    compass.getDirection(dir, az);

#if CALIBRATION_MODE
    bool updated = false;
    if (x < xMin) { xMin = x; updated = true; }
    if (x > xMax) { xMax = x; updated = true; }
    if (y < yMin) { yMin = y; updated = true; }
    if (y > yMax) { yMax = y; updated = true; }
    if (z < zMin) { zMin = z; updated = true; }
    if (z > zMax) { zMax = z; updated = true; }

    if (updated) {
        Serial.print("setCalibration(");
        Serial.print(xMin); Serial.print(", ");
        Serial.print(xMax); Serial.print(", ");
        Serial.print(yMin); Serial.print(", ");
        Serial.print(yMax); Serial.print(", ");
        Serial.print(zMin); Serial.print(", ");
        Serial.print(zMax); Serial.println(");");
    }

    if (oledOk) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("= CALIBRATION MODE =");
        display.print("Rotate board slowly");
        display.setCursor(0, 24);
        display.print("X: "); display.print(xMin); display.print(" / "); display.println(xMax);
        display.print("Y: "); display.print(yMin); display.print(" / "); display.println(yMax);
        display.print("Z: "); display.print(zMin); display.print(" / "); display.println(zMax);
        display.display();
    }

#else
    Serial.print(az); Serial.print("° ");
    Serial.print(dir[0]); Serial.print(dir[1]); Serial.print(dir[2]);
    Serial.print("   X:"); Serial.print(x);
    Serial.print("  Y:"); Serial.print(y);
    Serial.print("  Z:"); Serial.println(z);

    if (oledOk) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("== COMPASS TEST ==");

        display.setTextSize(2);
        display.setCursor(0, 16);
        display.print(az);
        display.print((char)247);  // degree symbol
        display.print(" ");
        display.print(dir[0]); display.print(dir[1]); display.print(dir[2]);

        display.setTextSize(1);
        display.setCursor(0, 40);
        display.print("X:"); display.print(x);
        display.print(" Y:"); display.print(y);
        display.setCursor(0, 52);
        display.print("Z:"); display.println(z);

        display.display();
    }
#endif

    delay(200);
}
