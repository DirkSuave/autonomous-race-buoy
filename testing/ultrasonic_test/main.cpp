#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "common/config.h"

// ---------------------------------------------------------------------------
// JSN-SR04T Waterproof Ultrasonic Collision Avoidance Test — Module 9
//
// 3 sensors, 150° forward arc:
//   Forward (0°)        TRIG=GPIO15  ECHO=GPIO20
//   Port-forward (-45°) TRIG=GPIO16  ECHO=GPIO22
//   Starboard-fwd (+45°)TRIG=GPIO19  ECHO=GPIO23
//
// Mode 1 (default — no R19 modification): manual TRIG/ECHO
//   10 µs HIGH pulse on TRIG → sensor fires burst → ECHO HIGH for round-trip
//   Distance (cm) = pulseIn(ECHO, HIGH, 30000) × 0.034 / 2
//
// Sensors fire sequentially — eliminates acoustic cross-talk.
// Total cycle ≈ 90 ms (padded to target if sensors return early).
//
// Status logic:
//   STOP       — any sensor < 50 cm  (emergency stop)
//   AVOID PORT — fwd < 200 cm AND port has more clearance than starboard
//   AVOID STBD — fwd < 200 cm AND starboard has more clearance
//   CLEAR      — all sensors ≥ 200 cm
//
// LED:
//   Green steady = CLEAR
//   Green flash  = avoidance zone
//   Red steady   = emergency stop
//
// Serial: CSV — time_ms, fwd_cm, port_cm, stbd_cm, status
// ---------------------------------------------------------------------------

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT    64
#define OLED_RESET       -1

// Zone thresholds
#define DIST_EMERGENCY_CM   50    // any sensor below → emergency stop
#define DIST_AVOIDANCE_CM  200    // forward sensor below → avoidance zone
#define DIST_NONE_CM       500    // pulseIn timeout — treat as no obstacle

// pulseIn timeout: 30 ms → ~510 cm theoretical max (JSN-SR04T range ≤ 450 cm)
#define ECHO_TIMEOUT_US  30000UL

// LED flash period for avoidance zone
#define FLASH_PERIOD_MS  250

// Target loop period
#define LOOP_PERIOD_MS    90

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledOk = false;

// ---------------------------------------------------------------------------
// Fire one JSN-SR04T sensor, return distance in cm.
// Returns DIST_NONE_CM on timeout (no obstacle in range).
// ---------------------------------------------------------------------------
static uint16_t readSensor(uint8_t trigPin, uint8_t echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, ECHO_TIMEOUT_US);
    if (duration == 0) return DIST_NONE_CM;
    return (uint16_t)(duration * 0.034f / 2.0f);
}

// ---------------------------------------------------------------------------
static void printDist(uint16_t cm) {
    if (cm >= DIST_NONE_CM) display.print("---");
    else                     display.print(cm);
    display.print(" cm");
}

// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) delay(10);

    Serial.println("JSN-SR04T Collision Avoidance Test -- Module 9");
    Serial.println("time_ms,fwd_cm,port_cm,stbd_cm,status");

    // Sensor trigger pins — output, start LOW
    pinMode(ULTRASONIC_TRIG_FWD,  OUTPUT); digitalWrite(ULTRASONIC_TRIG_FWD,  LOW);
    pinMode(ULTRASONIC_TRIG_PORT, OUTPUT); digitalWrite(ULTRASONIC_TRIG_PORT, LOW);
    pinMode(ULTRASONIC_TRIG_STBD, OUTPUT); digitalWrite(ULTRASONIC_TRIG_STBD, LOW);

    // Sensor echo pins — input (JSN-SR04T drives 3.3V-compatible HIGH)
    pinMode(ULTRASONIC_ECHO_FWD,  INPUT);
    pinMode(ULTRASONIC_ECHO_PORT, INPUT);
    pinMode(ULTRASONIC_ECHO_STBD, INPUT);

    // Status LEDs
    pinMode(LED_GREEN_PIN, OUTPUT); digitalWrite(LED_GREEN_PIN, LOW);
    pinMode(LED_RED_PIN,   OUTPUT); digitalWrite(LED_RED_PIN,   LOW);

    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    oledOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (!oledOk) {
        Serial.println("OLED not found -- serial output only");
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("= ULTRASONIC M9 =");
        display.println("Initialising...");
        display.display();
    }

    delay(500);
    Serial.println("Ready. Sweep hand in front of each sensor to verify TRIG/ECHO wiring.");
}

// ---------------------------------------------------------------------------
void loop() {
    uint32_t cycleStart = millis();

    // Fire all 3 sensors sequentially — prevents acoustic cross-talk
    uint16_t fwd_cm  = readSensor(ULTRASONIC_TRIG_FWD,  ULTRASONIC_ECHO_FWD);
    uint16_t port_cm = readSensor(ULTRASONIC_TRIG_PORT, ULTRASONIC_ECHO_PORT);
    uint16_t stbd_cm = readSensor(ULTRASONIC_TRIG_STBD, ULTRASONIC_ECHO_STBD);

    // Determine status
    const char* status;
    bool emergencyStop = (fwd_cm  < DIST_EMERGENCY_CM ||
                          port_cm < DIST_EMERGENCY_CM ||
                          stbd_cm < DIST_EMERGENCY_CM);

    if (emergencyStop) {
        status = "STOP";
    } else if (fwd_cm < DIST_AVOIDANCE_CM) {
        // Turn toward the side with more clearance
        status = (port_cm > stbd_cm) ? "AVOID PORT" : "AVOID STBD";
    } else {
        status = "CLEAR";
    }

    // LED control
    static uint32_t lastFlash = 0;
    static bool     flashOn   = false;

    if (emergencyStop) {
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_RED_PIN,   HIGH);
    } else if (fwd_cm < DIST_AVOIDANCE_CM) {
        // Flashing green — avoidance zone
        digitalWrite(LED_RED_PIN, LOW);
        uint32_t now = millis();
        if (now - lastFlash >= FLASH_PERIOD_MS) {
            flashOn   = !flashOn;
            lastFlash = now;
        }
        digitalWrite(LED_GREEN_PIN, flashOn ? HIGH : LOW);
    } else {
        // Steady green — all clear
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(LED_RED_PIN,   LOW);
    }

    // Serial — one CSV line per cycle for logging
    Serial.print(millis());   Serial.print(",");
    Serial.print(fwd_cm);     Serial.print(",");
    Serial.print(port_cm);    Serial.print(",");
    Serial.print(stbd_cm);    Serial.print(",");
    Serial.println(status);

    // OLED
    if (oledOk) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        display.setCursor(0, 0);
        display.println("= ULTRASONIC M9 =");

        display.setCursor(0, 12);
        display.print("FWD:  "); printDist(fwd_cm);

        display.setCursor(0, 22);
        display.print("PORT: "); printDist(port_cm);

        display.setCursor(0, 32);
        display.print("STBD: "); printDist(stbd_cm);

        display.setCursor(0, 50);
        display.print("> ");
        display.println(status);

        display.display();
    }

    // Pad to target cycle period (sensors may return faster when obstacle is near)
    uint32_t elapsed = millis() - cycleStart;
    if (elapsed < LOOP_PERIOD_MS) delay(LOOP_PERIOD_MS - elapsed);
}
