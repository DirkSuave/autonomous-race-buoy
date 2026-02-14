# Autonomous Race Buoy - Getting Started

## Project Structure

```
autonomous_race_buoy/
├── common/          # Shared code (protocol, config)
├── master/          # Master buoy firmware
├── slave/           # Slave buoy firmware
├── tests/           # Hardware test sketches
└── docs/            # Documentation
```

## Hardware Test Sketches

To validate your hardware before integration, use these test sketches:

### 1. LoRa Communication Test

**Test the Adafruit RFM95W LoRa module:**

- Upload `tests/lora_test_tx.ino` to one ESP32-S3
- Upload `tests/lora_test_rx.ino` to another ESP32-S3
- Open serial monitors for both (115200 baud)
- Verify packets are transmitted and acknowledged
- Check RSSI and SNR values (should be strong at close range)

**Expected output:**
- TX: "Sending: Packet #X" followed by "Received: ACK from Slave"
- RX: "Received: Packet #X" with RSSI/SNR values

### 2. GPS/Compass Test

**Test the BE-880 GPS Compass module:**

- Upload `tests/gps_test.ino` to ESP32-S3
- Connect BE-880 to RX/TX pins (see config.h)
- Open serial monitor (115200 baud)
- Wait for GPS fix (may take 1-2 minutes outdoors)
- Verify latitude, longitude, heading, and satellite count

**Expected output:**
```
--- GPS Fix ---
Lat: 37.1234567 Lon: -122.9876543
Fix: 1 Sats: 8 HDOP: 1.2
Alt: 25.3 m
Heading: 145.2°
```

### 3. Wind Sensor Test

**Test the Davis Vantage Pro anemometer:**

- Upload `tests/wind_sensor_test.ino` to ESP32-S3
- Connect wind speed pulse output to WIND_SPEED_PIN
- Connect wind direction analog output to WIND_DIR_PIN
- Open serial monitor (115200 baud)
- Spin the anemometer to generate pulses

**Expected output:**
```
Wind Speed: 12.50 km/h (6.75 knots) | Direction: 245.3° | Raw ADC: 2785
```

**Calibration notes:**
- Davis outputs 1 pulse/sec per 1 mph wind speed
- Direction voltage (0-3.3V) maps to 0-360°
- You may need to calibrate direction offset based on physical mounting

## Pin Configuration

All pin assignments are in `common/config.h`. Default pins:

**LoRa (RFM95W):**
- CS: GPIO 5
- RST: GPIO 14
- IRQ: GPIO 2

**GPS (BE-880):**
- RX: GPIO 16
- TX: GPIO 17
- Baud: 115200

**Wind Sensors:**
- Speed (pulse): GPIO 34
- Direction (analog): GPIO 35

**Motors (future):**
- Left PWM: GPIO 25, DIR: GPIO 26
- Right PWM: GPIO 27, DIR: GPIO 33

## Dependencies

Install these libraries via Arduino IDE Library Manager:

1. **RadioHead** - for RFM95W LoRa communication
   - Search: "RadioHead" by Mike McCauley

## Next Steps

After validating all hardware:

1. Review `docs/project_summary.md` for system architecture
2. Review `common/protocol.h` for packet structures and states
3. Begin master buoy integration (milestone 5)
4. Implement slave state machine (milestone 6)

## Troubleshooting

**LoRa not initializing:**
- Check SPI wiring (MISO, MOSI, SCK)
- Verify CS, RST, IRQ pin connections
- Ensure antenna is connected

**GPS no fix:**
- Must have clear sky view (outdoors)
- First fix can take 1-2 minutes
- Check baud rate matches BE-880 (usually 9600 or 115200)

**Wind sensor no readings:**
- Verify pull-up resistor on speed pin
- Check analog reference voltage for direction
- Davis sensors need proper power (typically 12V for full operation)
