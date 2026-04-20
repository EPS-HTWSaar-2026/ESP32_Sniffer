#include "wifi.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include <string.h>

static const char *TAG = "AeroScout";
static const uint8_t AEROSCOUT_OUI[3] = {0x48, 0x02, 0x01};
static info *_wifi_info;

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_DATA && type != WIFI_PKT_MGMT)
    return;

  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t *payload = pkt->payload;
  uint16_t fctl = payload[0] | (payload[1] << 8);

  uint8_t to_ds = (fctl >> 8) & 0x01;
  uint8_t from_ds = (fctl >> 9) & 0x01;

  // Address positions in 802.11 frame:
  uint8_t *addr1 = payload + 4;
  uint8_t *addr2 = payload + 10;
  uint8_t *addr3 = payload + 16;
  uint8_t *addr4 = payload + 24; // <-- Add Address 4 definition

  uint8_t *src = NULL;
  if (!to_ds && !from_ds) {
    src = addr1;
  } else if (!to_ds && from_ds) {
    src = addr3;
  } else if (to_ds && !from_ds) {
    src = addr2;
  } else {
    // Both ToDS and FromDS are 1. This is a 4-address frame.
    // Ensure the packet is long enough to contain a 4th address to prevent
    // crashes
    if (pkt->rx_ctrl.sig_len < 30)
      return;
    src = addr4; // <-- Extract Source from Address 4
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
}

void init_wifi(info *wifi_info) {
  _wifi_info = wifi_info;

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());

  // Allow ALL frame types through – critical for AeroScout
  wifi_promiscuous_filter_t filter = {.filter_mask =
                                          WIFI_PROMIS_FILTER_MASK_ALL};
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&sniffer));
  ESP_ERROR_CHECK(esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE));

  ESP_LOGI(TAG, "Sniffer running on channel 6");
}
