#ifndef A2035H
#define A2035H

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
//#include <stm32f0xx_tim.h>
#include <stm32f0xx_misc.h>

//These are the Define statements
#define WAKEUP_PIN GPIO_Pin_3					//PC3
#define RESET_PIN GPIO_Pin_2					//PC2
#define ON_PIN GPIO_Pin_4						//PF4

//These are the prototypes for the routines
void turnGpsOn(void);
void hibernateGps(void);
void setupGpsGpio(void);
//void setupGpsTimer(void);
//void setupGpsTimerInterrupt(void);

#endif
