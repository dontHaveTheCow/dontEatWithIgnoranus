#include <stm32f0xx.h>
#include <stdlib.h>

#include "USART1.h"
#include "myStringFunctions.h"
#include "SPI2.h"
#include "Xbee.h"
#include "IndicationGPIOs.h"

// Led define
#define TOGGLE_REDLED_SERIAL() GPIOB->ODR ^= GPIO_Pin_5
#define TOGGLE_REDLED_XBEE() GPIOB->ODR ^= GPIO_Pin_6

//DEBUGGING macro
#define DEBUG_SERIAL  Usart1_SendString
#define DEBUG_SERIAL_BYTE Usart1_Send

// Defines serial
#define PACKET_HEADER_SIZE 7
#define PACKET_END_SYMBOL '\0'
#define PACKET_DATA_SYMBOL 21
#define ASCII_DIGIT_OFFSET 0x30

// Defines xbee
#define XBEE_PACKET_ERROR_LIMIT 30
// Defines xbee packet indexes
#define XBEE_TYPE_OF_FRAME_INDEX 0
#define XBEE_FRAME_ID_INDEX 1
// Defines for xbee frame types
#define XBEE_AT_COMMAND_RESPONSE 0x88
#define XBEE_AT_COMMAND_INDEX 2
#define XBEE_AT_COMMAND_STATUS 4
#define XBEE_AT_COMMAND_DATA 5
#define XBEE_RECEIVE_PACKET 0x90
#define XBEE_MODEM_STATUS 0x8A
#define XBEE_TRANSMIT_REQUEST 0x10
#define XBEE_TRANSMIT_STATUS 0x8B
#define ZIGBEE_TRANSMIT_STATUS 0x89
#define XBEE_TRANSMIT_STATUS_DELIVERY_OFFSET 0x05
// Defines for xbee FRAME_ID
#define XBEE_FRAME_ID_REQUEST 0x52
#define XBEE_FRAME_ID_APPLY   0x50

typedef enum {SENDER, RECIEVER, TWO_WAY, NONE} mote_role;

char s_delimiter[2] = "#";

// Radio
char serialBuffer[256];
uint8_t packetLenght = 0;
bool serialReceived = false;

// Variables for test
mote_role role = NONE;
uint8_t addressCount = 0;
uint32_t addresses[10][2];
uint16_t packetSize = 0;
uint16_t packetCount = 0;

// For listener
int recievedPackets = 0;
char receivedPacketsString[10];
bool experimentStarted = false;

// For Xbee
uint8_t xbeeBuffer[256];
bool xbeePacketReady = false;
uint8_t xbeePacketLenght;
uint8_t xbeeChecksum;
uint8_t xbeeErrorTimer;

uint32_t xbeeAddressHigh;
uint32_t xbeeAddressLow;
char xbeeAddressHighString[12];
char xbeeAddressLowString[12];

void processSerial(char* key, char* value){

	if(strcmp(key,"SET_MODE") == 0){

		int i_value = atoi(value);
		switch(i_value){
		case 1:
			role = RECIEVER;
			Usart1_SendString("LISTENING_MODE_SET");
			break;
		case 2:
			role = SENDER;
			Usart1_SendString("SENDER_MODE_SET");
			break;
		}
	}else if(strcmp(key,"SET_TARGET") == 0){

		str_splitter(value,xbeeAddressHighString,xbeeAddressLowString,s_delimiter);
		addresses[addressCount][0] = atoi(xbeeAddressHighString);
		addresses[addressCount++][1] = atoi(xbeeAddressLowString);

		Usart1_SendString("TARGET_ADDED:");
		Usart1_SendString(xbeeAddressHighString);
		Usart1_SendString(",");
		Usart1_SendString(xbeeAddressLowString);
		Usart1_SendString("\n");

	}else if(strcmp(key,"CLEAR_TARGETS") == 0){

		Usart1_SendString("TARGETS_CLEARED\n");
		addressCount = 0;

	}else if(strcmp(key,"SET_PACKET_SIZE") == 0){

		int i_value = atoi(value);
		packetSize = i_value;
		Usart1_SendString("PACKET_SIZE_SET_TO:");
		Usart1_SendString(value);
		Usart1_SendString("\n");

	}else if(strcmp(key,"SET_PACKET_COUNT") == 0){

		int i_value = atoi(value);
		packetCount = i_value;
		Usart1_SendString("PACKET_COUNT_SET_TO:");
		Usart1_SendString(value);
		Usart1_SendString("\n");

	}else if(strcmp(key,"START_EXPERIMENT") == 0){

		experimentStarted = true;


	}else if(strcmp(key,"END_EXPERIMENT") == 0){

		experimentStarted = false;
		Usart1_SendString("PACKETS RECEIVED#");
		itoa(recievedPackets,receivedPacketsString);
		Usart1_SendString(receivedPacketsString);
		Usart1_SendString("\n");
		recievedPackets = 0;

	}else if(strcmp(key,"SET_POWER") == 0){

		xbeeApplyParamter("PL",atoi(value),XBEE_FRAME_ID_APPLY);

	}else if(strcmp(key,"GET_POWER") == 0){

		askXbeeParam("PL",XBEE_FRAME_ID_REQUEST);

	}
	else if(strcmp(key,"GET_ADDRESS_HIGH") == 0){

		askXbeeParam("SH",XBEE_FRAME_ID_REQUEST);

	}
	else if(strcmp(key,"GET_ADDRESS_LOW") == 0){

		askXbeeParam("SL",XBEE_FRAME_ID_REQUEST);

	}else{
		// VERYWRONG DATA
		Usart1_SendString("WRONG INPUT DATA");
		Usart1_SendString("\n");
	}
}

void processXbeePacket(uint8_t* _xbeeBuffer){

	switch(_xbeeBuffer[XBEE_TYPE_OF_FRAME_INDEX]){

	case XBEE_AT_COMMAND_RESPONSE:
		if (strncmp((char*)&_xbeeBuffer[XBEE_AT_COMMAND_INDEX], "SH", 2) == 0) {
			if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
				Usart1_SendString("ADDR_HIGH#");
				xbeeAddressHigh = (_xbeeBuffer[XBEE_AT_COMMAND_DATA] << 24)
						+(_xbeeBuffer[XBEE_AT_COMMAND_DATA+1] << 16)
						+(_xbeeBuffer[XBEE_AT_COMMAND_DATA+2] << 8)
						+(_xbeeBuffer[XBEE_AT_COMMAND_DATA+3]);
				itoa(xbeeAddressHigh,xbeeAddressHighString);
				Usart1_SendString(xbeeAddressHighString);
				Usart1_SendString("\n");
			}else{
				Usart1_SendString("SH_AT_COMMAND_REQUEST_ERROR");
				Usart1_SendString("\n");
			}

		}else if (strncmp((char*)&_xbeeBuffer[XBEE_AT_COMMAND_INDEX], "SL", 2) == 0) {
			if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
				Usart1_SendString("ADDR_LOW#");
				xbeeAddressLow = (_xbeeBuffer[XBEE_AT_COMMAND_DATA] << 24)
						+(_xbeeBuffer[XBEE_AT_COMMAND_DATA+1] << 16)
						+(_xbeeBuffer[XBEE_AT_COMMAND_DATA+2] << 8)
						+(_xbeeBuffer[XBEE_AT_COMMAND_DATA+3]);
				itoa(xbeeAddressLow,xbeeAddressLowString);
				Usart1_SendString(xbeeAddressLowString);
				Usart1_SendString("\n");
			}else{
				Usart1_SendString("SL_AT_COMMAND_REQUEST_ERROR");
				Usart1_SendString("\n");
			}

		}else if (strncmp((char*)&_xbeeBuffer[XBEE_AT_COMMAND_INDEX], "PL", 2) == 0) {
			if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == XBEE_FRAME_ID_REQUEST){
				if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
					Usart1_SendString("XBEE POWER LEVEL:");
					Usart1_Send(_xbeeBuffer[XBEE_AT_COMMAND_DATA]+ASCII_DIGIT_OFFSET);
					Usart1_SendString("\n");

				}else{
					Usart1_SendString("PL_AT_COMMAND_REQUEST_ERROR");
					Usart1_SendString("\n");
				}
			}else if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == XBEE_FRAME_ID_APPLY){
				if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
					Usart1_SendString("PL_AT_COMMAND_APPLIED");
					Usart1_SendString("\n");

				}else{
					Usart1_SendString("PL_AT_COMMAND_APPLY_ERROR");
					Usart1_SendString("\n");
				}
			}
		}else {
			Usart1_SendString("UNEXPECTED_AT_COMMAND_RESPONSE XBEE PACKET");
			Usart1_SendString("\n");
		}
		break;
	case XBEE_RECEIVE_PACKET:
		Usart1_SendString("XBEE_RECEIVE_PACKET");
		Usart1_SendString("\n");
		recievedPackets++;
		break;

	case XBEE_MODEM_STATUS:
		Usart1_SendString("XBEE_MODEM_STATUS:");
		if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == 0x00){
			Usart1_SendString("HARDWARE_RESET");
		}else {
			Usart1_Send(_xbeeBuffer[XBEE_FRAME_ID_INDEX]);
		}
		Usart1_SendString("\n");
		break;

	case XBEE_TRANSMIT_STATUS:
		Usart1_SendString("XBEE_TRANSMIT_STATUS:");
		if(_xbeeBuffer[XBEE_TRANSMIT_STATUS_DELIVERY_OFFSET] == 0){
			Usart1_SendString("SUCCESS");
			Usart1_SendString("\n");
		}
		else{
			Usart1_SendString("FAILURE");
			Usart1_SendString("\n");
		}
		break;

	case ZIGBEE_TRANSMIT_STATUS:
		Usart1_SendString("ZIGBEE_TRANSMIT_STATUS:");
		if(_xbeeBuffer[XBEE_TRANSMIT_STATUS_DELIVERY_OFFSET] == 0){
			Usart1_SendString("SUCCESS");
			Usart1_SendString("\n");
		}
		else if(_xbeeBuffer[XBEE_TRANSMIT_STATUS_DELIVERY_OFFSET] == 0x74){
			Usart1_SendString("FAILURE - PAYOLOAD TOO LARGE");
			Usart1_SendString("\n");
		}
		else{
			Usart1_SendString("FAILURE");
			Usart1_SendString("\n");
		}
		break;
	default:
		Usart1_SendString("UNEXPECTED XBEE PACKET(dec):");
		Usart1_Send(_xbeeBuffer[XBEE_TYPE_OF_FRAME_INDEX]/100 + 0x30);
		Usart1_Send((_xbeeBuffer[XBEE_TYPE_OF_FRAME_INDEX]%100)/10 + 0x30);
		Usart1_Send(_xbeeBuffer[XBEE_TYPE_OF_FRAME_INDEX]%10 + 0x30);
		Usart1_SendString("\n");
	}
}

int main(void)
{
	//Local var's for serial
	char key[16] = "";
	char value[64];
	char xbeeSendPacket[1024];

	//Local var's for xbee
	initializeRedLed1();
	initializeRedLed2();
	TOGGLE_REDLED_SERIAL();

	//Usart1 for serial communication
	Usart1_Init(BAUD_115200);
	ConfigureUsart1Interrupt();

	//SPI2 for Xbee
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	initializeXbeeATTnPin();
	/*
	 * Xbee might have some boot data ready
	 * But our interrupt pin might not be initialized
	 * So do a manual startup check
	 */
	xbeePacketReady =  xbeeStartupParamRead(XBEE_PACKET_ERROR_LIMIT,xbeeBuffer);

	DEBUG_SERIAL("DEWI MODULE READY\n");

    while(1){

    	if(serialReceived == true){

    		str_splitter(serialBuffer,key,value,s_delimiter);
			processSerial(key, value );
			serialReceived = false;
		}
    	if(xbeePacketReady == true){

    		processXbeePacket(xbeeBuffer);
    		xbeePacketReady = false;
    	}
    	if(experimentStarted == true){

    		int packetCounter = 0;

    		for(packetCounter = 0; packetCounter < packetSize; packetCounter++){

    			if(packetCounter % 4 == 0){
    				xbeeSendPacket[packetCounter] = 'x';
    			}else if(packetCounter % 4 == 1){
    				xbeeSendPacket[packetCounter] = 'b';
    			}else if(packetCounter % 4 == 2){
    				xbeeSendPacket[packetCounter] = 'e';
    			}else if(packetCounter % 4 == 3){
    				xbeeSendPacket[packetCounter] = 'e';
    			}
    		}
    		xbeeSendPacket[packetSize] = '\0';

			Usart1_SendString("SENDING_PACKETS_TO ");
			Usart1_Send(addressCount+0x30);
			Usart1_SendString(" TARGET(S)!");
			Usart1_SendString(" EXPERIMENT STARTING...");
			Usart1_SendString("\n");

    		for(packetCounter = 0; packetCounter < packetCount; packetCounter++){

    			uint8_t targetCounter = 0;
    			for(targetCounter = 0; targetCounter < addressCount; targetCounter++){
    				TOGGLE_REDLED_XBEE();
    				transmitRequest(addresses[targetCounter][0],addresses[targetCounter][1],0x80, 0x00, xbeeSendPacket);
    			}
    		}
    		experimentStarted = false;
    		Usart1_SendString("EXPERIMENT FINISHED...");
    		Usart1_SendString("\n");
    	}
    }
}

// Serial data interrupt handler
void USART1_IRQHandler(void){
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){

		serialBuffer[packetLenght] = USART_ReceiveData(USART1);
		Usart1_Send(serialBuffer[packetLenght]);
		if(serialBuffer[packetLenght++] == '\r'){
			Usart1_Send(' ');
			TOGGLE_REDLED_SERIAL();
			serialBuffer[packetLenght-1] = '\0';
			serialReceived = true;
			packetLenght = 0;
		}
	}
}

// Xbee data ready interrupt handler
void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) == SET){

			xbeeErrorTimer = 0;
			xbeeChecksum = 0;
			XBEE_CS_LOW();
			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				xbeeErrorTimer++;
				if(xbeeErrorTimer > XBEE_PACKET_ERROR_LIMIT)			//Exit loop if there is no start delimiter
					break;
			}
			if(xbeeErrorTimer < XBEE_PACKET_ERROR_LIMIT){
				xbeePacketLenght = SPI1_TransRecieve(0x00) << 8; // Need to check this one
				xbeePacketLenght |= SPI1_TransRecieve(0x00);
				uint8_t i = 0;
				for(; i < xbeePacketLenght; i ++ ){				//Read data based on packet length
					xbeeChecksum +=(xbeeBuffer[i] = SPI1_TransRecieve(0x00));
				}
				xbeeBuffer[xbeePacketLenght] = '\0';
				xbeeChecksum += SPI1_TransRecieve(0x00);
				if(xbeeChecksum == 0xFF)
					xbeePacketReady = true;					//Data is updated if checksum is true
			}
			XBEE_CS_HIGH();
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}





