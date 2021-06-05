#ifndef LIB__SI7021_I2C__SI7021_I2C__H
#define LIB__SI7021_I2C__SI7021_I2C__H

#define SI_7021_ADDRESS 0x40
#define SI_7021_MESURE_HUMIDITY 0xE5
#define SI_7021_MESURE_TEMPERATURE 0xE3
#define SI_7021_TIMEOUT (1000 / portTICK_RATE_MS)

esp_err_t init_sensor(void);
esp_err_t read_temp(float *tem);
esp_err_t read_humid(float *hum);
void si7021_read_task(void* pvParam);

#endif