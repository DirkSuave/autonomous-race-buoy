# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Autonomous buoy system for sailboat racing. Buoys use GPS navigation, LoRa radio communication, and differential-thrust propulsion to hold racing course positions relative to wind direction. Hardware target is **ESP32-S3-DevKitC-1**.

## Development Environment

No build system is configured yet. Test sketches in `testing/` are standalone Arduino IDE `.ino` files. Firmware development will use PlatformIO targeting `esp32s3dev`.

To develop with Arduino IDE or PlatformIO, the `common/` directory contains shared headers (`config.h`, `protocol.h`, `protocol.cpp`) that must be available to all firmware sketches.

## Critical: ESP32-S3 Pin Assignments

**`common/config.h` contains OUTDATED pin assignments from the original ESP32.** Always use the correct pins from `hardware/ESP32-S3-MIGRATION-SUMMARY.md` and `hardware/pinouts/`. Key corrections:

| Interface | Config.h (WRONG) | Correct ESP32-S3 |
|-----------|-----------------|-----------------|
| I2C (OLED) | GPIO 21/22 | GPIO 8/9 |
| SPI (LoRa) | GPIO 23/19/18/5 | GPIO 11/13/12/10 |
| Motor PWM | GPIO 25/26/27/33 | GPIO 47/48/... |
| Battery ADC | GPIO 36 | GPIO 4 |
| Wind Direction | GPIO 35 | GPIO 5/6 |
| LoRa IRQ | GPIO 27 | GPIO 21 |

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
- Return-to-home failsafe on LoRa signal loss (30s timeout)
- No wind sensor; driven purely by master-assigned coordinates

### Communication Protocol (`common/protocol.h`)

Star topology LoRa network at 915 MHz (SF7, 125 kHz BW, CR4/5):
- `ASSIGN` packet: master → slave target GPS coordinates
- `ACK_ASSIGN`: slave → master confirmation
- `STATUS` packet: slave → master position/battery/error telemetry
- `PING_STATUS`: heartbeat

Buoy state enum: `INIT → DEPLOY → HOLD → ADJUST → RECOVER → FAILSAFE`

Error flags: `GPS_LOST`, `COMPASS_FAIL`, `LOW_BATTERY`, `MOTOR_FAIL`, `COMMS_LOST`, `WIND_FAIL`

Checksum: simple XOR over packet bytes (implemented in `protocol.cpp`).

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

- **MCU:** ESP32-S3-DevKitC-1 (dual-core Xtensa, WiFi/BT, USB OTG)
- **Radio:** Adafruit RFM95W LoRa 915 MHz (RadioHead `RH_RF95`)
- **GPS/Compass:** BE-880 module (UART NMEA + I2C QMC5883)
- **Anemometer:** Davis Vantage Pro (pulse-count speed + analog 0–3.3V direction)
- **Display:** SSD1306 0.96" I2C OLED (128×64)
- **Power:** 4S LiPo → 5V buck converter; monitor via ADC with 11:1 voltage divider

## Reference Documentation

- `docs/project-summary.md` — full system specification
- `docs/firmware-architecture.md` — planned firmware module breakdown
- `hardware/ESP32-S3-MIGRATION-SUMMARY.md` — **read before touching pin assignments**
- `hardware/pinouts/master-pinout.md` / `slave-pinout.md` — authoritative pin tables
- `hardware/components.md` — BOM and component status
- `testing/` — working hardware validation sketches for GPS, LoRa TX/RX, wind sensor
