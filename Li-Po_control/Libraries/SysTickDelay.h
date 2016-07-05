#ifndef DELAY_LIBRARY
#define DELAY_LIBRARY

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>

//These are the Define statements
static __IO uint32_t sysTickCounter;
//These are the prototypes for the routines
void initialiseSysTick(void);
void delay_1ms(void);
void delay_400ms(void);
void delay_10us(void);
void delay_1s(void);
void delay_80ms(void);
void delay_10ms(void);
void delayMs(int ms);

#endif
