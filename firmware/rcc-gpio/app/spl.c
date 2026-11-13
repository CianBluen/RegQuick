#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "board.h"

int main(void)
{
    GPIO_InitTypeDef config;

#if !LAB_SKIP_GPIO_CLOCK
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    (void)RCC->APB2ENR;
#endif

    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    config.GPIO_Pin = GPIO_Pin_1;
    config.GPIO_Mode = GPIO_Mode_Out_PP;
    config.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &config);

    config.GPIO_Pin = GPIO_Pin_0;
    config.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &config);

    for (;;) {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
            GPIO_SetBits(GPIOA, GPIO_Pin_1);
        } else {
            GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        }
    }
}
