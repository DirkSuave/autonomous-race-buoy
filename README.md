# Autonomous Race Buoy System

_Project started: 2026-02-15 — Last updated: 2026-02-25_

## Overview

Autonomous buoy system for sailboat racing. Four ESP32-S3 buoys use GPS navigation,
LoRa radio communication, and differential-thrust bilge thrusters to hold racing course
positions relative to wind direction. A handheld waterproof remote control manages race
start sequences. The master buoy fires a 12V marine horn for the IRSA start sequence.

## Quick Links

- [Project Summary](docs/project-summary.md) — Full system specification
- [Firmware Architecture](docs/firmware-architecture.md) — Module breakdown and state machines
- [Hardware Components](hardware/components.md) — BOM and component status
- [Hardware README](hardware/README.md) — Power budgets, assembly notes, test checklist
- [Test Plan](testing/TEST-PLAN.md) — 9-module hardware validation plan
- [Master Pinout](hardware/pinouts/master-pinout.md) — ESP32-S3 pin assignments (master)
- [Slave Pinout](hardware/pinouts/slave-pinout.md) — ESP32-S3 pin assignments (slaves)
- [Remote Pinout](hardware/pinouts/remote-pinout.md) — NodeMCU-32S RC unit pin assignments

## System Components

| Unit | MCU | Qty | Role |
|------|-----|-----|------|
| Master Buoy (Wind Master) | ESP32-S3-DevKitC-1 | 1 | Wind monitoring, race control, horn |
| Pin Buoy | ESP32-S3-DevKitC-1 | 1 | Start line mark |
| Windward Buoy | ESP32-S3-DevKitC-1 | 1 | Upwind mark |
| Leeward Buoy | ESP32-S3-DevKitC-1 | 1 | Downwind mark |
| Remote Control | NodeMCU-32S | 2 | Race start commands, fleet status |

## Hardware Platform

**Buoys (ESP32-S3-DevKitC-1):**
- Adafruit RFM95W LoRa 915 MHz (SF7, 125 kHz BW) — RadioHead RH_RF95
- BE-880 GPS/Compass — UART NMEA at **115200 baud** + I2C QMC5883L
- SSD1306 0.96" OLED (I2C)
- 3× AJ-SR04M waterproof ultrasonic sensors (collision avoidance, 150° forward arc)
- Bilge thruster ESCs × 2 (LEDC PWM, GPIO 47/48) — hardware under procurement
- 4S LiPo → 5V buck converter; battery monitor via ADC (GPIO 4)
- Davis Vantage Pro anemometer (master only — GPIO 5/6)
- 12V marine horn via IRLZ44N MOSFET (master only — GPIO 7)

**Remote Control (NodeMCU-32S):**
- Adafruit RFM95W LoRa 915 MHz (VSPI)
- 4 LEDs (blue/green/red/white) + buzzer + 2 waterproof buttons
- IP67/IP68 waterproof enclosure — capsize rated
- 500–1000 mAh LiPo via TP4056 USB-C charger

## Current Status

| Area | Status |
|------|--------|
| Hardware specification | Complete |
| Pin assignments (`common/config.h`) | Complete — verified on hardware |
| Communication protocol (`common/protocol.h`) | Complete |
| GPS + OLED test (Module 2) | Passing |
| LoRa TX/RX test (Modules 4 & 5) | Passing |
| Wind sensor test (Module 6) | Passing |
| Collision avoidance test (Module 9) | Sketch complete — ready to flash |
| OLED standalone (Module 1) | TODO |
| Compass test (Module 3) | Passing |
| Battery monitor (Module 7) | TODO |
| ESC / thrusters (Module 8) | Pending hardware |
| Firmware (master, slave, remote) | Not started |

## Directory Structure

```
autonomous-race-buoy/
├── common/            # Shared headers (config.h, protocol.h, protocol.cpp)
├── docs/              # Project documentation and specifications
├── firmware/          # Firmware (currently empty — planned)
│   ├── master/        # Master buoy (ESP32-S3)
│   ├── slave/         # Slave buoys (ESP32-S3)
│   ├── remote/        # Remote control (NodeMCU-32S)
│   └── common/        # Shared runtime libraries
├── hardware/          # Pinouts, specs, BOM, schematics
│   ├── pinouts/       # master-pinout.md, slave-pinout.md, remote-pinout.md
│   └── specs/         # Per-component spec files
├── testing/           # Hardware validation sketches (PlatformIO)
└── notes/             # Development notes
```

## Building a Test Sketch

```bash
# Build
pio run -e <env>

# Upload + monitor
pio run -e <env> -t upload && pio device monitor -e <env>
```

Available environments: `gps_test`, `compass_test`, `lora_tx`, `lora_rx`,
`wind_sensor`, `ultrasonic_test`
