# Davis Vantage Pro Anemometer (Master Buoy Only)

## Overview
- **Type:** Wind speed and direction sensor
- **Installation:** Master buoy only
- **Purpose:** Monitor wind stability for race readiness
- **Mounting:** Must be elevated above buoy body for clear wind

## Specifications

### Wind Speed
- **Range:** 1-200 mph (0.5-89 m/s)
- **Resolution:** 1 mph (0.5 m/s)
- **Accuracy:** ±1 mph or ±5% (whichever is greater)
- **Sensor:** Rotating cup anemometer with magnetic switch
- **Output:** Pulse train (frequency proportional to wind speed)

### Wind Direction
- **Range:** 0-360 degrees (16 compass points)
- **Resolution:** 1 degree
- **Accuracy:** ±3 degrees
- **Sensor:** Potentiometer vane
- **Output:** Analog voltage (0-5V or 0-3.3V depending on model)

## Electrical Interface

### Wind Speed Signal
- **Type:** Digital pulse output
- **Signal:** Open collector or TTL
- **Connection:** ESP32 GPIO 35 (interrupt capable)
- **Logic:** Each pulse = specific wind speed increment
- **Calculation:** Speed = (pulses per second) * conversion factor

### Wind Direction Signal
- **Type:** Analog voltage
- **Range:** 0-3.3V (or use voltage divider from 5V)
- **Connection:** ESP32 GPIO 36 (ADC1_CH0)
- **Calculation:** Direction = (ADC_value / ADC_max) * 360

## Wind Speed Calculation

### Pulse Counting Method
```cpp
volatile unsigned long pulseCount = 0;
unsigned long lastMeasurement = 0;

void IRAM_ATTR windSpeedISR() {
    pulseCount++;
}

void setup() {
    pinMode(35, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(35), windSpeedISR, FALLING);
}

float getWindSpeed() {
    unsigned long now = millis();
    unsigned long elapsed = now - lastMeasurement;
    
    if (elapsed >= 3000) {  // Sample every 3 seconds
        float hz = pulseCount / (elapsed / 1000.0);
        float windSpeed_mph = hz * 1.492;  // Davis conversion factor
        float windSpeed_knots = windSpeed_mph * 0.868976;
        
        pulseCount = 0;
        lastMeasurement = now;
        return windSpeed_knots;
    }
    return -1;  // No new data
}
```

## Wind Direction Calculation

### ADC Reading Method
```cpp
float getWindDirection() {
    int adcValue = analogRead(36);  // 12-bit ADC (0-4095)
    float voltage = (adcValue / 4095.0) * 3.3;
    float direction = (voltage / 3.3) * 360.0;
    return direction;
}
```

### Direction Mapping (if using resistor network)
Some Davis units use resistors for 16 compass points. May need calibration table:
```cpp
float directionLookup[16] = {
    0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5,
    180, 202.5, 225, 247.5, 270, 292.5, 315, 337.5
};
// Map ADC ranges to nearest direction
```

## Wind Stability Logic

### Requirements
- Wind must not shift more than **15°** within a rolling **60-second window**
- If stable for 60s: Transition to READY state
- If unstable: Stay in REPOSITIONING mode

### Implementation
```cpp
#define BUFFER_SIZE 60      // 60 seconds
#define SAMPLE_RATE 1000    // Sample every 1 second
#define STABILITY_THRESHOLD 15.0  // 15 degrees

float windBuffer[BUFFER_SIZE];
int bufferIndex = 0;
unsigned long lastSample = 0;

void updateWindBuffer(float direction) {
    windBuffer[bufferIndex] = direction;
    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
}

bool isWindStable() {
    float min = 360.0, max = 0.0;
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (windBuffer[i] < min) min = windBuffer[i];
        if (windBuffer[i] > max) max = windBuffer[i];
    }
    
    float range = max - min;
    
    // Handle wraparound at 0/360
    if (range > 180) {
        range = 360 - range;
    }
    
    return (range <= STABILITY_THRESHOLD);
}
```

## Race State Machine (Master)

### States
1. **REPOSITIONING**
   - Wind unstable
   - Continuously update slave target positions
   - Red LED on
   - Broadcast ASSIGN every 5 seconds

2. **STABLE**
   - Wind stable for 60 seconds
   - Freeze geometry calculations
   - Green LED on
   - Transition to READY

3. **READY**
   - Awaiting start signal from remote control
   - Positions locked
   - Green LED on
   - Broadcast keepalive every 30 seconds

4. **LOCKED**
   - Race active (countdown started)
   - Ignore all wind shifts
   - Positions remain frozen
   - Continue until "End Race" signal

### Override Logic
- Remote control can force READY state
- Override active even if wind > 15° shift
- Use for urgent race starts in unstable conditions

## Power Requirements
- **Supply Voltage:** 5-12V (check specific model)
- **Current Draw:** ~5-10mA
- **Power from:** 5V buck converter rail

## Mounting Considerations
- Mount vane at least 1 meter above buoy body
- Ensure free rotation (no obstructions)
- Weatherproof connections
- Use shielded cable to ESP32 (reduce noise)

## Wiring
- **Wind Speed Pulse:** GPIO 35 (with pull-up)
- **Wind Direction Analog:** GPIO 36
- **Power:** 5V rail
- **Ground:** Common ground

## Calibration
1. **Wind Speed:** Compare to known reference (handheld anemometer)
2. **Wind Direction:** Use compass to verify 0° = North, 90° = East, etc.
3. **Stability Threshold:** May need tuning based on field conditions

## Testing Checklist
- [ ] Verify pulse counting (indoor fan test)
- [ ] Validate wind speed calculation (compare to reference)
- [ ] Test wind direction accuracy (manual rotation)
- [ ] Verify 60-second rolling buffer
- [ ] Test stability detection (simulate stable/unstable wind)
- [ ] Validate state transitions (REPOSITIONING -> STABLE -> READY)
- [ ] Test manual override functionality
- [ ] Confirm LoRa broadcasts update correctly

## Datasheet Location
- Store in: `hardware/datasheets/Davis_Vantage_Pro_Anemometer.pdf`

## Notes
- Wind speed below 1 mph may not register (threshold of cup anemometer)
- Direction accuracy degrades at very low wind speeds
- Consider averaging multiple direction samples (reduce noise)
- Stability logic may need adjustment for local wind patterns
- Failsafe: If anemometer fails, allow manual override to proceed
