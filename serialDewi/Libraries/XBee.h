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

//These are the Defines and global variables
extern bool readingPacket;

#define XBEE_CS_LOW() GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define XBEE_CS_HIGH() GPIO_SetBits(GPIOA,GPIO_Pin_4)

//Setup Xbee
void initializeXbeeATTnPin(void);

//Functions for AT commands, parameters
void xbeeApplyParamter(char* atCommand, uint8_t parameter, uint8_t frameID);
void askXbeeParam(char* atCommand, uint8_t frameID);
bool xbeeStartupParamRead(uint8_t _packetErrorLimit, uint8_t* _xbeeBuffer);
//Data transmission functions
void transmitRequest(uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, uint8_t frameID, char* data);
#endif
