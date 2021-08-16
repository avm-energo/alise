#pragma once

#ifndef DEBUG
#ifdef __OPTIMIZE__
#define DEBUG
#endif
#endif

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif
const static char *gpio_device_prefix = "/dev/gpiochip";
void gpio_list(const char *dev_name);
uint8_t gpio_read(uint8_t dev_name, int offset);
uint8_t gpio_read_detail(const char *dev_name, int offset);
void gpio_write(uint8_t dev_name, int offset, uint8_t value);
void gpio_write_detail(const char *dev_name, int offset, uint8_t value);
#ifdef __cplusplus
}
#endif
