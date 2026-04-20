#ifndef INFO_H
#define INFO_H

#include <stddef.h>
#include <stdint.h>

#define INFO_ARRAY_LENGTH 255
#define MAC_LENGTH 32
#define NAME_LENGTH 64
#define RAW_PACKET_MAX_LEN 256

typedef enum { BLE, WIFI } source;

typedef struct {
  uint8_t mac[MAC_LENGTH];
  size_t mac_len;
  char name[NAME_LENGTH];
  size_t name_len;
  int rssi;
  source source;
  uint8_t raw_packet[RAW_PACKET_MAX_LEN];
  size_t packet_len;
} info;

void update_info(info *info_to_add, uint8_t *mac, size_t mac_len, char *name,
                 size_t name_len, int rssi, source source, uint8_t *raw_packet,
                 size_t packet_len);
void send_info(info *info_to_send);

#endif // INFO_H
