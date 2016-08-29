#ifndef XBEE_LIBRARY
#define XBEE_LIBRARY

//These are the Includes
#include <string.h>
#include <stdbool.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_syscfg.h>

#include "SysTickDelay.h"
#include "SPI1.h"
#include "IndicationGPIOs.h"
#include "USART1.h"

//debugging
//#include "debug.h"

//These are the Defines and global variables
extern bool readingPacket;

#define XBEE_CS_LOW() GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define XBEE_CS_HIGH() GPIO_SetBits(GPIOA,GPIO_Pin_4)

//Setup Xbee
void initializeXbeeATTnPin(void);

//Functions for AT commands, parameters
void apply1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1);
void queue1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1);
uint32_t readModuleParams(uint8_t MSbyte, uint8_t LSbyte);
void askModuleParams(uint8_t MSbyte, uint8_t LSbyte, uint8_t frameID);

//Data transmission functions
void transmitRequest(uint8_t adr1, uint8_t adr2, uint8_t adr3, uint8_t adr4, uint8_t adr5, uint8_t adr6, uint8_t adr7, uint8_t adr8, char* data);

#endif
