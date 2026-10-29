#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/**
 * @file gpio.h
 * @brief Bare-metal GPIO driver (register-level, no HAL)
 *
 * Works across STM32 families that share the same GPIO register layout
 * (F4, F7, L4, G4, H7, N6, C5, etc.)
 *
 * Usage:
 *   #include "stm32f407xx.h"  // for GPIOD, RCC, etc.
 *   #include "gpio.h"
 *
 *   gpio_config_t cfg = { .mode = GPIO_MODE_OUTPUT, .speed = GPIO_SPEED_LOW };
 *   gpio_init(GPIOD, 12, &cfg);
 *   gpio_set(GPIOD, 12);
 */

typedef enum {
    GPIO_MODE_INPUT  = 0x00,
    GPIO_MODE_OUTPUT = 0x01,
    GPIO_MODE_AF     = 0x02,
    GPIO_MODE_ANALOG = 0x03
} gpio_mode_t;

typedef enum {
    GPIO_SPEED_LOW    = 0x00,
    GPIO_SPEED_MEDIUM = 0x01,
    GPIO_SPEED_FAST   = 0x02,
    GPIO_SPEED_HIGH   = 0x03
} gpio_speed_t;

typedef enum {
    GPIO_PULL_NONE = 0x00,
    GPIO_PULL_UP   = 0x01,
    GPIO_PULL_DOWN = 0x02
} gpio_pull_t;

typedef enum {
    GPIO_OTYPE_PP = 0x00, /* Push-pull */
    GPIO_OTYPE_OD = 0x01  /* Open-drain */
} gpio_otype_t;

typedef struct {
    gpio_mode_t  mode;
    gpio_speed_t speed;
    gpio_pull_t  pull;
    gpio_otype_t otype;
    uint8_t      af; /* Alternate function number (0-15), used when mode == GPIO_MODE_AF */
} gpio_config_t;

/**
 * @brief Initialize a GPIO pin
 * @param port  Pointer to GPIO peripheral (e.g., GPIOD from CMSIS)
 * @param pin   Pin number (0-15)
 * @param config  Pin configuration
 */
void gpio_init(void *port, uint8_t pin, const gpio_config_t *config);

/** @brief Set pin output high (via BSRR — atomic) */
void gpio_set(void *port, uint8_t pin);

/** @brief Set pin output low (via BSRR — atomic) */
void gpio_clear(void *port, uint8_t pin);

/** @brief Toggle pin output (via ODR — not atomic) */
void gpio_toggle(void *port, uint8_t pin);

/** @brief Read pin input state
 *  @return 0 or 1
 */
uint8_t gpio_read(void *port, uint8_t pin);

#endif /* GPIO_H */
