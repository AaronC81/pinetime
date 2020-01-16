#pragma once

#include <stdint.h>
#include <zephyr.h>
#include <device.h>
#include <display.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>

// Display uses two-byte colour, RGB565
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define DISPLAY_DEPTH 2

#define DISPLAY_MAX_BUFFER_WIDTH DISPLAY_WIDTH / 4
#define DISPLAY_MAX_BUFFER_HEIGHT DISPLAY_HEIGHT / 4

#define DISPLAY_WHITE 0b1111111111111111
#define DISPLAY_BLACK 0

struct graphics_context {
    struct device *display;
    uint8_t *buffer;
    struct display_buffer_descriptor descriptor;
};

void graphics_blanking_off(struct graphics_context *ctx);
void graphics_blanking_on(struct graphics_context *ctx);

struct graphics_context graphics_init(struct device *display);

bool graphics_resize_buffer(struct graphics_context *ctx, uint8_t width, uint8_t height);
void graphics_fill_buffer(struct graphics_context *ctx, uint16_t colour);
void graphics_write_buffer(struct graphics_context *ctx, uint16_t x, uint16_t y);
void graphics_draw_rect_fast(struct graphics_context *ctx, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour);
void graphics_draw_rect(struct graphics_context *ctx, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour);
void graphics_draw_qr_code(struct graphics_context *ctx, uint8_t x, uint8_t y, char *data);

void graphics_clear_display(struct graphics_context *ctx);

void graphics_text_write_char(struct graphics_context *ctx, uint16_t x, uint16_t y, char character, uint16_t fg_colour, uint16_t bg_colour);
void graphics_text_write_string(struct graphics_context *ctx, uint16_t x, uint16_t y, char* str, uint16_t fg_colour, uint16_t bg_colour);