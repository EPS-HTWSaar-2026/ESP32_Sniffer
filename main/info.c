#include "info.h"
#include <stdio.h>

void print_aeroscout_info(const sniffer_packet_t *packet) {
  if (packet == NULL)
    return;

  printf("\r\n=== AeroScout Tag Detected ===\r\n");
  printf("Packet Length: %zu bytes\r\n", packet->rx_ctrl.sig_len);

  printf("RX Channel: %d\r\n", packet->rx_ctrl.channel);
  printf("RX Rate: %d\r\n", packet->rx_ctrl.rate);

  printf("Raw Packet:\r\n");
  for (size_t i = 0; i < packet->rx_ctrl.sig_len; i++) {
    printf("%02x ", packet->payload[i]);
    if ((i + 1) % 16 == 0) {
      printf("\r\n");
    }
  }
  if (packet->rx_ctrl.sig_len % 16 != 0) {
    printf("\r\n");
  }
  printf("==============================\r\n");
}
