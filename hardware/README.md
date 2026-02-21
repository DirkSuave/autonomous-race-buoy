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

### Datasheets
Store vendor datasheets in `datasheets/` folder for reference:
- ESP32-S3-DevKitC-1.pdf
- RFM95W_datasheet.pdf
- BE-880_GPS_module.pdf
- HMC5883L_compass.pdf
- Propulsion_system_TBD.pdf (ESC and thruster specs - under evaluation)
- Davis_Vantage_Pro_Anemometer.pdf
- SSD1306_OLED.pdf

### Schematics
Circuit diagrams and wiring schematics in `schematics/` folder

## Bill of Materials (BOM)

### Per Buoy (All 4 buoys)
| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32-S3-DevKitC-1 | 1 | Main controller |
| RFM95W LoRa Module | 1 | 915 MHz with antenna |
| BE-880 GPS/Compass | 1 | UART + I2C |
| 0.96" OLED Display | 1 | I2C, 128x64 |
| Propulsion System | 1 | TBD - thrusters and ESCs under evaluation |
| 4S LiPo Battery | 1 | 5000-10000 mAh |
| 5V Buck Converter | 1 | 1.5A min, 14.8V input |
| 1000µF Capacitor | 1 | 16V+ rating |
| Green LED | 1 | With 220Ω resistor |
| Red LED | 1 | With 220Ω resistor |
| Voltage Divider Resistors | 2 | For battery monitor |

### Master Buoy Additional
| Component | Quantity | Notes |
|-----------|----------|-------|
| Davis Vantage Pro Anemometer | 1 | Wind speed/direction |

### Remote Control Unit (separate device)
| Component | Quantity | Notes |
|-----------|----------|-------|
| Controller Board | 1 | TBD - ESP32 or similar |
| LoRa Module | 1 | RFM95W 915 MHz |
| Display | 1 | TBD - OLED or LCD |
| Controls | TBD | Buttons/switches for race start and status |
| Battery | 1 | TBD - rechargeable Li-ion |

### Total System (4 Buoys + Remote Control)
- 1 Master Buoy (Wind Master)
- 3 Slave Buoys (Pin, Windward, Leeward)
- 1 Remote Control Unit

## Power Budget

### Master Buoy
| Component | Current (mA) | Notes |
|-----------|--------------|-------|
| ESP32 (WiFi) | 80-120 | Peak during config |
| ESP32 (normal) | 40-60 | Operating mode |
| GPS Module | 25-40 | Active tracking |
| LoRa TX | 120 | Transmitting |
| LoRa RX | 10-12 | Receiving/idle |
| OLED | 8-20 | Typical/max |
| Anemometer | 5-10 | Continuous |
| Compass | 0.1 | Minimal |
| LEDs | 20-40 | Both on |
| **Electronics Total** | ~200-300 mA | Typical |
| Thrusters (idle) | 1000 mA | ESC idle |
| Thrusters (low) | 5000-10000 mA | Position hold |
| Thrusters (high) | 20000-40000 mA | 15kt wind |
| **System Total** | 5-40 A | Depends on conditions |

### Battery Runtime Estimate
- **5000 mAh @ 10A avg:** ~30 minutes
- **5000 mAh @ 5A avg:** ~1 hour
- **10000 mAh @ 10A avg:** ~1 hour
- **10000 mAh @ 3A avg:** ~3 hours ✓ (target)

Recommendation: 10000 mAh minimum for 3-4 hour operation

## Interface Summary

### Communication Buses
- **SPI:** LoRa module (shared bus)
- **I2C:** OLED + Compass (shared bus, address 0x3C, 0x0D/0x1E)
- **UART:** GPS module (dedicated)

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
- [ ] Battery monitoring (accurate voltage reading)
- [ ] GPS fix acquisition (<60s outdoors)
- [ ] Compass heading (accurate within 2°)
- [ ] LoRa range (500m minimum)
- [ ] Thruster operation (forward/reverse/stop)
- [ ] ESC slew rate limiting (smooth acceleration)
- [ ] OLED display (all information visible)
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
   - Configure baud rate (9600 typical for BE-880)
   - Verify NMEA sentence reception outdoors
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

### Phase 4: Propulsion System (TBD - awaiting hardware selection)
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

3. **Remote Control Integration (TBD)**
   - LoRa communication with Master established
   - Command transmission successful
   - Status reception and display working
   - Battery life adequate for full race day

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
- ESP32: Espressif or Adafruit
- LoRa: Adafruit (Product ID: 3072)
- GPS: Various (u-blox compatible)
- Propulsion: TBD - evaluating marine thrusters and ESCs
- Anemometer: Davis Instruments
- Anemometer: Davis Instruments

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
