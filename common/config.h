#ifndef CONFIG_H
#define CONFIG_H

// LoRa (SPI) — HSPI defaults on ESP32-S3
#define LORA_CS_PIN     10
#define LORA_RST_PIN    14
#define LORA_IRQ_PIN    21
#define LORA_FREQ       915.0

// SPI bus (shared with LoRa)
#define SPI_MOSI_PIN    11
#define SPI_MISO_PIN    13
#define SPI_SCK_PIN     12

// GPS (UART)
#define GPS_RX_PIN      18
#define GPS_TX_PIN      17
#define GPS_BAUD        115200

// OLED display (I2C) — ESP32-S3 defaults
#define OLED_SDA_PIN    8
#define OLED_SCL_PIN    9
#define OLED_ADDRESS    0x3C

// Thruster ESCs (PWM via LEDC, 100 Hz)
#define MOTOR_LEFT_PWM  47
#define MOTOR_RIGHT_PWM 48

// Analog inputs — ADC1 only (GPIO 1–10), WiFi-safe
#define BATTERY_ADC_PIN         4   // 11:1 voltage divider
#define WIND_DIR_PIN            5   // Master only: 0–3.3V → 0–360°
#define WIND_SPEED_PIN          6   // Master only: pulse counter

// Ultrasonic collision avoidance (JSN-SR04T Mode 1 — 3× sensors, forward arc)
#define ULTRASONIC_TRIG_FWD   15   // Forward sensor trigger
#define ULTRASONIC_TRIG_PORT  16   // Port-45° sensor trigger
#define ULTRASONIC_TRIG_STBD  19   // Starboard-45° sensor trigger
#define ULTRASONIC_ECHO_FWD   20   // Forward sensor echo
#define ULTRASONIC_ECHO_PORT  22   // Port-45° echo
#define ULTRASONIC_ECHO_STBD  23   // Starboard-45° echo

// Status LEDs
#define LED_GREEN_PIN   38
#define LED_RED_PIN     39

#define VOLTAGE_DIVIDER_RATIO   11.0f
#define LOOP_RATE_HZ            10
#define STATUS_REPORT_INTERVAL_MS 5000

#endif // CONFIG_H
