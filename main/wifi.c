#include "wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "info.h"
#include <string.h>

static int wifi_channel = 6;
static const char *TAG = "AeroScout";
static const uint8_t AEROSCOUT_OUI[3] = {0x01, 0x0C, 0xCC};

uint8_t esp_mac[6] = {0};

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (buf == NULL)
    return;
  if (type != WIFI_PKT_DATA && type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *pkt = (const wifi_promiscuous_pkt_t *)buf;
  if (pkt->rx_ctrl.sig_len < 16)
    return;

  if (pkt->rx_ctrl.sig_len > MAX_QUEUED_PAYLOAD)
    return;

  if (memcmp(pkt->payload + 4, AEROSCOUT_OUI, 3) != 0)
    return;

  sniffer_packet_t *q_pkt = malloc(sizeof(sniffer_packet_t));
  if (q_pkt == NULL)
    return;

  q_pkt->rx_ctrl = pkt->rx_ctrl;
  q_pkt->len = (uint16_t)pkt->rx_ctrl.sig_len;
  memcpy(q_pkt->payload, pkt->payload, q_pkt->len);

  if (packetQueue != NULL) {
    // Queue holds the pointer itself, so pass &q_pkt
    if (xQueueSend(packetQueue, &q_pkt, 0) != pdTRUE) {
      free(q_pkt);
    }
  } else {
    free(q_pkt);
  }
}

void init_wifi(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_wifi_get_mac(WIFI_IF_STA, esp_mac);

  const wifi_promiscuous_filter_t filter = {
      .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL,
  };
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&sniffer));
  ESP_ERROR_CHECK(esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));

  ESP_LOGI(TAG,
           "Sniffer running on channel %d, MAC: "
           "%02x:%02x:%02x:%02x:%02x:%02x",
           wifi_channel, esp_mac[0], esp_mac[1], esp_mac[2], esp_mac[3],
           esp_mac[4], esp_mac[5]);
}

void change_wifi_channel(int channel) {
  if (channel < 1 || channel > 13) {
    ESP_LOGW(TAG, "Invalid channel %d, must be between 1 and 13", channel);
    return;
  }
  wifi_channel = channel;
  ESP_ERROR_CHECK(esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE));
  ESP_LOGI(TAG, "Switched to channel %d", wifi_channel);
}
