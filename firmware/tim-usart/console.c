#include "console.h"

#ifdef LAB_USE_SPL
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#endif

void Console_Init(void)
{
#ifdef LAB_USE_SPL
    GPIO_InitTypeDef pin;
    USART_InitTypeDef serial;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    (void)RCC->APB2ENR;
    GPIO_StructInit(&pin);
    pin.GPIO_Pin = GPIO_Pin_9;
    pin.GPIO_Mode = GPIO_Mode_AF_PP;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &pin);
    pin.GPIO_Pin = GPIO_Pin_10;
    pin.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &pin);
    USART_StructInit(&serial);
    serial.USART_BaudRate = LAB_BAUD;
    USART_Init(USART1, &serial);
    USART_Cmd(USART1, ENABLE);
#else
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    (void)RCC->APB2ENR;
    PinMode(GPIOA, 9u, 0xAu);    /* 2 MHz alternate-function push-pull. */
    GPIOA->BSRR = 1u << 10;
    PinMode(GPIOA, 10u, 0x8u);   /* Input pull-up. */
    USART1->BRR = (LAB_CLOCK_HZ + LAB_BAUD / 2u) / LAB_BAUD;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
#endif
}

static void Console_Byte(uint8_t value)
{
    while (!(USART1->SR & USART_SR_TXE)) {}
    USART1->DR = value;
}

void Console_Text(const char *text)
{
    while (*text != '\0') {
        Console_Byte((uint8_t)*text++);
    }
}

void Console_Number(uint32_t value)
{
    char digits[10];
    unsigned count = 0u;
    do {
        digits[count++] = (char)('0' + value % 10u);
        value /= 10u;
    } while (value != 0u);
    while (count != 0u) {
        Console_Byte((uint8_t)digits[--count]);
    }
}
