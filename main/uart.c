#include "uart.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "info.h"

#define UART_PORT UART_NUM_1
static const char *TAG = "UART";

extern QueueHandle_t packetQueue;

static void uart_tx_task(void *pvParameters) {
  aeroScoutPacket packet;

  while (1) {
    if (xQueueReceive(packetQueue, &packet, portMAX_DELAY) == pdTRUE) {

      if (packet.packet_len > 0) {
        uart_write_bytes(UART_PORT, (const void *)packet.raw_packet,
                         packet.packet_len);
      }
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

  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
  // Set UART pins (TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT, 4, 5, 18, 19, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));
  // Install UART driver
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, uart_buffer_size,
                                      uart_buffer_size, 10, &uart_driver_queue,
                                      0));

  xTaskCreate(uart_tx_task, "uart_tx_task", 4096, NULL, 5, NULL);
  ESP_LOGI(TAG, "UART initialized and TX background task started.");
}
