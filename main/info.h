#ifndef INFO_H
#define INFO_H

#include "esp_wifi_types_native.h"
#include <stddef.h>
#include <stdint.h>
#define MAC_LENGTH (6)
#define RAW_PACKET_MAX_LEN (256)
#define MAX_QUEUED_PAYLOAD (256)

typedef struct {
  wifi_pkt_rx_ctrl_t rx_ctrl;
  uint16_t len;
  uint8_t payload[MAX_QUEUED_PAYLOAD]; // Or your MAX_QUEUED_PAYLOAD
} sniffer_packet_t;

void print_aeroscout_info(const sniffer_packet_t *packet);

#endif // INFO_H
