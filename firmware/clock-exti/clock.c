#include "stm32f10x.h"
#include "clock.h"

#define READY_TIMEOUT 1000000u

uint32_t LabClock_Start(void)
{
#ifdef LAB_USE_HSE
    uint32_t remaining = READY_TIMEOUT;

    /* The PLL profile requires an 8 MHz external crystal. */
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY) && remaining != 0u) {
        --remaining;
    }
    if (!(RCC->CR & RCC_CR_HSERDY)) {
        RCC->CR &= ~RCC_CR_HSEON;
        return 8000000u;
    }

    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
    /* HCLK /1, APB1 /2, APB2 /1, ADC /6, USB PLL /1.5. */
    RCC->CFGR = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6 |
                RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;
    RCC->CR |= RCC_CR_PLLON;
    remaining = READY_TIMEOUT;
    while (!(RCC->CR & RCC_CR_PLLRDY) && remaining != 0u) {
        --remaining;
    }
    if (!(RCC->CR & RCC_CR_PLLRDY)) {
        RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON);
        RCC->CFGR = 0u;
        return 8000000u;
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    remaining = READY_TIMEOUT;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL && remaining != 0u) {
        --remaining;
    }
    if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) {
        return 72000000u;
    }

    /* Cancel the switch before disabling the PLL. */
    RCC->CFGR &= ~RCC_CFGR_SW;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {}
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON);
    RCC->CFGR = 0u;
#endif
    return 8000000u;
}
