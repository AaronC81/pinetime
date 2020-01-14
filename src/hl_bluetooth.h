#pragma once

#include <stdint.h>

void hl_bluetooth_init(void);
void hl_bluetooth_bas_notify(uint8_t battery_level);
void hl_bluetooth_hrs_notify(uint16_t heart_rate);

void cts_notify(void);
uint8_t* fetch_ct();