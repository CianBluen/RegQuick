#include "console.h"

int main(void)
{
    Console_Init();
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_TIM1EN;
    (void)RCC->APB2ENR;
    PinMode(GPIOA, 8u, 0xAu);    /* TIM1_CH1. */
    PinMode(GPIOB, 13u, 0xAu);   /* TIM1_CH1N. */
    GPIOB->BRR = 1u << 12;
    PinMode(GPIOB, 12u, 0x8u);   /* TIM1_BKIN, active high with pull-down. */
    TIM1->PSC = 7u;
    TIM1->ARR = 999u;
    TIM1->CCR1 = 400u;
    TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC1NE;
    TIM1->CR1 = TIM_CR1_ARPE;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = 0u;
    /* CKD = 0, so 80 tDTS at 8 MHz gives 10 us dead time. */
    TIM1->BDTR = 80u | TIM_BDTR_BKE | TIM_BDTR_BKP | TIM_BDTR_OSSI | TIM_BDTR_OSSR;
#ifndef LAB_NO_MOE
    TIM1->BDTR |= TIM_BDTR_MOE;
#endif
    TIM1->CR1 |= TIM_CR1_CEN;
    Console_Text("TIM1 CH1 PA8, CH1N PB13, BKIN PB12\r\n");
    for (;;) {
        if (TIM1->SR & TIM_SR_BIF) {
            TIM1->SR = (uint16_t)~TIM_SR_BIF;
            Console_Text("BREAK, release BKIN then reset\r\n");
            /* Leave the break latched until a board reset. */
            for (;;) {}
        }
    }
}
