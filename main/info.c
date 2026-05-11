#include "info.h"
#include <stdio.h>

void print_aeroscout_info(const aeroScoutPacket *packet) {
  if (packet == NULL)
    return;

  printf("\r\n=== AeroScout Tag Detected ===\r\n");
  printf("RSSI: %3d dBm\r\n", packet->rssi);
  printf("Packet Length: %zu bytes\r\n", packet->packet_len);

  printf("RX Channel: %d\r\n", packet->rx_ctrl.channel);
  printf("RX Rate: %d\r\n", packet->rx_ctrl.rate);

  printf("Raw Packet:\r\n");
  for (size_t i = 0; i < packet->packet_len; i++) {
    printf("%02x ", packet->raw_packet[i]);
    if ((i + 1) % 16 == 0) {
      printf("\r\n");
    }
  }
  if (packet->packet_len % 16 != 0) {
    printf("\r\n");
  }
  printf("==============================\r\n");
}