#include "stm32f407xx.h"
#include "rcc.h"
#include "gpio.h"
#include "exti.h"
#include "uart.h"
#include "board.h"

#define CYCLES_PER_MS     (BOARD_SYSCLK_FREQ / 1000) // 168,000 cycles per ms
#define ITERATIONS_PER_MS (CYCLES_PER_MS / 8) // 8 cycles per iteration, 21,000 iterations per MS
#define DEBOUNCE_LIMIT    50                  // ms
#define MIN               1000                // ms (minimum wait)
#define RANGE             10000               // ms (span)
#define BUFFER_SIZE       14
#define CAP_MS            10000 // ms
#define CAP_IN_CYCLES     (CAP_MS * CYCLES_PER_MS)
#define BAUD_SPEED        115200
#define WINDOW_CAP        50 // ms
#define WINDOW_CYCLES     (WINDOW_CAP * CYCLES_PER_MS)
#define PRESSED           1
#define RELEASED          0
#define HIGH              1
#define LOW               0

#define LED_ON_MS  2000
#define LED_GAP_MS 1000

#define BUTTON_PORT   GPIOB
#define BUTTON_PIN_12 12
#define BUTTON_PIN_13 13
#define BUTTON_PIN_14 14
#define BUTTON_PIN_15 15

_Static_assert(BUFFER_SIZE >= 10, "BUFFER_SIZE must be >= 10 (digits in UINT32_MAX)");
_Static_assert(CAP_MS <= UINT32_MAX / CYCLES_PER_MS, "CAP_MS_CYCLES must fit in uint32_t");
_Static_assert(WINDOW_CAP <= UINT32_MAX / CYCLES_PER_MS, "WINDOW_CYCLES must fit in uint32_t");

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

static volatile uint8_t pressed[4];
static volatile uint8_t state[4];
static volatile uint32_t last_edge_ts[4];
static volatile uint32_t press_count;

/* Simple busy-wait delay*/
static void delay(volatile uint32_t count)
{
    while (count--)
        ;
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

void EXTI15_10_IRQHandler(void)
{
    uint16_t snapshot = EXTI->PR & 0xF000;

    uint8_t bit = 0;
    for (bit = BUTTON_PIN_12; bit <= BUTTON_PIN_15; bit++) {
        uint16_t mask = 1U << bit;
        uint8_t idx = bit - BUTTON_PIN_12;
        if (snapshot & mask) {
            EXTI_ClearPending(bit);
            uint32_t now = DWT->CYCCNT;

            isr_count++;
            uint8_t quiet = (now - last_edge_ts[idx]) >= WINDOW_CYCLES;
            last_edge_ts[idx] = now;
            uint8_t level = (BUTTON_PORT->IDR >> bit) & 1;

            if ((state[idx] == RELEASED) && (level == LOW) && quiet) {
                pressed[idx] = 1;
                state[idx] = PRESSED;
                press_count++;
            } else if ((state[idx] == PRESSED) && (level == HIGH)) {
                state[idx] = RELEASED;
            }
        }
    }
}

void flush_presses(void)
{
    uint8_t idx = 0;
    while (idx < 4) {
        pressed[idx] = 0;
        idx++;
    }
}

void play_sequence(const uint8_t *seq, uint8_t seq_len)
{
    gpio_clear(LED_PORT, LED_GREEN_PIN);
    gpio_clear(LED_PORT, LED_RED_PIN);
    gpio_clear(LED_PORT, LED_ORANGE_PIN);
    gpio_clear(LED_PORT, LED_BLUE_PIN);

    uint8_t idx = 0;
    while (idx < seq_len) {
        gpio_toggle(LED_PORT, seq[idx]);
        delay(ITERATIONS_PER_MS * LED_ON_MS);
        gpio_toggle(LED_PORT, seq[idx]);
        delay(ITERATIONS_PER_MS * LED_GAP_MS);
        idx++;
    }
}

int main(void)
{

    RCC_EnableGPIOClock(BUTTON_PORT);
    RCC_EnableGPIOClock(LED_PORT);

    // small delay after enabling clocks
    delay(10);

    gpio_config_t led_cfg = {.mode = GPIO_MODE_OUTPUT,
                             .otype = GPIO_OTYPE_PP,
                             .speed = GPIO_SPEED_LOW,
                             .pull = GPIO_PULL_NONE,
                             .af = 0};

    // Configure LED pins as outputs
    gpio_init(LED_PORT, LED_GREEN_PIN, &led_cfg);
    gpio_init(LED_PORT, LED_ORANGE_PIN, &led_cfg);
    gpio_init(LED_PORT, LED_RED_PIN, &led_cfg);
    gpio_init(LED_PORT, LED_BLUE_PIN, &led_cfg);

    gpio_config_t button_cfg = {.mode = GPIO_MODE_INPUT, .pull = GPIO_PULL_UP};
    gpio_init(BUTTON_PORT, BUTTON_PIN_12, &button_cfg);
    gpio_init(BUTTON_PORT, BUTTON_PIN_13, &button_cfg);
    gpio_init(BUTTON_PORT, BUTTON_PIN_14, &button_cfg);
    gpio_init(BUTTON_PORT, BUTTON_PIN_15, &button_cfg);

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Activate debug/trace block (for DWT)
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // Start CYCCNT (168 per microsecond)

    // enable interrupt handler 10-15, for PB pins 12-15
    EXTI_ConfigLine(BUTTON_PORT, BUTTON_PIN_12, EXTI_TRIG_BOTH);
    EXTI_ConfigLine(BUTTON_PORT, BUTTON_PIN_13, EXTI_TRIG_BOTH);
    EXTI_ConfigLine(BUTTON_PORT, BUTTON_PIN_14, EXTI_TRIG_BOTH);
    EXTI_ConfigLine(BUTTON_PORT, BUTTON_PIN_15, EXTI_TRIG_BOTH);
    EXTI_SetPriority(EXTI15_10_IRQn, 1);
    EXTI_EnableIRQ(EXTI15_10_IRQn);

    uint8_t sequence[5] = {LED_BLUE_PIN, LED_RED_PIN, LED_BLUE_PIN, LED_ORANGE_PIN, LED_GREEN_PIN};
    play_sequence(sequence, (sizeof(sequence) / sizeof(sequence[0])));
    flush_presses();

    while (1) {

        uint8_t idx = 0;
        for (idx = 0; idx < 4; idx++) {
            if (pressed[idx]) {
                pressed[idx] = 0;
                gpio_toggle(LED_PORT, (idx + BUTTON_PIN_12));
            }
        }
    }

    return 0;
}
