#include "uart.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <string.h>

#include "info.h"

#define UART_PORT UART_NUM_1
static const char *TAG = "UART";

extern QueueHandle_t packetQueue;

static void uart_tx_task(void *pvParameters) {
  aeroScoutPacket packet;
  while (1) {
    if (xQueueReceive(packetQueue, &packet, portMAX_DELAY) == pdTRUE) {
      print_aeroscout_info(&packet);

      cJSON *root = cJSON_CreateObject();

      char mac_str[12 + 1] = {0};
      for (int i = 0; i < 6; i++) {
        sprintf(&mac_str[i * 2], "%02X", packet.transmitterAddr[i]);
      }
      cJSON_AddStringToObject(root, "tAddr", mac_str);

      cJSON_AddNumberToObject(root, "rssi", packet.rssi);

      char raw_mac[6 + 1] = {0};
      for (int i = 0; i < 6; i++) {
        sprintf(&raw_mac[i], "%02X", esp_mac[i]);
      }
      cJSON_AddStringToObject(root, "esp_mac", raw_mac);

      char *json_string = cJSON_PrintUnformatted(root);

      uart_write_bytes(UART_PORT, json_string, strlen(json_string));
      uart_write_bytes(UART_PORT, "\n", 1);
      cJSON_free(json_string);
      cJSON_Delete(root);
    }
  }
}

void init_uart(void) {
  const int uart_buffer_size = 1024 * 2;
  QueueHandle_t uart_driver_queue;

  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
      .rx_flow_ctrl_thresh = 122,
      .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT, 4, 5, 18, 19));
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, uart_buffer_size,
                                      uart_buffer_size, 10, &uart_driver_queue,
                                      0));

  xTaskCreate(uart_tx_task, "uart_tx_task", 4096, NULL, 5, NULL);
  ESP_LOGI(TAG, "UART initialized and TX background task started.");
}
