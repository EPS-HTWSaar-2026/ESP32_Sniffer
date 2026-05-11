#include "info.h"
#include <stdio.h>

void print_aeroscout_info(const aeroScoutPacket *packet) {
  if (packet == NULL)
    return;

  printf("\r\n=== AeroScout Tag Detected ===\r\n");
  printf("Tag MAC (Source): %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         packet->sourceAddr[0], packet->sourceAddr[1], packet->sourceAddr[2],
         packet->sourceAddr[3], packet->sourceAddr[4], packet->sourceAddr[5]);

  printf("Transmitter MAC:  %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         packet->transmitterAddr[0], packet->transmitterAddr[1],
         packet->transmitterAddr[2], packet->transmitterAddr[3],
         packet->transmitterAddr[4], packet->transmitterAddr[5]);

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

  printf("{\"transmitterAddr\": \"%02x%02x%02x%02x%02x%02x\", "
         "\"sourceAddr\": \"%02x%02x%02x%02x%02x%02x\", "
         "\"rssi\": %d, \"packet_len\": %zu, \"channel\": %d}\r\n",
         packet->transmitterAddr[0], packet->transmitterAddr[1],
         packet->transmitterAddr[2], packet->transmitterAddr[3],
         packet->transmitterAddr[4], packet->transmitterAddr[5],
         packet->sourceAddr[0], packet->sourceAddr[1], packet->sourceAddr[2],
         packet->sourceAddr[3], packet->sourceAddr[4], packet->sourceAddr[5],
         packet->rssi, packet->packet_len, packet->rx_ctrl.channel);
}