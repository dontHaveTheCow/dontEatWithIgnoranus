#ifndef XBEE_LIBRARY
#define XBEE_LIBRARY

//These are the Includes

#include "SysTickDelay.h"
#include <stm32f0xx_misc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_syscfg.h>
#include "SPI1.h"
#include "USART1.h"
#include "IndicationGPIOs.h"

#include <stdbool.h>
#include <string.h>

//These are the Defines and global variables
#define TRANSOPT_DISACK 0x00
#define TRANSOPT_DIGIMESH 0xC0
extern bool readingPacket;

//Setup
void initializeXbeeAPI(void);
void initializeXbeeATTnPin(void);

//Functions
void apply1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1);
void queue1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1);

uint32_t readModuleParams(uint8_t MSbyte, uint8_t LSbyte);
void transmitRequest(uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, uint8_t frameID, char* data);

#endif
