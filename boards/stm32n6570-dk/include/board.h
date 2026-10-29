#ifndef BOARD_H
#define BOARD_H

/*
 * STM32N6570-DK Board Configuration
 * MCU: STM32N657xx (Cortex-M55)
 *
 * NOTE: Pin assignments should be verified against
 * the STM32N6570-DK user manual and STM32N6 reference manual.
 */

/* HSE crystal */
#define BOARD_HSE_FREQ 48000000U /* 48 MHz schematic-confirmed for STM32n6570 discovery kit */

/* CPU and BUS - two independently-clocked domains
 * fed by different ICs off the same PLL
 */
#define BOARD_CPU_FREQ 576000000U /* 576 MHz */
#define BOARD_BUS_FREQ 192000000U /* 192 MHz */

/* User LED — verified pins from board schematic */
/* LED GREEN - GPIOO Pin 01, Active HIGH*/
#define LED_GREEN_PIN  1U
#define LED_GREEN_PORT GPIOO

/* LED RED - GPIOG Pin 10, Active LOW*/
#define LED_RED_PIN  10U
#define LED_RED_PORT GPIOG

/* User button — verified pins from board schematic */
#define USER_BTN_PIN  13U
#define USER_BTN_PORT GPIOC

#endif /* BOARD_H */
