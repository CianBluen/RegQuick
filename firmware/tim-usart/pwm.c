#include "console.h"

#ifdef LAB_USE_SPL
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#endif

#define PWM_COUNTS 1000u
#define RX_ERRORS (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)

volatile uint16_t lab_period;
volatile uint16_t lab_high;
volatile uint32_t lab_captures;
volatile uint32_t lab_rx_errors;
static volatile uint32_t elapsed_ms;
static volatile uint32_t last_capture_ms;
static uint16_t duty = 25u;

void SysTick_Handler(void)
{
    ++elapsed_ms;
}

void TIM3_IRQHandler(void)
{
    const uint16_t status = TIM3->SR;
    const uint16_t period = TIM3->CCR1;
    const uint16_t high = TIM3->CCR2;
    if (status & (TIM_SR_CC1OF | TIM_SR_CC2OF)) {
        /* Capture reads clear CCxIF. Clear overcapture by writing zero. */
        TIM3->SR = (uint16_t)~(TIM_SR_CC1OF | TIM_SR_CC2OF);
        return;
    }
    if ((status & (TIM_SR_CC1IF | TIM_SR_CC2IF)) == (TIM_SR_CC1IF | TIM_SR_CC2IF)
        && high >= 3u && (uint32_t)high + 3u <= period) {
        lab_period = period;
        lab_high = high;
        ++lab_captures;
        last_capture_ms = elapsed_ms;
    }
}

static void Pwm_Init(void)
{
#ifdef LAB_USE_SPL
    TIM_TimeBaseInitTypeDef base;
    TIM_OCInitTypeDef output;
    TIM_ICInitTypeDef input;
    GPIO_InitTypeDef pin;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
    (void)RCC->APB1ENR;
    GPIO_StructInit(&pin);
    pin.GPIO_Pin = GPIO_Pin_1;
    pin.GPIO_Mode = GPIO_Mode_AF_PP;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &pin);
    pin.GPIO_Pin = GPIO_Pin_6;
    pin.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &pin);
    TIM_TimeBaseStructInit(&base);
    base.TIM_Prescaler = 7u;
    base.TIM_Period = PWM_COUNTS - 1u;
    TIM_TimeBaseInit(TIM2, &base);
    TIM_OCStructInit(&output);
    output.TIM_OCMode = TIM_OCMode_PWM1;
    output.TIM_OutputState = TIM_OutputState_Enable;
    output.TIM_Pulse = duty * 10u;
    TIM_OC2Init(TIM2, &output);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_GenerateEvent(TIM2, TIM_EventSource_Update);

    base.TIM_Period = 0xFFFFu;
    TIM_TimeBaseInit(TIM3, &base);
    TIM_ICStructInit(&input);
    input.TIM_Channel = TIM_Channel_1;
    input.TIM_ICPolarity = TIM_ICPolarity_Rising;
    input.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_PWMIConfig(TIM3, &input);
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
#else
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;
    PinMode(GPIOA, 1u, 0xAu);
    GPIOA->BRR = 1u << 6;
    PinMode(GPIOA, 6u, 0x8u);
    TIM2->PSC = 7u;
    TIM2->ARR = PWM_COUNTS - 1u;
    TIM2->CCR2 = duty * 10u;
    TIM2->CCMR1 = TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
    TIM2->CCER = TIM_CCER_CC2E;
    TIM2->CR1 = TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;

    TIM3->PSC = 7u;
    TIM3->ARR = 0xFFFFu;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
    TIM3->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
#ifndef LAB_WRONG_EDGE
    TIM3->CCER |= TIM_CCER_CC2P;
#endif
    TIM3->SMCR = TIM_SMCR_TS_2 | TIM_SMCR_TS_0 | TIM_SMCR_SMS_2;
#endif
    TIM3->SR = 0u;
    TIM3->DIER = TIM_DIER_CC1IE;
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM3->CR1 |= TIM_CR1_CEN;
    TIM2->CR1 |= TIM_CR1_CEN;
}

static void Print_Measurement(void)
{
    uint16_t period;
    uint16_t high;
    uint32_t age;
    uint32_t captures;
    __disable_irq();
    period = lab_period;
    high = lab_high;
    age = elapsed_ms - last_capture_ms;
    captures = lab_captures;
    __enable_irq();
    if (duty == 0u || duty == 100u || captures == 0u || age > 10u) {
        Console_Text("NO EDGE\r\n");
        return;
    }
    Console_Text("PERIOD_US ");
    Console_Number(period);
    Console_Text(" HIGH_US ");
    Console_Number(high);
    Console_Text("\r\n");
}

static void Apply_Command(const char *line, unsigned count)
{
    unsigned value = 0u;
    if (count == 1u && line[0] == '?') {
        Print_Measurement();
        return;
    }
    for (unsigned i = 0u; i < count; ++i) {
        if (line[i] < '0' || line[i] > '9') {
            Console_Text("ERR use 0..100 or ?\r\n");
            return;
        }
        value = 10u * value + (unsigned)(line[i] - '0');
    }
    if (count == 0u || value > 100u) {
        Console_Text("ERR use 0..100 or ?\r\n");
        return;
    }
    duty = (uint16_t)value;
#ifdef LAB_USE_SPL
    TIM_SetCompare2(TIM2, duty * 10u);
#else
    TIM2->CCR2 = duty * 10u;
#endif
    /* Let the preloaded compare and one full capture cycle settle. */
    const uint32_t start = elapsed_ms;
    while (elapsed_ms - start < 3u) {}
    Console_Text("OK ");
    Console_Number(duty);
    Console_Text("\r\n");
}

int main(void)
{
    char line[3];
    unsigned count = 0u;
    uint8_t discard = 0u;

    Console_Init();
    SysTick_Config(LAB_CLOCK_HZ / 1000u);
    Pwm_Init();
    Console_Text("PWM 0..100, ? measure\r\n");
    for (;;) {
        const uint32_t status = USART1->SR;
        if (!(status & (RX_ERRORS | USART_SR_RXNE))) {
            continue;
        }
        const char byte = (char)USART1->DR;
        if (status & RX_ERRORS) {
            ++lab_rx_errors;
            discard = 1u;
            continue;
        }
        if (byte == '\r' || byte == '\n') {
            if (discard) {
                Console_Text("ERR invalid line\r\n");
            } else if (count != 0u) {
                Apply_Command(line, count);
            }
            count = 0u;
            discard = 0u;
        } else if (count == sizeof line) {
            discard = 1u;
        } else if (!discard) {
            line[count++] = byte;
        }
    }
}
