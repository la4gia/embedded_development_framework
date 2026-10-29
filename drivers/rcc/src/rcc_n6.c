/**
 * @file rcc_n6.c
 * @brief RCC driver implementation — STM32N6 register layout
 *
 * GPIO clock gating lives in RCC_AHB4ENR (one bit per port, ports A-H at
 * bits 0-7, then non-contiguous: N/O/P/Q at bits 13-16 — no I/J/K/L/M).
 * Unlike F4, this family also has atomic write-1-to-set/clear alias
 * registers (RCC_AHB4ENSR/RCC_AHB4ENCR) — same pattern already used for
 * the IC divider enables in SystemInit(). Prefer those aliases
 * over read-modify-write on RCC_AHB4ENR directly.
 * The public interface (rcc.h) is identical across families; the board's
 * CMakeLists selects which one gets compiled (drivers/rcc/CMakeLists.txt).
 */

#include "rcc.h"
#include "stm32n657xx.h"

void RCC_EnableGPIOClock(void *port)
{
    GPIO_TypeDef *p = (GPIO_TypeDef *)port;

    if (p == GPIOA) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOAENS;
    } else if (p == GPIOB) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOBENS;
    } else if (p == GPIOC) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOCENS;
    } else if (p == GPIOD) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIODENS;
    } else if (p == GPIOE) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOEENS;
    } else if (p == GPIOF) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOFENS;
    } else if (p == GPIOG) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOGENS;
    } else if (p == GPIOH) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOHENS;
    } else if (p == GPION) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIONENS;
    } else if (p == GPIOO) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOOENS;
    } else if (p == GPIOP) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOPENS;
    } else if (p == GPIOQ) {
        RCC->AHB4ENSR |= RCC_AHB4ENSR_GPIOQENS;
    }

    (void)RCC->AHB4ENR;
}

void RCC_DisableGPIOClock(void *port)
{
    GPIO_TypeDef *p = (GPIO_TypeDef *)port;

    if (p == GPIOA) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOAENC;
    } else if (p == GPIOB) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOBENC;
    } else if (p == GPIOC) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOCENC;
    } else if (p == GPIOD) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIODENC;
    } else if (p == GPIOE) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOEENC;
    } else if (p == GPIOF) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOFENC;
    } else if (p == GPIOG) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOGENC;
    } else if (p == GPIOH) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOHENC;
    } else if (p == GPION) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIONENC;
    } else if (p == GPIOO) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOOENC;
    } else if (p == GPIOP) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOPENC;
    } else if (p == GPIOQ) {
        RCC->AHB4ENCR |= RCC_AHB4ENCR_GPIOQENC;
    }

    (void)RCC->AHB4ENR;
}

void RCC_ResetGPIO(void *port)
{
    GPIO_TypeDef *p = (GPIO_TypeDef *)port;

    if (p == GPIOA) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOARSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOARSTC;
    } else if (p == GPIOB) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOBRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOBRSTC;
    } else if (p == GPIOC) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOCRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOCRSTC;
    } else if (p == GPIOD) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIODRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIODRSTC;
    } else if (p == GPIOE) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOERSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOERSTC;
    } else if (p == GPIOF) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOFRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOFRSTC;
    } else if (p == GPIOG) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOGRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOGRSTC;
    } else if (p == GPIOH) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOHRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOHRSTC;
    } else if (p == GPION) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIONRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIONRSTC;
    } else if (p == GPIOO) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOORSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOORSTC;
    } else if (p == GPIOP) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOPRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOPRSTC;
    } else if (p == GPIOQ) {
        RCC->AHB4RSTSR |= RCC_AHB4RSTSR_GPIOQRSTS;
        RCC->AHB4RSTCR |= RCC_AHB4RSTCR_GPIOQRSTC;
    }

    (void)RCC->AHB4RSTR;
}
