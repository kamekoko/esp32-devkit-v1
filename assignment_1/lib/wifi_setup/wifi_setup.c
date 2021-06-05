#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "wifi_setup.h"
#include "wifi_config.h"

static EventGroupHandle_t s_wifi_event_group;
static int retry_count;

const char *TAG = "main";

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_count < WIFI_SETUP_MAXIMUM_RETRY_COUNT) {
            esp_wifi_connect();
            retry_count++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAILED_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;

        ESP_LOGI(TAG, "assined IP address:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_station_init(uint8_t* mac_addr)
{
    s_wifi_event_group = xEventGroupCreate();
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_CONFIG_SSID,
            // .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
        },
    };
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_err_t ret;

    esp_log_level_set(TAG, ESP_LOG_INFO);

    if ((ret = nvs_flash_init()) == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        GIVEUP_ON_ERROR(nvs_flash_erase(), "nvs_flash_erase");
        ret = nvs_flash_init();
    }
    GIVEUP_ON_ERROR(ret, "nvs_flash_init");
    GIVEUP_ON_ERROR(esp_netif_init(), "esp_netif_init");

    GIVEUP_ON_ERROR(esp_event_loop_create_default(), "esp_event_loop_create_default");
    esp_netif_create_default_wifi_sta();

    GIVEUP_ON_ERROR(esp_wifi_init(&config), "esp_wifi_init");

    GIVEUP_ON_ERROR(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, 
        NULL, &instance_any_id), "esp_event_handler_instance_register"
    );
    GIVEUP_ON_ERROR(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler,
        NULL, &instance_got_ip), "esp_event_handler_instanve_register"
    );
    GIVEUP_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), "esp_wifi_set_mode");
    GIVEUP_ON_ERROR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config), "esp_wifi_set_config");
    GIVEUP_ON_ERROR(esp_wifi_start(), "esp_wifi_start");

    if (mac_addr != NULL) {
        GIVEUP_ON_ERROR(esp_read_mac(mac_addr, ESP_MAC_WIFI_STA), "esp_read_mac");
    }

    ESP_LOGI(TAG, "wifi_station_init_finished.");

    if (xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT, pdFALSE, pdFALSE, portMAX_DELAY)
        & WIFI_CONNECTED_BIT) {
            return ESP_OK;
    }
    return ESP_FAIL;
}
