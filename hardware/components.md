# Hardware Components

## Controllers

### Buoys (Master + 3 Slaves)
- **ESP32-S3-DevKitC-1**
  - Dual-core Xtensa LX7, 240 MHz
  - WiFi + Bluetooth, USB OTG
  - 45 GPIO; HSPI defaults GPIO 11/13/12/10; I2C defaults GPIO 8/9
  - Pin assignments: `common/config.h` (authoritative)

### Remote Control Units (×2)
- **NodeMCU-32S (ESP32-WROOM-32)**
  - Original ESP32 — different SPI/I2C defaults from ESP32-S3
  - VSPI defaults: MOSI=23, MISO=19, SCK=18, CS=5
  - Pin assignments: `hardware/pinouts/remote-pinout.md`

## Propulsion System
- **Type:** Bilge thruster ESCs with brushless thrusters — hardware under procurement
- **Drive:** Differential thrust (two independent thrusters, left/right)
- **PWM:** LEDC peripheral, 100 Hz, GPIO 47 (left) / GPIO 48 (right)
- **Signal:** 1.0 ms = full reverse, 1.5 ms = stop, 2.0 ms = full forward
- **Arm sequence:** 1.5 ms for 2 s on power-up (standard ESC arming)
- **Mixing:** Deadband ±25 µs; slew-rate limiting for smooth thrust ramping
- **Requirements:** Marine-grade (saltwater), sufficient thrust for 15 kt wind hold

## Power System

### Buoys (Master + 3 Slaves)
- **4S LiPo Battery** — 14.8 V nominal (16.8 V full, 13.2 V cutoff)
  - Recommended capacity: 5000–10000 mAh; target runtime 3–4 hours
  - Feeds ESCs directly + 5V buck converter for electronics
- **5V Buck Converter** (≥1.5 A)
  - Electronics rail: ESP32, GPS, LoRa, OLED
  - 1000 µF output capacitor for inductive load protection
- **Battery Monitor:** GPIO 4 (ADC1), 11:1 voltage divider (e.g. 100 kΩ + 10 kΩ)
  - 4S range: 13.2–16.8 V → 1.20–1.53 V at ADC pin

### Remote Control Units (×2)
- **LiPo Battery:** 500–1000 mAh (single cell)
  - 500 mAh: ~5–8 hours runtime; 1000 mAh: ~10–16 hours
- **TP4056 LiPo charger module** with USB-C input; rubber weatherproof port plug
- **Battery Monitor:** GPIO 35 (ADC1), 11:1 voltage divider

## Navigation & Sensors

### GPS/Compass — BE-880 Module (all buoys)
- UART GPS (NMEA) + I2C QMC5883L compass on shared bus with OLED
- **Baud rate: 115200** (module reconfigured from factory 9600 — hardware-verified; do not revert)
- GPIO: RX=18 (GPS TX), TX=17 (GPS RX); I2C SDA=8, SCL=9, compass address 0x0D
- Calibration: rotate 360° at first target; store min/max X/Y/Z offsets in flash

### Collision Avoidance — AJ-SR04M Waterproof Ultrasonic ×3 (all buoys)
- JSN-SR04T-compatible; Mode 1 (no R19 modification) — manual TRIG/ECHO
- 150° forward arc: Forward (0°), Port-forward (−45°), Starboard-forward (+45°)
- **Supply: 3.3V rail** (rated 3–5V; 3.3V eliminates need for ECHO level-shifting)
- GPIO: TRIG 15/16/19, ECHO 20/22/23
- Zones: CLEAR ≥200 cm, AVOID PORT/STBD <200 cm fwd, STOP <50 cm any sensor
- Active during transit only (STATE_DEPLOY, STATE_FAILSAFE/RTH)

### Wind Sensor — Davis Vantage Pro Anemometer (Master only)
- Speed: pulse input GPIO 6 (interrupt); 1 pulse/s ≈ 1 mph
- Direction: analog 0–3.3V on GPIO 5 (ADC1); 0 V = 0°, 3.3 V = 360°
- Used for 60 s rolling stability window (threshold: 15° shift)

## Communication
- **Adafruit RFM95W LoRa Module** (one per buoy + one per RC unit)
  - Frequency: 915 MHz; SF7, 125 kHz BW, CR4/5
  - Library: RadioHead RH_RF95
  - Star topology: Master ↔ Slaves + Master ↔ RC units
  - Target range: 500 m open water
  - Buoy SPI: GPIO 11/13/12/10 (CS/RST/IRQ: GPIO 10/14/21)
  - RC SPI: GPIO 23/19/18/5 (CS=5, RST=14, IRQ=4) — VSPI on NodeMCU-32S

## User Interface
- **0.96" I2C OLED Display (SSD1306)**
  - GPS status
  - LoRa connectivity
  - Mode diagnostics
  - Per buoy installation

## Start Horn (Master Buoy Only)
- **Self-contained 12V electronic marine horn** (e.g. Fiamm MR3, Marco UP/C, or similar) — built-in oscillator, no separate amplifier needed; >100 dB
  - ⚠️ The Whelen SA-350-MH is a **passive speaker** and requires a siren amplifier — do NOT use directly
- **N-channel MOSFET** (e.g. IRLZ44N or similar logic-level gate) — driven from GPIO 7; switches 12V supply to horn
- **1kΩ gate resistor** + **1N4007 flyback diode** across horn terminals
- Horn fires 3-minute (default) or 5-minute IRSA start sequence; blast durations: long=2s, short=0.75s

## Indicators

### Buoy Status LEDs (each buoy)
- Green LED (GPIO 38): Ready status
- Red LED (GPIO 39): Not ready / fault

### Remote Control LEDs (4 LEDs per unit)
- **Blue LED** (GPIO 26): Repositioning — buoys navigating to GPS targets
- **Green LED** (GPIO 25): Race Ready / all buoys on-station
- **Red LED** (GPIO 33): Fault or comms lost
- **White LED** (GPIO 13): Race In Progress (LOCKED) / RTH in progress

> LED set upgraded from 3 (amber/green/red) to 4 (blue/green/red/white).
> Blue replaces amber on GPIO 26 (same pin). White is new on GPIO 13.

### Remote Control Interface
- BTN_START (GPIO 32): Single press → initiate new race (reposition + auto-countdown)
- BTN_STOP (GPIO 34): Single press → cancel/abort; Hold 3s → RTH all buoys
- Buzzer (GPIO 27): Button confirms, drift alerts (>10m), fault alerts, RTH confirmation
- RC does NOT play start horn — horn is on the master buoy only

## Navigation Parameters
- **Hold Radius:** Adjustable
  - Default: 3m
  - Heavy Wind (15kt): 6m
- **Differential Thrust Mixing**
  - Deadband: ±25µs
  - Slew-rate limiting for smooth thrust ramping

## Remote Control Unit (qty: 2 identical units)
- **MCU:** NodeMCU-32S (ESP32-WROOM-32) — smaller form factor than buoy boards
- **Radio:** Adafruit RFM95W LoRa 915 MHz (same as buoys)
- **Interface:** 4× status LEDs (blue/green/red/white), 2× waterproof buttons, buzzer
- **Power:** Small LiPo (500–1000 mAh) via TP4056 charger
- **Enclosure:** IP67/IP68 waterproof — dinghy racing, capsize exposure
- **Accessories required:** TP4056 LiPo charger module, IP67 enclosure, sealed LED holders, waterproof push buttons, rubber USB-C port plug

## Bill of Materials Status

### Specified and Sourced
- [x] ESP32-S3-DevKitC-1 × 4 (buoys)
- [x] NodeMCU-32S × 2 (remote control)
- [x] Adafruit RFM95W LoRa 915 MHz × 6 (4 buoys + 2 RC)
- [x] BE-880 GPS/Compass module × 4 (buoys)
- [x] AJ-SR04M waterproof ultrasonic × 12 (3 per buoy × 4 buoys)
- [x] SSD1306 0.96" OLED × 4 (buoys)
- [x] Davis Vantage Pro anemometer × 1 (master only)
- [x] IRLZ44N N-channel MOSFET × 1 (horn driver, master)
- [x] 12V marine horn × 1 (master — e.g. Fiamm MR3 or Marco UP/C)
- [x] TP4056 LiPo charger module × 2 (RC units)
- [x] IP67/IP68 enclosure × 2 (RC units)

### Pending Procurement
- [ ] Bilge thruster ESCs × 8 (2 per buoy × 4 buoys) — type TBD
- [ ] Brushless bilge thrusters × 8 (2 per buoy × 4 buoys) — type TBD
- [ ] 4S LiPo batteries × 4 (buoys) — 5000–10000 mAh
- [ ] 5V buck converters × 4 (buoys) — ≥1.5 A
- [ ] Single-cell LiPo × 2 (RC units) — 500–1000 mAh
- [ ] Buoy hulls / enclosures × 4
- [ ] Detailed BOM with part numbers, supplier links, and per-buoy cost
