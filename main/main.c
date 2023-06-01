#include "freertos/FreeRTOS.h"
#include "mdns_ams.h"
#include "wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/apps/netbiosns.h"
#include "rest_server.h"
#include "sd_storage.h"
#include "sensors.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

#define FIND_SENSOR_INTERVAL 10

static const char *TAG = "Main";

const TickType_t xDelay = 2500 / portTICK_PERIOD_MS;

void sensorTask(void* arg)
{   int find_sensor_tick = 0;
    
    while(1)
    {
        get_sensors_data();

        if(find_sensor_tick >= FIND_SENSOR_INTERVAL)
        {
            find_new_sensors();
            find_sensor_tick = 0;
        }
        else
        {
            vTaskDelay(xDelay); 
        }
        find_sensor_tick++;
    }
}

void app_main(void)
{
#ifdef CONFIG_USE_SD_STORAGE
    init_fs_sdcard();
#endif
    wifi_init();
    init_mdns();
    netbiosns_init();
    netbiosns_set_name(CONFIG_MDNS_HOST_NAME);   
    init_sensors();

    //xTaskCreatePinnedToCore(sensorTask, "sensorTask", 4096, NULL, 1, NULL, 0);
}
