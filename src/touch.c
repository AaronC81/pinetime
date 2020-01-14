#include "touch.h"

struct gpio_callback _touch_callback;

void touch_init(void cb(struct device *gpiob, struct gpio_callback *cb, uint32_t pins)) {
    struct device *dev = device_get_binding(TOUCH_INT_PORT);
	gpio_pin_configure(dev, TOUCH_INT_PIN, GPIO_DIR_IN | GPIO_INT);
	
	gpio_init_callback(&_touch_callback, cb, BIT(TOUCH_INT_PIN));
	gpio_add_callback(dev, &_touch_callback);
	gpio_pin_enable_callback(dev, TOUCH_INT_PIN);
}