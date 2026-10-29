#ifndef BOARD_H
#define BOARD_H

/*
 * NUCLEO-C562RE Board Configuration
 * MCU: STM32C562RE (Cortex-M33, TrustZone)
 *
 * NOTE: Pin mappings and clock values are placeholders.
 * Update when the datasheet/reference manual is confirmed.
 */

/* HSE crystal (typical Nucleo: no HSE, uses HSI or MSI) */
/* TODO: Verify — some Nucleo boards use an 8 MHz bypass from ST-Link */
#define BOARD_HSE_FREQ      8000000U

/* System clock target — TODO: verify max frequency */
#define BOARD_SYSCLK_FREQ   160000000U

/* User LED — Nucleo-64 convention: LD2 on PA5 */
#define LED_USER_PIN        5U
#define LED_USER_PORT       GPIOA  /* TODO: verify with CMSIS headers */

/* User button — Nucleo-64 convention: B1 on PC13 */
#define USER_BTN_PIN        13U
#define USER_BTN_PORT       GPIOC  /* TODO: verify */

#endif /* BOARD_H */
