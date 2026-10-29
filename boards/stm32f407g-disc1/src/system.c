/*
 * system.c — System initialization for STM32F407G-DISC1
 *
 * Configures the clock tree at register level:
 *   HSE (8 MHz) → PLL → SYSCLK (168 MHz)
 *   AHB /1 = 168 MHz, APB1 /4 = 42 MHz, APB2 /2 = 84 MHz
 *
 * Called by the startup code before main().
 */

#include "stm32f407xx.h"
#include "board.h"

void SystemInit(void)
{
    /* Enable HSE */
    RCC->CR |= RCC_CR_HSEON;

    /* Wait for HSE ready */
    while (!(RCC->CR & RCC_CR_HSERDY))
        ;

    /* Enable power interface clock */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    /* Set voltage regulator scale 1 (required for 168 MHz) */
    PWR->CR |= PWR_CR_VOS;

    /* Configure flash: 5 wait states, prefetch, instruction cache, data cache */
    FLASH->ACR = FLASH_ACR_LATENCY_5WS
               | FLASH_ACR_PRFTEN
               | FLASH_ACR_ICEN
               | FLASH_ACR_DCEN;

    /* Configure PLL:
     * PLLM = 8   → VCO input  = 8 MHz / 8 = 1 MHz
     * PLLN = 336 → VCO output = 1 MHz × 336 = 336 MHz
     * PLLP = 2   → SYSCLK     = 336 MHz / 2 = 168 MHz
     * PLLQ = 7   → USB clock  = 336 MHz / 7 = 48 MHz
     */
    RCC->PLLCFGR = (BOARD_PLL_M << RCC_PLLCFGR_PLLM_Pos)
                 | (BOARD_PLL_N << RCC_PLLCFGR_PLLN_Pos)
                 | (((BOARD_PLL_P / 2U) - 1U) << RCC_PLLCFGR_PLLP_Pos)
                 | RCC_PLLCFGR_PLLSRC_HSE
                 | (BOARD_PLL_Q << RCC_PLLCFGR_PLLQ_Pos);

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait for PLL lock */
    while (!(RCC->CR & RCC_CR_PLLRDY))
        ;

    /* Configure bus prescalers:
     * HPRE  = /1  (AHB  = 168 MHz)
     * PPRE1 = /4  (APB1 = 42 MHz)
     * PPRE2 = /2  (APB2 = 84 MHz)
     */
    RCC->CFGR = RCC_CFGR_HPRE_DIV1
              | RCC_CFGR_PPRE1_DIV4
              | RCC_CFGR_PPRE2_DIV2;

    /* Switch system clock to PLL */
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    /* Wait until PLL is the system clock source */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        ;
}
