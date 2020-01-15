#include "touch.h"

#include <i2c.h>

struct gpio_callback _touch_callback;
struct device *_i2c_dev;
bool _touch_available_flag;

// A callback invoked immediately when the device detects a touch.
void _touch_pressed_cb(struct device *gpiob, struct gpio_callback *cb, u32_t pins) {
	_touch_available_flag = true;
}

// Initialises the touch controller.
void touch_init(void) {
	// Configure GPIO interrupt
	struct device *gpio_dev = device_get_binding(TOUCH_INT_PORT);
	gpio_pin_configure(gpio_dev, TOUCH_INT_PIN, GPIO_DIR_IN | GPIO_INT);
	
	gpio_init_callback(&_touch_callback, _touch_pressed_cb, BIT(TOUCH_INT_PIN));
	gpio_add_callback(gpio_dev, &_touch_callback);
	gpio_pin_enable_callback(gpio_dev, TOUCH_INT_PIN);

	// Configure I2C for reading touch data
	_i2c_dev = device_get_binding("I2C_1");
	i2c_configure(_i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER);
}

// Reads data for a touch from the touch controller, writing the x and y
// coordinates of the touch into the given x and y pointers. Returns a boolean
// indicating whether the I2C read was successful.
bool _touch_read(uint16_t *x, uint16_t *y) {
	// Initialise a buffer large enough for the data we'll read, but keep a zero
	// in it which we then write to prepare the touch controller for reading
	uint8_t b[63] = { 0 };
	i2c_write(_i2c_dev, b, 1, 0x15);
	
	// Read all 63 data bytes
	int read_stat = i2c_read(_i2c_dev, b, 63, 0x15);

	// Construct x and y from their bytes
	*x = ((b[3] & 0xF) << 8) | b[4];
	*y = ((b[5] & 0xF) << 8) | b[6];

	// Return a boolean indicating whether read was successful
	return (read_stat == 0);
}

// If a touch was detected, writes the x and y of the touch into the given
// pointers and returns true, otherwise returns false. Call this from your UI
// event loop to handle input.
bool touch_get(uint16_t *x, uint16_t *y) {
	if (_touch_available_flag) {
		_touch_available_flag = false;
		_touch_read(x, y);
		return true;
	}
	return false;
}
