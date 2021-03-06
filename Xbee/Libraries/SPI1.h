#ifndef SPI1_LIBRARY
#define SPI1_LIBRARY

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_spi.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_misc.h>

//These are the Define statements

//These are the prototypes for the routines
void InitialiseSPI_GPIO(void);
void InitialiseSPI(void);
void SPI_send_byte(uint8_t byte);
void Configure_SPI_interrupt(void);
uint8_t SPI_RecieveByte(void);
void SPI_ManualSendByte(uint8_t byte);
uint8_t SPI_TransRecieve(uint8_t data);


#endif
