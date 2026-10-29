#ifndef BOARD_H
#define BOARD_H

/*
 * STM32F407G-DISC1 Board Configuration
 * MCU: STM32F407VGT6 (Cortex-M4, 1MB Flash, 192KB RAM)
 */

/* HSE crystal: 8 MHz */
#define BOARD_HSE_FREQ      8000000U

/* System clock target: 168 MHz */
#define BOARD_SYSCLK_FREQ   168000000U

/* AHB clock: 168 MHz */
#define BOARD_HCLK_FREQ     168000000U

/* APB1 clock: 42 MHz (max for APB1) */
#define BOARD_PCLK1_FREQ    42000000U

/* APB2 clock: 84 MHz (max for APB2) */
#define BOARD_PCLK2_FREQ    84000000U

/* PLL configuration (HSE = 8 MHz → SYSCLK = 168 MHz) */
#define BOARD_PLL_M         8U
#define BOARD_PLL_N         336U
#define BOARD_PLL_P         2U    /* PLLP divider value (register: 0b00) */
#define BOARD_PLL_Q         7U

/* User LEDs — GPIOD pins 12-15 */
#define LED_GREEN_PIN       12U
#define LED_ORANGE_PIN      13U
#define LED_RED_PIN         14U
#define LED_BLUE_PIN        15U
#define LED_PORT            GPIOD

/* User button — GPIOA pin 0 */
#define USER_BTN_PIN        0U
#define USER_BTN_PORT       GPIOA

#endif /* BOARD_H */
