#ifndef _SD_STORAGE_H_
#define _SD_STORAGE_H_

#include "esp_err.h"

#define SD_MOUNT_POINT "/sdcard"

esp_err_t init_fs_sdcard(void);

esp_err_t append_to_file(char* file_path, char* data);

esp_err_t read_last_save_from_file(char* file_path, char* data, char stopChar);

#endif