#ifndef _SENSORS_H_
#define _SENSORS_H_

#include "cJSON.h"
#include <string.h>
#include "esp_err.h"
#include "sdkconfig.h" 
#include "ds1820.h"

typedef struct 
{
    int id;
    float temp;
    ds18x20_addr_t address;
    int isActive;
}sensor_t;

esp_err_t init_sensors();

esp_err_t get_sensors_data();

esp_err_t save_sensor_data();

esp_err_t scan_sensors();

void deactivate_all_cJSON_OBJ();

esp_err_t find_and_activate(sensor_t* sensor);

esp_err_t append_new_cJSON_Obj(sensor_t* sensor);

void delete_deactive_cJSON_Obj();

void deactivate_all_sensors();

bool create_or_activate( ds18x20_addr_t addr);

esp_err_t get_data_JSON(cJSON* data);

#endif