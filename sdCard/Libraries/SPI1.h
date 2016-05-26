#ifndef SPI1_LIBRARY
#define SPI1_LIBRARY

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_spi.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_misc.h>

//These are the Define statements
enum speed_setting {
	INTERFACE_SLOW, INTERFACE_FAST
};

//These are the prototypes for the routines
void InitialiseSPI1_GPIO(void);
void InitialiseSPI1(void);
void Configure_SPI1_interrupt(void);

uint8_t spi_rw(uint8_t out);
void interface_speed(enum speed_setting speed);
uint8_t spi_r(void);
void release_spi();
void spi_r_m(uint8_t *byte);
/*void pull_cs_low(void);
void pull_cs_high(void);*/



#endif
