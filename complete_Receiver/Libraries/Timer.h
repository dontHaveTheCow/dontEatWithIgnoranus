#ifndef TIMER_LIBRARY
#define TIMER_LIBRARY

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_misc.h>

//These are the Define statements
#define PRESCALER_VALUE 800
#define PERIOD_VALUE 1000

//These are the prototypes for the routines
void Initialize_timer(void);
void Timer_interrupt_enable(void);

#endif
