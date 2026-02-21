# Firmware Architecture

## Overview
ESP32-S3 firmware for Master and Slave buoys with shared common libraries.

## Directory Structure

```
firmware/
├── common/           # Shared libraries
│   ├── gps/         # GPS and navigation utilities
│   ├── lora/        # LoRa communication protocol
│   ├── display/     # OLED display interface
│   └── utils/       # General utilities
├── master/          # Master buoy firmware
│   └── src/
└── slave/           # Slave buoy firmware
    └── src/
```

## Common Libraries

### GPS Module (`common/gps/`)
- GPS coordinate parsing
- Distance and bearing calculations
- Haversine formula implementation
- Compass integration
- Hold radius calculations

### LoRa Protocol (`common/lora/`)
- Message structure definitions
- ASSIGN message (Master -> Slave)
- STATUS message (Slave -> Master)
- Packet encoding/decoding
- Retry logic and timeouts

### Display Module (`common/display/`)
- OLED (SSD1306) driver wrapper
- Status screen layouts
- GPS coordinates display
- LoRa connectivity indicator
- Mode/state display

### Utilities (`common/utils/`)
- Rolling buffer for wind data
- State machine helpers
- Circular buffer implementation
- Geometry calculations

## Master Buoy Firmware

### State Machine
```
States:
- REPOSITIONING: Wind unstable, updating slave positions
- STABLE: Wind stable for 60s, ready to start
- READY: Green light, awaiting start signal
- LOCKED: Race active, positions frozen
```

### Core Modules
- **Wind Monitor:** Davis anemometer interface, stability checking
- **Race Controller:** State machine management
- **Geometry Engine:** Calculate perpendicular start line and mark positions
- **LoRa Coordinator:** Broadcast ASSIGN messages to slaves
- **WiFi AP:** Configuration portal for race setup
- **LED Controller:** Green/Red status indicators

### Wind Stability Logic
- Rolling 60-second buffer of wind direction samples
- Check for shifts > 15° within window
- Trigger repositioning if unstable
- Manual override capability

## Slave Buoy Firmware

### Core Modules
- **Navigation Controller:** Drive to target GPS coordinates
- **Position Holder:** Maintain position within hold radius
- **Thruster Controller:** Differential drive mixing
- **LoRa Client:** Receive ASSIGN, send STATUS
- **Failsafe Manager:** Return-to-home on LoRa loss
- **Compass Calibration:** One-time calibration routine

### Navigation Logic
- Calculate bearing to target
- Compute distance from target
- Differential thrust for heading correction
- PWM control with slew-rate limiting
- Hold radius check (3m default, 6m heavy wind)

### Failsafe Behavior
- Monitor LoRa last-received timestamp
- If no signal for X seconds, enter RTH mode
- Navigate to stored "Home" GPS coordinate
- Resume normal operation when signal restored

## Development Priorities

### Phase 1: Core Infrastructure
1. Set up ESP32 development environment (PlatformIO or Arduino IDE)
2. Implement GPS parsing and coordinate math
3. Create LoRa message protocol
4. Basic OLED display output

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
1. Master-Slave communication testing
2. GPS accuracy validation
3. Hold radius performance (calm vs 15kt wind)
4. LoRa range testing (500m target)
5. Battery runtime validation (3-4 hour target)

## Configuration Parameters

### Master Buoy
- Fixed GPS position (lat/lon)
- Start line length (meters)
- Course distance (Windward/Leeward marks)
- Stability threshold (degrees)
- Stability window (seconds)

### Slave Buoy
- Buoy ID (Pin, Windward, Leeward)
- Home GPS coordinate
- Hold radius (default/heavy wind)
- LoRa timeout for failsafe
- Propulsion calibration values (TBD - motor/ESC dependent)

## Communication Protocol

### ASSIGN Message (Master -> Slave)
```
[ASSIGN][BuoyID][Lat][Lon][HoldRadius][Checksum]
```

### STATUS Message (Slave -> Master)
```
[STATUS][BuoyID][CurrentLat][CurrentLon][DistanceToTarget][BatteryVoltage][Checksum]
```

## Notes
- All ESP32 code should use FreeRTOS tasks for concurrent operations
- Watchdog timers for critical loops
- Non-blocking I/O where possible
- Proper error handling and logging to display
