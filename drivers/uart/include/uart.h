#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 * @file uart.h
 * @brief Bare-metal UART/USART driver interface
 *
 * NOTE: UART registers differ more significantly across STM32 families
 * than GPIO. Each family's register layout gets its own source file
 * (uart_f4.c, uart_h7n6.c, uart_c5.c) implementing this same interface;
 * drivers/uart/CMakeLists.txt picks the right one for BOARD at configure
 * time. See that file before adding a new family.
 *
 * Usage:
 *   uart_init(USART2, 115200, BOARD_PCLK1_FREQ);
 *   uart_write_byte(USART2, 'A');
 */

/**
 * @brief Initialize UART peripheral
 * @param usart  Pointer to USART peripheral (e.g., USART2)
 * @param baud   Desired baud rate
 * @param pclk   Peripheral clock frequency feeding this USART
 */
void uart_init(void *usart, uint32_t baud, uint32_t pclk);

/**
 * @brief Transmit one byte (blocking)
 * @param usart  Pointer to USART peripheral
 * @param data   Byte to transmit
 */
void uart_write_byte(void *usart, uint8_t data);

/**
 * @brief Receive one byte (blocking)
 * @param usart  Pointer to USART peripheral
 * @return Received byte
 */
uint8_t uart_read_byte(void *usart);

/**
 * @brief Transmit a buffer (blocking)
 * @param usart  Pointer to USART peripheral
 * @param data   Pointer to data buffer
 * @param len    Number of bytes to send
 */
void uart_write(void *usart, const uint8_t *data, uint32_t len);

/**
 * @brief Check if data is available to read
 * @param usart  Pointer to USART peripheral
 * @return 1 if data available, 0 otherwise
 */
uint8_t uart_data_available(void *usart);

#endif /* UART_H */
