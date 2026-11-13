#include "stm32f10x.h"
#include "clock.h"

#define BUTTON (1u << 0)
#define LED (1u << 1)
#define DEBOUNCE_MS 25u
#define PRIORITY_GROUP 5u  /* Two preemption bits and two subpriority bits. */

/* Inspect these in the debugger. Set a request to 1 while paused in main. */
volatile uint32_t lab_core_hz;
volatile uint32_t lab_ms;
volatile uint32_t lab_irq_count;
volatile uint32_t lab_presses;
volatile uint32_t lab_software_request;
volatile uint32_t lab_priority_request;
volatile uint32_t lab_priority_trace;

static volatile uint32_t button_pending;
static volatile uint32_t priority_probe;
static volatile uint32_t priority_probe_active;

void SysTick_Handler(void)
{
    ++lab_ms;
}

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & BUTTON) {
        EXTI->PR = BUTTON;
        ++lab_irq_count;
        button_pending = 1u;
        if (priority_probe) {
            lab_priority_trace = 1u;
            NVIC_SetPendingIRQ(EXTI1_IRQn);
            __DSB();
            __ISB();
            lab_priority_trace = lab_priority_trace * 10u + 3u;
            priority_probe = 0u;
        }
    }
}

/* Software-pended only. EXTI1 never handles the LED pin's transitions. */
void EXTI1_IRQHandler(void)
{
    if (priority_probe_active) {
        lab_priority_trace = lab_priority_trace * 10u + 2u;
        priority_probe_active = 0u;
    }
}

static void ConfigurePinsAndInterrupts(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    (void)RCC->APB2ENR;
    GPIOA->BSRR = BUTTON | (LED << 16);
    GPIOA->CRL = (GPIOA->CRL & ~0xFFu) | 0x28u;

    EXTI->IMR &= ~BUTTON;
#ifdef LAB_WRONG_PORT
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR;
    GPIOB->BSRR = BUTTON;
    GPIOB->CRL = (GPIOB->CRL & ~0xFu) | 0x8u;  /* Keep unconnected PB0 stable. */
    AFIO->EXTICR[0] = (AFIO->EXTICR[0] & ~0xFu) | 1u;  /* PB0 */
#else
    AFIO->EXTICR[0] &= ~0xFu;  /* PA0 */
#endif
    EXTI->RTSR &= ~BUTTON;
    EXTI->FTSR |= BUTTON;
    EXTI->PR = BUTTON;
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    NVIC_SetPriorityGrouping(PRIORITY_GROUP);
    NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(PRIORITY_GROUP, 1u, 0u));
#ifdef LAB_NO_PREEMPT
    NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(PRIORITY_GROUP, 1u, 1u));
#else
    NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(PRIORITY_GROUP, 0u, 0u));
#endif
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI0_IRQn);
    EXTI->IMR |= BUTTON;
}

int main(void)
{
    uint32_t stable = BUTTON;
    uint32_t candidate = BUTTON;
    uint32_t candidate_since = 0u;
    uint32_t tracking = 0u;
    uint32_t led_on = 0u;

    lab_core_hz = LabClock_Start();
    if (SysTick_Config(lab_core_hz / 1000u) != 0u) {
        for (;;) {}
    }
    ConfigurePinsAndInterrupts();

    for (;;) {
        uint32_t now;
        uint32_t level;
        uint32_t edge;

        if (lab_software_request != 0u) {
            lab_software_request = 0u;
            EXTI->SWIER = BUTTON;
        }
        if (lab_priority_request != 0u && priority_probe_active == 0u) {
            lab_priority_request = 0u;
            lab_priority_trace = 0u;
            priority_probe_active = 1u;
            priority_probe = 1u;
            EXTI->SWIER = BUTTON;
        }

        __disable_irq();
        edge = button_pending;
        button_pending = 0u;
        __enable_irq();
        if (edge != 0u || stable == 0u) {
            tracking = 1u;  /* Observe release until it is stable. */
        }
        if (tracking == 0u) {
            continue;
        }
        /* Sample after taking the edge so it cannot refer to an older sample. */
        level = GPIOA->IDR & BUTTON;
        now = lab_ms;
        if (level != candidate) {
            candidate = level;
            candidate_since = now;
        }
        if ((uint32_t)(now - candidate_since) < DEBOUNCE_MS) {
            continue;
        }
        if (candidate != stable) {
            stable = candidate;
            if (stable == 0u) {
                led_on ^= 1u;
                GPIOA->BSRR = led_on ? LED : (LED << 16);
                ++lab_presses;
            }
        }
        tracking = stable == 0u;
    }
}
