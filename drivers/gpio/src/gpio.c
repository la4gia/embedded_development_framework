/**
 * @file gpio.c
 * @brief GPIO driver implementation — direct register access
 *
 * Uses a local register struct that matches the STM32 GPIO peripheral layout.
 * This avoids needing a specific CMSIS header at compile time while remaining
 * compatible with all STM32 families that use this register layout.
 */

#include "gpio.h"

/*
 * STM32 GPIO register layout (common across F4, F7, L4, G4, H7, N6, etc.)
 * Offsets verified against RM0090 (F4), RM0433 (H7), RM0481 (N6).
 */
typedef struct {
    volatile uint32_t MODER;   /* 0x00 — Mode register */
    volatile uint32_t OTYPER;  /* 0x04 — Output type register */
    volatile uint32_t OSPEEDR; /* 0x08 — Output speed register */
    volatile uint32_t PUPDR;   /* 0x0C — Pull-up/pull-down register */
    volatile uint32_t IDR;     /* 0x10 — Input data register */
    volatile uint32_t ODR;     /* 0x14 — Output data register */
    volatile uint32_t BSRR;    /* 0x18 — Bit set/reset register */
    volatile uint32_t LCKR;    /* 0x1C — Lock register */
    volatile uint32_t AFR[2];  /* 0x20 — Alternate function (low [0], high [1]) */
} gpio_regs_t;

void gpio_init(void *port, uint8_t pin, const gpio_config_t *config)
{
    gpio_regs_t *gpio = (gpio_regs_t *)port;

    /* Mode (2 bits per pin) */
    gpio->MODER &= ~(0x03UL << (pin * 2));
    gpio->MODER |= ((uint32_t)config->mode << (pin * 2));

    /* Output type (1 bit per pin) */
    gpio->OTYPER &= ~(0x01UL << pin);
    gpio->OTYPER |= ((uint32_t)config->otype << pin);

    /* Speed (2 bits per pin) */
    gpio->OSPEEDR &= ~(0x03UL << (pin * 2));
    gpio->OSPEEDR |= ((uint32_t)config->speed << (pin * 2));

    /* Pull-up/pull-down (2 bits per pin) */
    gpio->PUPDR &= ~(0x03UL << (pin * 2));
    gpio->PUPDR |= ((uint32_t)config->pull << (pin * 2));

    /* Alternate function (4 bits per pin, split across AFR[0] and AFR[1]) */
    if (config->mode == GPIO_MODE_AF) {
        uint8_t idx = pin / 8;
        uint8_t pos = (pin % 8) * 4;
        gpio->AFR[idx] &= ~(0x0FUL << pos);
        gpio->AFR[idx] |= ((uint32_t)config->af << pos);
    }
}

void gpio_set(void *port, uint8_t pin)
{
    gpio_regs_t *gpio = (gpio_regs_t *)port;
    gpio->BSRR = (1UL << pin);
}

void gpio_clear(void *port, uint8_t pin)
{
    gpio_regs_t *gpio = (gpio_regs_t *)port;
    gpio->BSRR = (1UL << (pin + 16));
}

void gpio_toggle(void *port, uint8_t pin)
{
    gpio_regs_t *gpio = (gpio_regs_t *)port;
    gpio->ODR ^= (1UL << pin);
}

uint8_t gpio_read(void *port, uint8_t pin)
{
    gpio_regs_t *gpio = (gpio_regs_t *)port;
    return (uint8_t)((gpio->IDR >> pin) & 0x01U);
}
