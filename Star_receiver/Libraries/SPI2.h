#ifndef SPI2_LIBRARY
#define SPI2_LIBRARY

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_spi.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_misc.h>

//These are the Define statements


//These are the prototypes for the routines
void InitialiseSPI2_GPIO(void);
void InitialiseSPI2(void);
void SPI2_send_byte(uint8_t byte);
void Configure_SPI2_interrupt(void);
uint8_t SPI2_RecieveByte(void);
void SPI2_ManualSendByte(uint8_t byte);
uint8_t SPI2_TransRecieve(uint8_t data);


#endif
