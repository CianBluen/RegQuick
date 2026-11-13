#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

int main(void)
{
    GPIO_InitTypeDef led;
    TIM_TimeBaseInitTypeDef timer;
    uint8_t on = 0u;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    (void)RCC->APB2ENR;
    (void)RCC->APB1ENR;

    led.GPIO_Pin = GPIO_Pin_1;
    led.GPIO_Mode = GPIO_Mode_Out_PP;
    led.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &led);
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);

    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = 7999u;
    timer.TIM_Period = 499u;
    TIM_TimeBaseInit(TIM2, &timer);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_Cmd(TIM2, ENABLE);

    for (;;) {
        if (TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) != RESET) {
            TIM_ClearFlag(TIM2, TIM_FLAG_Update);
            on ^= 1u;
            GPIO_WriteBit(GPIOA, GPIO_Pin_1, on ? Bit_SET : Bit_RESET);
        }
    }
}
