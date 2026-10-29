#ifndef I2C_H
#define I2C_H

#include <stdint.h>

/**
 * @file i2c.h
 * @brief Bare-metal i2c driver interface
 *
 * NOTE: i2c registers differ more significantly across STM32 families
 * than GPIO. Each family's register layout gets its own source file
 * (i2c_f4.c, i2c_h7n6.c, i2c_c5.c) implementing this same interface;
 * drivers/i2c/CMakeLists.txt picks the right one for BOARD at configure
 * time. See that file before adding a new family.
 *
 * Usage:
 *   i2c_init(I2C1, 400000, 16000000);
 */

/**
 * @brief Initialize I2C peripheral
 * @param i2c  Pointer to I2C peripheral (e.g., I2C2)
 * @param target_hz   Desired Hz rate
 * @param i2c_clk   Peripheral clock frequency feeding this I2C
 */
void i2c_init(void *i2c, uint32_t target_hz, uint32_t i2c_clk);

/**
 * @brief Write I2C peripheral
 * @param i2c  Pointer to I2C peripheral (e.g., I2C2)
 * @param dev_addr   7-bit Device Address
 * @param reg_addr   16-bit Register Address
 * @param value      Data Value
 * @return 0 on success, nonzero if the peripheral NACKed or timed out
 */
uint8_t i2c_write_reg(void *i2c, uint8_t dev_addr, uint16_t reg_addr, uint8_t value);

/**
 * @brief Read I2C peripheral
 * @param i2c  Pointer to I2C peripheral (e.g., I2C2)
 * @param dev_addr   7-bit Device Address
 * @param reg_addr   16-bit Register Address
 * @param value      Pointer to Data Value
 * @return 0 on success, nonzero if the peripheral NACKed or timed out
 */
uint8_t i2c_read_reg(void *i2c, uint8_t dev_addr, uint16_t reg_addr, uint8_t *value);

#endif /* I2C_H */
