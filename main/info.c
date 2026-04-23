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
  printf("==============================\r\n");
}
