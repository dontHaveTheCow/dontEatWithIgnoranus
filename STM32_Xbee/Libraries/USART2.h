#ifndef USART2_LIBRARY
#define USART2_LIBRARY

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>

//These are the prototypes for the routines
void Usart2Init(void);
void Usart2Send(uint8_t data);
uint8_t Usart2Recieve(void);
void ConfigureUsartInterrupt(void);
void usart2SendString(char* string);

#endif
