// GPS + OLED Display Test
// ESP32-S3-DevKitC-1 + BE-880 GPS (UART) + SSD1306 0.96" OLED (I2C)
//
// platformio.ini lib_deps:
//   mikalhart/TinyGPSPlus
//   adafruit/Adafruit GFX Library
//   adafruit/Adafruit SSD1306
//
// Wiring:
//   BE-880 TX  → GPIO 18 (GPS_RX_PIN)   — GPS talks to ESP32 at 115200 baud
//   BE-880 RX  → GPIO 17 (GPS_TX_PIN)
//   OLED SDA   → GPIO 8  (OLED_SDA_PIN)
//   OLED SCL   → GPIO 9  (OLED_SCL_PIN)
//   3.3V + GND → both modules
//
// Baud rates:
//   GPS_BAUD (115200) — GPSSerial UART1 to BE-880 module (configured at 115200, not factory default)
//   115200            — Serial USB CDC to computer (serial monitor)

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include "common/config.h"

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL_MS = 1000;

// ── Setup ──────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);          // USB CDC — serial monitor on computer
    Serial.println("BE-880 GPS Test with OLED");

    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("ERROR: SSD1306 init failed — check I2C wiring");
        while (1);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("  === GPS TEST ===");
    display.println("  Initializing...");
    display.display();

    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);  // 9600 baud to BE-880
    Serial.println("Waiting for GPS data — raw NMEA below:");
}

// ── Display helpers ─────────────────────────────────────────────────────────

void showSearching() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("  === GPS TEST ===");

    display.setTextSize(2);
    display.setCursor(0, 16);
    display.println("SEARCHING");

    display.setTextSize(1);
    display.setCursor(0, 38);
    display.print("Sats visible: ");
    display.println(gps.satellites.isValid() ? (int)gps.satellites.value() : 0);

    display.print("Chars recv'd: ");
    display.println(gps.charsProcessed());

    display.print("Data flowing: ");
    display.println(gps.charsProcessed() > 10 ? "YES" : "NO");

    display.display();
}

void showFix() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("  === GPS TEST ===");

    display.print("Fix: GPS  Sats: ");
    display.println(gps.satellites.value());

    display.print("Lat: ");
    if (gps.location.lat() >= 0) display.print(" ");
    display.println(gps.location.lat(), 6);

    display.print("Lon: ");
    display.println(gps.location.lng(), 6);

    display.print("HDOP: ");
    display.print(gps.hdop.hdop(), 1);
    display.print("  Alt: ");
    display.print((int)gps.altitude.meters());
    display.println("m");

    display.print("Age: ");
    display.print(gps.location.age());
    display.println("ms");

    display.display();
}

// ── Main loop ───────────────────────────────────────────────────────────────

void loop() {
    // Feed every byte from GPS UART to TinyGPS++ and echo NMEA to serial monitor
    while (GPSSerial.available()) {
        char c = GPSSerial.read();
        gps.encode(c);

        static char lineBuf[128];
        static uint8_t lineIdx = 0;
        if (c == '\n' || lineIdx >= sizeof(lineBuf) - 1) {
            lineBuf[lineIdx] = '\0';
            if (lineIdx > 1) Serial.println(lineBuf);
            lineIdx = 0;
        } else if (c != '\r') {
            lineBuf[lineIdx++] = c;
        }
    }

    // Refresh display at 1 Hz
    if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL_MS) {
        lastDisplayUpdate = millis();

        if (gps.location.isValid()) {
            showFix();
            Serial.print("[GPS] Fix: ");
            Serial.print(gps.location.lat(), 7);
            Serial.print(", ");
            Serial.print(gps.location.lng(), 7);
            Serial.print("  Sats: "); Serial.print(gps.satellites.value());
            Serial.print("  HDOP: "); Serial.print(gps.hdop.hdop(), 1);
            Serial.print("  Alt: "); Serial.print(gps.altitude.meters(), 1);
            Serial.print("m  Age: "); Serial.print(gps.location.age());
            Serial.println("ms");
        } else {
            showSearching();
        }
    }
}
