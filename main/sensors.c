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

#define BUF_SIZE 1000

#define MAX_SENSORS 20

static const char *TAG = "Sensor_Handle";

sensor_t sens_collection[MAX_SENSORS];

cJSON *current_JSON_OBJ;

int curr_address;

SemaphoreHandle_t xMutex;

esp_err_t init_sensors()
{
    char buf[BUF_SIZE];
    if(read_last_save_from_file(SENSORS_DATA_FILE_PATH,buf,'[') == ESP_OK)
    {
        current_JSON_OBJ = cJSON_Parse(buf);
        cJSON *tmpObj;
        for (int i = 0 ; i < cJSON_GetArraySize(current_JSON_OBJ); i++)
            {  
               tmpObj = cJSON_GetArrayItem(current_JSON_OBJ, i);
               sens_collection[i].id = cJSON_GetObjectItem(tmpObj,"id")->valueint;
               sens_collection[i].temp = (float)(cJSON_GetObjectItem(tmpObj,"temp")->valuedouble);
               sens_collection[i].address = cJSON_GetObjectItem(tmpObj,"address")->valueint;
               sens_collection[i].humidity = cJSON_GetObjectItem(tmpObj,"humidity")->valueint;
               sens_collection[i].isActive = cJSON_GetObjectItem(tmpObj,"isActive")->valueint;
            }          
        }
        xMutex = xSemaphoreCreateMutex();
        return ESP_OK;
}

esp_err_t get_sensors_data()
{ 
    for (int i = 0; i< MAX_SENSORS ;i++)
    {
        if(sens_collection[i].isActive)
        {
                // A function that takes a pointer to the sens_collection ithe element,
                // and uprate temp and humidity.
                // If sensor is not responding deactivate it;
            if( xSemaphoreTake( xMutex, ( TickType_t ) 100 ) == pdTRUE )
            {   
                if(!search_current_JSON_OBJ(&sens_collection[i]))
                {
                    if(sens_collection[i].isActive)
                    {
                        cJSON* newItem = cJSON_Parse( "{ \"id\" : 0, \"temp\" : 0.0, \"address\" : 0, \"humidity\" : 0, \"isActive\" : 0}");
                        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"id") , sens_collection[i].id );
                        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"temp") , sens_collection[i].temp);
                        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"address") , sens_collection[i].address);
                        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"humidity") , sens_collection[i].humidity);
                        cJSON_SetIntValue(cJSON_GetObjectItem(newItem,"isActive") , true);
                        cJSON_AddItemToArray(current_JSON_OBJ,newItem);
                    }
                }
                xSemaphoreGive( xMutex );
            }
            else
            {
                return ESP_FAIL;
            }
        }
    }
    //ESP_LOGI(TAG,"%s",cJSON_Print(current_JSON_OBJ));
    return save_sensor_data();
}
    
cJSON get_data_JSON()
{
    if( xSemaphoreTake( xMutex, ( TickType_t ) 100 ) == pdTRUE )
    {
        cJSON ret = *current_JSON_OBJ;
        xSemaphoreGive( xMutex );

        return ret;
    }  
    else
    {
        return *current_JSON_OBJ;
    }

}

esp_err_t save_sensor_data()
{
    char* string_to_save = cJSON_PrintUnformatted(current_JSON_OBJ);
    return append_to_file(SENSORS_DATA_FILE_PATH,string_to_save);

}

esp_err_t find_new_sensors()
{   
   int addr = curr_address;
   do{
        if(addr == 0 || addr == 1)
        {
            if(!search_sens_collection(addr, true))
            {
                for(int i = 0; i < MAX_SENSORS; i++)
                {
                    if(sens_collection[i].id == 0)
                    {
                        sens_collection[i].id = sens_collection[i-1].id + 1; 
                        sens_collection[i].address = addr;
                        sens_collection[i].isActive = true;
                    }
                }
            }
        }
        else
        {
            search_sens_collection(addr, false);
        }
        addr++;
    } while(addr % 10 != 0);
    curr_address = addr;
    return ESP_OK;
}

int search_sens_collection(int addr, int isActive)
{
    for ( int i = 0; i < MAX_SENSORS; i++)
    {
        if(sens_collection[i].address == addr)
        {
            sens_collection[i].isActive = isActive;
            return true;
        }
    }
    return false;    
}

int search_current_JSON_OBJ(sensor_t* sensor)
{
    cJSON *tmpObj;
    for (int n = 0 ; n < cJSON_GetArraySize(current_JSON_OBJ); n++)
    {   
        tmpObj = cJSON_GetArrayItem(current_JSON_OBJ, n);
        if(sensor->address == cJSON_GetObjectItem(tmpObj,"address")->valueint)
        {
            if(sensor->isActive)
            {
                cJSON_SetIntValue(cJSON_GetObjectItem(tmpObj,"temp") , sens_collection->temp);
                cJSON_SetIntValue(cJSON_GetObjectItem(tmpObj,"humidity") , sens_collection->humidity);
            }
            else
            {
                cJSON_DeleteItemFromArray(current_JSON_OBJ,n);
            }
            return true;
        }
    }
    return false;   
}