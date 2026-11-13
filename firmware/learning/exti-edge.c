#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

volatile uint32_t lab_edges;

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line0);
        ++lab_edges;
    }
}

int main(void)
{
    GPIO_InitTypeDef pin;
    EXTI_InitTypeDef edge;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    (void)RCC->APB2ENR;

    pin.GPIO_Pin = GPIO_Pin_0;
    pin.GPIO_Mode = GPIO_Mode_IPU;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &pin);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    edge.EXTI_Line = EXTI_Line0;
    edge.EXTI_Mode = EXTI_Mode_Interrupt;
    edge.EXTI_Trigger = EXTI_Trigger_Falling;
    edge.EXTI_LineCmd = ENABLE;
    EXTI_Init(&edge);
    EXTI_ClearITPendingBit(EXTI_Line0);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI0_IRQn);
    for (;;) {}
}
