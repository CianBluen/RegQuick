#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

int main(void)
{
    GPIO_InitTypeDef pin;
    USART_InitTypeDef serial;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    (void)RCC->APB2ENR;

    pin.GPIO_Pin = GPIO_Pin_9;
    pin.GPIO_Mode = GPIO_Mode_AF_PP;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &pin);
    pin.GPIO_Pin = GPIO_Pin_10;
    pin.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &pin);

    USART_StructInit(&serial);
    serial.USART_BaudRate = 9600u;
    USART_Init(USART1, &serial);
    USART_Cmd(USART1, ENABLE);

    for (;;) {
        if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
            const uint16_t word = USART_ReceiveData(USART1);
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {}
            USART_SendData(USART1, word);
        }
    }
}
