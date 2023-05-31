#ifndef _TEMPREADER_H_
#define _TEMPREADER_H_

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

void search_owb_for_sensors(OneWireBus* owb, OneWireBus_ROMCode* device_rom_codes, DS18B20_Info* devices[]);
void temp_sensors_read_task(DS18B20_Info* devices[], int num_devices);










#endif