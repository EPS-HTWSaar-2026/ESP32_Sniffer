#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t packetQueue;
extern uint8_t esp_mac[6];

void init_wifi(void);
void change_wifi_channel(int channel);

#endif // WIFI_H
