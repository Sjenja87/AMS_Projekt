#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "wifi.h"
#include "tempReader.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include "OLED.h"

#define MAX_OWB_DEVICES (8)
#define GPIO_DS18B20_PIN       (GPIO_NUM_27)

static const char *TAG = "main";

void app_main(void)
{

    wifi_init();

    // wait a bit before creating owb bus. Something with better readings acording to where the i got the code for the setup from.
    // implemented with help from https://github.com/DavidAntliff/esp32-ds18b20-example/blob/master/main/app_main.c
    vTaskDelay(2000.0 / portTICK_PERIOD_MS);

    // Setup for one wire bus (owb)
    ESP_LOGI(TAG, "creating owb in main.");
    OneWireBus * owb;
    owb_rmt_driver_info rmt_driver_info;
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_PIN, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true);
    OneWireBus_ROMCode device_rom_codes[MAX_OWB_DEVICES] = {0};
    DS18B20_Info devices[MAX_OWB_DEVICES] = {0};
    vTaskDelay(2000.0 / portTICK_PERIOD_MS); // just wait a bit before trying to search for sensors.
    ESP_LOGI(TAG, "looking for devices.");
    search_owb_for_sensors(owb, device_rom_codes, &devices);



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
        temp_sensors_read_task(&devices, sizeof(devices) / sizeof(devices[0]));
    }
    

    owb_uninitialize(owb);
}
