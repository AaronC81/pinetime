#include <zephyr.h>
#include <device.h>
#include <display.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <string.h>
#include <stdio.h>

#include "qrcodegen.h"

#define BL_PORT DT_ALIAS_LED1_GPIOS_CONTROLLER
#define BL_PIN DT_ALIAS_LED1_GPIOS_PIN

// Display uses two-byte colour, RGB565
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define DISPLAY_DEPTH 2

#define DISPLAY_MAX_BUFFER_WIDTH DISPLAY_WIDTH / 4
#define DISPLAY_MAX_BUFFER_HEIGHT DISPLAY_HEIGHT / 4

#define DISPLAY_WHITE 0b1111111111111111
#define DISPLAY_BLACK 0

// Enables the backlight.
void backlight_init(void)
{
	struct device *dev = device_get_binding(BL_PORT);
	gpio_pin_configure(dev, BL_PIN, GPIO_DIR_OUT);
	gpio_pin_write(dev, BL_PIN, 0);
}

// Note: The display library works somewhat like a "stamp".
// Your buffer is the stamp, and you can resize it (within RAM limits) and 
// operate within it, before applying the changes to the screen.
// AFAIK, you can only use one at a time.

struct graphics_context {
	struct device *display;
	uint8_t *buffer;
	struct display_buffer_descriptor descriptor;
};

void graphics_blanking_off(struct graphics_context *ctx) {
	display_blanking_off(ctx->display);
}

void graphics_blanking_on(struct graphics_context *ctx) {
	display_blanking_on(ctx->display);
}

// Creates the graphics context.
struct graphics_context graphics_init(struct device *display) {
	// The buffer shall occupy a quarter of each dimension, a sixteenth overall
	uint16_t buffer_width = DISPLAY_MAX_BUFFER_WIDTH;
	uint16_t buffer_height = DISPLAY_MAX_BUFFER_HEIGHT;
	uint16_t buffer_size = buffer_width * buffer_height * DISPLAY_DEPTH;

	struct graphics_context ctx = {
		.display = display,
		.buffer = k_malloc(buffer_size),
		.descriptor = (struct display_buffer_descriptor){
			.buf_size = buffer_size,
			.pitch = buffer_width,
			.width = buffer_width,
			.height = buffer_height
		}
	};

	memset(ctx.buffer, 0, buffer_size);
	graphics_blanking_off(&ctx);

	return ctx;
}

// Resizes the graphics context buffer to the given dimensions. The area cannot
// be larger than a sixteenth of the display area. Returns a bool indicating
// if the given resize was possible.
// Note: The actual allocated buffer is not resized - it is merely the 
// width, height and pitch which the descriptor indicates.
bool graphics_resize_buffer(struct graphics_context *ctx, uint8_t width, uint8_t height) {
	if (width * height * 2 > ctx->descriptor.buf_size) return false;

	ctx->descriptor.width = width;
	ctx->descriptor.height = height;
	ctx->descriptor.pitch = width;

	return true;
}

// Clears the display.
void graphics_clear_display(struct graphics_context *ctx) {
	// Zero out the buffer and make sure it's the correct size
	memset(ctx->buffer, 0, ctx->descriptor.buf_size);
	graphics_resize_buffer(ctx, DISPLAY_MAX_BUFFER_WIDTH, DISPLAY_MAX_BUFFER_HEIGHT);

	// Tile this buffer over the screen
	for (uint8_t xc = 0; xc < 4; xc++) {
		for (uint8_t yc = 0; yc < 4; yc++) {
			display_write(ctx->display, xc * DISPLAY_MAX_BUFFER_WIDTH, yc * DISPLAY_MAX_BUFFER_HEIGHT, &ctx->descriptor, ctx->buffer);
		}
	}
}

// Fills the buffer with the given RGB565 colour.
void graphics_fill_buffer(struct graphics_context *ctx, uint16_t colour) {
	memset(ctx->buffer, colour, ctx->descriptor.buf_size);
}

// Applies the buffer to the display at a given location.
void graphics_write_buffer(struct graphics_context *ctx, uint16_t x, uint16_t y) {
	display_write(ctx->display, x, y, &ctx->descriptor, ctx->buffer);
}

// Draws a rectangle onto the display. The rectangle cannot have an area larger
// than a sixteenth of the display.
void graphics_draw_rect_fast(struct graphics_context *ctx, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour) {
	graphics_resize_buffer(ctx, w, h);
	graphics_fill_buffer(ctx, colour);
	graphics_write_buffer(ctx, x, y);
}

// Draws a rectangle of any size onto the display.
void graphics_draw_rect(struct graphics_context *ctx, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour) {
	// Tile along each row
	uint16_t remaining_height = h;
	while (remaining_height > 0) {
		uint16_t this_row_height = MIN(remaining_height, DISPLAY_MAX_BUFFER_HEIGHT);

		uint16_t remaining_width = w;
		uint16_t current_x = x;
		while (remaining_width > 0) {
			uint16_t this_buffer_width = MIN(remaining_width, DISPLAY_MAX_BUFFER_WIDTH);
			graphics_draw_rect_fast(ctx, current_x, y, this_buffer_width, this_row_height, colour);
			current_x += this_buffer_width;
			remaining_width -= this_buffer_width;
		}

		y += this_row_height;
		remaining_height -= this_row_height;
	}
}

#define GRAPHICS_QR_CODE_MAX_VERSION 10
#define GRAPHICS_QR_CODE_BORDER_SIZE 5
#define GRAPHICS_QR_CODE_SCALE 4

uint8_t graphics_qr_code_result_buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(GRAPHICS_QR_CODE_MAX_VERSION)];
uint8_t graphics_qr_code_temp_buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(GRAPHICS_QR_CODE_MAX_VERSION)];

void graphics_draw_qr_code(struct graphics_context *ctx, uint8_t x, uint8_t y, char *data) {
	bool qr_result = qrcodegen_encodeText(
		data,
		graphics_qr_code_temp_buffer,
		graphics_qr_code_result_buffer,
		qrcodegen_Ecc_LOW,
		1,
		GRAPHICS_QR_CODE_MAX_VERSION,
		qrcodegen_Mask_0,
		false
	);

	if (!qr_result) return;
	int qr_size = qrcodegen_getSize(graphics_qr_code_result_buffer);

	graphics_draw_rect(
		ctx, x, y,
		qr_size * GRAPHICS_QR_CODE_SCALE + GRAPHICS_QR_CODE_BORDER_SIZE * 2,
		qr_size * GRAPHICS_QR_CODE_SCALE + GRAPHICS_QR_CODE_BORDER_SIZE * 2,
		DISPLAY_WHITE
	);
	x += GRAPHICS_QR_CODE_BORDER_SIZE;
	y += GRAPHICS_QR_CODE_BORDER_SIZE;

	for (int qy = 0; qy < qr_size; qy++) {
		for (int qx = 0; qx < qr_size; qx++) {
			uint16_t colour = qrcodegen_getModule(graphics_qr_code_result_buffer, qx, qy)
				? DISPLAY_BLACK : DISPLAY_WHITE;
			graphics_draw_rect_fast(
				ctx,
				x + GRAPHICS_QR_CODE_SCALE * qx,
				y + GRAPHICS_QR_CODE_SCALE * qy,
				GRAPHICS_QR_CODE_SCALE, GRAPHICS_QR_CODE_SCALE,
				colour
			);
		}
	}
}

#define qrprintf(ctx, ...) do { \
		char sprintfed[400]; \
		snprintf(sprintfed, 400, __VA_ARGS__); \
		graphics_clear_display(ctx); \
		graphics_draw_qr_code(ctx, 0, 0, sprintfed); \
	} while (0)

void main(void)
{	
	struct device *display = device_get_binding(DT_INST_0_SITRONIX_ST7789V_LABEL);

	backlight_init();

	struct graphics_context ctx = graphics_init(display);
	graphics_clear_display(&ctx);

	struct device *i2c_dev = device_get_binding("I2C_1");
	uint8_t cfg_res = i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER);

	qrprintf(&ctx, "dev: %x, cfg: %x", (unsigned int*)i2c_dev, (unsigned int*)cfg_res);

	while (1) {	
		k_sleep(100);
	}
}
