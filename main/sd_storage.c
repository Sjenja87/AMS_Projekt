#include "driver/gpio.h"
#include "sd_storage.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdkconfig.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>


static const char *TAG = "SD_Storage";

#define PIN_NUM_MISO  2
#define PIN_NUM_MOSI  15
#define PIN_NUM_CLK   14
#define PIN_NUM_CS    13

sdmmc_card_t *card;

sdmmc_host_t host = SDSPI_HOST_DEFAULT();

esp_err_t init_fs_sdcard(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing SD card with SPI peripheral");

    gpio_set_pull_mode(PIN_NUM_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_NUM_MISO, GPIO_PULLUP_ONLY); 
  
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    /* print card info if mount successfully */
    sdmmc_card_print_info(stdout, card);
 
    return ret;
}

esp_err_t append_to_file(char* file_path, char* data)
{

    FILE *f = fopen(file_path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_ERR_NOT_FOUND;
    } 
    fwrite(data ,strlen(data) + 1, 1, f);
    fclose(f);
    ESP_LOGI(TAG, "Data written to file");

    return ESP_OK;
}

esp_err_t read_last_save_from_file(char* file_path, char* data, char stopChar)
{
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_ERR_NOT_FOUND;
    }
    fseek(f,-1, SEEK_END);
    int lenth = 0;
    while(fgetc(f) != stopChar)
    {   
        lenth++;
        fseek(f,-2, SEEK_CUR);
    }
    fseek(f,-1, SEEK_CUR);
    fread(data, lenth+1, 1, f);
    fclose(f);
    return ESP_OK;
}
