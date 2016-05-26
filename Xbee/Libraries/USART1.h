#ifndef USART1_LIBRARY
#define USART1_LIBRARY

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>
#include <stdbool.h>
#include "IndicationGPIOs.h"

//These are the variables for recieving function
static bool stringRecieved = false;
static char usartRecievedStringBuffer[20];
static uint8_t usartRecievedStringLenght = 0;

//These are the prototypes for the routines
void Usart1Init(void);
void Usart1Send(uint8_t data);
char* Usart1Recieve(void);
void ConfigureUsartInterrupt(void);
void Usart1SendString(char* string);
char* Usart1RecieveString(void);
void waitForOkResponse(void);


#endif
