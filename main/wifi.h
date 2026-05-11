#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t packetQueue;
extern uint8_t esp_mac[6];

void init_wifi(void);

static const int channel = 6;
static const char *TAG = "AeroScout";
static const uint8_t AEROSCOUT_OUI[3] = {0x01, 0x0C, 0xCC};


#endif // WIFI_H
