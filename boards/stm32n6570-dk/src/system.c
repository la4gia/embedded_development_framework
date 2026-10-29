/*
 * system.c — System initialization for STM32N6570-DK
 *
 * Cortex-M55 based. Clock configuration at register level.
 * TODO: Implement full clock tree once reference manual is studied.
 */

#include "stm32n657xx.h"
#include "board.h"

void SystemInit(void)
{

    /* Enable HSE */
    RCC->CR |= RCC_CR_HSEON;

    /* Wait for HSE ready*/
    while (!(RCC->SR & RCC_SR_HSERDY))
        ;

    /* MULTI-BIT FIELDS REQUIRE CLEARING FIRST*/

    /* PLL1 CFGR1 SEL = 2 (0b010) -> hse_clk*/
    RCC->PLL1CFGR1 &= ~RCC_PLL1CFGR1_PLL1SEL_Msk;
    RCC->PLL1CFGR1 |= RCC_PLL1CFGR1_PLL1SEL_1;

    /* PLL1 CFGR1 DIVM = 1 -> PFD input divider*/
    RCC->PLL1CFGR1 &= ~RCC_PLL1CFGR1_PLL1DIVM_Msk;
    RCC->PLL1CFGR1 |= (0X1UL << RCC_PLL1CFGR1_PLL1DIVM_Pos);

    /* PLL1 CFGR1 DIVN = 48 -> VCO multiplier*/
    RCC->PLL1CFGR1 &= ~RCC_PLL1CFGR1_PLL1DIVN_Msk;
    RCC->PLL1CFGR1 |= (0X30UL << RCC_PLL1CFGR1_PLL1DIVN_Pos);

    /* explicit PLL1 bypass. cleared only*/
    RCC->PLL1CFGR1 &= ~RCC_PLL1CFGR1_PLL1BYP_Msk;

    /* PLL1 CFGR3 PDIV1 = 2 -> PLL1 VCO frequency divider level 1*/
    RCC->PLL1CFGR3 &= ~RCC_PLL1CFGR3_PLL1PDIV1_Msk;
    RCC->PLL1CFGR3 |= (0x2UL << RCC_PLL1CFGR3_PLL1PDIV1_Pos);

    /* PLL1 CFGR3 PDIV2 = 2 -> PLL1 VCO frequency divider level 2*/
    RCC->PLL1CFGR3 &= ~RCC_PLL1CFGR3_PLL1PDIV2_Msk;
    RCC->PLL1CFGR3 |= (0x2UL << RCC_PLL1CFGR3_PLL1PDIV2_Pos);

    /* PLL1 CFGR3 PDIVEN = 1 -> PLL1 post divider POSTDIV1 and PLL clock output enable  */
    /* single bit, no clearing needed*/
    RCC->PLL1CFGR3 |= RCC_PLL1CFGR3_PLL1PDIVEN;

    /* Enable PLL1 */
    RCC->CR |= RCC_CR_PLL1ON;

    /* Wait for PLL1 ready*/
    while (!(RCC->SR & RCC_SR_PLL1RDY))
        ;

    /* Configure IC1*/

    /* IC1 SEL = 0 -> select pll1_ck as IC1's source*/
    RCC->IC1CFGR &= ~RCC_IC1CFGR_IC1SEL;

    /* IC1 INT = 0 -> divide by 1 (passthrough) - icx_ck = pllx_ck / (ICxINT + 1)*/
    RCC->IC1CFGR &= ~RCC_IC1CFGR_IC1INT;

    /* Configure IC2*/

    /* IC2 SEL = 0 -> select pll1_ck as IC2's source*/
    RCC->IC2CFGR &= ~RCC_IC2CFGR_IC2SEL;

    /* IC2 INT = 2 -> divide by 3 - icx_ck = pllx_ck / (ICxINT + 1)*/
    RCC->IC2CFGR &= ~RCC_IC2CFGR_IC2INT;
    RCC->IC2CFGR |= (0x2UL << RCC_IC2CFGR_IC2INT_Pos);

    /* Enable IC1 and IC2*/
    RCC->DIVENSR |= RCC_DIVENSR_IC1ENS;
    RCC->DIVENSR |= RCC_DIVENSR_IC2ENS;

    /* Enable IC6 and IC11*/
    /* RM0486 14.6.6 states:
     * The SYSSW only selects the ic[2,6,11]_ck if all three IC dividers are enabled."
     */
    RCC->DIVENSR |= RCC_DIVENSR_IC6ENS;
    RCC->DIVENSR |= RCC_DIVENSR_IC11ENS;

    /* SELECT ic1_ck as CPU clock*/
    RCC->CFGR1 &= ~RCC_CFGR1_CPUSW;
    RCC->CFGR1 |= (0x3UL << RCC_CFGR1_CPUSW_Pos);

    /* CPUSW and SYSCW are multi-bit fields*/
    /* A simple RDY busy-wait check will not work.*/
    /* Need to use an available confirmation poll (status)*/

    /* CPUSW confirmation poll (CPUSWS)*/
    /* Mask RCC->CFGR1 with CPUSW status to get actual value. */
    /* Confirm if it matches desired target. Wait if it doesn't*/
    while ((RCC->CFGR1 & RCC_CFGR1_CPUSWS_Msk) != (0x3UL << RCC_CFGR1_CPUSWS_Pos))
        ;

    /* SELECT ic2_ck as BUS clock*/
    RCC->CFGR1 &= ~RCC_CFGR1_SYSSW;
    RCC->CFGR1 |= (0x3UL << RCC_CFGR1_SYSSW_Pos);

    /* SYSSW confirmation poll (SYSSWS)*/
    /* Mask RCC->CFGR1 with SYSSW status to get actual value. */
    /* Confirm if it matches desired target. Wait if it doesn't*/
    while ((RCC->CFGR1 & RCC_CFGR1_SYSSWS_Msk) != (0x3UL << RCC_CFGR1_SYSSWS_Pos))
        ;
}
