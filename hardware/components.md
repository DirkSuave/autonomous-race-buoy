# Hardware Components

## Controller
- **ESP32-S3-DevKitC-1**
  - Dual-core processor
  - WiFi + Bluetooth
  - Multiple GPIO, I2C, SPI interfaces

## Propulsion System (TBD)
- **Status:** Under evaluation - motors and ESCs not yet finalized
- **Previously Considered:** APISQUEEN Feather-45A ESC with brushless thrusters
- **Requirements:**
  - Differential drive capability for navigation
  - PWM Control: 100Hz refresh rate target
  - Signal: 1.0ms (Rev) / 1.5ms (Stop) / 2.0ms (Fwd)
  - Marine-grade for saltwater operation
  - Sufficient thrust for 15kt wind capability

## Power System
- **4S LiPo Battery** (per buoy)
  - Target runtime: 3-4 hours
  - 15kt wind capability
- **5V Buck Converter** (1.5A)
  - For electronics rail
  - 1000µF capacitor for inductive load protection

## Navigation & Sensors
- **BE-880 GPS/Compass Module**
  - GPS positioning
  - Integrated compass for heading
  - Calibration required on first target reach

### Master Buoy Only
- **Davis Vantage Pro Anemometer**
  - Wind speed monitoring
  - Wind direction tracking
  - Used for stability calculations

## Communication
- **Adafruit RFM95W LoRa Module**
  - Frequency: 915 MHz
  - Star topology (Master -> Slaves)
  - Target range: 500m

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
- [ ] Create detailed BOM with part numbers
- [ ] Add supplier links
- [ ] Calculate per-buoy cost
- [ ] Determine quantity for 4 buoys (1 Master + 3 Slaves: Pin, Windward, Leeward)
- [ ] Finalize propulsion system selection
- [ ] Specify remote control unit hardware and Interface
