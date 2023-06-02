#include "ds1820.h"
#include <inttypes.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ds18x20.h>
#include <esp_log.h>
#include <esp_err.h>

static const gpio_num_t SENSOR_GPIO = 27;

static const char *TAG = "ds1820";

float temps[MAX_SENSORS];

esp_err_t init_ds1820()
{
    return gpio_set_pull_mode(SENSOR_GPIO, GPIO_PULLUP_ONLY);
}

esp_err_t ds1820_scan_devices(ds18x20_addr_t* addrs, size_t *sensor_count)
{
    esp_err_t res = ds18x20_scan_devices(SENSOR_GPIO, addrs, MAX_SENSORS, sensor_count);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Sensors scan error %d (%s)", res, esp_err_to_name(res));
    }
    return res;
}

esp_err_t ds1820_read_devices(ds18x20_addr_t addrs, float* temp)
{
    return ds18b20_measure_and_read(SENSOR_GPIO, addrs, temp);
}
