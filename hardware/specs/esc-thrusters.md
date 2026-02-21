# Propulsion System Specifications (TBD)

**STATUS:** Hardware under evaluation - motors and ESCs not yet finalized

## Requirements

### Thruster Requirements
- **Type:** Brushless underwater thrusters (marine-grade)
- **Quantity:** 2 per buoy (differential drive)
- **Mounting:** Side-by-side, parallel thrust
- **Direction:** Reversible (forward/reverse)
- **Target Thrust:** Sufficient for 15kt (7.7 m/s) wind hold capability
- **Operating Voltage:** 14.8V nominal (4S LiPo compatible)
- **Expected Current:** 10-20A per thruster @ full load (estimate)
- **Propeller:** Sealed, marine-grade, suitable for saltwater

### ESC (Electronic Speed Controller) Requirements
- **Continuous Current Rating:** 40-50A minimum
- **Burst Current:** 60A+ for safety margin
- **Input Voltage:** Must support 4S LiPo (14.8V nominal, 13.2V-16.8V range)
- **BEC:** Not required (external 5V buck converter used)
- **Bidirectional:** Must support forward/reverse operation
- **Control Interface:** PWM (servo-style)
- **Protection Features:** Over-current, thermal, low voltage cutoff

### Control Interface Requirements
- **Signal:** PWM (servo-style)
- **Frequency:** 50-400 Hz (100 Hz target)
- **Pulse Width Range:** 1000-2000 µs standard
  - **1000 µs (1.0 ms):** Full reverse
  - **1500 µs (1.5 ms):** Stop/neutral
  - **2000 µs (2.0 ms):** Full forward
- **Deadband:** ±25 µs around neutral (desirable)

## Previously Considered Hardware

### APISQUEEN Feather-45A ESC
- **Continuous Current:** 45A
- **Burst Current:** 60A (10 seconds)
- **Input Voltage:** 2-6S LiPo (7.4V - 25.2V)
- **BEC:** None (compatible with external regulator design)
- **Features:** Bidirectional, active braking, protections
- **Status:** Under evaluation

### Generic Brushless Marine Thrusters
- Various manufacturers produce suitable marine thrusters
- BlueRobotics and similar vendors offer options
- **Status:** Evaluating options for thrust capacity and reliability

## Integration Notes

### ESP32 PWM Configuration (Example)
```cpp
const int PWM_FREQ = 100;
const int PWM_RESOLUTION = 16;
const int PWM_CHANNEL_L = 0;
const int PWM_CHANNEL_R = 1;

const int PWM_MIN = 6553;      // 1.0ms at 16-bit, 100Hz
const int PWM_NEUTRAL = 9830;  // 1.5ms neutral
const int PWM_MAX = 13107;     // 2.0ms at 16-bit, 100Hz
```

### GPIO Pin Assignments (from pinout documents)
- **Left Thruster:** GPIO 47 (Slave), varies for Master
- **Right Thruster:** GPIO 48 (Slave), varies for Master
- Refer to pinout documentation for specific assignments

## Testing Requirements

Before finalizing hardware selection:
1. **Thrust Testing:** Verify sufficient thrust for 15kt wind conditions
2. **Current Draw:** Measure actual current consumption
3. **Battery Runtime:** Validate 3-4 hour operation target
4. **Control Responsiveness:** Test PWM signal response and mixing
5. **Reliability:** Saltwater operation and corrosion resistance
6. **Calibration:** Verify ESC calibration procedure
7. **Slew Rate:** Test thrust ramping and smooth acceleration

## Wiring (Generic)
- **Battery +:** 4S LiPo positive terminal
- **Battery -:** Common ground plane
- **Signal:** ESP32 GPIO (per pinout specification)
- **Ground:** Common ground with ESP32
- **5V/BEC:** If ESC has BEC, do NOT connect (use external buck converter)

## Datasheet Location
- Store selected component datasheets in: `hardware/datasheets/`
- Update this document once hardware is finalized

## Decision Criteria

Factors for final hardware selection:
- Proven marine/saltwater reliability
- Adequate thrust for wind conditions
- Reasonable power consumption for 3-4 hour runtime
- Availability and lead time
- Cost per buoy (x4 buoys = 8 thrusters, 8 ESCs total)
- Ease of mounting and waterproofing
- Serviceability and propeller replacement
