#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "info.h"

// AeroScout OUI for detection highlight
static const uint8_t AEROSCOUT_OUI[3] = {0x48, 0x02, 0x01};

static int lock = 0;

static void acquire_lock(void) {
  while (lock == 1) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  lock = 1;
}

static void release_lock(void) { lock = 0; }

// Added payload and payload_len parameters
void update_info(info *info_to_add, uint8_t *mac, size_t mac_len, char *name,
                 size_t name_len, int rssi, source src, uint8_t *payload,
                 size_t payload_len) {
  acquire_lock();

  for (int i = 0; i < INFO_ARRAY_LENGTH; i++) {

    if (info_to_add[i].mac[0] == 0 ||
        memcmp(info_to_add[i].mac, mac, mac_len) == 0) {
      // Copy MAC
      memcpy(info_to_add[i].mac, mac, mac_len);
      info_to_add[i].mac_len = mac_len;

      // Copy name (guard against overflow)
      size_t safe_name = name_len < NAME_LENGTH ? name_len : NAME_LENGTH - 1;
      memcpy(info_to_add[i].name, name, safe_name);
      info_to_add[i].name[safe_name] = '\0';
      info_to_add[i].name_len = safe_name;

      info_to_add[i].rssi = rssi;
      info_to_add[i].source = src;

      // Copy the raw packet (guard against overflow based on a max buffer)
      if (payload != NULL && payload_len > 0) {
        size_t safe_payload_len =
            payload_len < RAW_PACKET_MAX_LEN ? payload_len : RAW_PACKET_MAX_LEN;
        memcpy(info_to_add[i].raw_packet, payload, safe_payload_len);
        info_to_add[i].packet_len = safe_payload_len;
      } else {
        info_to_add[i].packet_len = 0;
      }

      break;
    }
  }

  release_lock();
}

void clear_info(info *info_to_clear) {
  memset(info_to_clear, 0, sizeof(info) * INFO_ARRAY_LENGTH);
}

static void print_info(const info *entry) {
  bool is_aeroscout = (memcmp(entry->mac, AEROSCOUT_OUI, 3) == 0);

  printf("WIFI  | MAC: %02x:%02x:%02x:%02x:%02x:%02x | RSSI: %3d dBm | SSID: "
         "%s%s\r\n",
         entry->mac[0], entry->mac[1], entry->mac[2], entry->mac[3],
         entry->mac[4], entry->mac[5], entry->rssi, entry->name,
         is_aeroscout ? "  <-- AeroScout Tag" : "");

  if (entry->packet_len > 0) {
    printf("      | Raw Packet Dump (%u bytes):\r\n      | ",
           entry->packet_len);
    for (size_t i = 0; i < entry->packet_len; i++) {
      printf("%02X ", entry->raw_packet[i]);
      // Break into lines of 16 bytes for readability
      if ((i + 1) % 16 == 0 && (i + 1) != entry->packet_len) {
        printf("\r\n      | ");
      }
    }
    printf("\r\n\r\n");
  }
}

void send_info(info *info_to_send) {
  acquire_lock();

  for (int i = 0; i < INFO_ARRAY_LENGTH; i++) {
    if (info_to_send[i].mac[0] == 0) {
      break;
    }
    print_info(&info_to_send[i]);
  }

  clear_info(info_to_send);

  release_lock();
}
