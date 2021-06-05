#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_event_base.h"
#include "esp_system.h"

#include "task_manage.h"
#include "mqtt_publish.h"

static char* _FILE_ = "beebotte";

static bool connected;
static esp_mqtt_client_handle_t client;

void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    client = event->client;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED :
            ESP_LOGI(_FILE_, "MQTT_EVENT_CONNECTED");
            connected = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(_FILE_, "MQTT_EVENT_DISCONNECTED");
            connected = false;
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(_FILE_, "MQTT_EVENT_PUBLISHED, msg id=%d", event->msg_id);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(_FILE_, "MQTT_EVENT_ERROR, error_type: %d", event->error_handle->error_type);
            break;
        default:
            ESP_LOGI(_FILE_, "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_publish_task(void* pvParam)
{
    uint8_t mac_addr[6];
    char client_id[MQTT_PUBLISH_CLIENT_ID_MAX_LEN];
    esp_mqtt_client_config_t mqtt_client_config = {
        .host = MQTT_PUBLISH_BROKER_HOST,
        .port = MQTT_PUBLISH_BROKER_PORT,
        .username = MQTT_PUBLISH_USERNAME,
        .transport = MQTT_TRANSPORT_OVER_TCP,
    };

    esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
    snprintf(client_id, MQTT_PUBLISH_CLIENT_ID_MAX_LEN,
        "%s%02x%02x%02x%02x%02x%02x", MQTT_PUBLISH_CLIENT_ID_PREFIX,
        mac_addr[0], mac_addr[1], mac_addr[2],
        mac_addr[3], mac_addr[4], mac_addr[5]);
    mqtt_client_config.client_id = client_id;

    client = esp_mqtt_client_init(&mqtt_client_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    task_manage_t *handle = (task_manage_t*)pvParam;

    for (;;) {
        esp_err_t err;
        float data;
        printf("check queue\n");
        float num = sizeof(handle->queue);
        printf("%f\n", num);

        if (xQueueReceive(handle->queue, &data, portMAX_DELAY) == pdTRUE) {
            if (connected) {
                xSemaphoreTake(handle->mutex, portMAX_DELAY);
                int msg_id;
                static char payload[MQTT_PUBLISH_BUFFER_SIZE];

                snprintf(payload, MQTT_PUBLISH_BUFFER_SIZE,
                    "{\"ispublic\":true, \"write\":true, \"data\":%.3f}", data);
                ESP_LOGI(_FILE_, "publishing \"%s\" to \"%s\"", payload, MQTT_PUBLISH_TOPIC);
                msg_id = esp_mqtt_client_publish(client, MQTT_PUBLISH_TOPIC, payload, 0, 0, 0);
                ESP_LOGI(_FILE_, "published successfully, msg_id=%d", msg_id);
            
                xSemaphoreGive(handle->mutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}