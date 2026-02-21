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

## SPI Interface
- **MOSI:** Connect to ESP32 GPIO 23
- **MISO:** Connect to ESP32 GPIO 19
- **SCK:** Connect to ESP32 GPIO 18
- **CS (Chip Select):** Connect to ESP32 GPIO 5
- **RST (Reset):** Connect to ESP32 GPIO 14
- **G0/IRQ (DIO0):** Connect to ESP32 GPIO 27 (interrupt for RX/TX done)

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
- **Spreading Factor:** 7 or 8 (SF7 = faster, SF8 = longer range)
- **Coding Rate:** 4/5 (error correction)
- **Preamble Length:** 8 symbols
- **Sync Word:** 0x12 (private network)

### Output Power
- **Normal Operation:** +17 dBm (50mW)
- **Long Range (if needed):** +20 dBm (100mW)

## Communication Protocol

### Star Topology
- **Master:** Broadcasts to all slaves
- **Slaves:** Reply individually with STATUS

### Message Structure

#### ASSIGN (Master -> Slaves)
```
Byte 0: 0xA5 (Message type: ASSIGN)
Byte 1: Buoy ID (0=StartA, 1=StartB, 2=Windward, 3=Leeward)
Bytes 2-5: Target Latitude (float, IEEE 754)
Bytes 6-9: Target Longitude (float, IEEE 754)
Byte 10: Hold Radius (uint8, meters)
Bytes 11-12: CRC16
Total: 13 bytes
```

#### STATUS (Slave -> Master)
```
Byte 0: 0x5A (Message type: STATUS)
Byte 1: Buoy ID
Bytes 2-5: Current Latitude (float)
Bytes 6-9: Current Longitude (float)
Bytes 10-11: Distance to Target (uint16, cm)
Byte 12: Battery Voltage (uint8, 0.1V units)
Bytes 13-14: CRC16
Total: 15 bytes
```

## Timing

### Master Broadcast Interval
- **Repositioning Mode:** Every 5 seconds
- **Stable/Ready/Locked:** Every 30 seconds (keepalive)

### Slave Reply Timing
- Wait random delay (0-100ms) after receiving ASSIGN
- Send STATUS immediately after processing

### Failsafe Timeout
- If no ASSIGN received for 60 seconds: Enter RTH mode

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
