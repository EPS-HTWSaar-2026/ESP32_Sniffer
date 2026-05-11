#include "uart.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "info.h"
#include "wifi.h"
#include <string.h>

#define UART_PORT UART_NUM_1



extern QueueHandle_t packetQueue;

static void mac_to_str(const uint8_t *mac, char *out) {
  for (int i = 0; i < 6; i++)
    sprintf(&out[i * 2], "%02X", mac[i]);
}

static void uart_tx_task(void *pvParameters) {
  aeroScoutPacket packet;

  while (1) {
    if (xQueueReceive(packetQueue, &packet, portMAX_DELAY) != pdTRUE)
      continue;

    print_aeroscout_info(&packet);

    char t_addr[13] = {0};
    char e_addr[13] = {0};
    mac_to_str(packet.transmitterAddr, t_addr);
    mac_to_str(esp_mac, e_addr);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "tAddr", t_addr);
    cJSON_AddNumberToObject(root, "rssi", packet.rssi);
    cJSON_AddStringToObject(root, "espMac", e_addr);

    char *json = cJSON_PrintUnformatted(root);
    if (json != NULL) {
      uart_write_bytes(UART_PORT, json, strlen(json));
      uart_write_bytes(UART_PORT, "\n", 1);
      cJSON_free(json);
    }
    cJSON_Delete(root);
  }
}

void init_uart(void) {
  const int uart_buffer_size = 1024 * 2;
  QueueHandle_t uart_driver_queue;

  const uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
  ESP_ERROR_CHECK(
      uart_set_pin(UART_PORT, 4, 5, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, uart_buffer_size,
                                      uart_buffer_size, 10, &uart_driver_queue,
                                      0));

  xTaskCreate(uart_tx_task, "uart_tx_task", 4096, NULL, 5, NULL);
  ESP_LOGI(TAG, "UART initialised, TX task started");
}
