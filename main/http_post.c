#include "http_post.h"
#include "env.h"

#include <stdio.h>
#include <string.h>

#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "HTTP_POST";

#define HEX_BUF_SIZE (256 * 3 + 1)

#define JSON_BUF_SIZE 900

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  if (evt->event_id == HTTP_EVENT_ERROR) {
    ESP_LOGW(TAG, "HTTP error event");
  }
  return ESP_OK;
}

void http_post_packet(const uint8_t *mac, int rssi, const uint8_t *payload,
                      size_t payload_len) {
  static char hex_buf[HEX_BUF_SIZE];
  size_t pos = 0;
  for (size_t i = 0; i < payload_len && pos + 3 < HEX_BUF_SIZE; i++) {
    pos += snprintf(hex_buf + pos, HEX_BUF_SIZE - pos, "%02X", payload[i]);
    if (i + 1 < payload_len) {
      hex_buf[pos++] = ' ';
    }
  }
  hex_buf[pos] = '\0';

  static char json_buf[JSON_BUF_SIZE];
  int json_len =
      snprintf(json_buf, sizeof(json_buf),
               "{"
               "\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\","
               "\"rssi\":%d,"
               "\"raw\":\"%s\""
               "}",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], rssi, hex_buf);

  if (json_len < 0 || json_len >= (int)sizeof(json_buf)) {
    ESP_LOGE(TAG, "JSON buffer overflow, packet dropped");
    return;
  }

  char url[128];
  snprintf(url, sizeof(url), "http://%s:%d%s", SERVER_IP, SERVER_PORT,
           SERVER_PATH);

  esp_http_client_config_t config = {
      .url = url,
      .method = HTTP_METHOD_POST,
      .timeout_ms = 5000,
      .event_handler = _http_event_handler,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
    ESP_LOGE(TAG, "Failed to initialise HTTP client");
    return;
  }

  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, json_buf, json_len);

  esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK) {
    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "POST %s -> HTTP %d", url, status);
  } else {
    ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
  }

  esp_http_client_cleanup(client);
}
