#ifndef RCC_H
#define RCC_H

/**
 * @file rcc.h
 * @brief Bare-metal RCC (peripheral clock gating) driver interface
 *
 * NOTE: RCC register layout differs significantly across STM32 families
 * (more so than GPIO/UART — F4's flat AHB1ENR/AHB1RSTR vs. N6's IC-crossbar
 * clock tree with atomic set/clear alias registers). Each family's layout
 * gets its own source file (rcc_f4.c, rcc_n6.c) implementing this same
 * interface; drivers/rcc/CMakeLists.txt picks the right one for BOARD at
 * configure time. See that file before adding a new family.
 *
 * Usage:
 *   #include "stm32f407xx.h"  // for GPIOD, etc. — caller includes the
 *                              // family CMSIS header, not this one
 *   #include "rcc.h"
 *
 *   RCC_EnableGPIOClock(GPIOD);
 */

void RCC_EnableGPIOClock(void *port);
void RCC_DisableGPIOClock(void *port);
void RCC_ResetGPIO(void *port);

#endif
