#ifndef INFO_H
#define INFO_H

#include <stddef.h>
#include <stdint.h>
#include "esp_wifi_types_native.h"

#define MAC_LENGTH (6)
#define RAW_PACKET_MAX_LEN  (256)
#define MAX_QUEUED_PAYLOAD  (256)

typedef struct {
    int     rssi;
    size_t  packet_len;
    wifi_pkt_rx_ctrl_t rx_ctrl;
    uint8_t raw_packet[RAW_PACKET_MAX_LEN];
} aeroScoutPacket;

void print_aeroscout_info(const aeroScoutPacket *packet);

#endif // INFO_H