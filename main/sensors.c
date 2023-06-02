#include "sensors.h"
#include "sd_storage.h"
#include "unistd.h"
#include <fcntl.h>
#include "sdkconfig.h"
#include "cJSON.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define SENSORS_DATA_FILE "/Data.txt"

#ifdef CONFIG_USE_SD_STORAGE

    #define SENSORS_DATA_FILE_PATH (SD_MOUNT_POINT CONFIG_WEB_MOUNT_POINT SENSORS_DATA_FILE)

#endif

static const char *TAG = "Sensor_Handle";

sensor_t sens_collection[MAX_SENSORS];

cJSON *current_JSON_OBJ;

SemaphoreHandle_t xMutex;

int last_id = 0;

esp_err_t init_sensors()
{
    init_ds1820();
    current_JSON_OBJ = cJSON_CreateArray();
    xMutex = xSemaphoreCreateMutex();
    return ESP_OK;
}

esp_err_t get_sensors_data()
{ 
    if( xSemaphoreTake( xMutex, ( TickType_t ) 1000 ) == pdTRUE )
    {   
        deactivate_all_cJSON_OBJ();
        for (int i = 0; i< MAX_SENSORS ;i++)
        {
            if(sens_collection[i].isActive)
            {
                if(ds1820_read_devices(sens_collection[i].address, &sens_collection[i].temp) != ESP_OK)
                {
                    sens_collection[i].isActive = false;
                    continue;
                }
                sens_collection[i].temp = sens_collection[i].temp * 10;
                if(find_and_activate(&sens_collection[i]) != ESP_OK)
                {
                    append_new_cJSON_Obj(&sens_collection[i]);
                }
            }
        }
        delete_deactive_cJSON_Obj();
        xSemaphoreGive( xMutex );
    }
    else
    {
        return ESP_FAIL;
    } 
    return save_sensor_data();
}

void delete_deactive_cJSON_Obj()
{
    cJSON *tmpObj;
    for (int i = 0 ; i < cJSON_GetArraySize(current_JSON_OBJ); i++)
    {
        tmpObj = cJSON_GetArrayItem(current_JSON_OBJ, i);
        if(cJSON_GetObjectItem(tmpObj,"isActive")->valueint == false)
        {
            cJSON_DeleteItemFromArray(current_JSON_OBJ,i);
        }
    }
}

esp_err_t append_new_cJSON_Obj(sensor_t* sensor)
{
        cJSON* newItem = cJSON_Parse( "{ \"id\" : 0, \"temp\" : 0.0, \"address\" : 0, \"isActive\" : 0}");
        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"id") , sensor->id);
        cJSON_SetNumberValue(cJSON_GetObjectItem(newItem,"temp") , (double)sensor->temp);
        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"address") , sensor->address);
        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"isActive") , true);
        cJSON_AddItemToArray(current_JSON_OBJ,newItem);
    
    return ESP_OK;
}

esp_err_t find_and_activate(sensor_t* sensor)
{
    cJSON *tmpObj;
    for (int i = 0 ; i < cJSON_GetArraySize(current_JSON_OBJ); i++)
    {  
        tmpObj = cJSON_GetArrayItem(current_JSON_OBJ, i);
        if(sensor->id == cJSON_GetObjectItem(tmpObj,"id")->valueint)
        {
            cJSON_SetIntValue(cJSON_GetObjectItem(tmpObj,"isActive") , true);
            cJSON_SetNumberValue(cJSON_GetObjectItem(tmpObj,"temp") , (double)sensor->temp);
            ESP_LOGI(TAG,"ID: %d , Temp: %.3f", sensor->id ,sensor->temp);
            return ESP_OK;
        }  
    } 
    return ESP_FAIL;      
}

esp_err_t get_data_JSON(cJSON* data)
{
    esp_err_t resp;
    if( xSemaphoreTake( xMutex, ( TickType_t ) 100 ) == pdTRUE )
    {
        if(current_JSON_OBJ != NULL)
        {
            *data = *current_JSON_OBJ;
             resp = ESP_OK;
        }
        else
        {
            resp = ESP_FAIL;
        }
       
        xSemaphoreGive( xMutex );

        return resp;
    }  
    else
    {
        return ESP_FAIL;
    }

}

esp_err_t save_sensor_data()
{
    char* string_to_save = cJSON_PrintUnformatted(current_JSON_OBJ);
    if(string_to_save != NULL)
    {
        return append_to_file(SENSORS_DATA_FILE_PATH,string_to_save);
    }
    return ESP_FAIL;
}

esp_err_t scan_sensors()
{   
   ds18x20_addr_t addrs[MAX_SENSORS];
   size_t sensor_count = 0;
   if(ds1820_scan_devices(addrs, &sensor_count) != ESP_OK)
   {
      return ESP_FAIL;
   }
   ESP_LOGI(TAG, "FOUND: %d SENSORS", sensor_count);
   deactivate_all_sensors();
   for(int i = 0; i< sensor_count; i++)
   {
        if(create_or_activate(addrs[i]))
        {
            for(int n = 0 ; n < MAX_SENSORS; n++)
            {
                if(sens_collection[n].id == 0)
                {
                    sens_collection[n].id = last_id++;
                    sens_collection[n].address = addrs[i];
                    sens_collection[n].isActive = true;
                }
            }
        }
   }
    return ESP_OK;
}

bool create_or_activate( ds18x20_addr_t addr)
{
    for(int i = 0; i < MAX_SENSORS; i++)
    {
        if(sens_collection[i].address == addr)
        {
            sens_collection[i].isActive = true;
            return false;
        }
    }
    return true;
}

void deactivate_all_cJSON_OBJ()
{
    cJSON *tmpObj;
    for (int i = 0 ; i < cJSON_GetArraySize(current_JSON_OBJ); i++)
    {  
        tmpObj = cJSON_GetArrayItem(current_JSON_OBJ, i);
        cJSON_SetIntValue(cJSON_GetObjectItem(tmpObj,"isActive") , false);
    }          
}

void deactivate_all_sensors()
{
    for(int i = 0; i < MAX_SENSORS; i++)
    {
        sens_collection[i].isActive = false;
    }
}
