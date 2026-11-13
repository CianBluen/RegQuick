#include "stm32f10x_rcc.h"

volatile uint32_t lab_sysclk;
volatile uint32_t lab_hclk;
volatile uint32_t lab_pclk1;
volatile uint32_t lab_pclk2;

int main(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_GetClocksFreq(&clocks);
    lab_sysclk = clocks.SYSCLK_Frequency;
    lab_hclk = clocks.HCLK_Frequency;
    lab_pclk1 = clocks.PCLK1_Frequency;
    lab_pclk2 = clocks.PCLK2_Frequency;
    for (;;) {}
}
