#include "exti.h"

void EXTI_ConfigLine(GPIO_TypeDef *port, uint8_t pin, EXTI_Trigger_t trigger)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    uint8_t exticr = pin / 4;
    uint8_t shift = (pin % 4) * 4;

    uint32_t portcode = 0;

    if (port == GPIOA)
        portcode = 0;
    else if (port == GPIOB)
        portcode = 1;
    else if (port == GPIOC)
        portcode = 2;
    else if (port == GPIOD)
        portcode = 3;
    else if (port == GPIOE)
        portcode = 4;
    else if (port == GPIOF)
        portcode = 5;
    else if (port == GPIOG)
        portcode = 6;
    else if (port == GPIOH)
        portcode = 7;
    else if (port == GPIOI)
        portcode = 8;

    SYSCFG->EXTICR[exticr] &= ~(0xFU << shift);
    SYSCFG->EXTICR[exticr] |= (portcode << shift);

    EXTI->IMR |= (1U << pin);

    switch (trigger) {
    case EXTI_TRIG_FALLING:
        EXTI->FTSR |= (1U << pin);
        EXTI->RTSR &= ~(1U << pin);
        break;

    case EXTI_TRIG_RISING:
        EXTI->RTSR |= (1U << pin);
        EXTI->FTSR &= ~(1U << pin);
        break;

    case EXTI_TRIG_BOTH:
        EXTI->RTSR |= (1U << pin);
        EXTI->FTSR |= (1U << pin);
        break;
    }
}

void EXTI_EnableIRQ(IRQn_Type irq)
{

    NVIC_EnableIRQ(irq);
}

void EXTI_DisableIRQ(IRQn_Type irq)
{

    NVIC_DisableIRQ(irq);
}

void EXTI_SetPriority(IRQn_Type irq, uint32_t priority)
{

    NVIC_SetPriority(irq, priority);
}

void EXTI_ClearPending(uint8_t PinNumber)
{
    /* clear (manual says to set as 1)
     * must be '=', not '|='. OR will write to all bits that read as '1'...
     * thus clears all pending bits
     */
    EXTI->PR = (1 << PinNumber);
}
