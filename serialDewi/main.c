#include <stm32f0xx.h>
#include <stdlib.h>
#include <math.h>

#include "USART1.h"
#include "myStringFunctions.h"
#include "SPI2.h"
#include "Xbee.h"
#include "IndicationGPIOs.h"
#include "SysTickDelay.h"
#include "Timer.h"

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
uint16_t testCount = 10;
uint16_t packetCount = 0;

// For listener
int recievedPackets = 0;
char receivedPacketsString[10];
bool experimentStarted = false;
bool accExperimentStarted = false;
bool gpsExperimentStarted = false;

// For Xbee
uint8_t xbeeReceiveBuffer[256];
uint8_t xbeeTransmitBuffer[1024];
uint16_t xbeeBytesSent = 0;
uint16_t xbeeBytesToSend = 0;
uint16_t xbeeBytesReceived = 0;
uint16_t xbeeBytesToReceive = 0;
bool xbeeSendingPacket = false;
bool xbeeReceivingPacket = false;
char xbeeRssiString[6];
bool xbeeReadRSSI = false;
uint8_t rssiThreshold = 60;
char rssiThresholdString[10];

bool xbeePacketReady = false;
uint8_t xbeePacketLenght;
uint8_t xbeeChecksum;
uint8_t xbeeErrorTimer;

uint32_t xbeeAddressHigh;
uint32_t xbeeAddressLow;
char xbeeAddressHighString[12];
char xbeeAddressLowString[12];
uint32_t targetLowAddress;
char targetAddressLowString[12];

//module timer
uint32_t globalCounter = 0;

void processSerial(char* key, char* value){

	if(strcmp(key,"SET_MODE") == 0){

		int i_value = atoi(value);
		switch(i_value){
		case 1:
			role = RECIEVER;
			Usart1_SendString("MSG#LISTENING_MODE_SET\n");
			break;
		case 2:
			role = SENDER;
			Usart1_SendString("MSG#SENDER_MODE_SET\n");
			break;
		}
	}else if(strcmp(key,"SET_TARGET") == 0){

		str_splitter(value,xbeeAddressHighString,xbeeAddressLowString,s_delimiter);
		addresses[addressCount][0] = atoi(xbeeAddressHighString);
		addresses[addressCount++][1] = atoi(xbeeAddressLowString);

		Usart1_SendString("MSG#TARGET_ADDED:");
		Usart1_SendString(xbeeAddressHighString);
		Usart1_SendString(",");
		Usart1_SendString(xbeeAddressLowString);
		Usart1_SendString("\n");

	}else if(strcmp(key,"CLEAR_TARGETS") == 0){

		Usart1_SendString("MSG#TARGETS_CLEARED\n");
		addressCount = 0;

	}else if(strcmp(key,"SET_PACKET_SIZE") == 0){

		packetSize = atoi(value);
		Usart1_SendString("MSG#PACKET_SIZE_SET_TO:");
		Usart1_SendString(value);
		Usart1_SendString("\n");

	}else if(strcmp(key,"SET_PACKET_COUNT") == 0){

		packetCount = atoi(value);
		Usart1_SendString("MSG#PACKET_COUNT_SET_TO:");
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

		xbeeBytesSent = 0;
		xbeeBytesToSend = fillATPacket("PL",atoi(value),AT_FRAME_ID_APPLY,xbeeTransmitBuffer);
		transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);

	}else if(strcmp(key,"GET_POWER") == 0){

		xbeeBytesSent = 0;
		xbeeBytesToSend = fillATPacket("PL",0,AT_FRAME_ID_REQUEST,xbeeTransmitBuffer);
		transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);

	}
	else if(strcmp(key,"SET_RO") == 0){

		xbeeBytesSent = 0;
		xbeeBytesToSend = fillATPacket("RO",atoi(value),AT_FRAME_ID_APPLY,xbeeTransmitBuffer);
		transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);

	}else if(strcmp(key,"GET_RO") == 0){

		xbeeBytesSent = 0;
		xbeeBytesToSend = fillATPacket("RO",0,AT_FRAME_ID_REQUEST,xbeeTransmitBuffer);
		transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);

	}
	else if(strcmp(key,"GET_ADDRESS_HIGH") == 0){

		xbeeBytesSent = 0;
		xbeeBytesToSend = fillATPacket("SH",0,AT_FRAME_ID_REQUEST,xbeeTransmitBuffer);
		transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);

	}
	else if(strcmp(key,"GET_ADDRESS_LOW") == 0){

		xbeeBytesSent = 0;
		xbeeBytesToSend = fillATPacket("SL",0,AT_FRAME_ID_REQUEST,xbeeTransmitBuffer);
		transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);

	}
	else if(strcmp(key,"SET_RSSI_ON") == 0){

		xbeeReadRSSI = true;
		Usart1_SendString("MSG#RSSI_ON\n");

	}
	else if(strcmp(key,"SET_RSSI_LEVEL") == 0){

		rssiThreshold = atoi(value);
		Usart1_SendString("MSG#RSSI_LEVEL_SET\n");

	}
	else if(strcmp(key,"SET_RSSI_OFF") == 0){

		xbeeReadRSSI = false;
		Usart1_SendString("MSG#RSSI_OFF\n");

	}
	else if(strcmp(key,"GET_TIME") == 0){

		Usart1_SendString("MSG#TIMER:");
		Usart1_Send((globalCounter%100000)/10000 + ASCII_DIGIT_OFFSET);
		Usart1_Send((globalCounter%10000)/1000 + ASCII_DIGIT_OFFSET);
		Usart1_Send((globalCounter%1000)/100 + ASCII_DIGIT_OFFSET);
		Usart1_Send((globalCounter%100)/10 + ASCII_DIGIT_OFFSET);
		Usart1_Send(globalCounter%10 + ASCII_DIGIT_OFFSET);
		Usart1_Send('\n');
	}
	else if(strcmp(key,"SET_TEST_COUNT") == 0){

		testCount = atoi(value);
		Usart1_SendString("MSG#TEST_COUNT_SET_TO:");
		Usart1_SendString(value);
		Usart1_SendString("\n");
	}
	else if(strcmp(key,"START_ACC_TEST") == 0){

		accExperimentStarted = true;
	}
	else if(strcmp(key,"START_GPS_TEST") == 0){

		gpsExperimentStarted = true;
	}
	else{
		// VERYWRONG DATA
		Usart1_SendString("MSG#WRONG_INPUT_DATA\n");
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
				Usart1_SendString("SH_AT_COMMAND_REQUEST_ERROR\n");
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
				Usart1_SendString("SL_AT_COMMAND_REQUEST_ERROR\n");
			}

		}else if (strncmp((char*)&_xbeeBuffer[XBEE_AT_COMMAND_INDEX], "PL", 2) == 0) {
			if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == AT_FRAME_ID_REQUEST){
				if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
					Usart1_SendString("MSG#XBEE POWER LEVEL:");
					Usart1_Send(_xbeeBuffer[XBEE_AT_COMMAND_DATA]+ASCII_DIGIT_OFFSET);
					Usart1_SendString("\n");

				}else{
					Usart1_SendString("PL_AT_COMMAND_REQUEST_ERROR\n");
				}
			}else if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == AT_FRAME_ID_APPLY){
				if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
					Usart1_SendString("MSG#PL_AT_COMMAND_APPLIED\n");

				}else{
					Usart1_SendString("MSG#PL_AT_COMMAND_APPLY_ERROR\n");
				}
			}
		}else if (strncmp((char*)&_xbeeBuffer[XBEE_AT_COMMAND_INDEX], "RO", 2) == 0) {
			if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == AT_FRAME_ID_REQUEST){
				if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
					Usart1_SendString("MSG#XBEE_PACKET_TIMEOUT:");
					Usart1_Send(_xbeeBuffer[XBEE_AT_COMMAND_DATA]+ASCII_DIGIT_OFFSET);
					Usart1_SendString("\n");

				}else{
					Usart1_SendString("RO_AT_COMMAND_REQUEST_ERROR\n");
				}
			}else if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == AT_FRAME_ID_APPLY){
				if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
					Usart1_SendString("MSG#RO_AT_COMMAND_APPLIED\n");

				}else{
					Usart1_SendString("MSG#RO_AT_COMMAND_APPLY_ERROR\n");
				}
			}
		}else if (strncmp((char*)&_xbeeBuffer[XBEE_AT_COMMAND_INDEX], "DB", 2) == 0) {
			if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == AT_FRAME_ID_REQUEST){
				if(_xbeeBuffer[XBEE_AT_COMMAND_STATUS] == 0){
					if(_xbeeBuffer[XBEE_AT_COMMAND_DATA] < rssiThreshold){
						Usart1_SendString("RSSI#");
						itoa(targetLowAddress,targetAddressLowString);
						Usart1_SendString(targetAddressLowString);
						Usart1_Send('#');
						itoa(-(_xbeeBuffer[XBEE_AT_COMMAND_DATA]), xbeeRssiString);
						Usart1_SendString(xbeeRssiString);
						Usart1_SendString("\n");
					}
					else{
						Usart1_SendString("RSSI_ALARM#");
						itoa(targetLowAddress,targetAddressLowString);
						Usart1_SendString(targetAddressLowString);
						Usart1_Send('#');
						itoa(-(_xbeeBuffer[XBEE_AT_COMMAND_DATA]), xbeeRssiString);
						Usart1_SendString(xbeeRssiString);
						Usart1_SendString("\n");
					}
				}else{
					Usart1_SendString("MSG#DB_AT_COMMAND_REQUEST_ERROR\n");
				}
			}
		}else {
			Usart1_SendString("MSG#UNEXPECTED_AT_COMMAND_RESPONSE XBEE PACKET\n");
		}
		break;
	case XBEE_RECEIVE_PACKET:

		recievedPackets++;
		if(xbeeReadRSSI){

			targetLowAddress = (_xbeeBuffer[XBEE_RECEIVE_ADDRESS_LOW_INDEX] << 24)
								+(_xbeeBuffer[XBEE_RECEIVE_ADDRESS_LOW_INDEX+1] << 16)
								+(_xbeeBuffer[XBEE_RECEIVE_ADDRESS_LOW_INDEX+2] << 8)
								+(_xbeeBuffer[XBEE_RECEIVE_ADDRESS_LOW_INDEX+3]);

			xbeeBytesSent = 0;
			xbeeBytesToSend = fillATPacket("DB",0,AT_FRAME_ID_REQUEST,xbeeTransmitBuffer);
			transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);
		}
		else{
			Usart1_SendString("MSG#XBEE_RECEIVE_PACKET\n");
		}
		break;

	case XBEE_MODEM_STATUS:
		Usart1_SendString("MSG#XBEE_MODEM_STATUS:");
		if(_xbeeBuffer[XBEE_FRAME_ID_INDEX] == 0x00){
			Usart1_SendString("HARDWARE_RESET");
		}else {
			Usart1_Send(_xbeeBuffer[XBEE_FRAME_ID_INDEX]);
		}
		Usart1_SendString("\n");
		break;

	case XBEE_TRANSMIT_STATUS:
		Usart1_SendString("MSG#XBEE_TRANSMIT_STATUS:");
		if(_xbeeBuffer[XBEE_TRANSMIT_STATUS_DELIVERY_INDEX] == 0){
			Usart1_SendString("SUCCESS\n");
		}
		else{
			Usart1_SendString("FAILURE\n");
		}
		break;

	case ZIGBEE_TRANSMIT_STATUS:
		Usart1_SendString("MSG#ZIGBEE_TRANSMIT_STATUS:");
		if(_xbeeBuffer[XBEE_TRANSMIT_STATUS_DELIVERY_INDEX] == 0){
			Usart1_SendString("SUCCESS\n");
		}
		else if(_xbeeBuffer[XBEE_TRANSMIT_STATUS_DELIVERY_INDEX] == 0x74){
			Usart1_SendString("FAILURE - PAYOLOAD_TOO_LARGE\n");
		}
		else{
			Usart1_SendString("FAILURE\n");
		}
		break;
	default:
		Usart1_SendString("MSG#UNEXPECTED XBEE PACKET(dec):");
		Usart1_Send(_xbeeBuffer[XBEE_TYPE_OF_FRAME_INDEX]/100 + ASCII_DIGIT_OFFSET);
		Usart1_Send((_xbeeBuffer[XBEE_TYPE_OF_FRAME_INDEX]%100)/10 + ASCII_DIGIT_OFFSET);
		Usart1_Send(_xbeeBuffer[XBEE_TYPE_OF_FRAME_INDEX]%10 + ASCII_DIGIT_OFFSET);
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
	Usart1_Init(BAUD_9600);
	ConfigureUsart1Interrupt();
	initialiseSysTick();

	//SPI2 for Xbee
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	initializeXbeeATTnPin();

	//Timer for ms counting
	Initialize_timer();
	Timer_interrupt_enable();

	//accelerometer
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	initializeADXL362();

	/*
	 * Simulating gps
	 */
	double lat1 = 56.979088;
	double lon1 = 24.185272;
	double lat2 = 56.980915;
	double lon2 = 24.191795;
	double R = 6378.137; // Radius of earth in KM
	double dLat;
	double dLon;
	double a;
	double c;
	double d;

	/*
	 * Xbee might have some boot data ready
	 * But our interrupt pin might not be initialized
	 * So do a manual startup check
	 */
	xbeePacketReady =  xbeeStartupParamRead(XBEE_PACKET_ERROR_LIMIT,xbeeReceiveBuffer);

	DEBUG_SERIAL("DEWI MODULE READY\n");
	//Set broadcast and mesh hops to 1
	xbeeBytesToSend = fillATPacket("BH",1,AT_FRAME_ID_APPLY,xbeeTransmitBuffer);
	transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);
	delayMs(20);
	xbeeBytesToSend = fillATPacket("NH",1,AT_FRAME_ID_APPLY,xbeeTransmitBuffer);
	transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);
	DEBUG_SERIAL("NETWORK_HOPS_SET_TO_MINIMUM\n");

    while(1){

    	if(serialReceived == true){

    		str_splitter(serialBuffer,key,value,s_delimiter);
			processSerial(key, value );
			serialReceived = false;
		}
    	if(xbeePacketReady == true){

    		processXbeePacket(xbeeReceiveBuffer);
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

			Usart1_SendString("MSG#SENDING_PACKETS_TO ");
			Usart1_Send(addressCount+ASCII_DIGIT_OFFSET);
			Usart1_SendString(" TARGET(S)!");
			Usart1_SendString(" EXPERIMENT STARTING...\n");

    		for(packetCounter = 0; packetCounter < packetCount; packetCounter++){

    			uint8_t targetCounter = 0;
    			for(targetCounter = 0; targetCounter < addressCount; targetCounter++){
    				TOGGLE_REDLED_XBEE();
    				xbeeBytesSent = 0;
    				xbeeBytesToSend = fillTransmitPacket(addresses[targetCounter][0],addresses[targetCounter][1],0x00, 0x40, xbeeTransmitBuffer, xbeeSendPacket);
    				transreceiveAPIPacket(xbeeTransmitBuffer,xbeeReceiveBuffer,xbeeBytesToSend, &xbeeBytesSent);
    				delayMs(100);
    			}
    		}
    		experimentStarted = false;
    		Usart1_SendString("MSG#EXPERIMENT FINISHED...\n");
    	}
    	else if(accExperimentStarted == true){

			Usart1_SendString("MSG#PERFORMING ACC EXPERIMENT ");
			Usart1_Send(testCount+ASCII_DIGIT_OFFSET);
			Usart1_SendString(" TIMES!");
			Usart1_SendString(" EXPERIMENT STARTING...\n");

			int testCounter;
			int countCounter;
			uint32_t tmpTimer = globalCounter;
			uint32_t timeElapsed;

			for(testCounter = 0; testCounter < testCount; testCounter++){

				Usart1_Send((testCounter%1000)/100+ASCII_DIGIT_OFFSET);
				Usart1_Send((testCounter%100)/10+ASCII_DIGIT_OFFSET);
				Usart1_Send(testCounter%10+ASCII_DIGIT_OFFSET);
				timeElapsed = globalCounter - tmpTimer;
				for(countCounter = 0; countCounter < testCounter; countCounter++){
					returnZ_axis();
					abs(1203 - 1230);
				}
				Usart1_SendString(":TIME ");
				Usart1_Send((timeElapsed%100000)/10000 + ASCII_DIGIT_OFFSET);
				Usart1_Send((timeElapsed%10000)/1000 + ASCII_DIGIT_OFFSET);
				Usart1_Send((timeElapsed%1000)/100 + ASCII_DIGIT_OFFSET);
				Usart1_Send((timeElapsed%100)/10 + ASCII_DIGIT_OFFSET);
				Usart1_Send(timeElapsed%10 + ASCII_DIGIT_OFFSET);
				Usart1_Send(' ');
				Usart1_Send('\n');
			}

			Usart1_SendString(" EXPERIMENT FINISHED...\n");
    		accExperimentStarted = false;
    	}
    	else if(gpsExperimentStarted == true){

			Usart1_SendString("MSG#PERFORMING GPS EXPERIMENT WITH ");
			Usart1_Send(testCount+ASCII_DIGIT_OFFSET);
			Usart1_SendString(" TARGET(S)!");
			Usart1_SendString(" EXPERIMENT STARTING...\n");

			int testCounter;
			int countCounter;
			uint32_t tmpTimer = globalCounter;
			uint32_t timeElapsed;

			for(testCounter = 0; testCounter < testCount; testCounter++){

				Usart1_Send(testCounter/10+ASCII_DIGIT_OFFSET);
				Usart1_Send(testCounter%10+ASCII_DIGIT_OFFSET);
				timeElapsed = globalCounter - tmpTimer;
				for(countCounter = 0; countCounter < testCounter; countCounter++){

					/*
					 * Test routine here
					 */
					dLat = lat2 * M_PI / 180 - lat1 * M_PI / 180;
					dLon = lon2 * M_PI / 180 - lon1 * M_PI / 180;
					a = sin(dLat/2) * sin(dLat/2) + cos(lat1 * M_PI / 180) * cos(lat2 * M_PI / 180) * sin(dLon/2) * sin(dLon/2);
					c = 2 * atan2(sqrt(a), sqrt(1-a));
					d = R * c;
					d  = d * 1000; // meters
				}
				Usart1_SendString(":TIME ");
				Usart1_Send((timeElapsed%100000)/10000 + ASCII_DIGIT_OFFSET);
				Usart1_Send((timeElapsed%10000)/1000 + ASCII_DIGIT_OFFSET);
				Usart1_Send((timeElapsed%1000)/100 + ASCII_DIGIT_OFFSET);
				Usart1_Send((timeElapsed%100)/10 + ASCII_DIGIT_OFFSET);
				Usart1_Send(timeElapsed%10 + ASCII_DIGIT_OFFSET);
				Usart1_Send(' ');
				Usart1_Send('\n');
			}

			Usart1_SendString(" EXPERIMENT FINISHED...\n");
    		gpsExperimentStarted = false;
    	}
    }
}

// Serial data interrupt handler
void USART1_IRQHandler(void){
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){

		serialBuffer[packetLenght] = USART_ReceiveData(USART1);
		//Usart1_Send(serialBuffer[packetLenght]);
		if(serialBuffer[packetLenght++] == '\r'){
			serialBuffer[packetLenght-1] = '\0';
			//Usart1_Send(' ');
			TOGGLE_REDLED_SERIAL();
			serialReceived = true;
			packetLenght = 0;
		}
	}
}

// Xbee data ready interrupt handler
void EXTI4_15_IRQHandler(void){

	if(EXTI_GetITStatus(EXTI_Line4) == SET){

		xbeeChecksum = 0;
		xbeeBytesReceived = 0;
		xbeePacketReady = false;
		/*
		 * If xbee starts MISO communication while data is going through MOSI,
		 * Packet is received in the routine where the MOSI data is transmitted
		 */
		if(xbeeSendingPacket){
			xbeeReceivingPacket = true;
		}
		else{
			XBEE_CS_LOW();
			//If at the time packet is being sent, full-duplex operation will be performed
			SPI1_TransRecieve(0x00);						 //Start delimiter
			xbeePacketLenght = SPI1_TransRecieve(0x00) << 8; // Need to check this one
			xbeePacketLenght |= SPI1_TransRecieve(0x00);

			for(; xbeeBytesReceived < xbeePacketLenght; xbeeBytesReceived ++ ){				//Read data based on packet length
				xbeeChecksum +=(xbeeReceiveBuffer[xbeeBytesReceived] = SPI1_TransRecieve(0x00));
			}
			xbeeChecksum += SPI1_TransRecieve(0x00);
			if(xbeeChecksum == 0xFF)
				xbeePacketReady = true;					//Data is updated if checksum is true
			XBEE_CS_HIGH();
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

void TIM2_IRQHandler(){

	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		globalCounter++;

	}
}





