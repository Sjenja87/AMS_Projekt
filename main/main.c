#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "wifi.h"
#include "tempReader.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include "OLED.h"




static const char *TAG = "main";

void app_main(void)
{

    //wifi_init();

    // wait a bit before creating owb bus. Something with better readings acording to where the i got the code for the setup from.
    // implemented with help from https://github.com/DavidAntliff/esp32-ds18b20-example/blob/master/main/app_main.c
    vTaskDelay(2000.0 / portTICK_PERIOD_MS);

    // Setup for one wire bus (owb)
    ESP_LOGI(TAG, "creating owb in main.");

    init_owb_for_sensors();
    vTaskDelay(2000.0 / portTICK_PERIOD_MS); // just wait a bit before trying to search for sensors.
    ESP_LOGI(TAG, "looking for devices.");
    search_owb_for_sensors();



    init_i2c_driver();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    init_OLED();
    vTaskDelay(5000/portTICK_PERIOD_MS);

    while (true)
    {
        xTaskCreate(&clear_oled_task, "clear_screen",  2048, NULL, 6, NULL);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        xTaskCreate(&oled_text_task, "oled_text_task",  2048, (void *)"Hello world!\nMulitine is OK!\nAnother line", 6, NULL);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        temp_sensors_read();
    }
    

    owb_uninitialize(owb);
}
