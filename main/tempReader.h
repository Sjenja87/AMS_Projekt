#ifndef _TEMPREADER_H_
#define _TEMPREADER_H_

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

void search_owb_for_sensors();
void temp_sensors_read();
void init_owb_for_sensors();

#endif