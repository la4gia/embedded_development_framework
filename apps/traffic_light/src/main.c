/**
 * @file main.c
 * @brief traffic_light — bare-metal LED traffic light state machine for STM32F407G-DISC1
 */

#include "gpio.h"
#include "rcc.h"
#include "stm32f407xx.h"

#define LED_PORT       GPIOA
#define LED_RED_PIN    1U
#define LED_GREEN_PIN  2U
#define LED_YELLOW_PIN 3U

#define SECONDS_8 168000000
#define SECONDS_6 126000000
#define SECONDS_4 84000000
#define SECONDS_2 42000000

typedef enum { STATE_RED = 0, STATE_GREEN = 1, STATE_YELLOW = 2 } light_state_t;

/* Simple busy-wait delay */
static void delay(volatile uint32_t count)
{
    while (count--)
        ;
}

int main(void)
{
    gpio_config_t led_config = {.mode = GPIO_MODE_OUTPUT,
                                .speed = GPIO_SPEED_LOW,
                                .otype = GPIO_OTYPE_PP,
                                .pull = GPIO_PULL_NONE,
                                .af = 0};

    RCC_EnableGPIOClock(LED_PORT);

    gpio_init(LED_PORT, LED_RED_PIN, &led_config);
    gpio_init(LED_PORT, LED_GREEN_PIN, &led_config);
    gpio_init(LED_PORT, LED_YELLOW_PIN, &led_config);

    light_state_t state = STATE_RED;

    while (1) {
        switch (state) {
        case STATE_RED:
            gpio_set(LED_PORT, LED_RED_PIN);
            delay(SECONDS_4);
            gpio_clear(LED_PORT, LED_RED_PIN);
            state = STATE_GREEN;
            break;
        case STATE_GREEN:
            gpio_set(LED_PORT, LED_GREEN_PIN);
            delay(SECONDS_2);
            gpio_clear(LED_PORT, LED_GREEN_PIN);
            state = STATE_YELLOW;
            break;
        case STATE_YELLOW:
            gpio_set(LED_PORT, LED_YELLOW_PIN);
            delay(SECONDS_6);
            gpio_clear(LED_PORT, LED_YELLOW_PIN);
            state = STATE_RED;
            break;
        }
    }

    return 0;
}
