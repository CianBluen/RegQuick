#include "stm32f10x.h"
#include "board.h"

#define BUTTON_MASK (1UL << BUTTON_PIN)
#define LED_MASK (1UL << LED_PIN)
#define CONFIG_FIELD_MASK 0xFUL
#define INPUT_PULL 0x8UL
#define OUTPUT_PP_2MHZ 0x2UL

int main(void)
{
    uint32_t config;

#if !LAB_SKIP_GPIO_CLOCK
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    (void)RCC->APB2ENR;
#endif

    /* ODR0 = 1 selects pull-up in input-pull mode. Preload ODR1 = 0. */
    GPIOA->BSRR = BUTTON_MASK | (LED_MASK << 16U);
    config = GPIOA->CRL;
    config &= ~((CONFIG_FIELD_MASK << (4U * BUTTON_PIN)) |
                (CONFIG_FIELD_MASK << (4U * LED_PIN)));
    config |= (INPUT_PULL << (4U * BUTTON_PIN)) |
              (OUTPUT_PP_2MHZ << (4U * LED_PIN));
    GPIOA->CRL = config;

    for (;;) {
        if ((GPIOA->IDR & BUTTON_MASK) == 0U) {
            GPIOA->BSRR = LED_MASK;
        } else {
            GPIOA->BSRR = LED_MASK << 16U;
        }
    }
}
