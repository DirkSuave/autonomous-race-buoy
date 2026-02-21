# BE-880 GPS/Compass Module

## Overview
- **GPS Chipset:** u-blox compatible
- **Compass:** HMC5883L or QMC5883L (I2C)
- **Interface:** UART (GPS) + I2C (Compass)
- **Operating Voltage:** 3.3V - 5V
- **Update Rate:** 1-10 Hz (configurable)

## GPS Specifications

### Performance
- **Channels:** 56
- **Sensitivity:** -165 dBm (tracking)
- **Accuracy:** 2.5m CEP (typical, clear sky)
- **Cold Start:** <32 seconds
- **Hot Start:** <1 second
- **Max Altitude:** 18,000m
- **Max Velocity:** 515 m/s

### UART Configuration
- **Baud Rate:** 9600 (default), can configure to 115200
- **Format:** 8N1 (8 data bits, no parity, 1 stop bit)
- **Protocol:** NMEA 0183
- **TX:** Module TX -> ESP32 GPIO 16 (RX)
- **RX:** Module RX -> ESP32 GPIO 17 (TX)

### NMEA Sentences Used
- **$GPGGA:** Position, altitude, fix quality, satellites
- **$GPRMC:** Position, velocity, date/time
- **$GPVTG:** Course and speed over ground

### Key Data Extracted
- Latitude (decimal degrees)
- Longitude (decimal degrees)
- Fix quality (0=invalid, 1=GPS, 2=DGPS)
- Number of satellites
- HDOP (horizontal dilution of precision)
- Altitude
- Speed over ground
- Course over ground

## Compass Specifications (HMC5883L / QMC5883L)

### Performance
- **Resolution:** 1-2 degrees
- **Range:** ±1.3 to ±8.1 Gauss
- **Update Rate:** Up to 75 Hz
- **Accuracy:** 1-2° (after calibration)

### I2C Configuration
- **Address:** 0x0D (QMC5883L) or 0x1E (HMC5883L)
- **SDA:** Connect to ESP32 GPIO 21
- **SCL:** Connect to ESP32 GPIO 22
- **Pull-ups:** 2.2kΩ to 3.3V (may be on-board)

### Calibration Procedure
- Perform ONCE when slave reaches first target position
- Rotate buoy 360° slowly (motor-driven or manual)
- Collect min/max X, Y, Z magnetometer readings
- Calculate offsets: offset = (max + min) / 2
- Store calibration in EEPROM/flash
- Apply offsets to all subsequent readings

### Heading Calculation
```
heading = atan2(Y - offsetY, X - offsetX) * 180 / PI
if (heading < 0) heading += 360;
```

## Integration Notes

### GPS Parsing
- Use TinyGPS++ library (Arduino)
- Parse NMEA sentences in real-time
- Validate fix quality before using coordinates
- Require minimum 4 satellites for navigation

### Position Hold Logic
```
1. Get current GPS position (lat, lon)
2. Calculate distance to target using Haversine formula
3. Calculate bearing to target using compass
4. If distance > hold_radius:
   - Apply differential thrust toward target
5. Else:
   - Minimal thrust to counteract drift
```

## Haversine Distance Formula
```cpp
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = radians(lat2 - lat1);
    double dLon = radians(lon2 - lon1);
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(radians(lat1)) * cos(radians(lat2)) *
               sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return 6371000 * c;  // Earth radius in meters
}
```

## Bearing Calculation
```cpp
double bearing(double lat1, double lon1, double lat2, double lon2) {
    double dLon = radians(lon2 - lon1);
    double y = sin(dLon) * cos(radians(lat2));
    double x = cos(radians(lat1)) * sin(radians(lat2)) -
               sin(radians(lat1)) * cos(radians(lat2)) * cos(dLon);
    double brng = atan2(y, x);
    return fmod((degrees(brng) + 360), 360);
}
```

## Power Requirements
- **GPS Active:** ~25-40mA @ 3.3V
- **GPS Sleep:** ~1mA
- **Compass:** ~100µA

## Wiring
- **VCC:** 3.3V or 5V (check module specs)
- **GND:** Common ground
- **TX (GPS):** To ESP32 RX (GPIO 16)
- **RX (GPS):** To ESP32 TX (GPIO 17)
- **SDA (Compass):** To ESP32 SDA (GPIO 21)
- **SCL (Compass):** To ESP32 SCL (GPIO 22)

## Testing Checklist
- [ ] Verify GPS UART output (raw NMEA)
- [ ] Parse GPS coordinates successfully
- [ ] Achieve GPS fix outdoors (clear sky)
- [ ] Verify compass I2C communication
- [ ] Perform compass calibration
- [ ] Test heading accuracy (compare to phone compass)
- [ ] Validate Haversine distance calculation
- [ ] Test position hold (stationary target)

## Known Issues
- GPS requires clear sky view (no indoor operation)
- Compass affected by nearby magnets/motors (keep distance)
- Initial GPS fix can take 30+ seconds (cold start)
- Compass must be level for accurate heading (buoy tilt matters)

## Libraries
- **GPS:** TinyGPS++ or NeoGPS
- **Compass:** Adafruit HMC5883L or QMC5883L libraries

## Datasheet Location
- Store in: `hardware/datasheets/BE-880_GPS_module.pdf`
- HMC5883L: `hardware/datasheets/HMC5883L_compass.pdf`

## Notes
- Enable GPS NMEA sentence filtering to reduce UART traffic
- Use hardware serial for GPS (more reliable than software serial)
- Store last known good position for GPS loss scenarios
- Compass calibration degrades over time (recalibrate periodically)
