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

//These are the Defines
#define XBEE_PACKET_ERROR_LIMIT 30
// Defines xbee packet indexes
#define XBEE_TYPE_OF_FRAME_INDEX 0
#define XBEE_FRAME_ID_INDEX 1
#define XBEE_TRANSMIT_STATUS_DELIVERY_INDEX 0x05
#define XBEE_AT_COMMAND_INDEX 2
#define XBEE_AT_COMMAND_STATUS 4
#define XBEE_AT_COMMAND_DATA 5
// Defines for xbee frame types
#define XBEE_AT_COMMAND 0x08
#define XBEE_AT_COMMAND_RESPONSE 0x88
#define XBEE_TRANSMIT_REQUEST 0x10
#define XBEE_RECEIVE_PACKET 0x90
#define XBEE_MODEM_STATUS 0x8A
#define XBEE_TRANSMIT_STATUS 0x8B
#define ZIGBEE_TRANSMIT_STATUS 0x89
// Defines for xbee FRAME_ID
#define AT_FRAME_ID_REQUEST 0x52
#define AT_FRAME_ID_APPLY   0x50

//Defines for xbee RECEIVE_PACKET
#define XBEE_RECEIVE_ADDRESS_LOW_INDEX 5

//These are the Defines and global variables
#define XBEE_CS_LOW() GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define XBEE_CS_HIGH() GPIO_SetBits(GPIOA,GPIO_Pin_4)

//Global bool's for receive and transmit flags
extern bool xbeeSendingPacket;
extern bool xbeeReceivingPacket;
extern bool xbeePacketReady;

//Setup Xbee
void initializeXbeeATTnPin(void);

//Functions for AT commands, parameters
void xbeeApplyParamter(char* atCommand, uint8_t parameter, uint8_t frameID);
void askXbeeParam(char* atCommand, uint8_t frameID);
bool xbeeStartupParamRead(uint8_t _packetErrorLimit, uint8_t* _xbeeBuffer);
//Data transmission functions
void transmitRequest(uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, uint8_t frameID, char* data);

//Full-Duplex xbee api functions
uint8_t fillTransmitPacket( uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, uint8_t frameID, uint8_t* sendBuff, char* data);
uint8_t fillATPacket(char* atCommand, uint8_t parameter, uint8_t AT_FRAME, uint8_t* sendBuff);
void transreceiveAPIPacket(uint8_t* sendBuff, uint8_t* xbeeReceiveBuffer, uint16_t xbeeBytesToSend, uint16_t* xbeeBytesSent);

#endif
