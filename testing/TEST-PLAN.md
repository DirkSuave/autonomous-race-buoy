# Hardware Module Test Plan

Validation sketches for the autonomous race buoy system — ESP32-S3-DevKitC-1 target.
Test in the order listed (simplest first) to isolate dependencies.

---

## Module Overview

| # | Module | Test Sketch | Hardware Required | Status |
|---|--------|-------------|-------------------|--------|
| 1 | OLED Display | `oled_test.ino` | ESP32-S3 + SSD1306 | TODO |
| 2 | GPS + OLED | `gps_test_display.ino` | ESP32-S3 + BE-880 + SSD1306 | **Ready** |
| 3 | Compass | `compass_test.ino` | ESP32-S3 + BE-880 (I2C QMC5883) | TODO |
| 4 | LoRa TX | `lora_test_tx.ino` | 2× ESP32-S3 + RFM95W | Ready |
| 5 | LoRa RX | `lora_test_rx.ino` | 2× ESP32-S3 + RFM95W | Ready |
| 6 | Wind Sensor | `wind_sensor_test.ino` | ESP32-S3 + Davis Vantage Pro | Ready |
| 7 | Battery Monitor | `battery_monitor_test.ino` | ESP32-S3 + 11:1 voltage divider | TODO |
| 8 | ESC / Thrusters | TBD | Hardware not yet purchased | Pending |
| 9 | Collision Avoidance | `ultrasonic_test/main.cpp` | ESP32-S3 + 3× JSN-SR04T + SSD1306 | **Ready** |

**Common dependency:** All sketches `#include "../common/config.h"` for pin assignments.
Always verify `config.h` against `hardware/pinouts/` before flashing a new board.

---

## 1. OLED Display — `oled_test.ino` (TODO)

**Purpose:** Confirm I2C bus and SSD1306 display are wired and functional before
adding any other modules.

**Required libraries (`lib_deps`):**
- `adafruit/Adafruit GFX Library`
- `adafruit/Adafruit SSD1306`

**Wiring:**

| Signal | ESP32-S3 Pin | OLED Pin |
|--------|-------------|----------|
| 3.3V | 3V3 | VCC |
| GND | GND | GND |
| I2C SDA | GPIO 8 | SDA |
| I2C SCL | GPIO 9 | SCL |

**Expected serial output:**
```
OLED Test
I2C address: 0x3C — found
Display initialized OK
Drawing test pattern...
```

**Pass criteria:**
- OLED shows incrementing counter and filled/empty rectangles
- No `SSD1306 allocation failed` in serial monitor

---

## 2. GPS + OLED — `gps_test_display.ino`

**Purpose:** Confirm GPS module is receiving NMEA data and parsing coordinates correctly.
OLED shows live status so the board can be tested away from a computer.

**Required libraries (`lib_deps`):**
- `mikalhart/TinyGPSPlus`
- `adafruit/Adafruit GFX Library`
- `adafruit/Adafruit SSD1306`

**Wiring:**

| Signal | ESP32-S3 Pin | Target |
|--------|-------------|--------|
| 3.3V | 3V3 | GPS VCC, OLED VCC |
| GND | GND | GPS GND, OLED GND |
| GPS data in | GPIO 18 | BE-880 TX |
| GPS data out | GPIO 17 | BE-880 RX |
| I2C SDA | GPIO 8 | OLED SDA |
| I2C SCL | GPIO 9 | OLED SCL |

**Schematic:** `schematics/gps_oled_test.svg` — open in any web browser.

**Expected display — searching:**
```
  === GPS TEST ===
  SEARCHING...

  Sats visible: 7
  Chars recv'd: 12483
  Data flowing: YES
```

**Expected display — fix acquired:**
```
  === GPS TEST ===
  Fix: GPS  Sats: 8
  Lat:  43.654321
  Lon: -79.381234
  HDOP: 0.9  Alt: 85m
  Age: 800ms
```

**Expected serial output:**
- Raw NMEA sentences printed as received (e.g. `$GNGGA,123519,...`)
- `[GPS] Fix acquired: 43.654321, -79.381234` when lock obtained

**Pass criteria:**
- `Chars recv'd` counter increases immediately — confirms UART wiring is correct
- Within 60–90 seconds outdoors, valid lat/lon appear on OLED
- `Sats` ≥ 4, `HDOP` < 2.0 for a usable fix

**Troubleshooting:**
- `Chars recv'd: 0` → check UART wiring (GPS TX → GPIO 18); confirm 3.3V supply
- OLED blank → check I2C address (`0x3C` or `0x3D`); confirm GPIO 8/9 connections
- Fix never arrives indoors → move outdoors or near a window with sky view

---

## 3. Compass — `compass_test.ino` (TODO)

**Purpose:** Read heading from the QMC5883L magnetometer inside the BE-880 module
over I2C (shared bus with OLED).

**Required libraries (`lib_deps`):**
- `mprograms/QMC5883LCompass` (or `dfrobot/DFRobot_QMC5883`)

**Wiring:** Same I2C bus as OLED (GPIO 8/9). The BE-880 QMC5883 typically appears
at I2C address `0x0D`.

**Expected serial output:**
```
Compass Test — QMC5883L
I2C scan: 0x0D found
Heading: 274.3°  X: -120  Y: 45  Z: 890
Heading: 275.1°  ...
```

**Pass criteria:**
- I2C device found at `0x0D`
- Heading changes smoothly when board is rotated by hand
- No heading jump > 10° per reading cycle

---

## 4. LoRa TX — `lora_test_tx.ino`

**Purpose:** Confirm RFM95W transmitter can initialize and send packets.
Requires a second ESP32-S3 running `lora_test_rx.ino` nearby.

**Required libraries (`lib_deps`):**
- `mikem/RadioHead`

**Wiring:**

| Signal | ESP32-S3 Pin | RFM95W Pin |
|--------|-------------|------------|
| 3.3V | 3V3 | VIN |
| GND | GND | GND |
| SPI MOSI | GPIO 11 | MOSI |
| SPI MISO | GPIO 13 | MISO |
| SPI SCK | GPIO 12 | SCK |
| CS | GPIO 10 | CS |
| RST | GPIO 14 | RST |
| IRQ | GPIO 21 | G0 (DIO0) |

**Expected serial output (TX side):**
```
LoRa RFM95W Test - Transmitter
LoRa initialized at 915.00 MHz
Sending: Packet #0 - Test from Master
Received: ACK #0
RSSI: -42  SNR: 9
```

**Pass criteria:**
- TX initializes without `LoRa init failed!`
- RX board ACKs every packet
- RSSI > -100 dBm at < 10 m range

---

## 5. LoRa RX — `lora_test_rx.ino`

**Purpose:** Confirm RFM95W receiver can receive packets and send ACKs.
Run simultaneously with TX board.

**Wiring:** Same as TX above (identical hardware).

**Expected serial output (RX side):**
```
LoRa RFM95W Test - Receiver
LoRa initialized at 915.00 MHz
Received: Packet #0 - Test from Master
RSSI: -40  SNR: 10
Sent ACK #0
```

**Pass criteria:**
- No missed packets at < 10 m range (100% reception)
- RSSI > -100 dBm

---

## 6. Wind Sensor — `wind_sensor_test.ino`

**Purpose:** Validate Davis Vantage Pro anemometer speed (pulse) and direction
(analog 0–3.3V) inputs. Master buoy only.

**Required libraries:** None (bare Arduino API).

**Wiring:**

| Signal | ESP32-S3 Pin | Davis Sensor |
|--------|-------------|--------------|
| GND | GND | Ground |
| Speed pulse | GPIO 6 | Speed output |
| Direction | GPIO 5 | Direction (analog) |

**Expected serial output:**
```
Davis Vantage Pro Wind Sensor Test
Wind Speed: 12.34 km/h (6.67 knots) | Direction: 247.3° | Raw ADC: 3070
```

**Pass criteria:**
- Speed reads 0.00 in calm air; increases when cups are spun by hand
- Direction changes continuously across 0–360° as vane rotates
- Raw ADC spans 0–4095 across full rotation

**Calibration notes:**
- Speed: 1 pulse/second ≈ 1 mph (converted to km/h in sketch)
- Direction: 0 V = 0°, 3.3 V = 360° (may need offset calibration for true north)

---

## 7. Battery Monitor — `battery_monitor_test.ino` (TODO)

**Purpose:** Confirm 4S LiPo voltage is read correctly through 11:1 voltage divider.
ADC1 (GPIO 4) is used — ADC2 is unavailable when WiFi is active.

**Required libraries:** None.

**Wiring:**

| Component | Connection |
|-----------|------------|
| LiPo positive | Top of voltage divider (10 MΩ + 1 MΩ or 100 kΩ + 10 kΩ) |
| Divider output | GPIO 4 |
| Divider bottom | GND |

4S LiPo range: 13.2 V (empty) – 16.8 V (full). With 11:1 divider: 1.20 V – 1.53 V
→ well within ADC range.

**Expected serial output:**
```
Battery Monitor Test
Raw ADC: 1890  Voltage at pin: 1.524V
Battery voltage: 16.76V  (4S: 4.19V/cell)
State: FULL
```

**Pass criteria:**
- Measured voltage within ±0.2 V of multimeter reading
- Voltage decreases as battery is discharged under load

---

## 8. ESC / Thrusters (Pending Hardware)

**Purpose:** Confirm bilge thruster ESCs accept PWM signal and spin motors in
both forward and reverse.

**Status:** Hardware not yet purchased. Test sketch TBD.

**Notes:**
- Use LEDC PWM on GPIO 47 (left) and GPIO 48 (right)
- Standard RC ESC: 1000 µs = full reverse, 1500 µs = stop, 2000 µs = full forward
- Arm sequence required on power-up (send 1500 µs for 2 seconds)
- Perform initial tests with motors out of water and props removed

---

---

## 9. Collision Avoidance — `ultrasonic_test/main.cpp`

**Purpose:** Validate JSN-SR04T waterproof ultrasonic sensors for forward-arc obstacle
detection. Three sensors provide 150° coverage: forward (0°), port-forward (−45°),
and starboard-forward (+45°). Active during transit only (STATE_DEPLOY, STATE_FAILSAFE/RTH).

**Required libraries (`lib_deps`):**
- `adafruit/Adafruit SSD1306`
- `adafruit/Adafruit GFX Library`

No external distance library — `pulseIn()` is sufficient for Mode 1 (default) sensors.

**Wiring:**

| Signal | ESP32-S3 Pin | JSN-SR04T |
|--------|-------------|-----------|
| 3.3V | 3V3 | VCC |
| GND | GND | GND |
| TRIG_FWD | GPIO 15 | Forward TRIG |
| ECHO_FWD | GPIO 20 | Forward ECHO |
| TRIG_PORT | GPIO 16 | Port-45° TRIG |
| ECHO_PORT | GPIO 22 | Port-45° ECHO |
| TRIG_STBD | GPIO 19 | Starboard-45° TRIG |
| ECHO_STBD | GPIO 23 | Starboard-45° ECHO |

Sensors supplied from ESP32 3.3V rail — AJ-SR04M/JSN-SR04T rated 3–5V; 3.3V eliminates
ECHO level-shifting (no voltage divider needed). Space transducer heads ≥15 cm apart.
Mount 10–15 cm above waterline, tilted 5–10° upward.

**Sensor geometry:**

```
        Port-45°   Forward   Starboard-45°
            \         |         /
       ±30°  \   ±30°|±30°   /   ← 60° beam cone per sensor
               \      |      /
  ──────────────\─────│─────/──── hull bow
                  \   │   /
                    BOW
Coverage: 150° total arc
```

**Expected serial output:**
```
JSN-SR04T Collision Avoidance Test -- Module 9
time_ms,fwd_cm,port_cm,stbd_cm,status
1045,---,---,---,CLEAR
1136,---,---,---,CLEAR
1228,87,---,---,AVOID PORT
1319,42,---,---,STOP
```
(`---` = no obstacle in range, ≥ 500 cm / timeout)

**Expected OLED:**
```
= ULTRASONIC M9 =
FWD:  87 cm
PORT: --- cm
STBD: --- cm

> AVOID PORT
```

**Pass criteria:**
1. Build `ultrasonic_test` env — compiles cleanly: `pio run -e ultrasonic_test`
2. Flash and open serial monitor — CSV data flows every ~90 ms
3. Sweep hand in front of each sensor individually — only the swept sensor changes
   (confirms correct TRIG/ECHO pairing and no acoustic cross-talk)
4. Hold object at < 50 cm forward → OLED shows `> STOP`, red LED steady
5. Hold object at ~1 m forward, clear to port → OLED shows `> AVOID PORT`,
   green LED flashing
6. Hold object at ~1 m forward, clear to starboard → OLED shows `> AVOID STBD`,
   green LED flashing
7. All sensors clear → green LED steady, `> CLEAR`
8. Optional: water-tank test — confirm no false returns from water surface

**Troubleshooting:**
- All distances read `---` → check 5V supply and GND wiring; confirm GPIO assignments
- Echo never goes HIGH → TRIG pulse may not be reaching sensor; confirm TRIG GPIO
- Multiple sensors fire simultaneously → ensure sequential `readSensor()` calls are not
  parallelised; acoustic cross-talk is eliminated by sequential firing
- False returns from water → tilt sensors 5–10° upward away from water surface

---

## Running a Test

Development environment: **VS Code + PlatformIO**.

1. Open the repo in VS Code (PlatformIO extension installed)
2. Each test sketch needs a minimal `platformio.ini` in its directory, or add it as an environment in the root `platformio.ini`:
   ```ini
   [env:gps_test]
   platform = espressif32
   board = esp32-s3-devkitc-1
   framework = arduino
   monitor_speed = 115200
   lib_deps =
       mikalhart/TinyGPSPlus
       adafruit/Adafruit SSD1306
       adafruit/Adafruit GFX Library
   build_src_filter = +<../testing/gps_test_display.ino>
   ```
3. Connect ESP32-S3 via USB-C
4. **Upload:** PlatformIO toolbar → Upload (→), or `pio run -e gps_test -t upload`
5. **Serial monitor:** PlatformIO toolbar → Serial Monitor, or `pio device monitor`
6. Verify against pass criteria above

Libraries are declared in `lib_deps` — PlatformIO downloads them automatically on first build.

## Adding a New Test Sketch

- Place `.ino` file in `testing/`
- `#include "../common/config.h"` for pin assignments
- Add a PlatformIO environment to `platformio.ini` with the sketch's `lib_deps`
- Add a row to the Module Overview table above
- Document pass criteria and expected serial output
