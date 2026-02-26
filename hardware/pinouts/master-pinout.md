# Master Buoy Pinout

## ESP32-S3-DevKitC-1 Pin Assignments

### Communication Interfaces

#### SPI (LoRa RFM95W) - HSPI Bus
| ESP32 Pin | Function | RFM95W Pin | Notes |
|-----------|----------|------------|-------|
| GPIO 11   | MOSI     | MOSI       | HSPI default |
| GPIO 13   | MISO     | MISO       | HSPI default |
| GPIO 12   | SCK      | SCK        | HSPI default |
| GPIO 10   | CS       | CS         | HSPI default |
| GPIO 14   | Output   | RST        | Custom, safe GPIO |
| GPIO 21   | Input    | G0 (IRQ)   | RTC GPIO, interrupt capable |

⚠️ **Note:** Old pinout used GPIO 23/19/18/5 which are NOT the ESP32-S3 defaults. Updated to HSPI defaults.

#### I2C Bus (OLED Display + Compass)
| ESP32 Pin | Function | Connected To | Notes |
|-----------|----------|--------------|-------|
| GPIO 8    | SDA      | OLED + BE-880 QMC5883 Compass | Default Arduino I2C SDA |
| GPIO 9    | SCL      | OLED + BE-880 QMC5883 Compass | Default Arduino I2C SCL |

⚠️ **Note:** Old pinout used GPIO 21/22 which are ESP32 (non-S3) defaults. ESP32-S3 defaults are GPIO 8/9.

Pull-up resistors: 2.2kΩ to 3.3V (usually on-board modules)

**I2C Devices on Bus:**
- OLED Display (SSD1306): Address 0x3C
- BE-880 Compass (QMC5883): Address 0x0D

#### UART1 (GPS BE-880)
| ESP32 Pin | Function | BE-880 Pin | Notes |
|-----------|----------|------------|-------|
| GPIO 17   | TX       | RX         | UART1 default, can remap |
| GPIO 18   | RX       | TX         | UART1 default, can remap |

⚠️ **Note:** Old pinout had RX/TX swapped. Corrected: ESP32 RX connects to GPS TX.

**BE-880 Configuration:**
- Default Baud Rate: 115200 bps (configurable 4800-921600 bps)
- Protocol: NMEA (RMC, VTG, GGA, GSA, GSV, GLL) / UBX
- Update Rate: 1Hz default (configurable 0.25-18Hz)
- Connector: 6-pin 1.25mm

**BE-880 Dual Interface:**
- **UART:** GPS position data (NMEA sentences)
- **I2C:** Compass heading from QMC5883 chip (shared bus with OLED)

### PWM Outputs (Thrusters)
| ESP32 Pin | Function | ESC Connection | Notes |
|-----------|----------|----------------|-------|
| GPIO 47   | PWM      | Left ESC Signal | Safe GPIO, not strapping pin |
| GPIO 48   | PWM      | Right ESC Signal | RGB LED onboard, can still use |

⚠️ **Note:** Old pinout used GPIO 25/26 which don't exist on ESP32-S3 (reserved for flash). Updated to safe GPIOs.

PWM Config: 100 Hz, 16-bit resolution (LEDC peripheral)

### Analog Inputs

#### Davis Anemometer (Master Only)
| ESP32 Pin | Function | Sensor | Notes |
|-----------|----------|--------|-------|
| GPIO 6    | Pulse In | Wind Speed (interrupt) | ADC1_CH5, T6, RTC GPIO |
| GPIO 5    | ADC      | Wind Direction (analog) | ADC1_CH4, T5 |

⚠️ **Note:** Old pinout used GPIO 35/36 which may conflict with PSRAM on N8R8 variant. Updated to ADC1 channels (GPIO 1-10) which are WiFi-safe.

#### Battery Monitor
| ESP32 Pin | Function | Connection | Notes |
|-----------|----------|------------|-------|
| GPIO 4    | ADC      | Battery voltage (via 5:1 divider) | ADC1_CH3, T4, WiFi-safe |

⚠️ **Note:** Old pinout used GPIO 34 which doesn't exist on ESP32-S3. Updated to GPIO 4 (ADC1_CH3).

### Digital I/O

#### Status LEDs
| ESP32 Pin | Function | LED Color | Notes |
|-----------|----------|-----------|-------|
| GPIO 38   | Output   | Green (Ready) | Safe GPIO |
| GPIO 39   | Output   | Red (Not Ready) | Safe GPIO |

#### Start Horn
| ESP32 Pin | Function | Notes |
|-----------|----------|-------|
| GPIO 7    | Horn trigger (MOSFET gate) | ADC1_CH6; drives N-channel MOSFET (e.g. IRLZ44N) → 12V marine horn |

**Horn type:** Self-contained 12V electronic marine horn (built-in oscillator — e.g. Fiamm MR3, Marco UP/C). Do NOT use passive speakers (e.g. Whelen SA-350-MH) — they require a separate amplifier.
**Horn driver circuit:** GPIO 7 → 1kΩ gate resistor → MOSFET gate; MOSFET drain → horn (+); horn (–) → 12V battery negative. Flyback diode (e.g. 1N4007) across horn terminals (cathode to +12V). MOSFET source to GND shared with ESP32.

⚠️ **Note:** Old pinout used GPIO 32/33 which are reserved for flash on ESP32-S3. Updated to safe GPIOs 38/39.

#### Ultrasonic Collision Avoidance (JSN-SR04T × 3)
Active during transit states (STATE_DEPLOY, STATE_FAILSAFE/RTH). Idle during HOLD/LOCKED.

| ESP32 Pin | Function   | Sensor               | Notes |
|-----------|------------|----------------------|-------|
| GPIO 15   | TRIG_FWD   | Forward (0°) trigger | Pure digital — ADC2 restriction irrelevant |
| GPIO 16   | TRIG_PORT  | Port-45° trigger     | Pure digital |
| GPIO 19   | TRIG_STBD  | Starboard-45° trigger| Pure digital |
| GPIO 20   | ECHO_FWD   | Forward echo input   | Pure digital |
| GPIO 22   | ECHO_PORT  | Port-45° echo input  | Pure digital |
| GPIO 23   | ECHO_STBD  | Starboard-45° echo   | Pure digital |

Supply: 5V from buck converter. Spacing: ≥15 cm between transducer heads.
Mount: 10–15 cm above waterline, tilted 5–10° upward.

#### Remote Control Interface
**Note:** Remote control is a separate handheld device that communicates wirelessly via LoRa (not physical buttons on Master buoy).

All race control commands (start, override, end) are sent from the separate remote control unit via LoRa radio.

## Power Distribution

### 4S LiPo Battery
- **Voltage:** 14.8V nominal (16.8V full, 13.2V cutoff)
- **Capacity:** 5000-10000 mAh recommended
- **Output:** Feeds ESCs and 5V buck converter

### 5V Buck Converter
- **Input:** 4S battery (14.8V nominal)
- **Output:** 5V @ 1.5A minimum
- **Capacitor:** 1000µF on output for load stabilization
- **Feeds:** ESP32 5V pin, GPS module, anemometer

⚠️ **Important:** Feed ESP32 via 5V pin, NOT USB during operation

### 3.3V Rail
- **Source:** ESP32 on-board 3.3V regulator
- **Feeds:** LoRa, OLED, compass (shared with GPS)
- **Max Current:** ~500mA from onboard regulator

## Connection Summary

```
4S Battery (14.8V)
├─> ESC Left (Direct Battery Power)
├─> ESC Right (Direct Battery Power)
└─> 5V Buck Converter
    ├─> ESP32 (5V pin)
    ├─> GPS Module (5V)
    └─> Anemometer (5V)

ESP32 3.3V Pin
├─> LoRa Module (VCC + logic)
├─> OLED Display (VCC + logic)
└─> Compass (uses GPS module 3.3V typically)

ESC Signals (3.3V PWM from ESP32)
├─> Left ESC Signal (GPIO 47)
└─> Right ESC Signal (GPIO 48)
```

## Pin Usage Summary

### ⚠️ CRITICAL: ESP32-S3 Reserved Pins (NEVER USE)
- **GPIO 26-32:** Flash/PSRAM (not exposed on DevKitC-1)
- **GPIO 0, 3, 45, 46:** Strapping pins (avoid in design)
- **GPIO 43, 44:** USB/UART0 (programming only)

### Used Pins (Master Buoy)
| GPIO | Function | Type |
|------|----------|------|
| 4    | Battery Monitor | ADC Input |
| 5    | Wind Direction | ADC Input |
| 6    | Wind Speed | Digital Input (interrupt) |
| 7    | Horn trigger (MOSFET gate) | Digital Output |
| 8    | I2C SDA | Communication |
| 9    | I2C SCL | Communication |
| 10   | SPI CS | Communication |
| 11   | SPI MOSI | Communication |
| 12   | SPI SCK | Communication |
| 13   | SPI MISO | Communication |
| 14   | LoRa RST | Digital Output |
| 15   | Ultrasonic TRIG_FWD | Digital Output |
| 16   | Ultrasonic TRIG_PORT | Digital Output |
| 17   | UART TX | Communication |
| 18   | UART RX | Communication |
| 19   | Ultrasonic TRIG_STBD | Digital Output |
| 20   | Ultrasonic ECHO_FWD | Digital Input |
| 21   | LoRa IRQ | Digital Input (interrupt) |
| 22   | Ultrasonic ECHO_PORT | Digital Input |
| 23   | Ultrasonic ECHO_STBD | Digital Input |
| 38   | Green LED | Digital Output |
| 39   | Red LED | Digital Output |
| 47   | Left Thruster | PWM Output |
| 48   | Right Thruster | PWM Output |

### Available for Expansion
GPIO 1-3, 24-25, 33-37, 40-42 (verify PSRAM conflicts on GPIO 35-37 if using N8R8 variant)

## Grounding

### Critical Grounding Points
1. **Power Ground:** Battery negative to buck converter GND
2. **Signal Ground:** All 3.3V/5V devices share common GND with ESP32
3. **ESC Ground:** ESC signal ground to ESP32 GND (NOT battery ground directly)
4. **Antenna Ground:** LoRa antenna ground plane connected

### Ground Loop Prevention
- Use star grounding topology from battery
- Keep high-current (ESC) and low-current (signal) grounds separate until common point
- Twisted pair wiring for ESC power lines

## Reference Documentation
- [ESP32-S3 Specifications](../specs/esp32-s3.md) - Complete GPIO details and restrictions
- [RFM95W LoRa](../specs/rfm95w-lora.md) - LoRa module pinout
- [BE-880 GPS](../specs/be-880-gps.md) - GPS/Compass connections
- [Davis Anemometer](../specs/davis-anemometer.md) - Wind sensor interface

## Migration Notes from Old Pinout

### Major Changes (ESP32 → ESP32-S3)
1. **I2C:** GPIO 21/22 → GPIO 8/9 (S3 defaults)
2. **SPI:** GPIO 23/19/18/5 → GPIO 11/13/12/10 (HSPI defaults)
3. **PWM:** GPIO 25/26 → GPIO 47/48 (25/26 reserved on S3)
4. **ADC:** GPIO 34/35/36 → GPIO 4/5/6 (old pins reserved or PSRAM conflict)
5. **LED:** GPIO 32/33 → GPIO 38/39 (32/33 reserved on S3)
6. **LoRa IRQ:** GPIO 27 → GPIO 21 (27 reserved on S3)

### Why These Changes?
- ESP32-S3 has different default pins for I2C and SPI
- GPIOs 26-32 are reserved for internal flash/PSRAM on S3
- GPIO 35-37 may conflict with PSRAM on N8R8 variant
- ADC2 (GPIO 11-20) conflicts with WiFi - moved sensors to ADC1 (GPIO 1-10)
