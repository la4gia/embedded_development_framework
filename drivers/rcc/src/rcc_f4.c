/**
 * @file rcc_f4.c
 * @brief RCC driver implementation — STM32F4 register layout
 *
 * Flat AHB1ENR/AHB1RSTR bus, one enable/reset bit per GPIO port, plain
 * read-modify-write (no atomic set/clear alias registers on this family).
 * See rcc_n6.c for the very different IC-crossbar-based layout.
 * The public interface (rcc.h) is identical across both; the board's
 * CMakeLists selects which one gets compiled (drivers/rcc/CMakeLists.txt).
 */

#include "rcc.h"
#include "stm32f4xx.h"

void RCC_EnableGPIOClock(void *port)
{
    GPIO_TypeDef *p = (GPIO_TypeDef *)port;

    if (p == GPIOA) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    } else if (p == GPIOB) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    } else if (p == GPIOC) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    } else if (p == GPIOD) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    } else if (p == GPIOE) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    } else if (p == GPIOF) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    } else if (p == GPIOG) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    } else if (p == GPIOH) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    } else if (p == GPIOI) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    }

    /* A clock-enable write takes a couple of bus cycles to take effect
     * read register back before touching peripheral; the read stalls until the write lands
     * prevents the next register access being made on a still-dead clock
     */
    (void)RCC->AHB1ENR;
}

void RCC_DisableGPIOClock(void *port)
{
    GPIO_TypeDef *p = (GPIO_TypeDef *)port;

    if (p == GPIOA) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
    } else if (p == GPIOB) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
    } else if (p == GPIOC) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;
    } else if (p == GPIOD) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIODEN;
    } else if (p == GPIOE) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOEEN;
    } else if (p == GPIOF) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOFEN;
    } else if (p == GPIOG) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOGEN;
    } else if (p == GPIOH) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOHEN;
    } else if (p == GPIOI) {
        RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOIEN;
    }

    // see note in RCC_EnableGPIOClock()
    (void)RCC->AHB1ENR;
}

void RCC_ResetGPIO(void *port)
{
    GPIO_TypeDef *p = (GPIO_TypeDef *)port;

    if (p == GPIOA) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOARST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOARST;
    } else if (p == GPIOB) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOBRST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOBRST;
    } else if (p == GPIOC) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOCRST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOCRST;
    } else if (p == GPIOD) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIODRST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIODRST;
    } else if (p == GPIOE) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOERST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOERST;
    } else if (p == GPIOF) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOFRST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOFRST;
    } else if (p == GPIOG) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOGRST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOGRST;
    } else if (p == GPIOH) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOHRST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOHRST;
    } else if (p == GPIOI) {
        RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOIRST;
        RCC->AHB1RSTR &= ~RCC_AHB1RSTR_GPIOIRST;
    }

    // see note in RCC_EnableGPIOClock()
    (void)RCC->AHB1RSTR;
}
