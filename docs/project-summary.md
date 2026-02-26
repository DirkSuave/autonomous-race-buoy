# Autonomous Race Buoy System — Summary (Download)

_Last updated: 2026-02-25 (America/Los_Angeles)_

## Project goal
Build an autonomous buoy system for sailboat races:
- Buoys navigate to specified GPS coordinates and **hold position** within a practical radius.
- Master buoy monitors wind stability to determine when a race is "Ready to Start."
- Start line buoys form a line **perpendicular to wind direction** based on the Master's fixed position.
- System supports manual overrides and failsafe "Return to Home" behaviors.

## High-Level Architecture & Roles
### Master Buoy (“Wind Master”)
- **Fixed Reference:** Position is set at power-up/config and does not change during the race.
- **Wind Monitoring:** Uses Davis Vantage Pro to track speed and direction.
- **Race Control:** Manages the state machine (Repositioning -> Stable -> Ready -> Locked).
- **Comms:** Broadcasts target coordinates to Slaves via LoRa; hosts WiFi AP for initial setup.
- **Indicators:** Green/Red LEDs and Remote Control feedback for "Ready" status.

### Slave Buoys (Pin, Windward, Leeward)
- **Navigation:** Bilge thruster ESCs (PWM via LEDC, 100 Hz) driving differential thrust to reach assigned coordinates.
- **Autonomy:** Holds position within an adjustable radius (3m default, 6m in heavy wind).
- **Failsafe:** Returns to initial "Home" GPS coordinate if LoRa signal is lost for **60 seconds** (`COMMS_TIMEOUT_MS`).
- **Collision Avoidance:** 3× AJ-SR04M waterproof ultrasonic sensors (150° forward arc) active during transit. Avoidance zone < 200 cm; emergency stop < 50 cm.
- **Calibration:** Performs compass calibration once the initial GPS target is reached.
- **Orientation:** All buoys face into the wind to maintain course alignment.

### Remote Control Unit
- **Hardware:** NodeMCU-32S (ESP32-WROOM-32) in IP67/IP68 waterproof enclosure — carried on racing dinghy, capsize-rated.
- **Radio:** Adafruit RFM95W LoRa 915 MHz (same module as buoys).
- **Interface:** 4 LEDs + buzzer + 2 waterproof buttons (no display).
- **LEDs:** Blue = repositioning, Green = race ready, Red = fault/drift alert, White = race locked / RTH.
- **Buttons:** BTN_START (single press = initiate race), BTN_STOP (single press = cancel, hold 3s = RTH fleet).
- **Drift alert:** Red LED + buzzer if any buoy > 10m from target during race.
- **Quantity:** 2 identical units built (same firmware, same BUOY_REMOTE ID).
- **Power:** 500–1000 mAh LiPo via TP4056 USB-C charger; ~5–16 hours runtime.

## Race Logic & State Machine
### 1. Pre-Race / Repositioning
- **Stability Gate:** Wind must not shift more than **15° within a rolling 60-second window**.
- **Repositioning Mode:** If wind is unstable, Master continuously updates Slave targets. Countdown is blocked.
- **Ready State:** Once wind is stable for 60s, Master freezes geometry and signals "Ready to Start" (Green LED).
- **Override:** Remote control can force a "Ready" state even if wind is shifting > 15°.

### 2. Race Countdown & Start Horn
- **Trigger:** Single press of BTN_START on RC — no second press required.
- **Sequence:** Master repositions buoys → all on-station → auto-countdown begins.
- **Countdown Duration:** 3 minutes default; 5 minutes optional (configurable via WiFi AP).
- **Horn:** Master buoy only (12V marine horn via MOSFET). RC buzzer is NOT used for horn sequence.
- **Blast Pattern (IRSA):** Long blast (2s) at 3:00, 2:00, 1:00; short blast (0.75s) at 0:30, 0:15, 0:05; long blast (2s) at 0:00 (gun).

### 3. Race Active
- **Locked Mode:** Once countdown starts, buoy positions are **locked**. Wind shifts are ignored until the race is finished.
- **End Race:** BTN_STOP on RC; Master returns to monitoring wind stability for the next start.

## Hardware Specifications
- **Controller:** ESP32-S3-DevKitC-1 (buoys); NodeMCU-32S (remote control).
- **Propulsion:** Bilge thruster ESCs, PWM via LEDC at 100 Hz (GPIO 47/48). Hardware under procurement.
- **Collision Avoidance:** 3× AJ-SR04M waterproof ultrasonic sensors per buoy, powered from 3.3V rail.
- **Power:** 4S LiPo per buoy, 3–4 hour runtime target (15kt wind capability).
- **Regulation:** 5V Buck Converter (1.5A) for electronics rail + 1000µF cap for inductive load protection.
- **Sensors:** BE-880 GPS/Compass at **115200 baud** (hardware-verified), Davis Anemometer (Master only).
- **Comms:** Adafruit RFM95W LoRa 915 MHz (SF7, 125 kHz BW, CR4/5).
- **UI:** 0.96" I2C OLED (SSD1306) on each buoy for GPS/LoRa/Mode diagnostics.
- **Horn:** 12V self-contained marine horn on Master buoy only, switched via IRLZ44N MOSFET (GPIO 7).
- **Remote Control:** NodeMCU-32S, IP67/IP68 enclosure, LoRa link, 4 LEDs + buzzer + 2 buttons. See `hardware/pinouts/remote-pinout.md`.

## Course Layout
The system creates a **diamond-shaped course**:
- **Start Line:** Master Buoy to Pin Buoy, perpendicular to wind direction
- **Windward Mark:** Upwind from start line, perpendicular to the line
- **Leeward Mark:** Downwind at opposite end, creating diamond geometry
- **Buoy Orientation:** All buoys face into the wind to maintain position and alignment

## Navigation & Control
- **Hold Radius:** Adjustable (Default: 3m; Heavy Wind/15kt: 6m).
- **PWM Control:** 100Hz refresh rate; 1.0ms (Rev) / 1.5ms (Stop) / 2.0ms (Fwd).
- **Mixing:** Differential thrust with deadband (±25µs) and slew-rate limiting (thrust ramping).

## Comms & Setup
- **LoRa Protocol:** Star topology at 915 MHz (SF7, 125 kHz BW, CR4/5). CRC16-CCITT checksums.
  - `ASSIGN` (master → slave): target GPS coordinates
  - `ACK_ASSIGN` (slave → master): assignment confirmed
  - `STATUS` (slave → master): position, battery, error flags
  - `PING_STATUS`: heartbeat
  - `PKT_RC_START / STOP / RTH` (remote → master): race commands
  - `PKT_MASTER_STATUS` (master → remote): aggregate fleet state + fault flags
- **Error flags:** GPS_LOST, COMPASS_FAIL, LOW_BATTERY, MOTOR_FAIL, COMMS_LOST, WIND_FAIL, OBSTACLE
- **Initial Setup:** Master acts as a WiFi Access Point. Users configure:
  - Master GPS Fixed Position.
  - Start Line Length & Course Distance.
  - Stability Thresholds (15° / 60s).
  - Hold Radii.
  - Countdown Duration (3 or 5 minutes).

## Planned Implementation Milestones
1. **Master State Machine:** Implement wind stability rolling buffer and race state transitions.
2. **Slave Logic:** Implement "Home" coordinate logic and LoRa-loss failsafe.
3. **Geometry Engine:** Math for perpendicular start line and upwind marks.
4. **Web Portal:** ESP32 WiFi AP portal for race parameter entry.
5. **Field Testing:** Validate 15kt hold capability and LoRa range (500m).
