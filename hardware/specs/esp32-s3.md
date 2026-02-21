# ESP32-S3-DevKitC-1 Specifications

## Overview
- **Chip:** ESP32-S3-WROOM-1 or ESP32-S3-WROOM-2
- **CPU:** Xtensa dual-core 32-bit LX7
- **Clock Speed:** Up to 240 MHz
- **Flash:** 8 MB (typical for N8 variant)
- **PSRAM:** 0 MB (N8) or 8 MB (N8R8 variant)
- **WiFi:** 802.11 b/g/n (2.4 GHz)
- **Bluetooth:** BLE 5.0

## ESP32-S3 Peripherals
- 20 Analog-to-Digital Converter (ADC) channels (12-bit)
- 4 SPI interfaces (SPI0/1 internal, SPI2/HSPI and SPI3/VSPI available)
- 3 UART interfaces
- 2 I2C interfaces (can be mapped to any GPIO)
- 8 PWM channels (LED PWM controller)
- 2 I2S interfaces
- 14 Capacitive touch GPIOs (T1-T14 on GPIO 1-14)
- 22 RTC GPIOs (GPIO 0-21)

## Key Features for This Project
- Dual-core processing for concurrent tasks
- WiFi AP mode for configuration portal (Master only)
- Multiple I2C, SPI, UART interfaces
- Hardware PWM for ESC control
- Low power modes for battery operation
- 45 GPIO pins total on chip (not all exposed on DevKitC-1)

## GPIO Restrictions and Reserved Pins

### ⚠️ CRITICAL: Reserved for Flash/PSRAM (DO NOT USE)
GPIOs 26-32 are connected to integrated SPI flash and PSRAM:
- **GPIO 26** - Flash/PSRAM SPICS1
- **GPIO 27** - Flash/PSRAM SPIHD
- **GPIO 28** - Flash/PSRAM SPIWP
- **GPIO 29** - Flash/PSRAM SPICS0
- **GPIO 30** - Flash/PSRAM SPICLK
- **GPIO 31** - Flash/PSRAM SPIQ
- **GPIO 32** - Flash/PSRAM SPID

**These pins are NOT exposed on most DevKitC-1 boards and must not be used.**

### ⚠️ Strapping Pins (Avoid in Design)
These pins affect boot mode and should be avoided:
- **GPIO 0** - Boot mode (pulled up, connect to GND for flash mode)
- **GPIO 3** - JTAG control
- **GPIO 45** - VDD_SPI voltage
- **GPIO 46** - ROM message printing

### ⚠️ PSRAM Conflict Pins (N8R8 Variant Only)
If using N8R8 variant with 8MB PSRAM, GPIOs 35-37 may have conflicts:
- **GPIO 35** - May conflict with OPI PSRAM
- **GPIO 36** - May conflict with OPI PSRAM  
- **GPIO 37** - May conflict with OPI PSRAM

**Safe to use on N8 variant (no PSRAM), verify if using N8R8.**

### UART0 (USB/Serial - Cannot Change)
- **GPIO 43** - UART0 TX (USB bridge)
- **GPIO 44** - UART0 RX (USB bridge)

**These are hardwired to USB-to-UART bridge for programming.**

## Recommended Pin Configuration for This Project

### Communication Interfaces

#### Default I2C (OLED Display + Compass)
- **SDA:** GPIO 8 (default Arduino I2C SDA)
- **SCL:** GPIO 9 (default Arduino I2C SCL)

Note: I2C can be remapped to any GPIO using Wire.setPins(sda, scl) before Wire.begin()

#### HSPI (LoRa RFM95W) - SPI2 Bus
- **MOSI:** GPIO 11 (HSPI default)
- **MISO:** GPIO 13 (HSPI default)
- **SCK:** GPIO 12 (HSPI default)
- **CS:** GPIO 10 (HSPI default)
- **RST:** GPIO 14 (custom, safe to use)
- **IRQ (DIO0):** GPIO 21 (custom, RTC GPIO, safe to use)

Alternative VSPI (SPI3) if HSPI conflicts:
- **MOSI:** GPIO 35 (VSPI, check PSRAM variant)
- **MISO:** GPIO 37 (VSPI, check PSRAM variant)
- **SCK:** GPIO 36 (VSPI, check PSRAM variant)
- **CS:** GPIO 39 (VSPI, custom)

#### UART1 (GPS BE-880)
- **TX:** GPIO 17 (UART1 default, can be remapped)
- **RX:** GPIO 18 (UART1 default, can be remapped)

### PWM Outputs (Thrusters)
Any GPIO can be PWM, recommend these safe pins:
- **Left Thruster:** GPIO 47 (safe, not strapping pin)
- **Right Thruster:** GPIO 48 (safe, onboard RGB LED but can be used)

Alternative PWM pins: GPIO 1-7, 15-16, 19-20, 38-42

**PWM Config:** 100 Hz, 16-bit resolution, use ledc peripheral

### Analog Inputs (ADC)

#### Battery Monitor
- **GPIO 1-10:** ADC1_CH0 to ADC1_CH9 (12-bit, 0-3.3V)
- **GPIO 11-20:** ADC2_CH0 to ADC2_CH9 (12-bit, 0-3.3V)

**Recommended:** GPIO 4 (ADC1_CH3) for battery monitor
- Voltage divider for 4S battery (16.8V max)
- Divider ratio: 5:1 (3.3V ADC max)

**Note:** ADC2 may conflict with WiFi usage. Prefer ADC1 channels (GPIO 1-10).

#### Master Buoy: Wind Direction (Analog)
**Recommended:** GPIO 5 (ADC1_CH4) for anemometer wind direction

### Digital I/O

#### Status LEDs
- **Green LED:** GPIO 38 (safe, available)
- **Red LED:** GPIO 39 (safe, available)

Alternative: GPIO 40-42 are also safe choices

#### Master Buoy: Wind Speed (Pulse Counter)
- **Wind Speed Input:** GPIO 6 (RTC GPIO, interrupt capable, safe)

#### Remote Control Interface (Master Only - Optional)
- **Start Race Button:** GPIO 7 (RTC GPIO, safe)
- **Override Button:** GPIO 15 (safe, available)
- **End Race Button:** GPIO 16 (safe, available)

Use internal pull-ups, active LOW

### Capacitive Touch GPIOs
Touch sensors on GPIO 1-14 (T1-T14). Can still be used as regular GPIO.

### RTC GPIOs (Deep Sleep Wake)
GPIO 0-21 can wake ESP32 from deep sleep via ULP coprocessor.

## Pin Configuration Summary Table

| Function | GPIO | ADC/Touch | Notes |
|----------|------|-----------|-------|
| I2C SDA | 8 | - | Default, can remap |
| I2C SCL | 9 | - | Default, can remap |
| SPI MOSI | 11 | ADC2_CH0 | HSPI default |
| SPI MISO | 13 | ADC2_CH2 | HSPI default |
| SPI SCK | 12 | ADC2_CH1 | HSPI default |
| SPI CS | 10 | ADC1_CH9 | HSPI default |
| LoRa RST | 14 | ADC2_CH3, T13 | Custom |
| LoRa IRQ | 21 | - | RTC GPIO |
| UART1 TX | 17 | ADC2_CH6 | GPS, can remap |
| UART1 RX | 18 | ADC2_CH7 | GPS, can remap |
| Battery Mon | 4 | ADC1_CH3, T4 | Via voltage divider |
| Wind Direction | 5 | ADC1_CH4, T5 | Master only, analog |
| Wind Speed | 6 | ADC1_CH5, T6 | Master only, pulse |
| Left Thruster | 47 | - | PWM output |
| Right Thruster | 48 | - | PWM, RGB LED onboard |
| Green LED | 38 | - | Status indicator |
| Red LED | 39 | - | Status indicator |
| Start Button | 7 | ADC1_CH6, T7 | Master only, optional |
| Override Button | 15 | ADC2_CH4, T14 | Master only, optional |
| End Button | 16 | ADC2_CH5 | Master only, optional |

## Power Requirements
- **Operating Voltage:** 3.3V (regulated on board)
- **Input Voltage:** 5V via USB or 5V pin
- **Current Draw:** ~80-120mA (WiFi active), ~20mA (deep sleep)

## Power Supply Design
- 5V Buck converter from 4S battery (14.8-16.8V)
- 1.5A rated converter minimum
- 1000µF capacitor on 5V rail for inductive load protection
- Feed ESP32 via 5V pin (NOT USB during operation)

## Programming
- **Interface:** USB-C (built-in USB-to-UART on GPIO 43/44)
- **Framework:** Arduino or ESP-IDF
- **Upload Speed:** 921600 baud
- **Board Selection:** ESP32-S3-DevKitC-1 (8MB Flash, 0MB/8MB PSRAM depending on variant)

## Memory Considerations
- Use PSRAM for large buffers if available (wind history, GPS logs)
- Flash for firmware and config storage (8MB)
- Preferences library for persistent settings (NVS partition)

## Arduino IDE Configuration

### Default I2C
```cpp
Wire.begin();
// Uses GPIO 8 (SDA) and GPIO 9 (SCL) by default
```

### Custom I2C Pins
```cpp
Wire.setPins(sda_pin, scl_pin);
Wire.begin();
```

### SPI Configuration
```cpp
SPI.begin(sck, miso, mosi, cs);
```

### PWM (LEDC) Configuration
```cpp
ledcSetup(channel, freq, resolution);
ledcAttachPin(gpio, channel);
ledcWrite(channel, dutyCycle);
```

## Datasheet Location
- Store official datasheet in: `hardware/datasheets/ESP32-S3-DevKitC-1.pdf`
- Espressif documentation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/
- Pinout reference: https://randomnerdtutorials.com/esp32-s3-devkitc-pinout-guide/
- ESPBoards reference: https://www.espboards.dev/esp32/esp32-s3-devkitc-1/

## Important Notes
- ⚠️ **NEVER use GPIOs 26-32** - reserved for internal flash/PSRAM
- ⚠️ **Avoid strapping pins** (GPIO 0, 3, 45, 46) in your design
- ⚠️ **Check PSRAM variant** before using GPIO 35-37
- ⚠️ **ADC2 (GPIO 11-20)** may not work when WiFi is active - use ADC1 (GPIO 1-10) for critical sensors
- All GPIO pins are 3.3V logic - protect from 5V signals
- Use external pull-ups on I2C if needed (2.2kΩ to 4.7kΩ typical)
- LoRa antenna must be connected before power-on
- GPIO 48 has onboard RGB LED (WS2812) - can still be used for other purposes
