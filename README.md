# Autonomous Race Buoy System

_Project started: 2026-02-15_

## Overview
An autonomous buoy system for sailboat races that uses GPS positioning, wind monitoring, and LoRa communication to automatically set up and maintain race courses.

## Quick Links
- [Project Summary](docs/project-summary.md) - Complete project specifications
- [Hardware Specs](hardware/components.md) - Hardware list and specifications
- [Development Notes](notes/) - Working notes and progress logs

## Directory Structure

```
autonomous-race-buoy/
├── docs/              # Project documentation and specifications
├── firmware/          # ESP32 firmware code
│   ├── master/        # Master buoy (Wind Master) code
│   ├── slave/         # Slave buoy code
│   └── common/        # Shared libraries and utilities
├── hardware/          # Hardware specifications and schematics
├── testing/           # Test plans and results
└── notes/             # Development notes and session logs
```

## System Components
- **Master Buoy**: Wind monitoring and race control (fixed position)
- **Pin Buoy**: Part of the start line, positioned perpendicular to wind direction
- **Windward Buoy**: Upwind mark, perpendicular to start line
- **Leeward Buoy**: Downwind mark, perpendicular to start line (opposite end from Windward)
- **Remote Control**: Handheld unit for race start commands and buoy status monitoring (features TBD)

All buoys are designed to face into the wind direction to maintain position and course alignment.

## Hardware Platform
- ESP32-S3-DevKitC-1
- RFM95W LoRa (915 MHz)
- BE-880 GPS/Compass
- Davis Vantage Pro Anemometer (Master only)
- Propulsion system: TBD (brushless thrusters and ESCs under evaluation)

## Current Status
Project setup phase - organizing structure and specifications

## Next Steps
See [notes/development-log.md](notes/development-log.md) for current tasks and progress
