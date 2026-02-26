# Adafruit RFM95W LoRa Radio Module

## Overview
- **Chip:** Semtech SX1276
- **Frequency:** 915 MHz (North America)
- **Modulation:** LoRa (Long Range)
- **Interface:** SPI
- **Operating Voltage:** 3.3V
- **Range:** Up to 2km line-of-sight (target: 500m for this project)

## Key Features
- Low power consumption
- High sensitivity: -148 dBm
- Programmable output power: +5 to +23 dBm
- Integrated packet engine
- CRC error detection
- Automatic frequency correction

## SPI Interface (ESP32-S3-DevKitC-1 — buoys)
- **MOSI:** Connect to ESP32-S3 GPIO 11
- **MISO:** Connect to ESP32-S3 GPIO 13
- **SCK:** Connect to ESP32-S3 GPIO 12
- **CS (Chip Select):** Connect to ESP32-S3 GPIO 10
- **RST (Reset):** Connect to ESP32-S3 GPIO 14
- **G0/IRQ (DIO0):** Connect to ESP32-S3 GPIO 21 (interrupt for RX/TX done)

## SPI Interface (NodeMCU-32S — remote control)
- **MOSI:** Connect to NodeMCU-32S GPIO 23
- **MISO:** Connect to NodeMCU-32S GPIO 19
- **SCK:** Connect to NodeMCU-32S GPIO 18
- **CS (Chip Select):** Connect to NodeMCU-32S GPIO 5
- **RST (Reset):** Connect to NodeMCU-32S GPIO 14
- **G0/IRQ (DIO0):** Connect to NodeMCU-32S GPIO 4

## Power Requirements
- **Supply Voltage:** 3.3V (from ESP32 3.3V rail)
- **Transmit Current:** ~120mA @ +20dBm
- **Receive Current:** ~10-12mA
- **Sleep Current:** <1µA

## Configuration Settings

### Frequency
```
915 MHz (US ISM band)
```

### LoRa Parameters
- **Bandwidth:** 125 kHz (balance of range and data rate)
- **Spreading Factor:** SF7 (chosen for speed — adequate range for 500m open water)
- **Coding Rate:** 4/5 (error correction)
- **Preamble Length:** 8 symbols
- **Sync Word:** 0x12 (private network)

### Output Power
- **Normal Operation:** +17 dBm (50mW)
- **Long Range (if needed):** +20 dBm (100mW)

## Communication Protocol

### Star Topology
- **Master:** Broadcasts ASSIGN to all slaves; sends MASTER_STATUS to RC units
- **Slaves:** Reply with ACK_ASSIGN then periodic STATUS
- **RC units:** Send race commands (RC_START/STOP/RTH); receive MASTER_STATUS

### Packet Types
All 7 packet types and their structs are defined in `common/protocol.h`.
CRC16-CCITT checksums on all packets.

| Packet | Hex | Direction | Notes |
|--------|-----|-----------|-------|
| ASSIGN | 0xA5 | Master → Slave | Target GPS + hold radius |
| ACK_ASSIGN | 0xAA | Slave → Master | Confirms receipt |
| STATUS | 0x5A | Slave → Master | Position, battery, error flags |
| PING_STATUS | 0x55 | Any | Heartbeat |
| PKT_RC_START | 0xB1 | RC → Master | Initiate race |
| PKT_RC_STOP | 0xB2 | RC → Master | Cancel / abort |
| PKT_RC_RTH | 0xB3 | RC → Master | Recall fleet to home |
| PKT_MASTER_STATUS | 0xC1 | Master → RC | Fleet state + fault flags |

### Buoy IDs (`common/protocol.h`)
- `BUOY_MASTER` = 0, `BUOY_START_A` = 1, `BUOY_START_B` = 2
- `BUOY_WINDWARD` = 3, `BUOY_LEEWARD` = 4, `BUOY_REMOTE` = 5

## Timing

### Master Broadcast Interval
- **Repositioning Mode:** Every 5 seconds
- **Ready/Locked:** Every 30 seconds (keepalive)

### Slave Reply Timing
Deterministic stagger to avoid collisions (not random):
```
delay = REPLY_DELAY_BASE_MS + (buoy_id × REPLY_DELAY_PER_ID_MS)
```
Values defined in `common/protocol.h`.

### Failsafe Timeout
- If no ASSIGN received for 60 seconds (`COMMS_TIMEOUT_MS`): Enter RTH mode

## Arduino Library
- Use: `RadioHead` library (RH_RF95 class)
- Alternative: `LoRa` library by Sandeep Mistry

## Antenna
- **Type:** Wire antenna (1/4 wave = 8.2cm for 915 MHz)
- **Connector:** uFL or SMA (depending on breakout)
- **Impedance:** 50Ω

## Wiring Notes
- Keep antenna wire away from metal/electronics
- Use short SPI wires (<10cm if possible)
- Add 100nF bypass capacitor close to VCC pin
- Pull CS high when not in use (handled by library)

## Testing Checklist
- [ ] Verify SPI communication (read version register)
- [ ] Test broadcast from Master
- [ ] Verify slave reception and reply
- [ ] Measure range in open field (target: 500m)
- [ ] Test with obstacles (buoys floating, low antenna height)
- [ ] Validate packet loss rate (<5% target)

## Datasheet Location
- Store in: `hardware/datasheets/RFM95W_datasheet.pdf`
- Adafruit guide: https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts

## Notes
- LoRa performance degrades close to water surface (antenna height important)
- Use spread spectrum for interference resistance
- CRC checking is essential for GPS coordinate integrity
- Consider adding sequence numbers to detect missed packets
