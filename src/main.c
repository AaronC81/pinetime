#include <string.h>
#include <stdio.h>

#include "qrcodegen.h"
#include "graphics.h"
#include "hl_bluetooth.h"

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

	struct graphics_context ctx = graphics_init(display);
	global_ctx = &ctx;
	graphics_clear_display(&ctx);

	struct device *i2c_dev = device_get_binding("I2C_1");
	uint8_t cfg_res = i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER);

	qrprintf(&ctx, "Starting...");

	hl_bluetooth_init();

	qrprintf(&ctx, "Bluetooth ready!");
	
	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(MSEC_PER_SEC * 5);

		/* Heartrate measurements simulation */
		hl_bluetooth_hrs_notify(69);

		/* Battery level simulation */
		hl_bluetooth_bas_notify(69);

		uint8_t* ct = fetch_ct();
		qrprintf(&ctx, "%d %d %d %d %d %d %d %d %d %d",
			ct[0], ct[1], ct[2], ct[3], ct[4], ct[5], ct[6], ct[7], ct[8], ct[9]);

		cts_notify();
	}
}
