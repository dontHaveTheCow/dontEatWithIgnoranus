/*
 * STM32 and C libraries
 */
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
/*
 * DEW| libraries
 */
#include "SPI1.h"
#include "SysTickDelay.h"
#include "USART1.h"
#include "MyStringFunctions.h"
#include "Timer.h"
#include "Xbee.h"
#include "IndicationGPIOs.h"

/*
 * XBEE packet defines
 */

/*
 * XBEE defines
 */
#define ERROR_TIMER_COUNT 30
#define XBEE_DATA_MODE_OFFSET 12
#define XBEE_DATA_TYPE_OFFSET 14
#define TIMER_SYNC_DELAY 5

#define COORDINATOR_ADDR_HIGH 0x0013A200
#define COORDINATOR_ADDR_LOW 0x40E3E13C

#define NUMBER_OF_NODES 5
/*
 * SERIAL defines
 */
#define ASCII_DIGIT_OFFSET 0x30
/*
 * Any other defines
 */
#define TOGGLE_REDLED_SERIAL() GPIOB->ODR ^= GPIO_Pin_5
#define TOGGLE_REDLED_XBEE() GPIOB->ODR ^= GPIO_Pin_6
#define SET_REDLED_SERIAL() GPIOB->ODR |= GPIO_Pin_5;
/*
 * Serial globals
 */
char serialBuffer[256];
uint8_t packetLenght = 0;
bool serialUpdated = false;
/*
 * XBEE globals
 */
char xbeeReceiveBuffer[255] = " ";
volatile bool xbeeDataUpdated = false;
volatile uint8_t length,errorTimer,cheksum;
bool SPI1_Busy = false;
bool xbeeReading = false;
/*
 * Module globals
 */
uint32_t globalCounter = 0;

int main(void)
{
	//Node struct
	struct node{
		uint32_t addressHigh;
		uint32_t addressLow;
	};

	//Nodes
	struct node node[NUMBER_OF_NODES];
	node[0].addressHigh = 0x0013A200;
	node[0].addressLow = 0x409783D9;

	node[1].addressHigh = 0x0013A200;
	node[1].addressLow = 0x409783DA;

	node[2].addressHigh = 0x0013A200;
	node[2].addressLow = 0x40E3E13D;

	node[3].addressHigh = 0x0013A200;
	node[3].addressLow = 0x40E3E13A;

	//coordinator node
	node[4].addressHigh = COORDINATOR_ADDR_HIGH;
	node[4].addressLow = COORDINATOR_ADDR_LOW;

	//Local variables - serial
	char s_delimiter[2] = "#";
	char key[32] = "";
	char value[64];
	char timerString[10];
	//Local variables - XBEE
	uint32_t xbeeAddressHigh;
	uint32_t xbeeAddressLow;
	char xbeeAddressHighString[12];
	char xbeeAddressLowString[12];
	char xbeeTransmitString[64];
	uint8_t frameID;

	uint32_t receivedAddressHigh = 0;
	uint32_t receivedAddressLow = 0;
	uint8_t iterator = 1;
	uint8_t niterator = 0;
	uint8_t tmpNode = 0;
	//Initialize peripherals
	initializeEveryRedLed();
	initializeEveryGreenLed();
	initializeXbeeATTnPin();

	SET_REDLED_SERIAL();

	Usart1_Init(BAUD_9600);
	ConfigureUsart1Interrupt();
	initialiseSysTick();
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	Configure_SPI1_interrupt();
	Initialize_timer();
	Timer_interrupt_enable();

	//Wait 1 second for XBEE to start UpP...
	delayMs(1000);
	xbeeDataUpdated = xbeeStartupParamRead(ERROR_TIMER_COUNT,(uint8_t*)xbeeReceiveBuffer);
	/*
	 * Synchronize time with coordinator
	 */

	/*
	 * Module ready
	 */
	SEND_SERIAL_MSG("MSG#SERIAL_MODULE_READY\r\n");
	TOGGLE_REDLED_SERIAL();
	TOGGLE_REDLED_XBEE();


	 while(1){
    	if(serialUpdated){
    		/*
    		 * Split received serial message
    		 */
    		str_splitter(serialBuffer,key,value,s_delimiter);
    		/*
    		 * Process serial
    		 */
			if(strcmp(key,"GET_TIME") == 0){

				SEND_SERIAL_MSG("MSG#CURRENT_TIME#");
				SEND_SERIAL_BYTE((globalCounter%1000000)/100000 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_BYTE((globalCounter%100000)/10000 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_BYTE((globalCounter%10000)/1000 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_BYTE((globalCounter%1000)/100 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_BYTE((globalCounter%100)/10 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_BYTE(globalCounter%10 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_MSG("\r\n");
			}
			else if(strcmp(key,"GET_ADDRESS_HIGH") == 0){

				askModuleParams('S','H',0x01);
			}
			else if(strcmp(key,"GET_ADDRESS_LOW") == 0){

				askModuleParams('S','L',0x01);
			}
			else if(strcmp(key,"SYNC_TIME") == 0){

				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,"C 0");
			}
			else if(strcmp(key,"SET_EVENT") == 0){

				SEND_SERIAL_MSG("EVENTS#");
				SEND_SERIAL_MSG(value);
				SEND_SERIAL_BYTE('#');
				itoa(globalCounter,timerString);
				SEND_SERIAL_MSG(timerString);
				SEND_SERIAL_MSG("\r\n");
			}
			else if(strcmp(key,"SET_EVENT_COORD") == 0){

				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,"C 2");
				SEND_SERIAL_MSG("DEBUG#CMD#ACCEPTED...\r\n");
			}
			else if(strcmp(key,"SET_THRACC") == 0){

				strcpy(&xbeeTransmitString[0],"C 3 ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#CMD#ACCEPTED...\r\n");
			}
			else if(strcmp(key,"SET_THRGPS") == 0){

				strcpy(&xbeeTransmitString[0],"C 4 ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#CMD#ACCEPTED...\r\n");
			}
			else if(strcmp(key,"SET_THRRSSI") == 0){

				strcpy(&xbeeTransmitString[0],"C 5 ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#CMD#ACCEPTED...\r\n");
			}
			else if(strcmp(key,"GET_THRRSSI") == 0){

				strcpy(&xbeeTransmitString[0],"C 6");
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
			}
			else if(strcmp(key,"GET_THRACC") == 0){

				strcpy(&xbeeTransmitString[0],"C 7");
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
			}
			else if(strcmp(key,"GET_THRGPS") == 0){

				strcpy(&xbeeTransmitString[0],"C 8");
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
			}
			else if(strcmp(key,"GET_DATA_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C C ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
			}
			else if(strcmp(key,"GET_DATA_ALL") == 0){

				//Parameter "7" tells to get data from every node
				strcpy(&xbeeTransmitString[0],"C C ");
				strcat(xbeeTransmitString,"7");
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
			}
			else if(strcmp(key,"STOP_DATA_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C D ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
			}
			else if(strcmp(key,"STOP_DATA_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C D ");
				strcat(xbeeTransmitString,"7");
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
			}
			else if(strcmp(key,"INIT_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C G ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"INIT_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C G ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[0].addressHigh,node[0].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[1].addressHigh,node[1].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[2].addressHigh,node[2].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[3].addressHigh,node[3].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKETS#SENT...\r\n");
			}
			else if(strcmp(key,"INIT_ACC_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C H ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");

			}
			else if(strcmp(key,"INIT_ACC_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C H ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[0].addressHigh,node[0].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[1].addressHigh,node[1].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[2].addressHigh,node[2].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[3].addressHigh,node[3].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKETS#SENT...\r\n");

			}
			else if(strcmp(key,"INIT_GPS_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C I ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"INIT_GPS_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C I ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[0].addressHigh,node[0].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[1].addressHigh,node[1].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[2].addressHigh,node[2].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[3].addressHigh,node[3].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKETS#SENT...\r\n");
			}
			else if(strcmp(key,"INIT_SD_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C J ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"INIT_SD_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C J ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[0].addressHigh,node[0].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[1].addressHigh,node[1].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[2].addressHigh,node[2].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(2000);
				transmitRequest(node[3].addressHigh,node[3].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKETS#SENT...\r\n");
			}
			else if(strcmp(key,"START_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C K ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(150);
				transmitRequest(node[0].addressHigh,node[0].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(150);
				transmitRequest(node[1].addressHigh,node[1].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(150);
				transmitRequest(node[2].addressHigh,node[2].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(150);
				transmitRequest(node[3].addressHigh,node[3].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKETS#SENT...\r\n");
			}
			else if(strcmp(key,"START_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C K ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"IDLE_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C L ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[0].addressHigh,node[0].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(node[1].addressHigh,node[1].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(node[2].addressHigh,node[2].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(node[3].addressHigh,node[3].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKETS#SENT...\r\n");
			}
			else if(strcmp(key,"IDLE_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C L ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"STOP_ALL") == 0){

				strcpy(&xbeeTransmitString[0],"C M ");
				strcat(xbeeTransmitString,value);

				transmitRequest(node[0].addressHigh,node[0].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(node[1].addressHigh,node[1].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(node[2].addressHigh,node[2].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(node[3].addressHigh,node[3].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				delayMs(1000);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);

				SEND_SERIAL_MSG("DEBUG#PACKETS#SENT...\r\n");
			}
			else if(strcmp(key,"STOP_NODE") == 0){

				strcpy(&xbeeTransmitString[0],"C M ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"SET_THRTIM") == 0){

				strcpy(&xbeeTransmitString[0],"C P ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#CMD#ACCEPTED...\r\n");
			}
			else if(strcmp(key,"GET_THRTIM") == 0){

				strcpy(&xbeeTransmitString[0],"C R ");
				strcat(xbeeTransmitString,value);
				transmitRequest(COORDINATOR_ADDR_HIGH,COORDINATOR_ADDR_LOW,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"GET_BATTERY") == 0){

				strcpy(&xbeeTransmitString[0],"C S ");
				strcat(xbeeTransmitString,value);
				transmitRequest(node[atoi(value)].addressHigh,node[atoi(value)].addressLow,TRANSOPT_DISACK, 0x00,xbeeTransmitString);
				SEND_SERIAL_MSG("DEBUG#PACKET#SENT...\r\n");
			}
			else if(strcmp(key,"PRINT_CMD") == 0){

				SEND_SERIAL_MSG("\nMSG#GET_TIME\r\n");
				SEND_SERIAL_MSG("MSG#SYNC_TIME\r\n");
				SEND_SERIAL_MSG("MSG#GET_ADDRESS_HIGH\r\n");
				SEND_SERIAL_MSG("MSG#GET_ADDRESS_LOW\r\n");
				SEND_SERIAL_MSG("MSG#SET_EVENT#\r\n");
				SEND_SERIAL_MSG("MSG#SET_EVENT_COORD\r\n");
				SEND_SERIAL_MSG("MSG#SET_THRACC#\r\n");
				SEND_SERIAL_MSG("MSG#SET_THRGPS#\r\n");
				SEND_SERIAL_MSG("MSG#SET_THRRSSI#\r\n");
				SEND_SERIAL_MSG("MSG#SET_THRTIM#\r\n");
				SEND_SERIAL_MSG("MSG#GET_THRACC#\r\n");
				SEND_SERIAL_MSG("MSG#GET_THRGPS#\r\n");
				SEND_SERIAL_MSG("MSG#GET_THRRSSI#\r\n");
				SEND_SERIAL_MSG("MSG#GET_THRTIM#\r\n");
				SEND_SERIAL_MSG("MSG#GET_BATTERY#\r\n");
				SEND_SERIAL_MSG("MSG#GET_DATA_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#STOP_DATA_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#<--INIT_COMMANDS-->");
				SEND_SERIAL_MSG("MSG#INIT_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#INIT_ALL\r\n");
				SEND_SERIAL_MSG("MSG#INIT_ACC_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#INIT_ACC_ALL\r\n");
				SEND_SERIAL_MSG("MSG#INIT_GPS_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#INIT_GPS_ALL\r\n");
				SEND_SERIAL_MSG("MSG#INIT_SD_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#INIT_GPS_ALL\r\n");
				SEND_SERIAL_MSG("MSG#START_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#START_ALL\r\n");
				SEND_SERIAL_MSG("MSG#IDLE_ALL\r\n");
				SEND_SERIAL_MSG("MSG#INIT_NODE#\r\n");
				SEND_SERIAL_MSG("MSG#STOP_ALL\r\n");
			}
			else{
				// VERYWRONG DATA
				SEND_SERIAL_MSG("MSG#WRONG_INPUT_DATA\r\n");
				SEND_SERIAL_MSG("MSG#>>>PRINT_CMD<<<\r\n");
			}
    		serialUpdated = false;
    	}
    	if(xbeeDataUpdated == true){

		switch(xbeeReceiveBuffer[XBEE_TYPE_OF_FRAME_INDEX]){
		case XBEE_AT_COMMAND_RESPONSE:

		if(strncmp((char*)&xbeeReceiveBuffer[XBEE_AT_COMMAND_INDEX], "SH", 2) == 0) {
			if(xbeeReceiveBuffer[XBEE_AT_COMMAND_STATUS] == 0){
				SEND_SERIAL_MSG("DEBUG#ADDRESS_HIGH#");
				xbeeAddressHigh = (xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA] << 24)
						+(xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA+1] << 16)
						+(xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA+2] << 8)
						+(xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA+3]);
				itoa(xbeeAddressHigh,xbeeAddressHighString);
				SEND_SERIAL_MSG(xbeeAddressHighString);
				SEND_SERIAL_MSG("\r\n");
			}else{
				SEND_SERIAL_MSG("DEBUG#SH_AT_COMMAND_REQUEST_ERROR\n");
			}
		}else if(strncmp((char*)&xbeeReceiveBuffer[XBEE_AT_COMMAND_INDEX], "SL", 2) == 0) {

			if(xbeeReceiveBuffer[XBEE_AT_COMMAND_STATUS] == 0){
				SEND_SERIAL_MSG("DEBUG#ADDR_LOW#");
				xbeeAddressLow = (xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA] << 24)
						+(xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA+1] << 16)
						+(xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA+2] << 8)
						+(xbeeReceiveBuffer[XBEE_AT_COMMAND_DATA+3]);
				itoa(xbeeAddressLow,xbeeAddressLowString);
				SEND_SERIAL_MSG(xbeeAddressLowString);
				SEND_SERIAL_MSG("\r\n");
			}
			else{
				SEND_SERIAL_MSG("DEBUG#SL_AT_COMMAND_REQUEST_ERROR\n");
			}
		}
		else {
			SEND_SERIAL_MSG("MSG#UNEXPECTED_AT_COMMAND_RESPONSE XBEE PACKET\n");
		}
		break;
		case XBEE_RECEIVE_PACKET:
			if(xbeeReceiveBuffer[XBEE_DATA_MODE_OFFSET] == 'M'){
				/*
				 * Serial node should not receive any measurements
				 */
			}
			else if(xbeeReceiveBuffer[XBEE_DATA_MODE_OFFSET] == 'C'){
				/*
				 * Example
				 * C 0 -> Request for time stamp
				 * C 1
				 */
				switch(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET]){

				case ('0'):
					break;
				case ('1'):
					globalCounter = atoi(&xbeeReceiveBuffer[16]) + TIMER_SYNC_DELAY;
					SEND_SERIAL_MSG("MSG#TIMER#SYNCHRONIZED...\r\n");
					break;
				case ('9'):
					SEND_SERIAL_MSG("MSG#RSSI_THRESHOLD#");
					SEND_SERIAL_MSG(&xbeeReceiveBuffer[16]);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('A'):
					SEND_SERIAL_MSG("MSG#ACC_THRESHOLD#");
					SEND_SERIAL_MSG(&xbeeReceiveBuffer[16]);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('B'):
					SEND_SERIAL_MSG("MSG#GPS_THRESHOLD#");
					SEND_SERIAL_MSG(&xbeeReceiveBuffer[16]);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('E'):
					//Transfered accelerometer and RSSI measurement
					//"C E#3#30#7#1224"

					//id
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2]);
					SEND_SERIAL_BYTE('#');

					if((xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+7] - 0x30) & 0x01){
						//If ACC error
						SEND_SERIAL_MSG("ACC_DNG");
					}
					else{
						SEND_SERIAL_MSG("ACC_MSR");
					}

					SEND_SERIAL_MSG(&xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+8]);
					SEND_SERIAL_MSG("\r\n");

					//id
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2]);
					SEND_SERIAL_BYTE('#');

					if(((xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+7] - 0x30) >> 1)&0x01){
						//If RSSI error
						SEND_SERIAL_MSG("RSSI_DNG#");
					}
					else{
						SEND_SERIAL_MSG("RSSI_MSR#");
					}
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+4]);
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+5]);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('F'):
					//Transfered velocity and RSSI measurement
					//"C F#3#30#7#5.8"

					//id
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2]);
					SEND_SERIAL_BYTE('#');

					if(((xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+7] - 0x30) >> 2)&0x01){
						//If GPS error
						SEND_SERIAL_MSG("GPS_DNG");
					}
					else{
						SEND_SERIAL_MSG("GPS_MSR");
					}
					SEND_SERIAL_MSG(&xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+8]);
					SEND_SERIAL_MSG("\r\n");

					//id
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2]);
					SEND_SERIAL_BYTE('#');
					if(((xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+7] - 0x30) >> 1)&0x01){
						//If RSSI error
						SEND_SERIAL_MSG("RSSI_DNG#");
					}
					else{
						SEND_SERIAL_MSG("RSSI_MSR#");
					}
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+4]);
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+5]);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('N'):
					//Transfered velocity and RSSI measurement
					//"C N#<state><\n>
					receivedAddressHigh = 0x00;
					receivedAddressLow = 0x00;

					for(iterator = 1; iterator < 9; iterator++){	//Read address from received packet

						if(iterator<5){
							receivedAddressHigh |= xbeeReceiveBuffer[iterator] << 8*(4-iterator);
						}
						else{
							receivedAddressLow |= xbeeReceiveBuffer[iterator] << 8*(8-iterator);
						}
					}

					for(niterator = 0; niterator < NUMBER_OF_NODES; niterator++){	//Find the matching node for the received address
						if(receivedAddressLow == node[niterator].addressLow )
							tmpNode = niterator;
					}
					SEND_SERIAL_MSG("MSG#NODE#");
					SEND_SERIAL_BYTE(tmpNode + ASCII_DIGIT_OFFSET);
					SEND_SERIAL_BYTE('#');
					if((xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2] - ASCII_DIGIT_OFFSET)&0x01){
						SEND_SERIAL_MSG("ACC#");
					}
					if(((xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2] - 0x30) >> 1)&0x01){
						SEND_SERIAL_MSG("GPS#");
					}
					if(((xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2] - 0x30) >> 2)&0x01){
						SEND_SERIAL_MSG("SD#");
					}

					SEND_SERIAL_MSG("READY\r\n");
					break;
				case ('O'):
					//Transfered velocity and RSSI measurement
					//"C N#<state><\n>
					receivedAddressHigh = 0x00;
					receivedAddressLow = 0x00;

					for(iterator = 1; iterator < 9; iterator++){	//Read address from received packet

						if(iterator<5){
							receivedAddressHigh |= xbeeReceiveBuffer[iterator] << 8*(4-iterator);
						}
						else{
							receivedAddressLow |= xbeeReceiveBuffer[iterator] << 8*(8-iterator);
						}
					}

					for(niterator = 0; niterator < NUMBER_OF_NODES; niterator++){	//Find the matching node for the received address
						if(receivedAddressLow == node[niterator].addressLow )
							tmpNode = niterator;
					}
					SEND_SERIAL_MSG("MSG#");
					SEND_SERIAL_MSG("CMD_ACCEPTED_NODE#");
					SEND_SERIAL_BYTE(tmpNode + ASCII_DIGIT_OFFSET);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('T'):
					SEND_SERIAL_MSG("MSG#TIM_THRESHOLD#");
					SEND_SERIAL_MSG(&xbeeReceiveBuffer[16]);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('U'):
					SEND_SERIAL_MSG("MSG#BATTERY#");
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[16]);
					SEND_SERIAL_BYTE('.');
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[17]);
					SEND_SERIAL_BYTE(xbeeReceiveBuffer[18]);
					SEND_SERIAL_MSG("\r\n");
					break;
				case ('V'):

				SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+2]);
				SEND_SERIAL_BYTE('#');
				SEND_SERIAL_MSG("TIM_DNG#");
				SEND_SERIAL_MSG(&xbeeReceiveBuffer[XBEE_DATA_TYPE_OFFSET+4]);
				SEND_SERIAL_MSG("\r\n");

				break;
				default:
					SEND_SERIAL_MSG("MSG#UNEXPECTED_XBEE_COMMAND...\r\n");
					break;
				}
			}
			break;
			case XBEE_MODEM_STATUS:
			SEND_SERIAL_MSG("DEBUG#MODEM STATUS#");
			if(xbeeReceiveBuffer[1] == 0x00){
				SEND_SERIAL_MSG("HARDWARE RESET...\r\n");
			}
			else{
				SEND_SERIAL_MSG("UNEXPECTED RESET...\r\n");
			}
			break;
			case XBEE_TRANSMIT_STATUS:
			SEND_SERIAL_MSG("DEBUG#TRANSMIT STATUS");
			if(xbeeReceiveBuffer[5] == 0x00){
				SEND_SERIAL_MSG("#SUCCESS...\r\n");
			}
			else{
				SEND_SERIAL_MSG("#FAIL-");
				SEND_SERIAL_BYTE(xbeeReceiveBuffer[5]);
				SEND_SERIAL_MSG(" \r\n");
			}
			break;
			/*
			 * Parse Any other XBEE packet
			 */
			default:
				SEND_SERIAL_MSG("DEBUG#UNEXPECTED XBEE PACKET(dec):");
				SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_TYPE_OF_FRAME_INDEX]/100 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_BYTE((xbeeReceiveBuffer[XBEE_TYPE_OF_FRAME_INDEX]%100)/10 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_BYTE(xbeeReceiveBuffer[XBEE_TYPE_OF_FRAME_INDEX]%10 + ASCII_DIGIT_OFFSET);
				SEND_SERIAL_MSG("\r\n");
			break;
			}
    		xbeeDataUpdated = false;
    	}
    	/*
    	 * Check whether there were no unread XBEE data
    	 */
    	else if(xbeeReading == false  && !GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4) && xbeeDataUpdated == false){

    		errorTimer = 0;
    		cheksum = 0;
    		xbeeReading = true;
			XBEE_CS_LOW();

			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				errorTimer++;
				if(errorTimer >ERROR_TIMER_COUNT)			//Exit loop if there is no start delimiter
					break;
			}
			if(errorTimer < ERROR_TIMER_COUNT){
				SPI1_TransRecieve(0x00);
				length = SPI1_TransRecieve(0x00);
				//printf("Lenght: %d\n", length);
				uint8_t i = 0;
				for(; i < length; i ++ ){				//Read data based on packet length
					xbeeReceiveBuffer[i] = SPI1_TransRecieve(0x00);
					cheksum += xbeeReceiveBuffer[i];
				}
				cheksum += SPI1_TransRecieve(0x00);
				if(cheksum == 0xFF){
					xbeeDataUpdated = true;
				}
				//printf("Checksum:%d\n",cheksum);
				//Data is updated if checksum is true
				xbeeReading = false;
				XBEE_CS_HIGH();
			}
			else{
				xbeeReading = false;
			}
    	}
    }
}

// Serial data interrupt handler
void USART1_IRQHandler(void){
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){

		Usart1_Send(serialBuffer[packetLenght] = USART_ReceiveData(USART1));
		//serialBuffer[packetLenght] = USART_ReceiveData(USART1);
		if(serialBuffer[packetLenght++] == '\r'){
			serialBuffer[packetLenght-1] = '\0';
			TOGGLE_REDLED_SERIAL();
			serialUpdated = true;
			packetLenght = 0;
		}
	}
}

void EXTI4_15_IRQHandler(void){

	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt (data ready indicator)

		TOGGLE_REDLED_XBEE();
		if(xbeeReading == false && SPI1_Busy == false){
			errorTimer, cheksum = 0;
			xbeeReading = true;
			GPIO_ResetBits(GPIOA,GPIO_Pin_4);
			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				errorTimer++;
				if(errorTimer > ERROR_TIMER_COUNT)			//Exit loop if there is no start delimiter
					break;
			}
			if(errorTimer < ERROR_TIMER_COUNT){
				SPI1_TransRecieve(0x00);
				length = SPI1_TransRecieve(0x00);
				uint8_t i = 0;
				for(; i < length; i ++ ){				//Read data based on packet length
					xbeeReceiveBuffer[i] = SPI1_TransRecieve(0x00);
					cheksum += xbeeReceiveBuffer[i];
				}
				xbeeReceiveBuffer[i] = '\0';
				cheksum += SPI1_TransRecieve(0x00);
				if(cheksum == 0xFF)
					xbeeDataUpdated = true;					//Data is updated if checksum is true
				xbeeReading = false;
				GPIO_SetBits(GPIOA,GPIO_Pin_4);
			}
			else{
				xbeeReading = false;
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		globalCounter++;
	}
}
