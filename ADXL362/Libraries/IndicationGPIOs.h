#ifndef GPIO_LIBRARY
#define GPIO_LIBRARY

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>

//These are the Define statements

//These are the prototypes for the routines
void initializeIndicationGPIO(void);
//void delayGPIOs(int count);
void blinkIndicationLedOnce(void);
void blinkIndicationLedTwice(void);
void blinkIndicationLedThrice(void);

#endif

