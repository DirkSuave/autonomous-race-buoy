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

| Component         | GPIO | Notes |
|------------------|------|-------|
| LED_BLUE          | 26   | Repositioning (buoys navigating to targets) |
| LED_GREEN         | 25   | Race ready / race in progress |
| LED_RED           | 33   | Fault or comms lost |
| LED_WHITE         | 13   | Race in progress (LOCKED) / RTH in progress |
| BTN_START         | 32   | Large waterproof momentary button |
| BTN_STOP          | 34   | Large waterproof momentary button (input-only pin) |
| BUZZER            | 27   | Button confirms, drift alerts, fault alerts, RTH confirm |

> **Note:** LED set changed from 3 LEDs (amber/green/red) to 4 LEDs (blue/green/red/white).
> Blue replaces amber on GPIO 26 (same pin, swap LED component).
> White is new on GPIO 13 (safe GPIO on NodeMCU-32S — not used by VSPI here).
> The RC buzzer is NOT used for the race start horn sequence (horn is on master buoy).

### Power Monitoring

| Signal       | GPIO | Notes |
|-------------|------|-------|
| BATTERY_ADC  | 35   | ADC1_CH7, input-only. 11:1 voltage divider from LiPo |

---

## LED Status Meanings

| System State | Red | Green | Blue | White |
|-------------|-----|-------|------|-------|
| Repositioning (buoys navigating) | off | off | fast flash | off |
| Race Ready (all on-station) | off | steady | off | off |
| Countdown (start sequence) | alt. flash | alt. flash | off | off |
| Race In Progress (LOCKED) | off | off | off | steady |
| Start / gun fires (0:00) | off | off | off | fast flash (brief) |
| Fault (any buoy error) | steady | off | off | off |
| Comms lost (>60s) | fast flash | off | fast flash | off |
| RTH in progress | off | off | off | slow flash |

> Red + Green **alternating** flash during countdown = each LED takes turns, not simultaneous.

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
- `PKT_RC_START` (0xB1): Initiate new race — master repositions buoys, auto-starts countdown when all on-station
- `PKT_RC_STOP` (0xB2): Cancel / abort — return all buoys to Repositioning state
- `PKT_RC_RTH` (0xB3): Recall fleet to shore — master sends each slave its home coordinate (BTN_STOP hold 3s)

The master sends back:
- `PKT_MASTER_STATUS` (0xC1): Aggregate fleet state byte + fault flags — RC uses this to drive LED patterns

Both RC units share `BUOY_REMOTE` (ID = 5). The master accepts commands from either unit without distinction. Packets use CRC16-CCITT checksum matching `common/protocol.h`.

### Button Commands

| Action | Button | Press Pattern |
|--------|--------|---------------|
| Initiate new race | BTN_START | Single press |
| Cancel / abort race | BTN_STOP | Single press |
| Recall fleet to shore (RTH) | BTN_STOP | Hold 3s (buzzer confirms before sending) |

---

## Power Budget (Remote Control Unit)

| Component | Current (mA) | Notes |
|-----------|--------------|-------|
| ESP32 (normal) | 40-80 | WROOM-32 module, no WiFi |
| LoRa TX | 120 | Brief — command packets only |
| LoRa RX | 10-12 | Mostly idle, polling for master status |
| LEDs (active) | 10-20 | Typically 1–2 active at a time |
| Buzzer | 10-30 | Brief alerts only |
| Buttons | <1 | Pull-up resistor current only |
| **Total (typical)** | ~60-100 mA | LoRa RX + 1 LED on |
| **Total (TX burst)** | ~170-220 mA | During packet send (brief) |

**Battery runtime:**

| Battery | Typical runtime |
|---------|----------------|
| 500 mAh LiPo | ~5–8 hours |
| 1000 mAh LiPo | ~10–16 hours |

A 500 mAh cell is sufficient for a full race day. 1000 mAh recommended for multi-day events or cold weather (reduced LiPo capacity).

**Charging:** TP4056 module via USB-C. Full charge from empty: ~1–2 hours (500 mAh), ~2–4 hours (1000 mAh).
