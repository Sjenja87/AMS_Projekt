#ifndef _SENSORS_H_
#define _SENSORS_H_

#include "cJSON.h"
#include <string.h>
#include "esp_err.h"
#include "sdkconfig.h" 

typedef struct 
{
    int id;
    float temp;
    int address;
    int humidity;
    int isActive;
}sensor_t;

esp_err_t init_sensors();

esp_err_t get_sensors_data();

esp_err_t save_sensor_data();

esp_err_t find_new_sensors();

cJSON get_data_JSON();

int search_sens_collection(int addr, int isActive);

int search_current_JSON_OBJ(sensor_t* sensor);

#endif