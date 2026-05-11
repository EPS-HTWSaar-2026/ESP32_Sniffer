#include "wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "info.h"
#include <string.h>

#define WIFI_CHANNEL (6)

static const char *TAG = "AeroScout";
static const uint8_t AEROSCOUT_OUI[3] = {0x01, 0x0C, 0xCC};

uint8_t esp_mac[6] = {0};

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_DATA && type != WIFI_PKT_MGMT)
    return;

  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;

  uint8_t *payload = pkt->payload;
  const uint8_t *destinationAddr = payload + 4;
  const uint8_t *transmitterAddr = payload + 10;

  if (memcmp(destinationAddr, AEROSCOUT_OUI, 3) != 0)
    return;

  ESP_LOGI(TAG, ">>> tag: %02x:%02x:%02x:%02x:%02x:%02x RSSI: %d",
           transmitterAddr[0], transmitterAddr[1], transmitterAddr[2],
           transmitterAddr[3], transmitterAddr[4], transmitterAddr[5],
           pkt->rx_ctrl.rssi);

  aeroScoutPacket packet;
  packet.rssi = pkt->rx_ctrl.rssi;
  packet.rx_ctrl = pkt->rx_ctrl;

  size_t safe_len = pkt->rx_ctrl.sig_len < MAX_QUEUED_PAYLOAD
                        ? pkt->rx_ctrl.sig_len
                        : MAX_QUEUED_PAYLOAD;

  packet.packet_len = safe_len;
  memcpy(packet.raw_packet, payload, safe_len);

  if (packetQueue != NULL) {
    if (xQueueSend(packetQueue, &packet, 0) != pdTRUE)
      ESP_LOGW(TAG, "Packet queue full — dropped");
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
  ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));

  ESP_LOGI(TAG,
           "Sniffer running on channel %d, MAC: "
           "%02x:%02x:%02x:%02x:%02x:%02x",
           WIFI_CHANNEL, esp_mac[0], esp_mac[1], esp_mac[2], esp_mac[3], esp_mac[4],
           esp_mac[5]);
}