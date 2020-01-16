#pragma once

#include <stdint.h>

void i2console_write_byte(uint8_t byte);
void i2console_write(const char* str);

#define LOG(...) do { \
        char sprintfed[1024]; \
        snprintf(sprintfed, 1024, __VA_ARGS__); \
        i2console_write(sprintfed); \
    } while (0)
