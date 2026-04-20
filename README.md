# AeroScout Wi-Fi Sniffer for ESP32-C6

## How to launch
1. Clone the repository:
   ```bash
   git clone
2. Navigate to the project directory:
   ```bash
   cd ESP32_Sniffer 
3. copy and adjust env:
    ```bash
    cp /main/env-example.h /main/env.h
    ```
    Edit the `.env` file to set your desired configuration.
4. Build and flash the firmware to your ESP32-C6 device:
   ```bash
   idf.py set-target esp32c6
   idf.py build flash monitor
   ```

## Overview
This project is an ESP-IDF based Wi-Fi sniffer designed specifically to detect and parse data from **AeroScout Wi-Fi Tags**. It operates in promiscuous mode, scanning for 802.11 management and data frames, and filters them based on the AeroScout Organizationally Unique Identifier (OUI: `48:02:01`). 

When an AeroScout tag is detected, the sniffer logs its MAC address, RSSI (signal strength), and provides a formatted hexadecimal dump of the raw packet payload.

## Hardware Requirement
* **Target Board:** ESP32-C6-WROOM (or compatible ESP32-C6 development board)

## Acknowledgements / Origin
> **Note:** This codebase is a modified clone of an existing Wi-Fi/BLE sniffer repository. The original repository reference is currently unknown and will be updated here once identified. The modifications adapt the code specifically for the ESP32-C6 architecture and focus heavily on AeroScout frame extraction and payload dumping.

## Features
* **ESP32-C6 Wi-Fi Promiscuous Mode:** Captures raw 802.11 frames over the air.
* **Targeted Filtering:** Ignores standard Wi-Fi traffic and isolates packets originating from the AeroScout OUI (`0x48, 0x02, 0x01`).
* **Advanced Frame Parsing:** Safely parses MAC addresses even from complex 4-address 802.11 frames (ToDS=1, FromDS=1).
* **Raw Packet Dumping:** Extracts and formats the raw packet payload (up to 256 bytes) for deeper reverse engineering and data analysis.
* **Thread-Safe Data Handling:** Uses FreeRTOS delays and basic locking to safely update and print device information.

## Default Configuration
* The sniffer is hardcoded to listen on **Channel 6** by default. To change this, modify the `esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);` line in `main/wifi.c`.

## Code Structure
* `main.c`: Initializes NVS flash and starts the Wi-Fi sniffer application.
* `main/wifi.c` & `main/wifi.h`: Contains the promiscuous mode setup and the `sniffer()` callback function which parses 802.11 headers and filters for the AeroScout OUI.
* `main/info.c` & `main/info.h`: Manages the data structures (`info` struct), handles thread-safe updating of detected devices, and formats the terminal output (MAC, RSSI, and payload dumps).

## Prerequisites
* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/index.html) (Espressif IoT Development Framework) installed and configured.

## Build and Flash

1. Set the target to ESP32-C6:
   ```bash
   idf.py set-target esp32c6
2. Build the project:
   ```bash
   idf.py build
3. Flash the firmware to your ESP32-C6 device:
   ```bash
   idf.py -p PORT flash monitor
    ```
Replace `PORT` with the appropriate serial port for your device (e.g., `/dev/ttyUSB0` on Linux or `COM3` on Windows).
