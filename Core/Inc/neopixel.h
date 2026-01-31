/*
 * WS2812 / NeoPixel driver for STM32 + TIM + DMA + FreeRTOS
 *
 * Copyright (c) 2026 Felipe Velázquez
 * Licensed under the MIT License
 *
 * GitHub: https://github.com/FelipeVelazquez/STM32_Neopixel_driver.git
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define NEOPIXEL_LED_COUNT 4   // Adjust to your neopixel max leds

void neopixel_init(void);
void neopixel_set_pixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
void neopixel_clear(void);
void neopixel_show(void);
