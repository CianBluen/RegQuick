#include "console.h"

#define BUTTON (1u << 0)
#define LED (1u << 1)
#define DEBOUNCE_MS 25u
#define RESPONSE_MS 2000u

typedef enum { WAIT_RELEASE, READY, WAIT_SIGNAL, WAIT_RESPONSE } RoundState;

volatile uint32_t lab_ms;
volatile uint32_t lab_rounds;
volatile uint32_t lab_last_reaction;

void TIM4_IRQHandler(void)
{
    TIM4->SR = (uint16_t)~TIM_SR_UIF;
    ++lab_ms;
}

int main(void)
{
    RoundState state = WAIT_RELEASE;
    uint8_t raw_down = 0u;
    uint8_t stable_down = 1u; /* Require stable release after reset. */
    uint32_t changed_at = 0u;
    uint32_t wait_at = 0u;
    uint32_t wait_ms = 0u;
    uint32_t signal_at = 0u;

    Console_Init();
    GPIOA->BSRR = BUTTON;
    PinMode(GPIOA, 0u, 0x8u);
    PinMode(GPIOA, 1u, 0x2u);
    GPIOA->BRR = LED;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    (void)RCC->APB1ENR;
#ifdef LAB_WRONG_TICK
    TIM4->PSC = 15u;
#else
    TIM4->PSC = 7u;
#endif
    TIM4->ARR = 999u;
    TIM4->EGR = TIM_EGR_UG;
    TIM4->SR = 0u;
    TIM4->DIER = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM4_IRQn);
    TIM4->CR1 = TIM_CR1_CEN;
    Console_Text("Release then press to start\r\n");

    for (;;) {
        const uint32_t now = lab_ms;
        const uint8_t down = (GPIOA->IDR & BUTTON) == 0u;
        uint8_t pressed = 0u;
        if (down != raw_down) {
            raw_down = down;
            changed_at = now;
        }
        if (raw_down != stable_down && now - changed_at >= DEBOUNCE_MS) {
            stable_down = raw_down;
            pressed = stable_down;
        }

        switch (state) {
        case WAIT_RELEASE:
            if (!stable_down && !raw_down) {
                state = READY;
                Console_Text("READY\r\n");
            }
            break;
        case READY:
            if (pressed) {
                wait_ms = 800u + (now * 2654435761u) % 1201u;
                wait_at = now;
                state = WAIT_SIGNAL;
                Console_Text("WAIT\r\n");
            }
            break;
        case WAIT_SIGNAL:
            if (pressed) {
                state = WAIT_RELEASE;
                ++lab_rounds;
                Console_Text("EARLY\r\n");
            } else if (now - wait_at >= wait_ms && !raw_down && !stable_down) {
                /* Require a stable release before opening the response window. */
                GPIOA->BSRR = LED;
                signal_at = lab_ms;
                state = WAIT_RESPONSE;
            }
            break;
        case WAIT_RESPONSE:
            if (pressed || (now - signal_at >= RESPONSE_MS
                && !(raw_down && !stable_down && changed_at - signal_at < RESPONSE_MS))) {
                GPIOA->BRR = LED;
                state = WAIT_RELEASE;
                ++lab_rounds;
                if (pressed && changed_at - signal_at < RESPONSE_MS) {
                    lab_last_reaction = changed_at - signal_at;
                    Console_Text("REACTION_MS ");
                    Console_Number(lab_last_reaction);
                    Console_Text("\r\n");
                } else {
                    Console_Text("TIMEOUT\r\n");
                }
            }
            break;
        }
    }
}
