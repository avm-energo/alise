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
#define GPIO_PREFIX "/dev/gpiochip"
void gpio_list(const char *dev_name);
uint8_t gpio_read(uint8_t dev_name, uint32_t offset);
uint8_t gpio_read_detail(const char *dev_name, uint32_t offset);
void gpio_write(uint8_t dev_name, uint32_t offset, uint8_t value);
void gpio_write_detail(const char *dev_name, uint32_t offset, uint8_t value);
#ifdef __cplusplus
}
#endif
