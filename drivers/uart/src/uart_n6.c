/**
 * @file uart_h7n6.c
 * @brief UART driver implementation — STM32H7/N6/G4/L4 register layout
 *
 * These families replace the F4-style SR/DR pair with a separate ISR
 * (status, read-only) and RDR/TDR (data). Verified against RM0433 (H7) and
 * RM0481 (N6); the RXNE/TC/TXE status bits and the TE/RE control bits kept
 * the same positions as F4, only UE moved (bit 13 on F4 -> bit 0 here).
 *
 * See uart_f4.c for the SR/DR-based layout. The public interface (uart.h)
 * is identical across both; the board's CMakeLists selects which one gets
 * compiled (drivers/uart/CMakeLists.txt).
 */

#include "uart.h"

/* STM32H7/N6/G4/L4 USART register layout */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
} uart_h7n6_regs_t;

/* Interrupt & status register bits */
#define USART_ISR_TXE  (1UL << 7)
#define USART_ISR_RXNE (1UL << 5)
#define USART_ISR_TC   (1UL << 6)

/* Control register 1 bits */
#define USART_CR1_UE (1UL << 0)
#define USART_CR1_TE (1UL << 3)
#define USART_CR1_RE (1UL << 2)

void uart_init(void *usart, uint32_t baud, uint32_t pclk)
{
    uart_h7n6_regs_t *uart = (uart_h7n6_regs_t *)usart;

    /* Disable USART during configuration */
    uart->CR1 &= ~USART_CR1_UE;

    /* Clear control registers */
    uart->CR1 = 0;
    uart->CR2 = 0;
    uart->CR3 = 0;

    /* Set baud rate (OVER8 = 0, default): BRR = pclk / baud */
    uart->BRR = (pclk + (baud / 2)) / baud;

    /* Enable TX, RX, and USART */
    uart->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_write_byte(void *usart, uint8_t data)
{
    uart_h7n6_regs_t *uart = (uart_h7n6_regs_t *)usart;

    /* Wait until TX buffer is empty */
    while (!(uart->ISR & USART_ISR_TXE))
        ;

    uart->TDR = data;
}

uint8_t uart_read_byte(void *usart)
{
    uart_h7n6_regs_t *uart = (uart_h7n6_regs_t *)usart;

    /* Wait until data is received */
    while (!(uart->ISR & USART_ISR_RXNE))
        ;

    return (uint8_t)(uart->RDR & 0xFF);
}

void uart_write(void *usart, const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        uart_write_byte(usart, data[i]);
    }
}

uint8_t uart_data_available(void *usart)
{
    uart_h7n6_regs_t *uart = (uart_h7n6_regs_t *)usart;
    return (uart->ISR & USART_ISR_RXNE) ? 1 : 0;
}
