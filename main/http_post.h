#ifndef HTTP_POST_H
#define HTTP_POST_H

#include <stddef.h>
#include <stdint.h>

void http_post_packet(const uint8_t *mac, int rssi, const uint8_t *payload,
                      size_t payload_len);

#endif // HTTP_POST_H
