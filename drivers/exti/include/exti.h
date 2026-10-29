#ifndef EXTI_H
#define EXTI_H

#include "stm32f407xx.h"
#include <stdint.h>

typedef enum { EXTI_TRIG_FALLING = 0, EXTI_TRIG_RISING, EXTI_TRIG_BOTH } EXTI_Trigger_t;

void EXTI_ConfigLine(GPIO_TypeDef *port, uint8_t pin, EXTI_Trigger_t trigger);

void EXTI_EnableIRQ(IRQn_Type irq);
void EXTI_DisableIRQ(IRQn_Type irq);
void EXTI_SetPriority(IRQn_Type irq, uint32_t priority);
void EXTI_ClearPending(uint8_t pin);

#endif