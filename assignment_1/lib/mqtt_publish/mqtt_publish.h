#ifndef LIB__MQTT_PUBLISH__MQTT_PUBLISH__H
#define LIB__MQTT_PUBLISH__MQTT_PUBLISH__H

#define MQTT_PUBLISH_BROKER_HOST "mqtt.beebotte.com"
#define MQTT_PUBLISH_BROKER_PORT 1883
#define MQTT_PUBLISH_CLIENT_ID_MAX_LEN (256)
#define MQTT_PUBLISH_CLIENT_ID_PREFIX ""
#define MQTT_PUBLISH_TOPIC "YOUR_TOPIC" // i483/temperature
#define MQTT_PUBLISH_BUFFER_SIZE (256)
#define MQTT_PUBLISH_USERNAME "YOUR_TOKEN"

void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
void mqtt_publish_task(void* pvParam);

#endif /* LIB__MQTT_PUBLISH__MQTT_PUBLISH__H */