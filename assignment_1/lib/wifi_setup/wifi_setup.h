#ifndef LIB__WIFI_SETUP__WIFI_SETUP__H
#define LIB__WIFI_SETUP__WIFI_SETUP__H

#define WIFI_SETUP_MAXIMUM_RETRY_COUNT (8)
#define WIFI_CONNECTED_BIT (BIT0)
#define WIFI_FAILED_BIT (BIT1)

#define GIVEUP_ON_ERROR(cond, name) do {esp_err_t err;if((err = cond) != ESP_OK) {ESP_LOGE(TAG, "%s failed (%s)", name, esp_err_to_name(err)); return err; } }while(0)

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
esp_err_t wifi_station_init(uint8_t* mac_addr);

#endif /*LIB__WIFI_SETUP__WIFI_SETUP__H*/