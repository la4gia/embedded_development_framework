/**
 * @file main.c
 * @brief Blinky — bare-metal LED toggle for STM32F407G-DISC1
 *
 * Toggles all four user LEDs (PD12-15) using register-level access.
 * No HAL, no LL, no SysTick — just a simple delay loop.
 */

#include "board.h"
#include "gpio.h"
#include "rcc.h"
// #include "common.h"

#if defined(STM32F407xx)
#include "stm32f407xx.h"
#elif defined(STM32N657xx)
#include "stm32n657xx.h"
#endif

/* Simple busy-wait delay */
static void delay(volatile uint32_t count)
{
    while (count--)
        ;
}

int main(void)
{
    gpio_config_t led_cfg = {.mode = GPIO_MODE_OUTPUT,
                             .otype = GPIO_OTYPE_PP,
                             .speed = GPIO_SPEED_LOW,
                             .pull = GPIO_PULL_NONE,
                             .af = 0};

#if defined(STM32F407xx)
    /* Enable GPIOD clock */
    RCC_EnableGPIOClock(LED_PORT);

    /* Small delay after enabling clock */
    delay(10);

    /* Configure LED pins as outputs */
    gpio_init(LED_PORT, LED_GREEN_PIN, &led_cfg);
    gpio_init(LED_PORT, LED_ORANGE_PIN, &led_cfg);
    gpio_init(LED_PORT, LED_RED_PIN, &led_cfg);
    gpio_init(LED_PORT, LED_BLUE_PIN, &led_cfg);

    while (1) {
        gpio_toggle(LED_PORT, LED_GREEN_PIN);
        delay(1000000);

        gpio_toggle(LED_PORT, LED_ORANGE_PIN);
        delay(1000000);

        gpio_toggle(LED_PORT, LED_RED_PIN);
        delay(1000000);

        gpio_toggle(LED_PORT, LED_BLUE_PIN);
        delay(1000000);
    }

#elif defined(STM32N657xx)

    /* Enable GPIO clocks */
    RCC_EnableGPIOClock(LED_GREEN_PORT);
    RCC_EnableGPIOClock(LED_RED_PORT);

    /* Enable PWR supply voltage for VDDIO2SV */
    /* Necessary for electrical and logical isolation purpose
     * This chip splits GPIO ports across several independent I/O power domains.
     * PO[5:0] is on VDDIO2 and will remain isolated, which includes GREEN Pin (P01)
     */
    PWR->SVMCR3 |= PWR_SVMCR3_VDDIO2SV;

    gpio_init(LED_GREEN_PORT, LED_GREEN_PIN, &led_cfg);
    gpio_init(LED_RED_PORT, LED_RED_PIN, &led_cfg);

    while (1) {
        gpio_toggle(LED_GREEN_PORT, LED_GREEN_PIN);
        delay(1000000);

        gpio_toggle(LED_RED_PORT, LED_RED_PIN);
        delay(1000000);
    }

#else
    /* Unknown board — just loop */
    UNUSED(led_cfg);
    while (1)
        ;
#endif

    /* Never reached */
    return 0;
}
