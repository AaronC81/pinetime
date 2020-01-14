#pragma once

#include <stdint.h>
#include <zephyr.h>
#include <gpio.h>

#define TOUCH_INT_PORT DT_ALIAS_TOUCH_INTERRUPT_GPIOS_CONTROLLER
#define TOUCH_INT_PIN DT_ALIAS_TOUCH_INTERRUPT_GPIOS_PIN

void touch_init(void cb(struct device *gpiob, struct gpio_callback *cb, uint32_t pins));