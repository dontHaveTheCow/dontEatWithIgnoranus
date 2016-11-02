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
/*
 * Xbee defines
 */
#define AT_COMMAND_RESPONSE 0x88
#define RECIEVE_PACKET 0x90
#define MODEM_STATUS 0x8A
#define AT_COMMAND_DATA_INDEX 0x05
#define ASCII_DIGIT_OFFSET 0x30

#define TRANSOPT_DISACK 0x00
#define TRANSOPT_DIGIMESH 0xC0

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
bool xbeeStartupParamRead(uint8_t _packetErrorLimit, uint8_t* _xbeeBuffer);

//Data transmission functions
void transmitRequest(uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, char* data);

#endif
