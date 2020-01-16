#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "qrcodegen.h"
#include "graphics.h"
#include "hl_bluetooth.h"
#include "touch.h"
#include "i2console.h"

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

void main(void) {
	struct device *display = device_get_binding(DT_INST_0_SITRONIX_ST7789V_LABEL);

	backlight_init();
	button_init();
	touch_init();

	struct graphics_context ctx = graphics_init(display);
	global_ctx = &ctx;
	graphics_clear_display(&ctx);

	LOG("Starting!");

	graphics_draw_rect(&ctx, 0, 0, 10, 10, DISPLAY_WHITE);
	graphics_clear_display(&ctx);

	graphics_text_write_string(&ctx, "Hello!", 30, 30);
	
	while (true) {
		k_usleep(1);

		uint16_t x, y;
		if (touch_get(&x, &y)) {
			graphics_draw_rect(&ctx, x, y, 10, 10, DISPLAY_WHITE);
		}
	}
}
