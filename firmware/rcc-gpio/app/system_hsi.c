#include "stm32f10x.h"

void SystemInit(void)
{
    /* Use the hardware reset clock (HSI, 8 MHz) for this lab. */
    SCB->VTOR = FLASH_BASE;
}
