# AeroScout Wi-Fi Sniffer for ESP32-C6

An ESP-IDF based Wi-Fi sniffer that detects and parses raw 802.11 frames from **AeroScout Wi-Fi Tags**. It operates in promiscuous mode, filters packets by the AeroScout OUI (`0x01:0x0C:0xCC`), and forwards structured JSON payloads over UART — as well as printing a hex dump to the serial console.

## Hardware

- **Target:** ESP32-C6 (tested on ESP32-C6-WROOM)
- **UART TX:** GPIO 4
- **UART RX:** GPIO 5 (used for channel-change commands)
- **Baud rate:** 115200

## Prerequisites

- [ESP-IDF v6.0.0](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/index.html) installed and sourced

## Getting Started

```bash
# 1. Clone the repository
git clone <repo-url>
cd wifi_ble_snif

# 2. Set the target
idf.py set-target esp32c6

# 3. (Optional) Configure via menuconfig
idf.py menuconfig

# 4. Build, flash, and monitor
idf.py build flash monitor
```

## Configuration

| Setting | Location | Default |
|---|---|---|
| Wi-Fi channel | `main/wifi.c` → `esp_wifi_set_channel(...)` | 6 |
| UART port | `main/uart.c` → `UART_PORT` | UART_NUM_1 |
| UART TX pin | `main/uart.c` → `UART_TX_PIN` | GPIO 4 |
| UART RX pin | `main/uart.c` → `UART_RX_PIN` | GPIO 5 |
| Packet queue depth | `main/main.c` → `POST_QUEUE_DEPTH` | 20 |
| Max payload size | `main/info.h` → `MAX_QUEUED_PAYLOAD` | 256 bytes |

## How It Works

1. `app_main` initialises NVS flash, creates a FreeRTOS packet queue, then starts the UART and Wi-Fi subsystems.
2. The Wi-Fi driver is put into promiscuous mode on the configured channel. The `sniffer()` callback fires for every `WIFI_PKT_DATA` and `WIFI_PKT_MGMT` frame.
3. Frames are filtered by checking bytes 4–6 of the payload against the AeroScout OUI. Matching packets are heap-allocated and pushed onto the queue.
4. The UART TX task dequeues packets, prints a hex dump to the console via `print_aeroscout_info()`, serialises the payload and RX metadata to JSON using cJSON, and writes it to UART1 followed by a newline.
5. The UART RX task listens for a single raw byte on UART1; receiving one calls `change_wifi_channel()` to switch channels at runtime (valid range: 1–13).

## UART JSON Output Format

Each detected packet produces one newline-delimited JSON object on UART1:

```json
{
  "raw_packets": "0104...",
  "rx_ctrl": {
    "rssi": -65,
    "rate": 11,
    "channel": 6,
    "sig_len": 128
  },
  "espMac": "AABBCCDDEEFF"
}
```

## Switching Channels at Runtime

Send a single raw byte (value 1–13) to UART1 (GPIO 5, 115200 baud) to change the sniffer channel immediately. Invalid values are silently rejected with a warning log.

## Code Structure

```
main/
├── main.c      — Entry point: NVS init, queue creation, starts UART + Wi-Fi
├── wifi.c/.h   — Promiscuous mode setup, sniffer() callback, OUI filtering
├── uart.c/.h   — UART init, JSON serialisation (TX task), channel-change (RX task)
├── info.c/.h   — sniffer_packet_t struct definition, hex-dump printer
└── env.h       — Local environment config (git-ignored, copy from env-example.h)
```

## Notes

- The AeroScout OUI bytes in `wifi.c` are `{0x01, 0x0C, 0xCC}` — verify this matches your tag deployment if no packets appear.
- Only channels 1–13 are accepted by `change_wifi_channel()`; the ESP32-C6 also supports channel 14 in some regions but it is not exposed here.
- `sdkconfig` is committed to the repository to pin the ESP-IDF 6.0.0 build configuration (ESP32-C6, 2 MB flash, DIO mode, 80 MHz).
