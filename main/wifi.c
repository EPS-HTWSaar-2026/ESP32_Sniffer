#include "wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "info.h"
#include <string.h>

static const char *TAG = "AeroScout";
static const uint8_t AEROSCOUT_OUI[3] = {0x48, 0x02, 0x01};

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_DATA && type != WIFI_PKT_MGMT)
    return;

  // Get Payload
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t *payload = pkt->payload;

  const uint8_t *transmitterAddr = payload + 10;
  const uint8_t *sourceAddr = payload + 24;

  // Check if AeroScout
  if (sourceAddr == NULL || memcmp(sourceAddr, AEROSCOUT_OUI, 3) != 0)
    return;
  ESP_LOGI(TAG, ">>> tag: %02x:%02x:%02x:%02x:%02x:%02x RSSI: %d",
           transmitterAddr[0], transmitterAddr[1], transmitterAddr[2],
           transmitterAddr[3], transmitterAddr[4], transmitterAddr[5],
           pkt->rx_ctrl.rssi);

  // Prepare Packet Info
  aeroScoutPacket packet;
  memcpy(packet.sourceAddr, sourceAddr, 6);
  memcpy(packet.transmitterAddr, transmitterAddr, 6);
  packet.rssi = pkt->rx_ctrl.rssi;

  size_t safe_len = pkt->rx_ctrl.sig_len < MAX_QUEUED_PAYLOAD
                        ? pkt->rx_ctrl.sig_len
                        : MAX_QUEUED_PAYLOAD;

  memcpy(packet.raw_packet, payload, safe_len);
  packet.packet_len = safe_len;
  packet.rx_ctrl = pkt->rx_ctrl;

  // Send Packet Info
  // print_aeroscout_info(&packet);
  if (packetQueue != NULL) {

    xQueueSendFromISR(packetQueue, &packet, NULL);
  }
}

void init_wifi(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());

  const wifi_promiscuous_filter_t filter = {.filter_mask =
                                                WIFI_PROMIS_FILTER_MASK_ALL};
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&sniffer));
  ESP_ERROR_CHECK(esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));

  ESP_LOGI(TAG, "Sniffer running on channel 6");
}
