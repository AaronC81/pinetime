#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "qrcodegen.h"
#include "graphics.h"
#include "hl_bluetooth.h"
#include "touch.h"

#define BL_PORT DT_ALIAS_LED1_GPIOS_CONTROLLER
#define BL_PIN DT_ALIAS_LED1_GPIOS_PIN

#define BTN_PORT DT_ALIAS_SW0_GPIOS_CONTROLLER
#define BTN_PIN DT_ALIAS_SW0_GPIOS_PIN

#define BTN_ENABLE_PORT DT_ALIAS_BUTTON_ENABLE_GPIOS_CONTROLLER
#define BTN_ENABLE_PIN DT_ALIAS_BUTTON_ENABLE_GPIOS_PIN

// Enables the backlight.
void backlight_init(void) {
	struct device *dev = device_get_binding(BL_PORT);
	gpio_pin_configure(dev, BL_PIN, GPIO_DIR_OUT);
	gpio_pin_write(dev, BL_PIN, 0);
}

void button_init(void) {
	struct device *dev = device_get_binding(BTN_PORT);
	gpio_pin_configure(dev, BTN_PIN, GPIO_DIR_IN | GPIO_PUD_PULL_DOWN);

	dev = device_get_binding(BTN_ENABLE_PORT);
	gpio_pin_configure(dev, BTN_ENABLE_PIN, GPIO_DIR_OUT);
	gpio_pin_write(dev, BTN_ENABLE_PIN, 1);
}

int button_read(void) {
	struct device *dev = device_get_binding(BTN_PORT);
	int value;
	gpio_pin_read(dev, BTN_PIN, &value);
	return value;
}

#define qrprintf(ctx, ...) do { \
		char sprintfed[400]; \
		snprintf(sprintfed, 400, __VA_ARGS__); \
		graphics_clear_display(ctx); \
		graphics_draw_qr_code(ctx, 0, 0, sprintfed); \
	} while (0)

static struct graphics_context *global_ctx;

struct device *i2c_dev;
int read_flag;

void touch_pressed(struct device *gpiob, struct gpio_callback *cb, u32_t pins) {
	read_flag = true;
}

bool touch_raw_read(uint16_t *x, uint16_t *y) {
	uint8_t b[63] = { 0 };
	i2c_write(i2c_dev, b, 1, 0x15);
	int read_stat = i2c_read(i2c_dev, b, 63, 0x15);

	*x = ((b[3] & 0xF) << 8) | b[4];
	*y = ((b[5] & 0xF) << 8) | b[6];

	return (read_stat == 0);
}

void main(void) {
	struct device *display = device_get_binding(DT_INST_0_SITRONIX_ST7789V_LABEL);

	backlight_init();
	button_init();
	touch_init(touch_pressed);

	struct graphics_context ctx = graphics_init(display);
	global_ctx = &ctx;
	graphics_clear_display(&ctx);

	graphics_draw_rect(&ctx, 30, 30, 10, 10, DISPLAY_WHITE);
	
	i2c_dev = device_get_binding("I2C_1");
	i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER);
	
	while (true) {
		k_usleep(1);

		if (read_flag) {
			read_flag = false;

			uint16_t x, y;
			touch_raw_read(&x, &y);
			graphics_draw_rect(&ctx, x, y, 10, 10, DISPLAY_WHITE);
		}
	}
}
