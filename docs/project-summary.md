# Autonomous Race Buoy System — Summary (Download)

_Last updated: 2026-02-15 (America/Los_Angeles)_

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
- **Navigation:** Propulsion system (TBD) to reach assigned coordinates.
- **Autonomy:** Holds position within an adjustable radius (3m default, 6m in heavy wind).
- **Failsafe:** Returns to initial "Home" GPS coordinate if LoRa signal is lost for a set duration.
- **Calibration:** Performs compass calibration once the initial GPS target is reached.
- **Orientation:** All buoys face into the wind to maintain course alignment.

### Remote Control Unit
- **Purpose:** Handheld device for race control and monitoring
- **Functions:**
  - Send start race commands to Master Buoy
  - Receive status updates from all buoys
  - Monitor race readiness status
  - Emergency override controls
- **Communication:** LoRa link with Master Buoy
- **Features:** TBD - requires further specification of display, controls, and feedback mechanisms

## Race Logic & State Machine
### 1. Pre-Race / Repositioning
- **Stability Gate:** Wind must not shift more than **15° within a rolling 60-second window**.
- **Repositioning Mode:** If wind is unstable, Master continuously updates Slave targets. Countdown is blocked.
- **Ready State:** Once wind is stable for 60s, Master freezes geometry and signals "Ready to Start" (Green LED).
- **Override:** Remote control can force a "Ready" state even if wind is shifting > 15°.

### 2. Race Active
- **Start Signal:** Triggered by Remote Control.
- **Locked Mode:** Once the race/countdown starts, buoy positions are **locked**. They ignore further wind shifts until the race is finished.
- **End Race:** Triggered by Remote; Master returns to monitoring wind stability for the next start.

## Hardware Specifications
- **Controller:** ESP32-S3-DevKitC-1.
- **Propulsion:** TBD - Brushless thrusters and ESCs under evaluation (previously considered: APISQUEEN Feather-45A ESCs with brushless thrusters).
- **Power:** 4S Battery per buoy, 3–4 hour runtime target (15kt wind capability).
- **Regulation:** 5V Buck Converter (1.5A) for electronics rail + 1000µF cap for inductive load protection.
- **Sensors:** BE-880 GPS/Compass, Davis Anemometer (Master only).
- **Comms:** Adafruit RFM95W LoRa (915 MHz).
- **UI:** 0.96" I2C OLED (SSD1306) on each buoy for GPS/LoRa/Mode diagnostics.
- **Remote Control:** Handheld unit with LoRa communication (specifications TBD).

## Course Layout
The system creates a **diamond-shaped course**:
- **Start Line:** Master Buoy to Pin Buoy, perpendicular to wind direction
- **Windward Mark:** Upwind from start line, perpendicular to the line
- **Leeward Mark:** Downwind at opposite end, creating diamond geometry
- **Buoy Orientation:** All buoys face into the wind to maintain position and alignment
- **Power:** 4S Battery per buoy, 3–4 hour runtime target (15kt wind capability).
- **Regulation:** 5V Buck Converter (1.5A) for electronics rail + 1000µF cap for inductive load protection.
- **Sensors:** BE-880 GPS/Compass, Davis Anemometer (Master only).
- **Comms:** Adafruit RFM95W LoRa (915 MHz).
- **UI:** 0.96" I2C OLED (SSD1306) on each buoy for GPS/LoRa/Mode diagnostics.

## Navigation & Control
- **Hold Radius:** Adjustable (Default: 3m; Heavy Wind/15kt: 6m).
- **PWM Control:** 100Hz refresh rate; 1.0ms (Rev) / 1.5ms (Stop) / 2.0ms (Fwd).
- **Mixing:** Differential thrust with deadband (±25µs) and slew-rate limiting (thrust ramping).

## Comms & Setup
- **LoRa Protocol:** Star topology. Master sends `ASSIGN`, Slaves reply with `STATUS`.
- **Initial Setup:** Master acts as a WiFi Access Point. Users configure:
  - Master GPS Fixed Position.
  - Start Line Length & Course Distance.
  - Stability Thresholds (15° / 60s).
  - Hold Radii.

## Planned Implementation Milestones
1. **Master State Machine:** Implement wind stability rolling buffer and race state transitions.
2. **Slave Logic:** Implement "Home" coordinate logic and LoRa-loss failsafe.
3. **Geometry Engine:** Math for perpendicular start line and upwind marks.
4. **Web Portal:** ESP32 WiFi AP portal for race parameter entry.
5. **Field Testing:** Validate 15kt hold capability and LoRa range (500m).
