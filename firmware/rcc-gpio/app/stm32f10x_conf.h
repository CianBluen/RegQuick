#ifndef STM32F10X_CONF_H
#define STM32F10X_CONF_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#ifdef LAB_USE_SPL
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#endif

#define assert_param(expression) ((void)0)

#endif
