#ifndef ADC_LIBRARY
#define ADC_LIBRARY

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_adc.h>
#include <stm32f0xx_misc.h>
#include "string.h"
//These are the Define statements

//These are the prototypes for the routines
void adcConfig(void);
void adcInterruptConfig(void);
void adcPinConfig(void);

void reverse(char s[]);
void itoa(int n, char s[]);

#endif
