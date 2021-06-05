#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "esp_system.h"

#include "wifi_config.h"
#include "wifi_setup.h"
#include "task_manage.h"
#include "mqtt_publish.h"
#include "si7021_i2c.h"

static char* _FILE_ = "main";

void app_main(void) 
{
    vTaskDelay(pdMS_TO_TICKS(5000));
    printf("start\n");

    int retry;
    uint8_t mac_addr[6];

    esp_log_level_set(_FILE_, ESP_LOG_INFO);

    for (retry = 0;; retry++) {
        if (wifi_station_init(mac_addr) != ESP_OK) {
            ESP_LOGE(_FILE_, "failed to setup WiFi");
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            break;
        }
        if (retry >= WIFI_SETUP_MAXIMUM_RETRY_COUNT) {
            ESP_LOGE(_FILE_, "give up to setup WiFi");
            goto fin;
        }
    }
    ESP_LOGI(_FILE_, "Mac address: %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    printf("wifi setup complete\n");

    task_manage_t tm;
    tm.mutex = xSemaphoreCreateMutex();
    tm.queue = xQueueCreate(16, sizeof(float));

    xTaskCreate(&si7021_read_task, "READ DATA", 8192, (void*)&tm, 1, NULL);
    xTaskCreate(&mqtt_publish_task, "MQTT PUBLISH", 4096, (void*)&tm, 1, NULL);

    printf("done\n");

fin:
    for(;;) {
        vTaskDelay(1);
    }
}