#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t packetQueue;

void init_wifi(void);

#endif // WIFI_H endif
