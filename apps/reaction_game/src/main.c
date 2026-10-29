#include "stm32f407xx.h"
#include "rcc.h"
#include "gpio.h"
#include "exti.h"
#include "uart.h"
#include "board.h"

#define USART3_PORT       GPIOD
#define USART3_PIN_TX     8
#define USART3_PIN_RX     9
#define LED_PORT_A        GPIOA
#define LED_PIN_01        1
#define BUTTON_PIN_01     2
#define CYCLES_PER_MS     (BOARD_SYSCLK_FREQ / 1000) // 168,000 cycles per ms
#define ITERATIONS_PER_MS (CYCLES_PER_MS / 8) // 8 cycles per iteration, 21,000 iterations per MS
#define DEBOUNCE_LIMIT    50                  // ms
#define MIN               1000                // ms (minimum wait)
#define RANGE             10000               // ms (span)
#define BUFFER_SIZE       14
#define CAP_MS            10000 // ms
#define CAP_IN_CYCLES     (CAP_MS * CYCLES_PER_MS)
#define BAUD_SPEED        115200

_Static_assert(BUFFER_SIZE >= 10, "BUFFER_SIZE must be >= 10 (digits in UINT32_MAX)");
_Static_assert(CAP_MS <= UINT32_MAX / CYCLES_PER_MS, "CAP_MS_CYCLES must fit in uint32_t");

// static to make it private, i.e. only used in this file
// static gets initial value before main runs (.data/.bss, loaded at startup)
// initializer must be a compile time constant (set at startup)
//   or "initializer element is not constant" error
static volatile uint32_t isr_count = 0;
static volatile uint8_t armed = 0;
static volatile uint8_t hit = 0;
static volatile uint32_t end_ts = 0;
static volatile uint32_t start_ts = 0;
static volatile uint32_t elapsed_ts = 0;

typedef enum { STATE_WAITING = 0, STATE_ARMED = 1 } game_state_t;

/* Simple busy-wait delay*/
static void delay(volatile uint32_t count)
{
    while (count--)
        ;
}

void EXTI2_IRQHandler(void)
{
    // clear comes first, to ensure EXTI-PR write to buffer happens or a repeated call can happen.
    EXTI_ClearPending(BUTTON_PIN_01);
    isr_count++;

    if (armed == 1) {
        end_ts = DWT->CYCCNT;
        armed = 0;
        hit = 1;
    }
}

void send_digit(uint32_t measured_time)
{
    int8_t i = BUFFER_SIZE - 3;
    char buf[BUFFER_SIZE];
    buf[BUFFER_SIZE - 1] = 's';
    buf[BUFFER_SIZE - 2] = 'm';

    while (i >= 0) {
        uint8_t end_digit = measured_time % 10;
        measured_time = measured_time / 10;
        buf[i] = '0' + end_digit;
        if (measured_time == 0) {
            break;
        }
        i--;
    }

    while (i < BUFFER_SIZE) {
        uart_write_byte(USART3, buf[i]);
        i++;
    }
    uart_write_byte(USART3, '\r');
    uart_write_byte(USART3, '\n');
}

int main(void)
{

    RCC_EnableGPIOClock(LED_PORT_A);
    RCC_EnableGPIOClock(USART3_PORT);

    // Manual RCC_EnableUSART
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    /* A clock-enable write takes a couple of bus cycles to take effect
     * read register back before touching peripheral; the read stalls until the write lands
     * prevents the next (USART3) register access being made on a still-dead clock
     */
    (void)RCC->APB1ENR;

    gpio_config_t led_cfg = {.mode = GPIO_MODE_OUTPUT,
                             .speed = GPIO_SPEED_LOW,
                             .pull = GPIO_PULL_NONE,
                             .otype = GPIO_OTYPE_PP,
                             .af = 0};

    // OSPEEDR/OTYPER only affect the output driver
    gpio_config_t button_cfg = {.mode = GPIO_MODE_INPUT, .pull = GPIO_PULL_UP, .af = 0};

    gpio_config_t usart_tx_cfg = {.mode = GPIO_MODE_AF,
                                  .af = 7,
                                  .pull = GPIO_PULL_NONE,
                                  .speed = GPIO_SPEED_LOW,
                                  .otype = GPIO_OTYPE_PP};

    // OSPEEDR/OTYPER only affect the output driver
    // RX idles high, pull up handles unplugged adapter scenario
    gpio_config_t usart_rx_cfg = {.mode = GPIO_MODE_AF, .af = 7, .pull = GPIO_PULL_UP};

    gpio_init(LED_PORT_A, LED_PIN_01, &led_cfg);
    gpio_init(LED_PORT_A, BUTTON_PIN_01, &button_cfg);
    gpio_init(USART3_PORT, USART3_PIN_TX, &usart_tx_cfg);
    gpio_init(USART3_PORT, USART3_PIN_RX, &usart_rx_cfg);

    uart_init(USART3, BAUD_SPEED, BOARD_PCLK1_FREQ);

    EXTI_ConfigLine(LED_PORT_A, BUTTON_PIN_01, EXTI_TRIG_FALLING);
    EXTI_SetPriority(EXTI2_IRQn, 1);
    EXTI_EnableIRQ(EXTI2_IRQn);

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Activate debug/trace block (for DWT)
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // Start CYCCNT (168 per microsecond)

    game_state_t state = STATE_WAITING;
    gpio_clear(LED_PORT_A, LED_PIN_01);

    while (1) {
        switch (state) {
        case STATE_WAITING: {

            gpio_clear(LED_PORT_A, LED_PIN_01);

            uint16_t wait_ms = 0;
            uint16_t count = 0;
            wait_ms = ((DWT->CYCCNT) % RANGE) + MIN;
            while (count < wait_ms) {
                delay(ITERATIONS_PER_MS);
                count++;
            }

            gpio_set(LED_PORT_A, LED_PIN_01);
            start_ts = DWT->CYCCNT; // start timer
            hit = 0;                // track for a button press
            armed = 1;              // button press can now be measured

            state = STATE_ARMED;
            break;
        }
        case STATE_ARMED: {

            if (hit == 1) {
                elapsed_ts = end_ts - start_ts;
                uint32_t ms = elapsed_ts / CYCLES_PER_MS;
                send_digit(ms);
                state = STATE_WAITING;
            } else if ((DWT->CYCCNT - start_ts) > CAP_IN_CYCLES) {
                // deadline passed with no press, end round
                armed = 0;
                state = STATE_WAITING;
            }
            break;
        }
        }
    }
    return 0;
}
