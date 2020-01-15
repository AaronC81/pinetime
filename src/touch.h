#pragma once

#include <stdint.h>
#include <zephyr.h>
#include <gpio.h>
#include <stdbool.h>

#define TOUCH_INT_PORT DT_ALIAS_TOUCH_INTERRUPT_GPIOS_CONTROLLER
#define TOUCH_INT_PIN DT_ALIAS_TOUCH_INTERRUPT_GPIOS_PIN

void touch_init(void);
bool touch_get(uint16_t *x, uint16_t *y);