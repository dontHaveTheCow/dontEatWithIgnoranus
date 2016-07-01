#ifndef USART1_LIBRARY
#define USART1_LIBRARY

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>
#include <stdbool.h>
#include "IndicationGPIOs.h"

//These are the prototypes for the routines
void Usart1Init(int baudrate);
void Usart1Send(uint8_t data);
char* Usart1Recieve(void);
void ConfigureUsart1Interrupt(void);
void Usart1SendString(char* string);
char* Usart1RecieveString(char* String);

#endif
