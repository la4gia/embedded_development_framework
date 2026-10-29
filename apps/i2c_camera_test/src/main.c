/**
 * @file main.c
 * @brief camera test - for testing on STM32n657-dk
 *
 */

#include "stm32n657xx.h"
#include "gpio.h"
#include "rcc.h"
#include "i2c.h"
#include "imx335_regs.h"
#include <stddef.h>

/* Simple busy-wait delay */
static void delay(volatile uint32_t count)
{
    while (count--)
        ;
}

int main(void)
{
    uint8_t status = 0;
    uint8_t chip_id;

    /* Enable PWR supply voltage for VDDIO4SV */
    /* Necessary for electrical and logical isolation purpose
     * This chip splits GPIO ports across several independent I/O power domains.
     * PC[1], PC[12:6], and PH[9:2] is on VDDIO4 and will remain isolated
     */
    PWR->SVMCR1 |= PWR_SVMCR1_VDDIO4SV;

    RCC_EnableGPIOClock(GPIOC);
    RCC_EnableGPIOClock(GPIOH);
    RCC_EnableGPIOClock(GPIOD);

    // PC1 & PH9
    gpio_config_t cam_cfg = {.mode = GPIO_MODE_AF,
                             .otype = GPIO_OTYPE_OD,
                             .speed = GPIO_SPEED_LOW,
                             .pull = GPIO_PULL_UP,
                             .af = 4};

    gpio_init(GPIOC, 1U, &cam_cfg);
    gpio_init(GPIOH, 9U, &cam_cfg);

    // PC8  NRST_CAM
    cam_cfg.mode = GPIO_MODE_OUTPUT;
    cam_cfg.otype = GPIO_OTYPE_PP;
    cam_cfg.pull = GPIO_PULL_NONE;
    cam_cfg.af = 0;

    // Enable I2C1
    RCC->APB1ENSR1 |= RCC_APB1ENSR1_I2C1ENS;

    // RCC Clock Configuration for Independent Peripheral register 4
    // 0b101 (hsi_div_ck)
    RCC->CCIPR4 &= ~RCC_CCIPR4_I2C1SEL_Msk;
    RCC->CCIPR4 |= (0x5UL << RCC_CCIPR4_I2C1SEL_Pos);

    // RCC HSI configuration register
    // 0b10 (HSIDIV_1) i.e. hsi_ck /  4
    RCC->HSICFGR &= ~RCC_HSICFGR_HSIDIV_Msk;
    RCC->HSICFGR |= (0x2UL << RCC_HSICFGR_HSIDIV_Pos);

    // RCC configuration register 2
    // b010 (PPRE5) i.e. rcc_pclk5 = sys_bus2_ck / 4 (24 MHz = 96 MHz / 4)
    // RCC->CFGR2 &= ~RCC_CFGR2_PPRE5_Msk;
    // RCC->CFGR2 |= (0x2UL << RCC_CFGR2_PPRE5_Pos);

    gpio_init(GPIOD, 2U, &cam_cfg);
    gpio_set(GPIOD, 2U); // EN_MODULE high (power up)

    delay(1000000); // random delay

    gpio_init(GPIOC, 8U, &cam_cfg);

    gpio_clear(GPIOC, 8U); // drive low

    delay(1000000); // random delay

    gpio_set(GPIOC, 8U); // drive high

    delay(1000000); // random delay

    // 400 kHz target, 16 MHz kernel clock
    i2c_init(I2C1, 400000, 16000000);

    delay(1000000); // random delay

    // IMX335 address = 0x1A
    status = i2c_read_reg(I2C1, 0x1A, 0x3912, &chip_id);

    if (status != 0) {
        while (1)
            ;
    }

    /* Linux Driver IMX335 Start Streaming steps
     * 1. Writes the whole mode_2592x1940_regs[] table (about 85 register writes).
     * 2. Runs the V4L2 control handler, which writes exposure and gain
     * 3. Writes MODE_SELECT = streaming.
     * 4. Waits about 20 ms.
     */

    // IMX335 Start Streaming Step 1
    const struct imx335_reg *reg = mode_2592x1940_regs;
    size_t sensor_array_len = sizeof(mode_2592x1940_regs) / sizeof(mode_2592x1940_regs[0]);
    const struct imx335_reg *end = reg + sensor_array_len;
    while (reg < end) {
        status = i2c_write_reg(I2C1, 0x1A, reg->addr, reg->val);

        if (status != 0) {
            while (1)
                ;
        }
        reg++;
    }

    // Configure and enable IC18 for CSI-PHY Kernel clock. ic18_ck, max 20 MHz
    // The divisor should be an even number, it can only make perfectly 50% duty-cycle output when
    // the divisor is even, because it can spend N/2 input cycles High and N/2 low.
    // PLL1 = 576 MHz. 576 / 30 = 19.2 MHz
    RCC->IC18CFGR &= ~RCC_IC18CFGR_IC18SEL; // Set to 00, for pll1_ck
    RCC->IC18CFGR &= ~RCC_IC18CFGR_IC18INT;
    RCC->IC18CFGR |= (0x1dUL << RCC_IC18CFGR_IC18INT_Pos); // 0x1d = 29 -> /30.

    RCC->DIVENSR |= RCC_DIVENSR_IC18ENS;

    // Enable CSI
    // Confirmed no GPIO via the UM (Camera Module Connector) and Data Sheet shows no AF for it.
    RCC->APB5ENSR |= RCC_APB5ENSR_CSIENS; // Gates the Clock
    CSI->PRCR |= CSI_PRCR_PEN;            // PHY enabled (out of reset)

    // CSI PHY frequence control register
    // DLD = 0 Data Lane Direction (Rx direction)
    CSI->PFCR &= ~CSI_PFCR_DLD_Msk;
    // HSFR = 0x0b PHY High Speed freq range (BT_1200 preset for the sensors ~1.19 Gbps/lane)
    CSI->PFCR &= ~CSI_PFCR_HSFR_Msk;
    CSI->PFCR |= (0x0bUL << CSI_PFCR_HSFR_Pos);
    // CCFR Configuration clock freq selection
    // CCFR = RoundUp((Fcfg_clk - 17) × 4). Fcfg_clk is IC18, 19.2 MHz (set above)
    // (19.2 - 17) = 2.2. Then 2.2 * 4 = 8.8. Then RoundUp. 9. 0x09, 0b1001
    CSI->PFCR &= ~CSI_PFCR_CCFR_Msk;
    CSI->PFCR |= (0x09UL << CSI_PFCR_CCFR_Pos);

    // Bring digital Host controller logic online
    CSI->CR |= CSI_CR_CSIEN;
    /*
     * CSI_VCxCFGR1
     * CSI-2 packets carry a "data type" tag (per the MIPI spec) telling the receiver how to
     * interpret each pixel word's bit width/format. The Host needs to be told what to expect, or it
     * can't correctly unpack the stream.
     * RAW12.
     * ALLDT = 1 ("apply one format to all data types" mode)
     * CDTFT = 00100, 0x4 (BPP12...RAW12)
     */
    CSI->VC0CFGR1 |= CSI_VC0CFGR1_ALLDT;
    CSI->VC0CFGR1 &= ~CSI_VC0CFGR1_CDTFT_Msk;
    CSI->VC0CFGR1 |= (0x04UL << CSI_VC0CFGR1_CDTFT_Pos);
    /*
     * CSI_PCR (D-PHY_RX lane control, offset 0x04)
     * NPWRDOWN, "1: Active state"
     * CLEN (bit 1, clock lane)
     * DL0EN/DL1EN (bits 2/3, data lanes 0/1).
     * All four to 1 — two data lanes, clock lane, and power-up.
     */
    CSI->PCR |= CSI_PCR_NPWRDOWN;
    CSI->PCR |= CSI_PCR_CLEN;
    CSI->PCR |= CSI_PCR_DL0EN;
    CSI->PCR |= CSI_PCR_DL1EN;
    // CSI_CR.VC0START. Starts virtual channel 0's reception
    // written last, after everything is configured
    CSI->CR |= CSI_CR_VC0START;

    // IMX335 Start Streaming Step 3
    // Minimal streaming start
    // MIPI camera sensors power to standby, we flip 1 bit to start emitting frames
    status = i2c_write_reg(I2C1, 0x1A, 0x3000, 0x00);

    if (status != 0) {
        while (1)
            ;
    }

    // IMX335 Start Streaming Step 4
    // Read back MODE_SELECT: 0x00 = streaming, 0x01 = standby (Linux imx335 driver)
    uint8_t mode_sel = 0xFF;
    delay(1000000);
    status = i2c_read_reg(I2C1, 0x1A, 0x3000, &mode_sel);

    if (status != 0) {
        while (1)
            ;
    }

    while (1)
        ;
}