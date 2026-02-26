# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Autonomous buoy system for sailboat racing. Buoys use GPS navigation, LoRa radio communication, and differential-thrust propulsion to hold racing course positions relative to wind direction. Hardware target is **ESP32-S3-DevKitC-1**.

## Development Environment

**Build system: VS Code + PlatformIO** (not Arduino IDE). Test sketches in `testing/` are PlatformIO `.cpp` files, each in their own subdirectory with a `[env:name]` entry in the root `platformio.ini`. Firmware development targets `esp32s3dev`.

To develop with Arduino IDE or PlatformIO, the `common/` directory contains shared headers (`config.h`, `protocol.h`, `protocol.cpp`) that must be available to all firmware sketches.

## Critical: ESP32-S3 Pin Assignments

**`common/config.h` has been corrected for ESP32-S3** and is the authoritative source for pin assignments. Cross-reference with `hardware/pinouts/` if in doubt. Key pins (all verified correct):

| Interface | GPIO |
|-----------|------|
| I2C (OLED + Compass) | SDA=8, SCL=9 |
| SPI (LoRa) | MOSI=11, MISO=13, SCK=12, CS=10, RST=14, IRQ=21 |
| GPS UART1 | RX=18 (GPS TX), TX=17 (GPS RX), **115200 baud** |
| Motor PWM | Left=47, Right=48 |
| Battery ADC | GPIO 4 (ADC1) |
| Wind Direction/Speed | GPIO 5/6 (ADC1, master only) |
| Ultrasonic (3× AJ-SR04M) | TRIG: 15/16/19 — ECHO: 20/22/23 |

**Never use GPIO 26–32** (flash/PSRAM), **GPIO 0, 3, 45, 46** (strapping), or **GPIO 43/44** (USB/UART0).

**ADC2 (GPIO 11–20) cannot be used while WiFi is active.** Use ADC1 (GPIO 1–10) for analog inputs.

## Architecture

### Buoy Types

**Master Buoy ("Wind Master")**
- Fixed anchor position set at power-up
- Davis Vantage Pro anemometer: monitors wind direction/speed
- Broadcasts target coordinates to slave buoys via LoRa
- Race state machine: `Repositioning → Stable → Ready → Locked`
- Hosts WiFi AP for initial configuration

**Slave Buoys** (Pin, Windward, Leeward)
- Navigate to GPS coordinates assigned by master
- Hold position within configurable radius (default 3m, 6m in heavy wind)
- Return-to-home failsafe on LoRa signal loss (60s timeout — `COMMS_TIMEOUT_MS` in `protocol.h`)
- No wind sensor; driven purely by master-assigned coordinates

### Communication Protocol (`common/protocol.h`)

Star topology LoRa network at 915 MHz (SF7, 125 kHz BW, CR4/5):
- `ASSIGN` packet: master → slave target GPS coordinates
- `ACK_ASSIGN`: slave → master confirmation
- `STATUS` packet: slave → master position/battery/error telemetry
- `PING_STATUS`: heartbeat

Buoy state enum: `INIT → DEPLOY → HOLD → ADJUST → RECOVER → FAILSAFE`

Error flags: `GPS_LOST`, `COMPASS_FAIL`, `LOW_BATTERY`, `MOTOR_FAIL`, `COMMS_LOST`, `WIND_FAIL`

Checksum: CRC16-CCITT over packet bytes (declared in `protocol.h`, implemented in `protocol.cpp`).

### Race State Machine (Master)

- **Repositioning:** Wind shifted >15° in 60s rolling window — master continuously recalculates and broadcasts new slave positions
- **Stable:** Wind stable ≥60s — master signals Ready (green LED)
- **Ready:** Awaiting race start from remote control unit
- **Locked:** Race active — positions frozen regardless of wind changes

### Firmware Directory Structure (planned, currently empty)

```
firmware/
  common/    # Shared libs: GPS utilities, LoRa wrapper, OLED driver
  master/    # Master buoy firmware
  slave/     # Slave buoy firmware
```

## Key Hardware

**Buoys (master + 3 slaves) — ESP32-S3-DevKitC-1:**
- **MCU:** ESP32-S3-DevKitC-1 (dual-core Xtensa, WiFi/BT, USB OTG)
- **Radio:** Adafruit RFM95W LoRa 915 MHz (RadioHead `RH_RF95`)
- **GPS/Compass:** BE-880 module (UART NMEA + I2C QMC5883) at **115200 baud** (reconfigured from factory 9600 — hardware-verified)
- **Anemometer:** Davis Vantage Pro (master only — pulse speed + analog 0–3.3V direction)
- **Display:** SSD1306 0.96" I2C OLED (128×64)
- **Power:** 4S LiPo → 5V buck converter; monitor via ADC with 11:1 voltage divider

**Remote Control (2 identical waterproof units) — NodeMCU-32S:**
- **MCU:** NodeMCU-32S (ESP32-WROOM-32, original ESP32 — different pin defaults than S3)
- **Radio:** Adafruit RFM95W LoRa 915 MHz (VSPI: MOSI=23, MISO=19, SCK=18, CS=5, IRQ=4)
- **Interface:** 4 LEDs (blue/green/red/white) + buzzer + 2 waterproof buttons — no display
- **Enclosure:** IP67/IP68 waterproof — unit carried on racing dinghy, capsize exposure
- **Pinout:** `hardware/pinouts/remote-pinout.md`

## Reference Documentation

- `docs/project-summary.md` — full system specification
- `docs/firmware-architecture.md` — planned firmware module breakdown
- `hardware/ESP32-S3-MIGRATION-SUMMARY.md` — **read before touching pin assignments**
- `hardware/pinouts/master-pinout.md` / `slave-pinout.md` — authoritative pin tables
- `hardware/components.md` — BOM and component status
- `testing/` — working hardware validation sketches for GPS, LoRa TX/RX, wind sensor
