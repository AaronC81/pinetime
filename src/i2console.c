#include "i2console.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <i2c.h>

void i2console_write_byte(uint8_t byte) {
    struct device *i2c_dev = device_get_binding("I2C_1");
    for (int i = 0; i < 8; i++) {
        if (byte & (0b10000000 >> i)) {
            i2c_read(i2c_dev, NULL, 0, 0x42);
        } else {
            i2c_write(i2c_dev, NULL, 0, 0x42);
        }
    }
}

void i2console_write(const char* str) {
    uint32_t i = 0;
    char c;
    while (c = str[i++]) {
        i2console_write_byte((uint8_t)c);
    }
}
