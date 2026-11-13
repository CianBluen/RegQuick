#ifndef LAB_CONSOLE_H
#define LAB_CONSOLE_H

#include "stm32f10x.h"

#define LAB_CLOCK_HZ 8000000u
#define LAB_BAUD 9600u

static inline void PinMode(GPIO_TypeDef *port, unsigned pin, uint32_t mode)
{
    volatile uint32_t *config = pin < 8u ? &port->CRL : &port->CRH;
    const unsigned shift = (pin % 8u) * 4u;
    *config = (*config & ~(15u << shift)) | (mode << shift);
}

void Console_Init(void);
void Console_Text(const char *text);
void Console_Number(uint32_t value);

#endif
