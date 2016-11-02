#ifndef USART2_LIBRARY
#define USART2_LIBRARY

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>

#define BAUD_9600 9600
#define BAUD_4800 4800

//These are the prototypes for the routines
void Usart2_Init(int baudrate);
void Usart2_Send(uint8_t data);
uint8_t Usart2_Recieve(void);
void ConfigureUsart2Interrupt(void);
void Usart2_SendString(char* string);

#endif
