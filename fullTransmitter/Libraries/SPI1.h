#ifndef SPI1_LIBRARY
#define SPI1_LIBRARY

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_spi.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_misc.h>

//These are the Define statements
#define XBEE_CS_LOW() GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define XBEE_CS_HIGH() GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define SDSELECT()      GPIOB->BRR = (1<<1)  // pin low, MMC CS = L
#define SDDESELECT()    GPIOB->BSRR = (1<<1) // pin high,MMC CS = H

//enums
enum speed_setting {
	INTERFACE_SLOW, INTERFACE_FAST
};

//These are the prototypes for the routines
void InitialiseSPI1_GPIO(void);
void InitialiseSPI1(void);
void Configure_SPI1_interrupt(void);
void SPI1_send_byte(uint8_t byte);
void SPI1_ManualSendByte(uint8_t byte);
uint8_t SPI1_RecieveByte(void);
uint8_t SPI1_TransRecieve(uint8_t data);

//Routines used for sd card
uint8_t spi_rw(uint8_t out);
void interface_speed(enum speed_setting speed);
uint8_t spi_r(void);
void release_spi();
void spi_r_m(uint8_t *byte);


#endif
