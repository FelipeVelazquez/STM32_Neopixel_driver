/*
 * WS2812 / NeoPixel driver for STM32 + TIM + DMA + FreeRTOS
 *
 * Copyright (c) 2026 Felipe Velázquez
 * Licensed under the MIT License
 *
 * GitHub: https://github.com/FelipeVelazquez/STM32_Neopixel_driver.git
 */

#include "neopixel.h"
#include "tim.h"
#include <string.h>
#include <stdbool.h>

/*
 * TIM1:
 * APB2 = 108 Mhz
 * Prescaler = 0
 * ARR = 135  -> periodo ≈ 1.28 µs (≈ 800 kHz)
 *
 * WS2812 timings:
 * 0: HIGH ≈ 0.35 µs
 * 1: HIGH ≈ 0.7  µs
 *
 * Note:
 * 1.- Considerate enable PLLCLK for APB2 & use HSE on the PLL Source Mux
 * 2.- On TIMx enable PWM Generation CH1 & use you ARR on Counter Peroid Settings
 * 3.- Also enable DMA (Direction Memory -> Peripheral / Priority Very High / Mode Normal /Increment Address Memory only / Data Width Half Word)
 */

#define TIMER_ARR   135 //APB2_Freq/0.8 = 135 Use your APB2 freq

#define T0H  ((uint16_t)(TIMER_ARR * 0.32))  // ≈ 10
#define T1H  ((uint16_t)(TIMER_ARR * 0.64))  // ≈ 20

#define RESET_SLOTS  200   // >50 µs for LOW garantee
#define BITS_PER_LED 24
#define BUFFER_SIZE  (NEOPIXEL_LED_COUNT * BITS_PER_LED + RESET_SLOTS)

static uint16_t pwm_buffer[BUFFER_SIZE];
static uint8_t  led_data[NEOPIXEL_LED_COUNT][3]; // GRB

static volatile bool neopixel_busy = false;

/* ------------------------------------------------------------ */
void neopixel_init(void)
{

    memset(led_data, 0, sizeof(led_data));
    neopixel_show();

}

void neopixel_set_pixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx >= NEOPIXEL_LED_COUNT) return;

    // For WS2812 = GRB
    led_data[idx][0] = g;
    led_data[idx][1] = r;
    led_data[idx][2] = b;
}

void neopixel_clear(void)
{
    memset(led_data, 0, sizeof(led_data));
}

void neopixel_show(void)
{
    // wait if DMA is busy
    while (neopixel_busy)
        osDelay(1);

    neopixel_busy = true;

    uint32_t i = 0;

    for (uint16_t led = 0; led < NEOPIXEL_LED_COUNT; led++)
    {
        for (uint8_t color = 0; color < 3; color++)
        {
            uint8_t v = led_data[led][color];
            for (int8_t bit = 7; bit >= 0; bit--)
            {
                pwm_buffer[i++] = (v & (1 << bit)) ? T1H : T0H;
            }
        }
    }

    // RESET / LATCH: LOW hold
    for (; i < BUFFER_SIZE; i++)
        pwm_buffer[i] = 0;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

    HAL_TIM_PWM_Start_DMA(
        &htim1,
        TIM_CHANNEL_1,
        (uint32_t *)pwm_buffer,
        BUFFER_SIZE
    );
}

/* ------------------------------------------------------------ */

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        neopixel_busy = false;
    }
}
