#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

int main(void)
{
    GPIO_InitTypeDef led;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    (void)RCC->APB2ENR;

    led.GPIO_Pin = GPIO_Pin_1;
    led.GPIO_Mode = GPIO_Mode_Out_PP;
    led.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &led);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
    for (;;) {}
}
