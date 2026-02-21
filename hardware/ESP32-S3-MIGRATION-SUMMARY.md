# ESP32-S3 DevKitC-1 Migration Summary

**Date:** 2026-02-19
**Status:** Documentation Updated ✅

## Overview
All hardware documentation has been updated to accurately reflect the ESP32-S3-DevKitC-1 pinout and specifications based on official references:
- https://randomnerdtutorials.com/esp32-s3-devkitc-pinout-guide/
- https://www.espboards.dev/esp32/esp32-s3-devkitc-1/
- Espressif ESP32-S3 documentation

## Critical Changes from Old Pinout

### ⚠️ BREAKING CHANGES - Old GPIO assignments were incorrect for ESP32-S3

| Interface | Old Pins (WRONG) | New Pins (CORRECT) | Reason |
|-----------|------------------|--------------------|---------| 
| **I2C** | GPIO 21/22 | **GPIO 8/9** | ESP32-S3 defaults are different from ESP32 |
| **SPI** | GPIO 23/19/18/5 | **GPIO 11/13/12/10** | HSPI defaults on S3 |
| **PWM (Thrusters)** | GPIO 25/26 | **GPIO 47/48** | GPIO 25-32 reserved for flash on S3 |
| **Battery ADC** | GPIO 34 | **GPIO 4** | GPIO 34 doesn't exist on S3 |
| **Wind Direction** | GPIO 36 | **GPIO 5** | GPIO 35-37 may conflict with PSRAM |
| **Wind Speed** | GPIO 35 | **GPIO 6** | Moved to ADC1 for WiFi compatibility |
| **Status LEDs** | GPIO 32/33 | **GPIO 38/39** | GPIO 32/33 reserved for flash on S3 |
| **LoRa IRQ** | GPIO 27 | **GPIO 21** | GPIO 27 reserved for flash on S3 |

## Reserved Pins (NEVER USE)

### GPIO 26-32: Flash/PSRAM (Critical!)
These are internally connected and will cause the ESP32-S3 to fail:
- GPIO 26, 27, 28, 29, 30, 31, 32

### GPIO 0, 3, 45, 46: Strapping Pins
These affect boot mode - avoid using in hardware design.

### GPIO 35-37: PSRAM Conflicts
May conflict with PSRAM on N8R8 variant (8MB PSRAM). Safe on N8 variant.

### GPIO 43, 44: USB/UART0
Hardwired to USB programming interface - cannot be used.

## ADC Channel Selection

### ADC1 (WiFi-Safe) - GPIO 1-10
✅ **USE THESE** for critical sensors when WiFi is active:
- GPIO 4: Battery Monitor (ADC1_CH3)
- GPIO 5: Wind Direction (ADC1_CH4)
- GPIO 6: Wind Speed pulse counter (ADC1_CH5)

### ADC2 (WiFi Conflict) - GPIO 11-20
⚠️ **AVOID** these for sensors - ADC2 doesn't work when WiFi is active
- SPI pins (GPIO 10-13) are on ADC2, but used for communication only

## Files Updated

### 1. hardware/specs/esp32-s3.md
- Added complete ESP32-S3 peripheral list
- Documented all GPIO restrictions and reserved pins
- Updated I2C, SPI, UART default pins for ESP32-S3
- Added strapping pin warnings
- Added ADC1 vs ADC2 WiFi conflict notes
- Provided Arduino code examples for pin configuration

### 2. hardware/pinouts/master-pinout.md
- Updated all pin assignments to ESP32-S3 compatible GPIOs
- Added migration notes explaining all changes
- Added warning callouts for incorrect old pins
- Updated I2C to GPIO 8/9 (S3 defaults)
- Updated SPI to GPIO 11/13/12/10 (HSPI defaults)
- Updated PWM to GPIO 47/48 (avoiding reserved pins)
- Moved ADC sensors to ADC1 channels (GPIO 4/5/6)
- Updated status LEDs to GPIO 38/39

### 3. hardware/pinouts/slave-pinout.md
- Same pin updates as master (for consistency)
- Documented available expansion GPIOs
- Added code example for shared pin definitions
- Maintains compatibility with master firmware

### 4. hardware/README.md
- No changes needed (already board-agnostic)

## Default Pin Assignments for Firmware

```cpp
// Communication Interfaces (ESP32-S3 defaults)
#define I2C_SDA 8
#define I2C_SCL 9
#define SPI_MOSI 11
#define SPI_MISO 13
#define SPI_SCK 12
#define SPI_CS 10

// LoRa Module
#define LORA_RST 14
#define LORA_IRQ 21

// GPS Module
#define GPS_TX 17
#define GPS_RX 18

// PWM Outputs (Thrusters)
#define PWM_LEFT 47
#define PWM_RIGHT 48

// Analog Inputs (ADC1 - WiFi safe)
#define BATTERY_ADC 4
#define WIND_DIR_ADC 5    // Master only
#define WIND_SPEED_PIN 6  // Master only (pulse counter)

// Digital I/O
#define LED_GREEN 38
#define LED_RED 39

// Remote Control (Master only, optional)
#define BTN_START 7
#define BTN_OVERRIDE 15
#define BTN_END 16
```

## Arduino IDE Setup

### Board Configuration
- Board: **ESP32-S3-DevKitC-1**
- Flash Size: **8MB**
- PSRAM: **0MB** (N8) or **8MB** (N8R8) depending on variant
- USB CDC On Boot: **Enabled** (for Serial over USB)
- Upload Speed: **921600**

### I2C Initialization
```cpp
Wire.begin();  // Uses GPIO 8 (SDA), GPIO 9 (SCL) by default
```

### SPI Initialization
```cpp
SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
```

### PWM (LEDC) Setup
```cpp
ledcSetup(0, 100, 16);      // Channel 0, 100Hz, 16-bit
ledcAttachPin(PWM_LEFT, 0);
ledcWrite(0, dutyCycle);
```

## Testing Checklist

Before deploying with new pin assignments:

- [ ] Verify I2C devices detected on GPIO 8/9 (not 21/22)
- [ ] Verify SPI LoRa communication on GPIO 10-13 (not 5/18/19/23)
- [ ] Verify UART GPS data on GPIO 17/18
- [ ] Test PWM thruster control on GPIO 47/48 (not 25/26)
- [ ] Test ADC battery reading on GPIO 4 (not 34)
- [ ] Test wind sensors on GPIO 5/6 (not 35/36)
- [ ] Test status LEDs on GPIO 38/39 (not 32/33)
- [ ] Verify no conflicts with reserved pins (26-32)

## Next Steps

1. **Hardware Assembly:** Use the updated pinouts when wiring
2. **Firmware Development:** Use the pin definitions above in code
3. **Testing:** Verify each peripheral works before integration
4. **Documentation:** Keep this file updated with any further changes

## References

- [ESP32-S3 Specifications](specs/esp32-s3.md)
- [Master Buoy Pinout](pinouts/master-pinout.md)
- [Slave Buoy Pinout](pinouts/slave-pinout.md)
- [Random Nerd Tutorials ESP32-S3 Pinout Guide](https://randomnerdtutorials.com/esp32-s3-devkitc-pinout-guide/)
- [ESPBoards ESP32-S3-DevKitC-1 Reference](https://www.espboards.dev/esp32/esp32-s3-devkitc-1/)
- [Espressif ESP32-S3 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)

## Important Notes

⚠️ **Hardware Impact:** If any hardware has been built using old pin assignments, it will need to be rewired.

⚠️ **PSRAM Variant:** Verify whether you have N8 (no PSRAM) or N8R8 (8MB PSRAM) variant before using GPIO 35-37.

⚠️ **ADC WiFi Conflict:** ADC2 (GPIO 11-20) cannot be used while WiFi is active. Critical sensors MUST use ADC1 (GPIO 1-10).

✅ **Firmware Not Yet Written:** No code changes needed since firmware directories are empty. Use the correct pin definitions from the start.
