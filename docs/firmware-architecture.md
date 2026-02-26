# Firmware Architecture

## Overview
ESP32-S3 firmware for Master and Slave buoys, plus separate NodeMCU-32S firmware for
the Remote Control unit. Shared pin assignments and protocol definitions live in `common/`
at the project root (`common/config.h`, `common/protocol.h`, `common/protocol.cpp`).
These are included in all firmware and test sketches via `-I${PROJECT_DIR}` in `platformio.ini`.

## Directory Structure

```
common/                   # Shared headers — included by all firmware and test sketches
├── config.h              # Authoritative pin assignments for ESP32-S3 buoys
├── protocol.h            # Packet types, structs, error flags, buoy states
└── protocol.cpp          # CRC16-CCITT checksum implementation

firmware/                 # Main firmware (all directories currently empty — planned)
├── common/               # Shared runtime libraries
│   ├── gps/             # GPS parsing, Haversine distance/bearing, coordinate math
│   ├── lora/            # LoRa wrapper (RadioHead RH_RF95), retry logic
│   ├── display/         # SSD1306 OLED driver wrapper, screen layouts
│   ├── ultrasonic/      # AJ-SR04M collision avoidance controller
│   └── utils/           # Rolling buffer, state machine helpers, geometry
├── master/               # Master buoy firmware (ESP32-S3-DevKitC-1)
│   └── src/
├── slave/                # Slave buoy firmware (ESP32-S3-DevKitC-1)
│   └── src/
└── remote/               # Remote control firmware (NodeMCU-32S / ESP32-WROOM-32)
    └── src/

testing/                  # Hardware validation sketches (PlatformIO .cpp, working)
├── gps_test_display/    # Module 2: GPS + OLED
├── compass_test/        # Module 3: QMC5883L compass
├── lora_test_tx/        # Module 4: LoRa TX
├── lora_test_rx/        # Module 5: LoRa RX
├── wind_sensor_test/    # Module 6: Davis anemometer + compass fusion
├── ultrasonic_test/     # Module 9: AJ-SR04M collision avoidance
└── TEST-PLAN.md         # 9-module validation plan with pass criteria
```

## Common Libraries

### GPS Module (`common/gps/`)
- GPS coordinate parsing
- Distance and bearing calculations
- Haversine formula implementation
- Compass integration
- Hold radius calculations

### LoRa Protocol (`common/lora/`)
- Wrapper around RadioHead `RH_RF95` for RFM95W @ 915 MHz (SF7, 125 kHz BW, CR4/5)
- Packet encoding/decoding using structs defined in `common/protocol.h`
- All 7 packet types: ASSIGN, ACK_ASSIGN, STATUS, PING_STATUS, RC_START/STOP/RTH, MASTER_STATUS
- CRC16-CCITT checksum via `calculate_checksum()` / `verify_checksum()` in `protocol.cpp`
- Deterministic reply stagger: `REPLY_DELAY_BASE_MS + (buoy_id × REPLY_DELAY_PER_ID_MS)`
- Retry logic and per-slave timeouts

### Display Module (`common/display/`)
- OLED (SSD1306) driver wrapper
- Status screen layouts
- GPS coordinates display
- LoRa connectivity indicator
- Mode/state display

### Collision Avoidance (`common/ultrasonic/`)
- Sequential fire of 3× AJ-SR04M sensors (TRIG/ECHO, Mode 1 — R19 open, no hardware mod)
- `readSensor(trigPin, echoPin)` → distance cm via `pulseIn()`, 30 ms timeout
- Cycle period: 90 ms (3 sensors × ~30 ms each)
- Active only during STATE_DEPLOY and STATE_FAILSAFE/RTH; inactive during HOLD/LOCKED
- Transit speed capped at **1 m/s** during avoidance

**Zone 1 — Avoidance (fwd_cm < 200):**
- Reduce thrust to 50%
- `port_cm > stbd_cm` → steer port 45°; else steer starboard 45°
- Every 1s: if direct GPS bearing is clear → resume course

**Zone 2 — Emergency Stop (any sensor < 50 cm):**
- Full stop / brief reverse
- Wait 2s, re-scan; if clear → re-enter Zone 1
- Sets `ERROR_FLAG_OBSTACLE` in next STATUS packet

> ⚠️ **Resolve before coding this module:**
> 1. **Both sides blocked** — current logic always picks a side; risk of steering into a wall. Consider: reverse and hold until clear?
> 2. **Moving obstacle hysteresis** — crossing boat triggers stop then self-clears. Add a minimum clear-time (e.g. 3 consecutive clear scans) before resuming?
> 3. **Course re-acquisition** — after 45° avoidance turn, navigation must steer back to GPS target bearing (not just resume old heading). Nav controller must handle this explicitly.
> 4. **Master response to OBSTACLE flag** — master currently ignores it. Consider: suppress new ASSIGN while slave is in avoidance mode?

### Utilities (`common/utils/`)
- Rolling buffer for wind data (60s window, shift detection)
- State machine helpers
- Geometry calculations (perpendicular start line, upwind/leeward marks)

## Master Buoy Firmware

### State Machines

The master runs two parallel state machines:

**Race State Machine** (master-level, drives RC LED feedback and horn):
```
REPOSITIONING → READY → COUNTDOWN → LOCKED
                                  ↘ RTH (BTN_STOP hold 3s at any time)
```
- `REPOSITIONING`: Wind unstable or buoys navigating to targets; blue LED on RC
- `READY`: Wind stable ≥60s, all buoys on-station; green LED on RC
- `COUNTDOWN`: Auto-starts on BTN_START press; horn sequence running; 3 or 5 min
- `LOCKED`: Race active, buoy positions frozen; white LED on RC
- `RTH`: Fleet recalled to home coordinates; white slow-flash on RC
- `FAULT`: One or more buoys in error; red LED on RC

**Buoy Operational State Machine** (per-buoy, reported in STATUS packets):
```
STATE_INIT → STATE_DEPLOY → STATE_HOLD ↔ STATE_ADJUST
                                ↓
                          STATE_RECOVER → STATE_FAILSAFE (RTH)
```
- `STATE_INIT`: Power-up, GPS acquiring, LoRa initialising
- `STATE_DEPLOY`: Navigating to assigned target (collision avoidance active)
- `STATE_HOLD`: On-station within hold radius
- `STATE_ADJUST`: Drift correction (briefly outside hold radius)
- `STATE_RECOVER`: Returning to hold after error
- `STATE_FAILSAFE`: LoRa lost for 60s — navigating to home coordinate

### Core Modules
- **Wind Monitor:** Davis Vantage Pro interface (GPIO 5 analog direction, GPIO 6 pulse speed); stability rolling buffer
- **Race Controller:** Race state machine; processes BTN_START/STOP/RTH from RC packets
- **Geometry Engine:** Calculate perpendicular start line and windward/leeward mark positions
- **LoRa Coordinator:** Broadcast ASSIGN to slaves; receive STATUS; send MASTER_STATUS to RC
- **Horn Controller:** GPIO 7 → IRLZ44N MOSFET → 12V marine horn; IRSA blast pattern (long=2s, short=0.75s)
- **WiFi AP:** Configuration portal for race setup (GPS position, course dimensions, countdown duration)
- **Battery Monitor:** ADC1 GPIO 4, 11:1 voltage divider → 4S LiPo voltage
- **LED Controller:** Green (GPIO 38) / Red (GPIO 39) status indicators

### Wind Stability Logic
- Rolling 60-second buffer of wind direction samples
- Check for shifts > 15° within window
- Trigger repositioning if unstable
- Manual override capability

## Slave Buoy Firmware

### Core Modules
- **Navigation Controller:** Drive to target GPS coordinates; bearing + distance via Haversine
- **Position Holder:** Maintain position within hold radius; triggers STATE_ADJUST on drift
- **Thruster Controller:** Differential drive mixing; PWM via LEDC (GPIO 47/48, 100 Hz); slew-rate limiting
- **Collision Avoidance:** AJ-SR04M sensors active during STATE_DEPLOY and STATE_FAILSAFE/RTH
- **LoRa Client:** Receive ASSIGN, send ACK_ASSIGN and STATUS; deterministic reply stagger
- **Failsafe Manager:** Monitor LoRa timestamp; enter STATE_FAILSAFE after 60s silence; navigate to home
- **Battery Monitor:** ADC1 GPIO 4, 11:1 voltage divider; report in STATUS packet (0.1V units)
- **Compass Calibration:** One-time routine at first target arrival; collect min/max X/Y/Z; store offsets

### Navigation Logic
- Calculate bearing to target
- Compute distance from target
- Differential thrust for heading correction
- PWM control with slew-rate limiting
- Hold radius check (3m default, 6m heavy wind)

### Failsafe Behavior
- Monitor LoRa last-received timestamp each loop
- If no ASSIGN received for **60 seconds** (`COMMS_TIMEOUT_MS`), enter `STATE_FAILSAFE`
- Navigate to stored home GPS coordinate (set at power-up from first GPS fix)
- Collision avoidance remains active during RTH transit
- Resume normal operation and return to `STATE_DEPLOY` when signal restored

## Remote Control Firmware

Runs on **NodeMCU-32S (ESP32-WROOM-32)** — different MCU from the buoys; uses VSPI defaults.
Pin assignments are in `hardware/pinouts/remote-pinout.md` (not `common/config.h`).

### Core Modules
- **LoRa Client:** Send PKT_RC_START / PKT_RC_STOP / PKT_RC_RTH to master; receive PKT_MASTER_STATUS
- **Button Handler:** BTN_START (GPIO 32) single press; BTN_STOP (GPIO 34) single press + 3s hold for RTH
- **LED Controller:** Blue (GPIO 26), Green (GPIO 25), Red (GPIO 33), White (GPIO 13) driven from MASTER_STATUS fleet_state
- **Buzzer:** GPIO 27 — button confirm beeps, drift alerts, RTH confirmation
- **Battery Monitor:** ADC1 GPIO 35, 11:1 divider; low-battery warning via buzzer
- **Comms Watchdog:** If no MASTER_STATUS received for >60s — flash red + blue (comms lost pattern)

### LED State Table
| Fleet State | Red | Green | Blue | White |
|-------------|-----|-------|------|-------|
| REPOSITIONING | off | off | fast flash | off |
| READY | off | steady | off | off |
| COUNTDOWN | alt. flash | alt. flash | off | off |
| LOCKED | off | off | off | steady |
| RTH | off | off | off | slow flash |
| FAULT | steady | off | off | off |
| Comms lost | fast flash | off | fast flash | off |

### Button Logic
- **BTN_START single press:** Send `PKT_RC_START`; buzzer confirm beep
- **BTN_STOP single press:** Send `PKT_RC_STOP`; buzzer confirm beep
- **BTN_STOP hold 3s:** Buzzer sounds at 3s; send `PKT_RC_RTH` on release

## Development Priorities

### Phase 1: Core Infrastructure
1. ~~Set up PlatformIO environment~~ — **Done** (`platformio.ini` configured, all test envs working)
2. ~~GPS parsing~~ — **Done** (TinyGPS++, Module 2 test passing)
3. ~~LoRa TX/RX~~ — **Done** (RadioHead RH_RF95, Modules 4 & 5 passing)
4. ~~Wind sensor~~ — **Done** (Module 6 passing)
5. ~~Collision avoidance sensor~~ — **Done** (Module 9 test sketch complete)
6. Implement `protocol.cpp` CRC16-CCITT checksum functions (stubs exist)
7. Battery monitor sketch (Module 7 — TODO)

### Phase 2: Master Implementation
1. Wind monitoring interface
2. State machine implementation
3. Geometry calculations for race course
4. LoRa broadcast logic
5. WiFi configuration portal

### Phase 3: Slave Implementation
1. Navigation controller
2. Position hold logic
3. Thruster PWM control with mixing
4. LoRa receive and STATUS replies
5. Failsafe return-to-home

### Phase 4: Integration & Testing
1. Master-Slave communication testing (ASSIGN → ACK_ASSIGN → STATUS loop)
2. Master-RC communication testing (MASTER_STATUS → LED state, BTN → RC packet)
3. GPS accuracy validation
4. Hold radius performance (calm vs 15kt wind)
5. Collision avoidance validation (obstacle detection at 1 m/s transit speed)
6. LoRa range testing (500m target)
7. Battery runtime validation (3–4 hour target for buoys; 8+ hours for RC)

## Configuration Parameters

Pin assignments and protocol constants are centralised in `common/config.h` and
`common/protocol.h`. Runtime configuration (WiFi AP) is stored in ESP32 flash/NVS.

### Master Buoy (WiFi AP — configurable at setup)
- Fixed GPS anchor position (lat/lon)
- Start line length (metres)
- Course distance (windward/leeward marks)
- Wind stability threshold (default 15°, `WIND_CHANGE_THRESHOLD_DEG`)
- Wind stability window (default 60s, `WIND_CHANGE_DURATION_S`)
- Hold radius default (3m) and heavy-wind override (6m)
- Countdown duration (3 or 5 minutes)

### Slave Buoy (assigned by master at runtime)
- Buoy ID (`BUOY_START_A`, `BUOY_START_B`, `BUOY_WINDWARD`, `BUOY_LEEWARD`)
- Target GPS coordinate (from ASSIGN packet)
- Hold radius (from ASSIGN packet, `hold_radius` field)
- Home GPS coordinate (recorded from first GPS fix at power-up)
- LoRa failsafe timeout (60s, `COMMS_TIMEOUT_MS`)

### Remote Control (compile-time only — no runtime config)
- Buoy ID: `BUOY_REMOTE` (= 5)
- BTN_STOP hold duration for RTH: 3 seconds

## Communication Protocol

Full definitions in `common/protocol.h`. All packets use CRC16-CCITT checksums.

| Packet | Hex | Direction | Purpose |
|--------|-----|-----------|---------|
| `PKT_ASSIGN` | 0xA5 | Master → Slave | Target GPS coordinates + hold radius |
| `PKT_ACK_ASSIGN` | 0xAA | Slave → Master | Assignment acknowledged |
| `PKT_STATUS` | 0x5A | Slave → Master | Position, battery, error flags |
| `PKT_PING_STATUS` | 0x55 | Any | Heartbeat with `millis()` timestamp |
| `PKT_RC_START` | 0xB1 | RC → Master | Initiate race |
| `PKT_RC_STOP` | 0xB2 | RC → Master | Cancel / abort |
| `PKT_RC_RTH` | 0xB3 | RC → Master | Recall fleet to home |
| `PKT_MASTER_STATUS` | 0xC1 | Master → RC | Fleet state + ORed fault flags |

### Error Flags (bitmask in STATUS / MASTER_STATUS)
| Flag | Value | Meaning |
|------|-------|---------|
| `ERROR_FLAG_GPS_LOST` | 0x01 | No GPS fix |
| `ERROR_FLAG_COMPASS_FAIL` | 0x02 | I2C compass not responding |
| `ERROR_FLAG_LOW_BATTERY` | 0x04 | Battery below threshold |
| `ERROR_FLAG_MOTOR_FAIL` | 0x08 | ESC / thruster fault |
| `ERROR_FLAG_COMMS_LOST` | 0x10 | LoRa timeout |
| `ERROR_FLAG_WIND_FAIL` | 0x20 | Anemometer fault (master only) |
| `ERROR_FLAG_OBSTACLE` | 0x40 | Collision avoidance emergency stop |

## Notes
- All ESP32 code should use FreeRTOS tasks for concurrent operations
- Watchdog timers for critical loops
- Non-blocking I/O where possible
- Proper error handling and logging to display
