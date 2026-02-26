# Hardware Documentation

## Quick Reference

### Component Specifications
- [ESP32-S3-DevKitC-1](specs/esp32-s3.md) - Main controller
- [RFM95W LoRa Module](specs/rfm95w-lora.md) - 915 MHz communication
- [BE-880 GPS/Compass](specs/be-880-gps.md) - Navigation and heading
- [ESC & Thrusters](specs/esc-thrusters.md) - Propulsion system
- [OLED Display](specs/oled-display.md) - Status display
- [Davis Anemometer](specs/davis-anemometer.md) - Wind monitoring (Master only)

### Pinout Diagrams
- [Master Buoy Pinout](pinouts/master-pinout.md) - Wind Master connections
- [Slave Buoy Pinout](pinouts/slave-pinout.md) - Slave buoy connections
- [Remote Control Pinout](pinouts/remote-pinout.md) - NodeMCU-32S RC unit connections

### Datasheets
Store vendor datasheets in `datasheets/` folder for reference:
- ESP32-S3-DevKitC-1.pdf
- RFM95W_datasheet.pdf
- BE-880_GPS_module.pdf
- QMC5883L_compass.pdf
- AJ-SR04M_ultrasonic.pdf
- Davis_Vantage_Pro_Anemometer.pdf
- SSD1306_OLED.pdf
- ESC_thruster_specs.pdf (add when hardware selected)

### Schematics
Circuit diagrams and wiring schematics in `schematics/` folder

## Bill of Materials (BOM)

### Per Buoy (All 4 buoys)
| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32-S3-DevKitC-1 | 1 | Main controller |
| RFM95W LoRa Module | 1 | 915 MHz with antenna |
| BE-880 GPS/Compass | 1 | UART 115200 baud + I2C |
| AJ-SR04M Ultrasonic | 3 | Collision avoidance, 3.3V supply |
| 0.96" OLED Display | 1 | I2C SSD1306, 128×64 |
| Bilge Thruster ESC | 2 | Pending procurement |
| Brushless Bilge Thruster | 2 | Pending procurement |
| 4S LiPo Battery | 1 | 5000–10000 mAh |
| 5V Buck Converter | 1 | ≥1.5 A, 14.8 V input |
| 1000 µF Capacitor | 1 | 16 V+ rating, output of buck converter |
| Green LED | 1 | GPIO 38, with current-limiting resistor |
| Red LED | 1 | GPIO 39, with current-limiting resistor |
| Voltage Divider Resistors | 2 | 100 kΩ + 10 kΩ, battery monitor (GPIO 4) |

### Master Buoy Additional
| Component | Quantity | Notes |
|-----------|----------|-------|
| Davis Vantage Pro Anemometer | 1 | Wind speed (GPIO 6) + direction (GPIO 5) |
| 12V Marine Horn | 1 | Self-contained electronic (e.g. Fiamm MR3) |
| IRLZ44N N-channel MOSFET | 1 | Horn driver, gate on GPIO 7 |
| 1 kΩ resistor | 1 | MOSFET gate resistor |
| 1N4007 diode | 1 | Flyback protection across horn terminals |

### Remote Control Units (×2)
| Component | Quantity per unit | Notes |
|-----------|------------------|-------|
| NodeMCU-32S (ESP32-WROOM-32) | 1 | Different pin defaults from ESP32-S3 |
| RFM95W LoRa Module | 1 | VSPI bus (GPIO 23/19/18/5) |
| Blue LED | 1 | GPIO 26 — Repositioning |
| Green LED | 1 | GPIO 25 — Race Ready |
| Red LED | 1 | GPIO 33 — Fault |
| White LED | 1 | GPIO 13 — Race Locked / RTH |
| Waterproof momentary button | 2 | BTN_START GPIO 32, BTN_STOP GPIO 34 |
| Piezo buzzer | 1 | GPIO 27 |
| Single-cell LiPo | 1 | 500–1000 mAh |
| TP4056 LiPo charger module | 1 | USB-C input |
| IP67/IP68 enclosure | 1 | Capsize-rated |
| Sealed LED holders | 4 | O-ring seal through enclosure wall |
| Rubber USB-C port plug | 1 | Weatherproof when not charging |

### Total System
- 1 Master Buoy (Wind Master)
- 3 Slave Buoys (Pin, Windward, Leeward)
- 2 Remote Control Units (identical, same firmware)

## Power Budget

Detailed per-unit budgets are in the pinout files. Summary below.

### Master Buoy
| Component | Current (mA) | Notes |
|-----------|--------------|-------|
| ESP32 (normal) | 40-60 | No WiFi |
| ESP32 (WiFi AP) | 160-240 | Config mode only |
| GPS Module | 25-40 | Active tracking |
| LoRa TX | 120 | Transmitting |
| LoRa RX | 10-12 | Receiving/idle |
| OLED | 8-20 | Typical/max |
| Davis anemometer | 1-5 | Resistive; analog + pulse only |
| Compass | 0.1 | Minimal |
| LEDs | 20-40 | Both on |
| Ultrasonic sensors (×3) | 5-15 | Transit/RTH only |
| **Electronics Total** | ~160-270 mA | Normal operation (no WiFi) |
| Horn (12V rail) | 1000-3000 | Direct from battery; brief blasts only |
| Thrusters (idle) | 1000 mA | ESC idle |
| Thrusters (low) | 5000-10000 mA | Position hold |
| Thrusters (high) | 20000-40000 mA | 15kt wind |
| **System Total** | 5-40 A | Depends on conditions |

### Slave Buoy
| Component | Current (mA) | Notes |
|-----------|--------------|-------|
| ESP32 (normal) | 40-60 | No WiFi |
| GPS + LoRa + OLED + Compass | ~55-92 | Combined peripherals |
| Ultrasonic sensors (×3) | 5-15 | Transit/RTH only |
| LEDs | 20-40 | Both on |
| **Electronics Total** | ~155-265 mA | Typical |
| Thrusters + **System Total** | 5-40 A | Same as master |

### Remote Control Unit
| Component | Current (mA) | Notes |
|-----------|--------------|-------|
| ESP32 (normal) | 40-80 | WROOM-32, no WiFi |
| LoRa RX | 10-12 | Mostly idle |
| LEDs (active) | 10-20 | 1–2 on at a time |
| Buzzer | 10-30 | Brief alerts |
| **Total typical** | ~60-100 mA | |

### Battery Runtime Estimates
| Unit | Battery | Runtime |
|------|---------|---------|
| Buoy | 10000 mAh @ 3A avg | ~3 hours ✓ (target) |
| Buoy | 5000 mAh @ 3A avg | ~1.5 hours |
| RC unit | 500 mAh | ~5–8 hours |
| RC unit | 1000 mAh | ~10–16 hours |

Recommendation: 10000 mAh minimum for buoys; 500 mAh sufficient for RC full race day.

## Interface Summary

### Communication Buses
- **SPI (HSPI):** LoRa RFM95W — GPIO 11/13/12/10 (MOSI/MISO/SCK/CS), RST=14, IRQ=21
- **I2C:** OLED (0x3C) + QMC5883L compass (0x0D) — GPIO 8 (SDA), 9 (SCL)
- **UART1:** GPS BE-880 — GPIO 18 (RX), 17 (TX), **115200 baud** (reconfigured from factory 9600)

### Pin Conflicts
- None, all interfaces use unique pins

### Common Issues
- I2C pull-ups: Usually on-board, but verify with scope
- LoRa antenna: Must be connected before power-on
- GPS cold start: Allow 30+ seconds for first fix
- ESC calibration: Required on first power-up

## Assembly Notes

### Order of Assembly
1. Power system first (battery, buck converter, test voltages)
2. ESP32 (program basic test code)
3. OLED display (verify I2C communication)
4. GPS module (test UART, get fix outdoors)
5. Compass (test I2C, calibrate)
6. LoRa module (test SPI, range test)
7. ESCs (calibrate, test with thrusters in water)
8. Anemometer (Master only)
9. Final integration and testing

### Safety Considerations
- **LiPo batteries:** Use fireproof charging bags, monitor voltage
- **Waterproofing:** All electronics must be sealed (IP67+ recommended)
- **Propeller safety:** Thrusters can cause injury, test in controlled environment
- **ESC arming:** Ensure ESCs won't arm unexpectedly
- **Failsafe:** LoRa loss should trigger neutral thrust and RTH

## Testing & Validation

### Hardware Tests
- [ ] Power system (5V, 3.3V rails stable under load)
- [ ] Battery monitoring (accurate voltage reading, GPIO 4)
- [x] GPS fix acquisition — Module 2 passing (TinyGPS++, 115200 baud)
- [x] Compass heading — Module 3 sketch ready, I2C verified at 0x0D
- [x] LoRa TX/RX — Modules 4 & 5 passing
- [x] Wind sensor — Module 6 passing
- [ ] Battery monitor sketch — Module 7 TODO
- [ ] Thruster operation (forward/reverse/stop) — Module 8 pending hardware
- [x] Collision avoidance sensors — Module 9 sketch complete
- [ ] OLED display standalone — Module 1 TODO
- [ ] LED indicators (correct state mapping)

### System Integration
- [ ] Master-Slave communication (all 3 slaves respond: Pin, Windward, Leeward)
- [ ] Wind stability detection (15° threshold)
- [ ] Position hold (3m radius in calm water)
- [ ] Position hold (6m radius in 15kt wind)
- [ ] Return-to-home on LoRa loss
- [ ] Battery runtime (3+ hours target)
- [ ] Waterproof enclosure (submersion test)
- [ ] Remote control communication with Master buoy
- [ ] Remote control race start command functionality

## Systematic Subsystem Testing

Testing should be performed in a systematic way to verify each subsystem independently before full integration:

### Phase 1: Power System Testing
1. **5V Buck Converter Test**
   - Verify 5V output with no load
   - Test with 1.5A load (full electronics load simulation)
   - Check ripple voltage with oscilloscope (<50mV)
   - Verify efficiency across battery voltage range (13.2V-16.8V)

2. **3.3V Rail Test**
   - Measure 3.3V output from ESP32 regulator
   - Test under 500mA load
   - Verify stable operation during WiFi transmission bursts

3. **Battery Monitor Test**
   - Verify voltage divider accuracy (5:1 ratio)
   - Test ADC readings across full battery range
   - Implement low voltage warning threshold (13.5V)

### Phase 2: Communication Subsystems
1. **I2C Bus Test (OLED + Compass)**
   - Scan for devices (expect 0x3C for OLED, 0x0D/0x1E for compass)
   - Verify pull-up resistors with oscilloscope
   - Test simultaneous communication with both devices
   - Verify no address conflicts

2. **UART GPS Test**
   - Baud rate: **115200** (module reconfigured from factory 9600 — do not change)
   - Verify NMEA sentence reception outdoors (char counter must increment immediately)
   - Test cold start acquisition time (<60s target)
   - Verify hot start time (<10s)

3. **SPI LoRa Test**
   - Verify chip communication (read version register)
   - Configure for 915 MHz operation
   - Test transmit/receive with second module
   - Measure range in open field (500m minimum)
   - Test packet loss rate at various distances

### Phase 3: Sensor Subsystems
1. **GPS Accuracy Test**
   - Compare readings with known surveyed position
   - Measure accuracy over 10-minute period
   - Test fix stability while stationary
   - Verify HDOP values (<2.0 for good fix)

2. **Compass Calibration & Test**
   - Perform full 360° calibration routine
   - Compare readings with known magnetic heading
   - Test accuracy (±2° target)
   - Verify stability and absence of drift

3. **Anemometer Test (Master only)**
   - Verify frequency/pulse counting from sensor
   - Calibrate wind speed readings
   - Test direction accuracy (±5° target)
   - Verify stable readings over time

4. **Collision Avoidance Test (all buoys) — Module 9**
   - Sketch: `testing/ultrasonic_test/main.cpp`
   - Sweep hand in front of each sensor — only that sensor should respond
   - Object at <50 cm forward → red LED, STOP status
   - Object at ~1 m forward, clear port → AVOID PORT
   - Object at ~1 m forward, clear starboard → AVOID STBD
   - All clear → green steady

### Phase 4: Propulsion System (pending hardware procurement)
1. **ESC Calibration**
   - Perform ESC calibration routine (full throttle range)
   - Verify neutral point (1500µs ±25µs)
   - Test forward range (1500-2000µs)
   - Test reverse range (1000-1500µs)

2. **Thruster Operation Test**
   - Test in water (controlled environment)
   - Verify thrust direction (forward/reverse)
   - Measure current draw at various throttle positions
   - Test differential thrust for turning

3. **PWM Signal Quality**
   - Verify 100Hz refresh rate with oscilloscope
   - Check pulse width accuracy (±5µs)
   - Test slew rate limiting (smooth ramping)

### Phase 5: Display & Indicators
1. **OLED Display Test**
   - Verify all pixels functional
   - Test various display content
   - Verify visibility in bright sunlight
   - Check power consumption

2. **LED Indicators**
   - Test Green LED (within radius / ready state)
   - Test Red LED (navigating / RTH)
   - Verify brightness and visibility
   - Check current draw

### Phase 6: Integration Testing
1. **Master Buoy Integration**
   - All subsystems powered and communicating
   - Wind monitoring active and stable
   - LoRa broadcasting functional
   - Web configuration portal accessible

2. **Slave Buoy Integration**
   - All subsystems powered and communicating
   - GPS lock acquired and stable
   - LoRa receiving commands from Master
   - Propulsion responding to navigation commands

3. **Remote Control Integration**
   - LoRa communication with Master established (PKT_RC_START → PKT_MASTER_STATUS loop)
   - BTN_START single press → master initiates repositioning
   - BTN_STOP hold 3s → master sends RTH to all slaves
   - LED patterns match MASTER_STATUS fleet_state correctly
   - Drift alert: buzzer + red LED when any slave > 10m from target
   - Battery life adequate for full race day (≥8 hours on 500 mAh)

### Phase 7: Field Testing
1. **Calm Water Tests**
   - Position hold within 3m radius
   - Return-to-home on signal loss
   - Battery runtime measurement
   - Communication range verification

2. **Wind Condition Tests**
   - Position hold in 5kt wind
   - Position hold in 10kt wind
   - Position hold in 15kt wind (target capability)
   - Adjust hold radius dynamically (3m to 6m)

3. **Full System Race Simulation**
   - Deploy all 3 slave buoys (Pin, Windward, Leeward) with Master
   - Test wind stability detection
   - Verify diamond course geometry
   - Test race start sequence from remote control
   - Monitor all systems for 3+ hours

### Test Documentation
- Record all test results in `testing/` directory
- Document any failures and resolutions
- Create test report for each phase
- Maintain checklist of completed tests
- [ ] Wind stability detection (15° threshold)
- [ ] Position hold (3m radius in calm water)
- [ ] Position hold (6m radius in 15kt wind)
- [ ] Return-to-home on LoRa loss
- [ ] Battery runtime (3+ hours target)
- [ ] Waterproof enclosure (submersion test)

## Design Files

### To Be Added
- [ ] PCB schematic (if custom board designed)
- [ ] Enclosure CAD models
- [ ] Mounting bracket designs
- [ ] Cable harness diagrams
- [ ] Waterproof gland specifications

## Supplier Links

### Components
- ESP32-S3-DevKitC-1: Espressif or Adafruit
- NodeMCU-32S: various (ESP32-WROOM-32 module)
- LoRa RFM95W: Adafruit (Product ID: 3072)
- GPS/Compass BE-880: various (u-blox compatible with QMC5883L)
- AJ-SR04M ultrasonic: various (compatible with JSN-SR04T Mode 1)
- Bilge thruster ESCs + thrusters: pending selection
- Anemometer: Davis Instruments (Vantage Pro)

### Tools Required
- Soldering iron & solder
- Heat shrink tubing
- Wire strippers
- Multimeter
- Oscilloscope (for debugging)
- LiPo charger/balancer
- Waterproof enclosure & sealant

## Maintenance Schedule

### Pre-Deployment
- Check battery voltage and balance
- Verify GPS fix
- Test LoRa communication
- Inspect thruster propellers
- Check all connections

### Post-Deployment
- Rinse with fresh water (remove salt)
- Inspect seals and gaskets
- Check battery capacity
- Download logs (if implemented)
- Inspect thrusters for fouling

### Periodic
- Recalibrate compass (if heading drift observed)
- Update firmware (bug fixes/features)
- Replace worn propellers
- Check ESC connections for corrosion
