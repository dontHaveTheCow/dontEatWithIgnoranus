#ifndef XBEE_LIBRARY
#define XBEE_LIBRARY

//These are the Includes
#include <string.h>
#include "SysTickDelay.h"
#include <stm32f0xx_misc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_syscfg.h>
#include "SPI1.h"
#include "IndicationGPIOs.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

//These are the Defines and global variables
#define TRANSOPT_DISACK 0x00
#define TRANSOPT_DIGIMESH 0xC0

//Setup
void initializeXbeeAPI(void);
void initializeXbeeATTnPin(void);

//Functions
void apply1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1);
void apply2Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2);
void apply3Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3);
void apply4Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);
void queue1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1);
void queue2Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2);
void queue3Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3);
void queue4Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);

void readModuleParams(uint8_t MSbyte, uint8_t LSbyte);
void restoreDefaults(void);
void transmitRequest(uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, char* data);
void processByte(uint8_t byte);
void processRemainingBytes(void);

#endif
