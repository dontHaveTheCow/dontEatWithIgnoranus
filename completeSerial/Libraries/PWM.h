#ifndef PWM_LIBRARY
#define PWM_LIBRARY

#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"

/*
 * Lenght must me between 0 and timer period
 */
#define TIMER_PERIOD 2000
#define TIMER_PRESCALER 48
#define PWM_PULSE_LENGHT(val) {TIM3->CCR1 = val;}

void InitializeTimer(void);
void InitializePwmPin(void);
void InitializePwm(void);

#endif
