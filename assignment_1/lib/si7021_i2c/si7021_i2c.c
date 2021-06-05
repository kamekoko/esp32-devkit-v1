#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_event_base.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "task_manage.h"
#include "si7021_i2c.h"

esp_err_t init_sensor(void) 
{
    esp_err_t error1;

    i2c_config_t config;
    config.sda_io_num = 21;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_io_num = 22;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.mode = I2C_MODE_MASTER;
    config.master.clk_speed = 100000;

    error1 = i2c_param_config(I2C_NUM_0, &config);

    return error1;
}

esp_err_t read_temp(float *tem)
{
    esp_err_t error;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SI_7021_ADDRESS << 1 | I2C_MASTER_WRITE, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, SI_7021_MESURE_TEMPERATURE, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    error = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    vTaskDelay(50 / portTICK_RATE_MS);
    if (error != ESP_OK) {
        printf("error : read temperature\n");
        return error;
    }

    uint8_t tmpMSB;
    uint8_t tmpLSB;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SI_7021_ADDRESS << 1 | I2C_MASTER_READ, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &tmpMSB, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &tmpLSB, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    error = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    if (error != ESP_OK) {
        printf("error : read temperature\n");
        return error;
    }

    float temp = ((uint16_t) tmpMSB << 8) | (uint16_t) tmpLSB;
    temp *= 175.72;
    temp /= 65536;
    temp -= 46.85;

    *tem = temp;

    return error;
}

esp_err_t read_humid(float *hum) 
{
    esp_err_t error;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SI_7021_ADDRESS << 1 | I2C_MASTER_WRITE, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, SI_7021_MESURE_HUMIDITY, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    error = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    vTaskDelay(50 / portTICK_RATE_MS);
    if (error != ESP_OK) {
        printf("error : read humidity\n");
        return error;
    }

    uint8_t humMSB;
    uint8_t humLSB;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SI_7021_ADDRESS << 1 | I2C_MASTER_READ, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &humMSB, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &humLSB, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    error = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    if (error != ESP_OK) {
        printf("error : read temperature\n");
        return error;
    }

    float humid = ((uint8_t)humMSB << 8) | (uint8_t)humLSB;
    humid *= 125;
    humid /= 65536;
    humid -= 6;

    *hum = humid;

    return error;
}

void si7021_read_task(void* pvParam) {
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    init_sensor();

    task_manage_t *handle = (task_manage_t*)pvParam;

    for (;;) {
        xSemaphoreTake(handle->mutex, portMAX_DELAY);
        float temp;
        read_temp(&temp);
        printf("temperature : %f\n", temp);
        // read_humid(&humid);
        // printf("humidity : %f\n", humid);
        xQueueSendToBack(handle->queue, &temp, portMAX_DELAY);
        printf("sensor data enqueue!\n");
        xSemaphoreGive(handle->mutex);

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}