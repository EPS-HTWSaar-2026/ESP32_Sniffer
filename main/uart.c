#include "uart.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "info.h"
#include "wifi.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define UART_PORT UART_NUM_1
#define UART_TX_PIN (4)
#define UART_RX_PIN (5)
#define UART_TASK_STACK_SIZE (4096)
#define UART_TASK_PRIORITY (5)
#define UART_BUFFER_SIZE (1024)

static const char *TAG = "UART";

extern QueueHandle_t packetQueue;

static void mac_to_str(const uint8_t *mac, char *out) {
  for (int i = 0; i < 6; i++)
    snprintf(&out[i * 2], 3, "%02X", mac[i]);
}

static char *create_raw_hex_string(const uint8_t *raw_data, size_t len) {
  if (raw_data == NULL || len == 0) {
    return NULL;
  }

  char *raw_hex = malloc(len * 2 + 1);
  if (raw_hex != NULL) {
    for (size_t i = 0; i < len; i++) {
      sprintf(&raw_hex[i * 2], "%02X", raw_data[i]);
    }
    raw_hex[len * 2] = '\0';
  }
  return raw_hex;
}

static cJSON *create_rx_ctrl_json(const wifi_pkt_rx_ctrl_t *rx_ctrl) {
  if (rx_ctrl == NULL) {
    return NULL;
  }

  cJSON *rx_ctrl_json = cJSON_CreateObject();
  if (rx_ctrl_json != NULL) {
    cJSON_AddNumberToObject(rx_ctrl_json, "rssi", rx_ctrl->rssi);
    cJSON_AddNumberToObject(rx_ctrl_json, "rate", rx_ctrl->rate);
    cJSON_AddNumberToObject(rx_ctrl_json, "channel", rx_ctrl->channel);
    cJSON_AddNumberToObject(rx_ctrl_json, "sig_len", rx_ctrl->sig_len);
  }
  return rx_ctrl_json;
}

static void uart_tx_task(void *pvParameters) {
  sniffer_packet_t *packet = NULL;

  while (1) {
    if (xQueueReceive(packetQueue, &packet, portMAX_DELAY) != pdTRUE)
      continue;
    if (packet == NULL)
      continue;

    print_aeroscout_info(packet);

    char e_addr[13] = {0};
    mac_to_str(esp_mac, e_addr);

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
      free(packet);
      packet = NULL;
      continue;
    }

    char *raw_hex =
        create_raw_hex_string(packet->payload, packet->rx_ctrl.sig_len);
    if (raw_hex != NULL) {
      cJSON_AddStringToObject(root, "raw_packets", raw_hex);
      free(raw_hex);
    }

    cJSON *rx_ctrl_json = create_rx_ctrl_json(&packet->rx_ctrl);
    if (rx_ctrl_json != NULL) {
      cJSON_AddItemToObject(root, "rx_ctrl", rx_ctrl_json);
    }
    cJSON_AddStringToObject(root, "espMac", e_addr);

    char *json = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    if (json != NULL) {
      uart_write_bytes(UART_PORT, json, strlen(json));
      uart_write_bytes(UART_PORT, "\n", 1);
      cJSON_free(json);
    }

    free(packet);
    packet = NULL;
  }
}

static void uart_rx_task(void *pvParameters) {
  uint8_t channel = 0;
  while (1) {
    int len = uart_read_bytes(UART_PORT, (uint8_t *)&channel, sizeof(channel),
                              portMAX_DELAY);
    if (len > 0) {
      ESP_LOGI(TAG, "Received channel change command: %d", channel);
      change_wifi_channel(channel);
    }
  }
}

void init_uart(void) {
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
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN,
                               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_BUFFER_SIZE,
                                      UART_BUFFER_SIZE, 10, &uart_driver_queue,
                                      0));

  xTaskCreate(uart_tx_task, "uart_tx_task", UART_TASK_STACK_SIZE, NULL,
              UART_TASK_PRIORITY, NULL);
  ESP_LOGI(TAG, "UART initialised, TX task started");

  xTaskCreate(uart_rx_task, "uart_rx_task", UART_TASK_STACK_SIZE, NULL,
              UART_TASK_PRIORITY, NULL);
  ESP_LOGI(TAG, "UART RX task started");
}
