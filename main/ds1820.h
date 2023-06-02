#ifndef _DS1820_H_
#define _DS1820_H_
#include <esp_err.h>
#include <ds18x20.h>

#define MAX_SENSORS 10

esp_err_t init_ds1820();

esp_err_t ds1820_scan_devices(ds18x20_addr_t* addrs, size_t *sensor_count);

esp_err_t ds1820_read_devices(ds18x20_addr_t addrs, float* temp);

#endif