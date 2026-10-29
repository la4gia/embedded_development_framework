/**
 * @file uart_c5.c
 * @brief UART driver implementation — STM32C5 register layout
 *
 * Placeholder: no public STM32C5 reference manual or CMSIS device headers
 * yet (see boards/nucleo-c562re/CMakeLists.txt). Unlike GPIO, UART register
 * layout is not assumed to be shared across families here — confirm whether
 * C5 matches the ISR/RDR/TDR layout in uart_h7n6.c before implementing.
 */

#include "uart.h"

void uart_init(void *usart, uint32_t baud, uint32_t pclk)
{
    (void)usart;
    (void)baud;
    (void)pclk;
    /* TODO: implement once STM32C5 USART register layout is confirmed */
}

void uart_write_byte(void *usart, uint8_t data)
{
    (void)usart;
    (void)data;
    /* TODO */
}

uint8_t uart_read_byte(void *usart)
{
    (void)usart;
    /* TODO */
    return 0;
}

void uart_write(void *usart, const uint8_t *data, uint32_t len)
{
    (void)usart;
    (void)data;
    (void)len;
    /* TODO */
}

uint8_t uart_data_available(void *usart)
{
    (void)usart;
    /* TODO */
    return 0;
}
