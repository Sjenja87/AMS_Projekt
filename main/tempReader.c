#include "tempReader.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#define GPIO_DS18B20_PIN  (GPIO_NUM_27)
#define DS18B20_RESOLUTION   (DS18B20_RESOLUTION_9_BIT) // since this is a DS1820 and not DS18b20 it can only read in 9 bit. 
#define SAMPLE_PERIOD        (1000)   // milliseconds
#define MAX_OWB_DEVICES (8)

OneWireBus * owb;
owb_rmt_driver_info rmt_driver_info;
OneWireBus_ROMCode device_rom_codes[MAX_OWB_DEVICES] = {0};
DS18B20_Info* devices[MAX_OWB_DEVICES] = {0};

static const char *TAG = "tempReader";

void init_owb_for_sensors()
{
    ESP_LOGI(TAG, "creating owb in main.");
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_PIN, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true);
}

// implemented with help from https://github.com/DavidAntliff/esp32-ds18b20-example/blob/master/main/app_main.c

void search_owb_for_sensors(){
    ESP_LOGI(TAG,"Looking for 1-wire divices...");
    int num_devices = 0;
    OneWireBus_SearchState search = {0};
    bool found = false;
    owb_search_first(owb, &search, &found);
    while (found)
    {
        char rom_codes[17];
        owb_string_from_rom_code(search.rom_code, rom_codes, sizeof(rom_codes));
        ESP_LOGI(TAG, "Device was found: %s", rom_codes);
        device_rom_codes[num_devices] = search.rom_code;
        ++num_devices;
        owb_search_next(owb, &search, &found);
    }
    ESP_LOGI(TAG, "Found %d device%s\n", num_devices, num_devices == 1 ? "" : "s");

    for (int i = 0; i < num_devices; ++i)
    {
        DS18B20_Info * ds18b20_info = ds18b20_malloc();  // allokerer memmory. 
        devices[i] = ds18b20_info;

        if (num_devices == 1)
        {
            ESP_LOGI(TAG, "Single device optimisations enabled\n");
            ds18b20_init_solo(ds18b20_info, owb);          // only one device on bus
        }
        else
        {
            ESP_LOGI(TAG, "Initializing device %i", i);
            ds18b20_init(ds18b20_info, owb, device_rom_codes[i]); // associate with bus and device
        }
        ds18b20_use_crc(ds18b20_info, true);           // enable CRC check on all reads
        ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
    }
    
};

void temp_sensors_read(){
float temp[MAX_OWB_DEVICES];
    ESP_LOGI(TAG,"Temperature readings (degrees C) from %d devices:\n", num_devices);
    for (int i = 0; i < MAX_OWB_DEVICES; ++i){
        if(devices[i] != NULL){
                ds18b20_read_temp(devices[i], &temp[i]);
                ESP_LOGI(TAG," %d: %.3f\n", i, temp[i]);
                
        }
    }
}