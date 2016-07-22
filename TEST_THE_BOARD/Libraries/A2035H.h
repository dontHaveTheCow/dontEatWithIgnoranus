#ifndef A2035H
#define A2035H

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
//These are the Define statements
#define WAKEUP_PIN GPIO_Pin_3					//PC3
#define GPS_RESET_PIN GPIO_Pin_2				//PC2
#define ON_PIN GPIO_Pin_4						//PF4
//These are the prototypes for the routines
void initializeA2035H(void);
void wake_da_A2035H(void);
void hibernate_da_A2035H(void);

#endif
