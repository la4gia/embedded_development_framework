/**
 * @file i2c_n6.c
 * @brief I2C driver implementation — STM32N6 register layout
 *
 * CMakeLists selects which one gets compiled (drivers/i2c/CMakeLists.txt).
 */

#include "i2c.h"
#include "stm32n657xx.h"

void i2c_init(void *i2c, uint32_t target_hz, uint32_t i2c_clk)
{
    I2C_TypeDef *i = (I2C_TypeDef *)i2c;

    i->CR1 &= ~I2C_CR1_PE;
    i->CR1 &= ~I2C_CR1_ANFOFF;
    i->CR1 &= ~I2C_CR1_DNF;

    uint32_t temp_reg = 0;

    if (i2c_clk == 8000000) {
        switch (target_hz) {
        case 10000:
            temp_reg |= (0xC7UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0xC3UL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x02UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x04UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x01UL << I2C_TIMINGR_PRESC_Pos);
            break;
        case 100000:
            temp_reg |= (0x13UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0x0FUL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x02UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x04UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x01UL << I2C_TIMINGR_PRESC_Pos);
            break;
        case 400000:
            temp_reg |= (0x09UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0x03UL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x01UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x03UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x00UL << I2C_TIMINGR_PRESC_Pos);
            break;
        case 500000:
            temp_reg |= (0x06UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0x03UL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x00UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x01UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x00UL << I2C_TIMINGR_PRESC_Pos);
            break;
        default:
            // TRAP
            while (1)
                ;
        }
    } else if (i2c_clk == 16000000) {
        switch (target_hz) {
        case 10000:
            temp_reg |= (0xC7UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0xC3UL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x02UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x04UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x03UL << I2C_TIMINGR_PRESC_Pos);
            break;
        case 100000:
            temp_reg |= (0x13UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0x0FUL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x02UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x04UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x03UL << I2C_TIMINGR_PRESC_Pos);
            break;
        case 400000:
            temp_reg |= (0x09UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0x03UL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x02UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x03UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x01UL << I2C_TIMINGR_PRESC_Pos);
            break;
        case 1000000:
            temp_reg |= (0x04UL << I2C_TIMINGR_SCLL_Pos);
            temp_reg |= (0x02UL << I2C_TIMINGR_SCLH_Pos);
            temp_reg |= (0x00UL << I2C_TIMINGR_SDADEL_Pos);
            temp_reg |= (0x02UL << I2C_TIMINGR_SCLDEL_Pos);
            temp_reg |= (0x00UL << I2C_TIMINGR_PRESC_Pos);
            break;
        default:
            // TRAP
            while (1)
                ;
        }
    } else {
        // TRAP
        while (1)
            ;
    }
    i->TIMINGR = temp_reg;

    i->CR1 &= ~I2C_CR1_NOSTRETCH;
    i->CR1 |= I2C_CR1_PE;
}

uint8_t i2c_write_reg(void *i2c, uint8_t dev_addr, uint16_t reg_addr, uint8_t value)
{

    I2C_TypeDef *i = (I2C_TypeDef *)i2c;

    // fresh register
    // leaves R/W bit as 0, means "write" from controller
    uint32_t temp_reg = 0;
    uint8_t counter = 3;

    temp_reg |= (dev_addr << 1);                 // SADD[0] is unused with 7-bit addresses
    temp_reg |= (0x03UL << I2C_CR2_NBYTES_Pos);  // hardware down-counter
    temp_reg |= (0x01UL << I2C_CR2_AUTOEND_Pos); // HARDWARE END MODE
    temp_reg |= (0x01UL << I2C_CR2_START_Pos);   // COMMIT AND GO

    // locked and loaded
    i->CR2 = temp_reg;

    // Three bytes are sent. Address high, Address Low, and the data Value
    while (counter > 0) {

        /* wait until ISR (Interrupt and Status Register) matches either
         * TXIS = Transmit Interrupt Status (Success)
         * NACKF = Not Acknowledged Received Flag (Fail)
         */
        while (!(i->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)))
            ;

        // if success
        if (!(i->ISR & I2C_ISR_NACKF)) {

            switch (counter) {
            case 3:
                // high 8 bits of 16 bit address (shift high to low)
                i->TXDR = (reg_addr >> 8);
                break;
            case 2:
                // low 8 bites of 16 bit address (mask the high)
                i->TXDR = (reg_addr & 0xFFUL);
                break;
            case 1:
                // 8 bit data value that is sent to the target
                i->TXDR = value;
                break;
            default:
                return 1;
            }

            counter--;
            continue;
        }

        return 1;
    }

    return 0;
}

uint8_t i2c_read_reg(void *i2c, uint8_t dev_addr, uint16_t reg_addr, uint8_t *value)
{

    I2C_TypeDef *i = (I2C_TypeDef *)i2c;

    /* fresh register
     * R/W bit stays 0, means "write" from controller (send address)
     * AUTOEND stays 0, does not STOP automatically
     */
    uint32_t temp_reg = 0;
    uint8_t counter = 2;

    temp_reg |= (dev_addr << 1);                // SADD[0] is unused with 7-bit addresses
    temp_reg |= (0x02UL << I2C_CR2_NBYTES_Pos); // hardware down-counter
    temp_reg |= (0x01UL << I2C_CR2_START_Pos);  // COMMIT AND GO

    // locked and loaded
    i->CR2 = temp_reg;

    // Two bytes are sent. Address high, Address Low
    while (counter > 0) {

        /* wait until ISR (Interrupt and Status Register) matches either
         * TXIS = Transmit Interrupt Status (Success)
         * NACKF = Not Acknowledged Received Flag (Fail)
         */
        while (!(i->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)))
            ;

        // if success
        if (!(i->ISR & I2C_ISR_NACKF)) {

            switch (counter) {
            case 2:
                // high 8 bits of 16 bit address (shift high to low)
                i->TXDR = (reg_addr >> 8);
                break;
            case 1:
                // low 8 bites of 16 bit address (mask the high)
                i->TXDR = (reg_addr & 0xFFUL);
                break;
            default:
                return 1;
            }

            counter--;
            continue;
        }

        return 1;
    }

    // TC "Transfer Complete"
    while (!(i->ISR & I2C_ISR_TC))
        ;

    temp_reg = 0;
    temp_reg |= (dev_addr << 1);                 // SADD[0] is unused with 7-bit addresses
    temp_reg |= (0x01UL << I2C_CR2_NBYTES_Pos);  // hardware down-counter
    temp_reg |= (0x01UL << I2C_CR2_RD_WRN_Pos);  // R/W bit. 1 means "read"
    temp_reg |= (0x01UL << I2C_CR2_AUTOEND_Pos); // Automatically End
    temp_reg |= (0x01UL << I2C_CR2_START_Pos);   // COMMIT AND GO

    // locked and loaded
    i->CR2 = temp_reg;

    /* wait until ISR (Interrupt and Status Register) matches either
     * RXNE = Receive Register Not Empty (Success)
     * NACKF = Not Acknowledged Received Flag (Fail)
     */
    while (!(i->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)))
        ;

    if (!(i->ISR & I2C_ISR_NACKF)) {
        *value = (uint8_t)i->RXDR;
        return 0;
    }

    return 1;
}
