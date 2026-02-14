# Autonomous Race Buoy System — Summary (Download)

_Last summarized: 2026-02-13 (America/Los_Angeles)_

## Project goal
Build an autonomous buoy system for sailboat races:

- Buoys navigate to specified GPS coordinates and **hold position** within a practical radius.
- After race start (and potentially during/after the race), buoys can **readjust** if wind direction changes.
- Start line buoys form a line **perpendicular to wind direction**.
- Windward/leeward buoys position based on **course distance** and the start-line midpoint.

Accuracy needs are relaxed: **~1–5 m GPS error is acceptable** (friendly racing). RTK not required.

## High-level architecture
### Roles
- **Base Station (phone/tablet app):** initial configuration and course setup (start buoy location, start line length, course distance, etc.).
- **Master Buoy (“Wind Master”):**
  - Reads GNSS position.
  - Reads wind direction/speed.
  - Computes target coordinates for all buoys.
  - Broadcasts assignments via LoRa and collects acknowledgements/status.
- **Slave Buoys (Start A, Start B, Windward, Leeward):**
  - Receive assignments over LoRa.
  - Navigate to assigned target coordinate.
  - Hold within a hold radius.
  - Report status and acknowledgements to Master.

### Comms
- **LoRa (915 MHz, USA) as primary inter-buoy comms**
  - Range requirement: up to ~500 m.
  - Network topology: **star** (Master coordinates, Slaves respond).
- **Wi‑Fi only for initial setup** (provisioning/parameters).

### Core software concept
Simple state machine with modes:
- `DEPLOY` (go to assigned coordinate)
- `HOLD` (station keep within radius)
- `ADJUST` (receive new assignment, reposition)
- `RECOVER` (return to known point / safe mode)
- `FAILSAFE` (lost comms / sensor issues)

## Hardware available (updated)
- **Compute**
  - **ESP32‑S3‑DevKitC‑1** (primary buoy controller)
  - **ESP32‑32S NodeMCU** (available for prototyping/spare)
- **GNSS + heading**
  - **BE-880 GPS Compass** (GNSS position + heading output)
- **Wind sensing (Master buoy)**
  - **Davis Vantage Pro anemometer + wind vane** (wind speed + wind direction)
- **Radio / comms**
  - **Adafruit RFM95W LoRa** (915 MHz)
- **Propulsion (concept)**
  - Two-thruster differential drive (motor driver/ESC details TBD)

## Navigation & control notes
- Start with a **3–5 m hold radius** given GPS error tolerance.
- Compass is useful when stationary/slow; GPS course-over-ground stabilizes only when moving.
- Wind-change logic should include hysteresis (example: reposition if **Δwind ≥ 10–15°** sustained for **~30 s**) to avoid constant churn.

## LoRa star protocol (draft)
### Packet types
- `ASSIGN` (Master → Slave): `buoy_id`, `target_lat`, `target_lon`, `hold_radius`, `timestamp`, `seq`
- `ACK_ASSIGN` (Slave → Master): `buoy_id`, `seq`, accepted/rejected, current lat/lon
- `STATUS` (Slave → Master): `buoy_id`, mode, gps_fix, current lat/lon, dist_to_target, battery, errors
- Optional `PING_STATUS` (Master → Slave): request immediate status

### Collision avoidance
- Stagger slave replies based on `buoy_id` (deterministic delay) and/or random backoff.

### “READY” definition (example)
- Slave is READY when: GPS fix valid and within hold radius for N seconds.
- Master is READY when: all slaves report HOLDING/READY.

## Planned implementation milestones (recommended)
1. Lock system contracts (roles, telemetry fields, packet formats, timing).
2. LoRa link bring-up (bench): reliable send/receive, RSSI/SNR visibility.
3. BE-880 bring-up (bench): confirm interface/baud; parse fix + heading.
4. Davis wind bring-up (bench): speed pulse counting + direction calibration.
5. Master integration: compute marks, broadcast assignments, track slave readiness.
6. Slave logic: state machine + distance/bearing + status reporting.
7. Propulsion integration: differential thrust control + hold tuning.
8. Field tests: dry-run → controlled water → real wind; validate comms-loss autonomy.

## Open decisions / TODOs
- Thruster selection: brushed + H-bridge vs brushless + ESC (impacts EMI, power, control).
- Battery/power: runtime targets, voltage rails, waterproofing.
- Mechanical: hull, thruster mounting, mast layout for GNSS/compass.
- Base-station UX: how course params are input and sent to Master.
