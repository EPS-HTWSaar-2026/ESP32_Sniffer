#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "info.h"
#include "nvs_flash.h"
#include "uart.h"
#include "wifi.h"

#define POST_QUEUE_DEPTH 20

QueueHandle_t packetQueue = NULL;

void app_main(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  packetQueue = xQueueCreate(POST_QUEUE_DEPTH, sizeof(sniffer_packet_t *));
  configASSERT(packetQueue != NULL);

  init_uart();
  init_wifi();
}
