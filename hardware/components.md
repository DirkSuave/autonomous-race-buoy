# Hardware Components

## Controller
- **ESP32-S3-DevKitC-1**
  - Dual-core processor
  - WiFi + Bluetooth
  - Multiple GPIO, I2C, SPI interfaces

## Propulsion System (TBD)
- **Status:** Under evaluation - motors and ESCs not yet finalized
- **Previously Considered:** APISQUEEN Feather-45A ESC with brushless thrusters
- **Requirements:**
  - Differential drive capability for navigation
  - PWM Control: 100Hz refresh rate target
  - Signal: 1.0ms (Rev) / 1.5ms (Stop) / 2.0ms (Fwd)
  - Marine-grade for saltwater operation
  - Sufficient thrust for 15kt wind capability

## Power System
- **4S LiPo Battery** (per buoy)
  - Target runtime: 3-4 hours
  - 15kt wind capability
- **5V Buck Converter** (1.5A)
  - For electronics rail
  - 1000µF capacitor for inductive load protection

## Navigation & Sensors
- **BE-880 GPS/Compass Module**
  - GPS positioning
  - Integrated compass for heading
  - Calibration required on first target reach

### Master Buoy Only
- **Davis Vantage Pro Anemometer**
  - Wind speed monitoring
  - Wind direction tracking
  - Used for stability calculations

## Communication
- **Adafruit RFM95W LoRa Module**
  - Frequency: 915 MHz
  - Star topology (Master -> Slaves)
  - Target range: 500m

## User Interface
- **0.96" I2C OLED Display (SSD1306)**
  - GPS status
  - LoRa connectivity
  - Mode diagnostics
  - Per buoy installation

## Indicators
- **LED System**
  - Green LED: Ready status
  - Red LED: Not ready/repositioning
  - Yellow LED: Race Started and Bouys keeping positions
- **Remote Control Interface**
  - Start race signal
  - Manual override capability
  - Race end trigger
  - Return to home trigger
  - Displays Master buoy status
  - Possible Display or LED Indicators

## Navigation Parameters
- **Hold Radius:** Adjustable
  - Default: 3m
  - Heavy Wind (15kt): 6m
- **Differential Thrust Mixing**
  - Deadband: ±25µs
  - Slew-rate limiting for smooth thrust ramping

## Remote Control Unit
- **Purpose:** Handheld device for race control
- **Status:** Features TBD - requires further specification
- **Planned Features:**
  - LoRa communication with Master buoy
  - Race start/stop commands
  - Buoy status monitoring display
  - Emergency override controls
  - Battery-powered (rechargeable)

## Bill of Materials Status
- [ ] Create detailed BOM with part numbers
- [ ] Add supplier links
- [ ] Calculate per-buoy cost
- [ ] Determine quantity for 4 buoys (1 Master + 3 Slaves: Pin, Windward, Leeward)
- [ ] Finalize propulsion system selection
- [ ] Specify remote control unit hardware and Interface
