/**
 * @file uart_f4.c
 * @brief UART driver implementation — STM32F4 register layout
 *
 * STM32F4 USART registers (RM0090):
 *   SR   (0x00) — Status register
 *   DR   (0x04) — Data register
 *   BRR  (0x08) — Baud rate register
 *   CR1  (0x0C) — Control register 1
 *   CR2  (0x10) — Control register 2
 *   CR3  (0x14) — Control register 3
 *
 * See uart_h7n6.c for the ISR/RDR/TDR-based layout used by H7/N6/G4/L4.
 * The public interface (uart.h) is identical across both; the board's
 * CMakeLists selects which one gets compiled (drivers/uart/CMakeLists.txt).
 */

#include "uart.h"

/* STM32F4 USART register layout */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} uart_f4_regs_t;

/* Status register bits (F4) */
#define USART_SR_TXE   (1UL << 7)
#define USART_SR_RXNE  (1UL << 5)
#define USART_SR_TC    (1UL << 6)

/* Control register 1 bits (F4) */
#define USART_CR1_UE   (1UL << 13)
#define USART_CR1_TE   (1UL << 3)
#define USART_CR1_RE   (1UL << 2)

void uart_init(void *usart, uint32_t baud, uint32_t pclk)
{
    uart_f4_regs_t *uart = (uart_f4_regs_t *)usart;

    /* Disable USART during configuration */
    uart->CR1 &= ~USART_CR1_UE;

    /* Clear control registers */
    uart->CR1 = 0;
    uart->CR2 = 0;
    uart->CR3 = 0;

    /* Set baud rate: BRR = pclk / baud */
    uart->BRR = (pclk + (baud / 2)) / baud;

    /* Enable TX, RX, and USART */
    uart->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_write_byte(void *usart, uint8_t data)
{
    uart_f4_regs_t *uart = (uart_f4_regs_t *)usart;

    /* Wait until TX buffer is empty */
    while (!(uart->SR & USART_SR_TXE))
        ;

    uart->DR = data;
}

uint8_t uart_read_byte(void *usart)
{
    uart_f4_regs_t *uart = (uart_f4_regs_t *)usart;

    /* Wait until data is received */
    while (!(uart->SR & USART_SR_RXNE))
        ;

    return (uint8_t)(uart->DR & 0xFF);
}

void uart_write(void *usart, const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        uart_write_byte(usart, data[i]);
    }
}

uint8_t uart_data_available(void *usart)
{
    uart_f4_regs_t *uart = (uart_f4_regs_t *)usart;
    return (uart->SR & USART_SR_RXNE) ? 1 : 0;
}
