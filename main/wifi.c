#include "wifi.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "http_post.h"
#include <string.h>

static const char *TAG = "AeroScout";
static const uint8_t AEROSCOUT_OUI[3] = {0x48, 0x02, 0x01};
static info *_wifi_info;

#define POST_QUEUE_DEPTH 8
#define MAX_QUEUED_PAYLOAD 256

typedef struct {
  uint8_t mac[6];
  int rssi;
  uint8_t payload[MAX_QUEUED_PAYLOAD];
  size_t payload_len;
} post_item_t;

static QueueHandle_t s_post_queue = NULL;

static void post_task(void *arg) {
  post_item_t item;
  for (;;) {
    if (xQueueReceive(s_post_queue, &item, portMAX_DELAY) == pdTRUE) {
      http_post_packet(item.mac, item.rssi, item.payload, item.payload_len);
    }
  }
}

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_DATA && type != WIFI_PKT_MGMT)
    return;

  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t *payload = pkt->payload;
  uint16_t fctl = payload[0] | (payload[1] << 8);

  uint8_t to_ds = (fctl >> 8) & 0x01;
  uint8_t from_ds = (fctl >> 9) & 0x01;

  uint8_t *addr1 = payload + 4;
  uint8_t *addr2 = payload + 10;
  uint8_t *addr3 = payload + 16;
  uint8_t *addr4 = payload + 24;

  uint8_t *src = NULL;
  if (!to_ds && !from_ds)
    src = addr1;
  else if (!to_ds && from_ds)
    src = addr3;
  else if (to_ds && !from_ds)
    src = addr2;
  else {
    if (pkt->rx_ctrl.sig_len < 30)
      return;
    src = addr4;
  }

  if (src == NULL)
    return;
  if (memcmp(src, AEROSCOUT_OUI, 3) != 0)
    return;

  ESP_LOGI(TAG,
           ">>> AeroScout tag: %02x:%02x:%02x:%02x:%02x:%02x  RSSI: %d dBm",
           src[0], src[1], src[2], src[3], src[4], src[5], pkt->rx_ctrl.rssi);

  update_info(_wifi_info, src, 6, "", 0, pkt->rx_ctrl.rssi, WIFI, payload,
              pkt->rx_ctrl.sig_len);
  send_info(_wifi_info);

  if (s_post_queue != NULL) {
    post_item_t item;
    memcpy(item.mac, src, 6);
    item.rssi = pkt->rx_ctrl.rssi;

    size_t safe_len = pkt->rx_ctrl.sig_len < MAX_QUEUED_PAYLOAD
                          ? pkt->rx_ctrl.sig_len
                          : MAX_QUEUED_PAYLOAD;
    memcpy(item.payload, payload, safe_len);
    item.payload_len = safe_len;

    if (xQueueSendFromISR(s_post_queue, &item, NULL) != pdTRUE) {
      ESP_LOGW(TAG, "POST queue full – packet dropped");
    }
  }
}

void init_wifi(info *wifi_info) {
  _wifi_info = wifi_info;

  s_post_queue = xQueueCreate(POST_QUEUE_DEPTH, sizeof(post_item_t));
  configASSERT(s_post_queue != NULL);

  xTaskCreate(post_task, "post_task", 4096, NULL, 5, NULL);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());

  wifi_promiscuous_filter_t filter = {.filter_mask =
                                          WIFI_PROMIS_FILTER_MASK_ALL};
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&sniffer));
  ESP_ERROR_CHECK(esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE));

  ESP_LOGI(TAG, "Sniffer running on channel 6");
}
