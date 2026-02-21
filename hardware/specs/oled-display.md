# 0.96" I2C OLED Display (SSD1306)

## Overview
- **Controller:** SSD1306
- **Size:** 0.96 inch diagonal
- **Resolution:** 128x64 pixels
- **Interface:** I2C
- **Operating Voltage:** 3.3V - 5V
- **Color:** Monochrome (white or blue)

## I2C Configuration
- **Address:** 0x3C (default) or 0x3D
- **SDA:** Connect to ESP32 GPIO 21
- **SCL:** Connect to ESP32 GPIO 22
- **Speed:** 400 kHz (fast mode)
- **Pull-ups:** 2.2kΩ to 3.3V (usually on-board)

## Power Requirements
- **Supply Voltage:** 3.3V (from ESP32 rail)
- **Current Draw:** ~20mA (all pixels on), ~8mA (typical)

## Display Layout for Buoys

### Master Buoy Screen
```
+----------------------+
| MASTER - STABLE      |  Line 1: Role & State
| GPS: 37.7749,-122.41 |  Line 2: GPS position
| Sats: 8  HDOP: 1.2   |  Line 3: GPS quality
| Wind: 245° @ 12kt    |  Line 4: Wind data
| LoRa: 3/3 Online     |  Line 5: Slave status (Pin, Windward, Leeward)
| Batt: 15.8V  [###-]  |  Line 6: Battery level
+----------------------+
```

### Slave Buoy Screen
```
+----------------------+
| PIN - NAVIGATE       |  Line 1: Role (PIN/WINDWARD/LEEWARD) & Mode
| Tgt: 37.7750,-122.42 |  Line 2: Target GPS
| Dst: 2.3m  Hdg: 045° |  Line 3: Distance/Heading
| GPS: 8 sats  Good    |  Line 4: GPS status
| LoRa: RX 2s ago      |  Line 5: LoRa status
| Batt: 14.9V  [##--]  |  Line 6: Battery
+----------------------+
```

## Display States

### Master States
- **REPOSITIONING** - Wind unstable, updating positions
- **STABLE** - Wind stable, ready to signal start
- **READY** - Green light, awaiting start signal
- **LOCKED** - Race active, positions frozen

### Slave States
- **IDLE** - Awaiting first ASSIGN
- **NAVIGATE** - Moving to target
- **HOLD** - Within hold radius
- **RTH** - Return to home (LoRa lost)
- **CALIBRATE** - Compass calibration in progress

## Arduino Library
- **Adafruit SSD1306**
- **Adafruit GFX** (graphics primitives)

## Example Code
```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("Buoy Online"));
  display.display();
}
```

## Update Strategy
- **Refresh Rate:** 1-2 Hz (no need for faster)
- **Partial Updates:** Only redraw changed values
- **Priority:** GPS and LoRa status most important

## Wiring
- **VCC:** 3.3V (ESP32 3.3V pin)
- **GND:** Common ground
- **SDA:** GPIO 21
- **SCL:** GPIO 22

## Testing Checklist
- [ ] Verify I2C communication (I2C scanner)
- [ ] Test full display update
- [ ] Verify text rendering at various sizes
- [ ] Test update rate (should not slow main loop)
- [ ] Validate visibility in sunlight (may be poor)
- [ ] Check power consumption

## Datasheet Location
- Store in: `hardware/datasheets/SSD1306_OLED.pdf`

## Notes
- OLED visibility is poor in direct sunlight (not critical for diagnostics)
- Display is for setup and debugging, not operational monitoring
- Consider adding external LED indicators for operational status
- I2C bus is shared with compass on BE-880 module
