# Remote Control Unit — Pin Assignments

## Hardware

- **MCU:** NodeMCU-32S (ESP32-WROOM-32, original ESP32)
- **Radio:** Adafruit RFM95W LoRa 915 MHz (same module as buoys)
- **Enclosure:** IP67/IP68 waterproof case — unit is carried on a racing dinghy that may capsize
- **Power:** Small LiPo (500–1000 mAh) via TP4056 charger module; USB-C port with rubber weatherproof plug
- **Quantity:** 2 identical units built (same firmware, same BUOY_REMOTE ID)

> ⚠️ The NodeMCU-32S uses the **original ESP32**, not the ESP32-S3. Pin assignments
> differ from the buoy boards. Do not use the buoy pinout for this board.

---

## Pin Assignments

### LoRa (SPI — VSPI defaults on ESP32)

| Signal    | GPIO | Notes |
|-----------|------|-------|
| MOSI      | 23   | VSPI default |
| MISO      | 19   | VSPI default |
| SCK       | 18   | VSPI default |
| CS        | 5    | VSPI default |
| RST       | 14   | |
| IRQ (DIO0)| 4    | Interrupt for RX/TX done |

### User Interface

| Component        | GPIO | Notes |
|-----------------|------|-------|
| LED_WIND (amber) | 26   | Wind unstable / repositioning |
| LED_READY (green)| 25   | All buoys in position, safe to start |
| LED_FAULT (red)  | 33   | Any buoy fault or error condition |
| BTN_START        | 32   | Large waterproof momentary button |
| BTN_STOP         | 34   | Large waterproof momentary button (input-only pin) |
| BUZZER           | 27   | Audible confirmation when command accepted |

### Power Monitoring

| Signal       | GPIO | Notes |
|-------------|------|-------|
| BATTERY_ADC  | 35   | ADC1_CH7, input-only. 11:1 voltage divider from LiPo |

---

## LED Status Meanings

| State | Amber | Green | Red |
|-------|-------|-------|-----|
| Repositioning (wind unstable) | ON | off | off |
| Ready (safe to start) | off | ON | off |
| Fault (any buoy error) | off | off | ON |
| Master comms lost | flash | flash | flash |
| Race active (LOCKED) | off | slow flash | off |

---

## Pins to Avoid (NodeMCU-32S / ESP32-WROOM-32)

| GPIO | Reason |
|------|--------|
| 0, 2, 15 | Strapping pins — affect boot mode |
| 1, 3 | UART0 / USB programming interface |
| 6–11 | Internal SPI flash — hardwired, never usable |
| 12 | Strapping pin — boot voltage selection |

---

## Enclosure & Waterproofing Notes

- Use IP67 or IP68 rated enclosure (e.g. Hammond 1554, Bopla Bocube, or similar)
- LED holders with O-ring seal through enclosure wall
- IP67-rated momentary push buttons (metal body recommended for durability)
- USB-C charging port with silicone rubber dust/water plug
- Antenna exits through SMA bulkhead with sealing washer
- TP4056 LiPo charger board mounted internally with thermal relief
- Apply conformal coating to PCB and NodeMCU-32S after assembly

---

## Communication

The remote control communicates with the **master buoy only** via LoRa using:
- `PKT_RC_START` (0xB1): Transition master from READY → LOCKED (race start)
- `PKT_RC_STOP` (0xB2): Return master from LOCKED → REPOSITIONING (abort/end race)

Both units share `BUOY_REMOTE` (ID = 5). The master accepts commands from either unit without distinction. Packets use CRC16-CCITT checksum matching `common/protocol.h`.
