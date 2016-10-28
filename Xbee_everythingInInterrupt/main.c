//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

//debugging

//My library includes
#include "SPI1.h"
#include "SPI2.h"
#include "SysTickDelay.h"
#include "USART1.h"
#include "USART2.h"
#include "MyStringFunctions.h"
#include "Timer.h"
#include "PWM.h"
#include "ADC.h"

//Device libraries
#include "Xbee.h"
#include "Button.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"
#include "A2035H.h"
#include "sdCard.h"

//Xbee packet Defines
#define AT_COMMAND_RESPONSE 0x88
#define RECIEVE_PACKET 0x90
#define MODEM_STATUS 0x8A
#define AT_COMMAND_DATA_INDEX 0x05
#define ASCII_DIGIT_OFFSET 0x30

//Changeable defines
#define NUMBER_OF_NODES 4
#define ERROR_TIMER_COUNT 30
#define STARTUP_ERROR_TIMER_COUNT 5

//ModuleStatus defines
#define MODULE_NOT_RUNNING 0x04
#define MODULE_SETUP_STATE 0x05
#define MODULE_APPLYING_PARAMS 0x06
#define MODULE_IDLE_READY 0x0C
#define MODULE_RUNNING 0x08
#define MODULE_TURNING_OFF 0x10
#define ACC_STATE_CASE 0x00
#define GPS_STATE_CASE 0x01

//Measurment defines
#define ACC_MEASUREMENT 0x00
#define RSSI_MEASUREMENT 0x01

//Xbee globals
char xbeeReceiveBuffer[255];
volatile bool xbeeDataUpdated = false;
volatile uint8_t length,errorTimer,cheksum;
bool xbeeReading = false;
//DEWI globals
uint8_t state = 1;
uint32_t globalCounter = 0;
uint8_t moduleStatus = MODULE_NOT_RUNNING;
uint8_t turnOffTimer = 0;
bool writingSD = false;
bool counterIncremented = false;
bool errorOnSomeNode = false;

//Gps globals
char gpsReceiveString[80];
uint8_t gpsReadIterator;
volatile bool gpsDataUpdated = false;

struct node{
	uint8_t adress[8];
	int16_t measurment[2];	//0 -> ACC 1 -> RSSI 2 -> GPS
	uint8_t tmpRSSI;
	uint32_t packetTime;
	uint8_t avarageRSSIcount;
	float velocity;
	/*
	 * if there is an error, bit is set to 1
	 * alarm will be called if there are more then 1 bit set (sum is
	 * 0b00000|gps|rssi|acc|
	 */
	uint8_t errorByte;
	/*
	 * state - 8 bit Config register
	 * |SD|COM|NETWORK|---|---|GPS|RSSI|ACC|
	 * default - 0x00
	 */
	uint8_t state;
};

//Dewi globals
struct node receiverNode;

int main(void)
{
	receiverNode.adress[0] = 0x00;
	receiverNode.adress[1] = 0x13;
	receiverNode.adress[2] = 0xA2;
	receiverNode.adress[3] = 0x00;
	receiverNode.adress[4] = 0x40;
	receiverNode.adress[5] = 0xE3;
	receiverNode.adress[6] = 0xE1;
	receiverNode.adress[7] = 0x3C;
	receiverNode.state = 0;		//0 -> ACC 1 -> RSSI 2 -> GPS
	receiverNode.velocity = 0.1;

	//Nodes
	struct node node[NUMBER_OF_NODES];
	node[0].adress[0] = 0x00;
	node[0].adress[1] = 0x13;
	node[0].adress[2] = 0xA2;
	node[0].adress[3] = 0x00;
	node[0].adress[4] = 0x40;
	node[0].adress[5] = 0x97;
	node[0].adress[6] = 0x83;
	node[0].adress[7] = 0xD9;
	node[0].state = 0;
	node[0].errorByte = 0;
	node[0].avarageRSSIcount = 0;
	node[0].packetTime = 0;

	node[1].adress[0] = 0x00;
	node[1].adress[1] = 0x13;
	node[1].adress[2] = 0xA2;
	node[1].adress[3] = 0x00;
	node[1].adress[4] = 0x40;
	node[1].adress[5] = 0xEA;
	node[1].adress[6] = 0xEF;
	node[1].adress[7] = 0xE9;
	node[1].state = 0;
	node[1].errorByte = 0;
	node[1].avarageRSSIcount = 0;
	node[1].packetTime = 0;

	node[2].adress[0] = 0x00;
	node[2].adress[1] = 0x13;
	node[2].adress[2] = 0xA2;
	node[2].adress[3] = 0x00;
	node[2].adress[4] = 0x40;
	node[2].adress[5] = 0xE3;
	node[2].adress[6] = 0xE1;
	node[2].adress[7] = 0x3D;
	node[2].state = 0;
	node[2].errorByte = 0;
	node[2].avarageRSSIcount = 0;
	node[2].packetTime = 0;

	node[3].adress[0] = 0x00;
	node[3].adress[1] = 0x13;
	node[3].adress[2] = 0xA2;
	node[3].adress[3] = 0x00;
	node[3].adress[4] = 0x40;
	node[3].adress[5] = 0xE3;
	node[3].adress[6] = 0xE1;
	node[3].adress[7] = 0x3A;
	node[3].state = 0;
	node[3].errorByte = 0;
	node[3].avarageRSSIcount = 0;
	node[4].packetTime = 0;

	//Usart for debugging and communication
	Usart1_Init(9600);
	//Our delay timer
	initialiseSysTick();
	//Led's and buttons
	initializeXbeeATTnPin();
	initializeUserButton();
	setupGpsGpio();
	initializeGreenLed1();
	initializeGreenLed2();
	initializeGreenLed3();
	initializeRedLed1();
	initializeRedLed2();
	initializeRedLed3();
	initializeRedLed4();
	initializeRedLed5();

	InitializePwmPin();
	InitializeTimer();
	InitializePwm();

	//Timer for second counter
	Initialize_timer();
	Timer_interrupt_enable();
	//ADC for voltage control
	adcPinConfig();
	adcConfig();

	// SPI for xbee and sdcard
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	//packet variables
	uint8_t typeOfFrame;
	//variables for AT_COMMAND_RESPONSE
	uint8_t commandStatus;
	uint8_t AT_data[4];
	uint8_t frameID;
	//Packet reading iterator
	int i = 0;
	//Node finding iterator
	int n = 0;
	//variables for RECIEVE_PACKET
	uint8_t tmpNode;
	char stringOfMessurement[6];
	//variables for gps
	char* ptr;
	char* tmpPtr;
    char ts[11] = " ";
    char lat[11] = " ";
    char latd[2]= " ";
    char lon[11]= " ";
    char lond[2]= " ";
    char fix[2]= "0";
    char sats[3]= " ";
    char velocityString[6] = " ";
    char *ptrToNMEA[] = {ts, lat, latd, lon, lond, fix, sats};
	uint8_t messageIterator;
	//Timer string
	char timerString[8];
	//Periph variable
	uint16_t ADC_value;

	//variables for sd card
	//initialize sdcard (it uses the same SPI as Xbee)
	uint8_t sdBuffer[512];
	uint16_t sdBufferCurrentSymbol = 0;
	uint8_t sector;
	uint32_t mstrDir;
	uint32_t cluster;
	uint32_t filesize;
	uint16_t fatSect, fsInfoSector;
	char sdTmpWriteString[20];

	//read dump
	xbeeDataUpdated = xbeeStartupParamRead(ERROR_TIMER_COUNT,(uint8_t*)xbeeReceiveBuffer);

	//Dewi Module wont start until the button is pressed
	SEND_SERIAL_MSG("Waiting for input...\r\n");
	while(moduleStatus == MODULE_NOT_RUNNING){
    	ADC_value = (ADC_GetConversionValue(ADC1));
    	ADC_value = (ADC_value * 330) / 128;
    	batteryIndicationStartup(ADC_value);
    	blinkGreenLeds(i++);
    	if(i > 7)
    		i = 1;

	}
	//12 seconds to choose the modules to use
	//One redStartup take about 500ms, so let it spin for twenty times
	//Default state is 1 (only acc is on)
	turnOnGreenLeds(state);
	for(errorTimer = 0 ; errorTimer < 6 ; errorTimer++){
		redStartup(REAL_REAL_SLOW_DELAY);
	}
	//Setup state is cleared
	moduleStatus = MODULE_APPLYING_PARAMS;

	if(state&0x01){
		//ADXL362Z (acc)
		InitialiseSPI2_GPIO();
		InitialiseSPI2();
		initializeADXL362();
		while(!return_ADXL_ready()){
			//wait time for caps to discharge
			delayMs(2000);
			initializeADXL362();
			delayMs(1000);
			blinkRedLed3();
		}
		SEND_SERIAL_MSG("Accelerometer ready!!!\r\n");
	}

	//GPS
	if((state&0x02) >> 1){
		//Usart 2 for gps communication
		Usart2_Init(BAUD_4800);
		ConfigureUsart2Interrupt();
		errorTimer = 50;

		//Wait for enough satellites
		SEND_SERIAL_MSG("Initializing gps...\r\n");

		turnGpsOn();

		//Make sure that gps module has waken up
		while(!GPIO_ReadInputDataBit(GPS_PORTC,WAKEUP_PIN) && errorTimer-- > 0){
			blinkRedLed4();
			delayMs(1000);
		}

		delayMs(400);
		blinkRedLed4();
		gps_dissableMessage($GPVTG);
		delayMs(400);
		blinkRedLed4();
		gps_dissableMessage($GPGSV);
		delayMs(400);
		blinkRedLed4();
		gps_dissableMessage($GPRMC);
		delayMs(400);
		blinkRedLed4();
		gps_setRate($GPGGA, 1);
		delayMs(400);
		blinkRedLed4();
		gps_dissableMessage($GPGSA);

		//Wait for enough satellites
		SEND_SERIAL_MSG("Searching for satellites...\r\n");
		while(fix[0] == '0' && errorTimer > 0 ){
			if(gpsDataUpdated){
				errorTimer--;
				gpsDataUpdated = false;
				messageIterator = 0;
				/*
				 * Make sure that you are comparing GPGGA message
				 * $PSRF and $GPVTG messages are possable at the startup
				 */
				if(strncmp(gpsReceiveString,"$GPGGA", 6) == 0){
					ptr = &gpsReceiveString[7]; //This value could change whether the $ is used or not
					for(; messageIterator < 7; messageIterator ++){
						tmpPtr = ptrToNMEA[messageIterator];
						while(*ptr++ != ','){
							*ptrToNMEA[messageIterator]++ = *(ptr-1);
						}
						ptrToNMEA[messageIterator] = tmpPtr;
					}
				}
				blinkRedLed4();
			}
		}
		//if not enough satellites are found, turn off gps
		if(!errorTimer){
			SEND_SERIAL_MSG("Satellites not found!!!\r\n");
			hibernateGps();
			state &= 0xFD;
		}
		else{
			//if satellites found -> turn on $GPVTG to monitor velocity
			gps_dissableMessage($GPGGA);
			delayMs(400);
			blinkRedLed4();
			gps_setRate($GPVTG, 1);
			SEND_SERIAL_MSG("GPS ready!!!\r\n");
		}
	}

	//initialize sdCard
	if((state&0x04) >> 2){

		SEND_SERIAL_MSG("Initializing sd card...\r\n");

		errorTimer = 10;
		while(!initializeSD() && errorTimer-- > 1){
			delayMs(300);
			blinkRedLed5();
		}
		if(!errorTimer){
			//If sd card doesnt turn on, dont log anything to it
			state &= 0xFB;
			SEND_SERIAL_MSG("Sd card fail...\r\n");
		}
		else{
			writingSD = true;

			findDetailsOfFAT(sdBuffer,&fatSect,&mstrDir, &fsInfoSector);

			findDetailsOfFile("LOGFILE",sdBuffer,mstrDir,&filesize,&cluster,&sector);

			findLastClusterOfFile("LOGFILE",sdBuffer, &cluster,fatSect,mstrDir);

			if(filesize < 512)
				filesize = 512;

			appendTextToTheSD("\nNEW LOG", '\n', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
			writingSD = false;

			SEND_SERIAL_MSG("Sd card initialized...\r\n");
		}
	}

	moduleStatus = MODULE_IDLE_READY;
	turnOnGreenLeds(state);
	SEND_SERIAL_MSG("Coordinator ready!!!\r\n");
	while(moduleStatus == MODULE_IDLE_READY){
		blinkGreenLeds(state);
		redStartup(DELAY);
	}

	while(1)
    {
    	if(xbeeDataUpdated == true){
    		typeOfFrame = xbeeReceiveBuffer[0];
    		switch(typeOfFrame){
    			case AT_COMMAND_RESPONSE:

				frameID = xbeeReceiveBuffer[1];
				//xbeeReceiveBuffer[2];	//TYPE OF AT COMMAND
				//xbeeReceiveBuffer[3];
				//xbeeReceiveBuffer[4];	//PACKET STATUS (FAIL OR NOT)

				if(xbeeReceiveBuffer[2] == 'D' && xbeeReceiveBuffer[3] == 'B'){
					if(node[frameID-1].avarageRSSIcount == 6){
						node[frameID-1].measurment[RSSI_MEASUREMENT] = xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX];
							//Check if measurement is in the threshold boundaries
							if(abs(node[frameID-1].tmpRSSI - node[frameID-1].measurment[RSSI_MEASUREMENT]) > 10){
								node[frameID-1].errorByte |= 0x02;
								//add sd card code for rssi messurement and alarm
								if((state&0x04) >> 2){
									//add sd card code only for rssi messurement
									sdTmpWriteString[0] = 'N';
									sdTmpWriteString[1] = 'o';
									sdTmpWriteString[2] = 'd';
									sdTmpWriteString[3] = 'e';
									sdTmpWriteString[4] = ' ';
									sdTmpWriteString[5] = frameID - 1 + ASCII_DIGIT_OFFSET;
									sdTmpWriteString[6] = ' ';
									sdTmpWriteString[7] = 'D';
									sdTmpWriteString[8] = 'N';
									sdTmpWriteString[9] = 'G';
									sdTmpWriteString[10] = 'R';
									sdTmpWriteString[11] = 'S';
									sdTmpWriteString[12] = 'S';
									sdTmpWriteString[13] = 'I';
									sdTmpWriteString[14] = ':';
									sdTmpWriteString[15] = (xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] / 10) + ASCII_DIGIT_OFFSET;
									sdTmpWriteString[16] = (xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] % 10) + ASCII_DIGIT_OFFSET;
									sdTmpWriteString[17] = '\0';
									//Firstly check if you wont overwrite the buffer
									writingSD = true;
									appendTextToTheSD(sdTmpWriteString,'\n',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									writingSD = false;
								}
							}
							else{
								//No error means setting rssi bit to 0 without affecting other bits
								node[frameID-1].errorByte &= 0x05;
								if((state&0x04) >> 2){
									//add sd card code only for rssi messurement
									sdTmpWriteString[0] = 'N';
									sdTmpWriteString[1] = 'o';
									sdTmpWriteString[2] = 'd';
									sdTmpWriteString[3] = 'e';
									sdTmpWriteString[4] = ' ';
									sdTmpWriteString[5] = frameID -1 + ASCII_DIGIT_OFFSET;
									sdTmpWriteString[6] = ' ';
									sdTmpWriteString[7] = 'R';
									sdTmpWriteString[8] = 'S';
									sdTmpWriteString[9] = 'S';
									sdTmpWriteString[10] = 'I';
									sdTmpWriteString[11] = ':';
									sdTmpWriteString[12] = (xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] / 10) + ASCII_DIGIT_OFFSET;
									sdTmpWriteString[13] = (xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] % 10) + ASCII_DIGIT_OFFSET;
									sdTmpWriteString[14] = '\0';
									//Firstly check if you wont overwrite the buffer
									writingSD = true;
									appendTextToTheSD(sdTmpWriteString,'\n',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									writingSD = false;
								}
							}
					}
					//calculate average
					else if(node[frameID-1].avarageRSSIcount++ < 6){
						node[frameID-1].tmpRSSI = xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX];
						if((state&0x04) >> 2){
							//add sd card code only for rssi messurement
							sdTmpWriteString[0] = 'N';
							sdTmpWriteString[1] = 'o';
							sdTmpWriteString[2] = 'd';
							sdTmpWriteString[3] = 'e';
							sdTmpWriteString[4] = ' ';
							sdTmpWriteString[5] = frameID -1 + ASCII_DIGIT_OFFSET;
							sdTmpWriteString[6] = ' ';
							sdTmpWriteString[7] = 'R';
							sdTmpWriteString[8] = 'S';
							sdTmpWriteString[9] = 'S';
							sdTmpWriteString[10] = 'I';
							sdTmpWriteString[11] = ':';
							sdTmpWriteString[12] = (xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] / 10) + ASCII_DIGIT_OFFSET;
							sdTmpWriteString[13] = (xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] % 10) + ASCII_DIGIT_OFFSET;
							sdTmpWriteString[14] = '\0';
							//Firstly check if you wont overwrite the buffer
							writingSD = true;
							appendTextToTheSD(sdTmpWriteString,'\n',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
							writingSD = false;
						}
					}
					/*
					 * *** ERROR CHECKING ROUTINE ***
					 *
					 * Error is checked after reading the RSSI value
					 * Node's id is stored in AT packets frame_id value
					 *
					 * Last 3 bits of errorByte represents gps,rssi and acc error
					 *
					 * If more then one sensor reads danger value, the alarm routine is called
					 *
					 * Blinking means that received measurement was good
					 * Solid light means error
					 */

					if((node[frameID-1].errorByte & 0x01)+((node[frameID-1].errorByte >> 1)&0x01)+(node[frameID-1].errorByte >> 2) > 0){

						GPIOB->ODR |= (1 << (5+tmpNode -1));
						//SEND_SERIAL_MSG("DANGER\r\n");
					}
					else{
						//indicate that one of the five nodes received packet was good
						GPIOB->ODR ^= (1 << (5+tmpNode -1));
					}
				}
				else{
					i = 5;
					for(; i < length; i++){		//Read AT data
						AT_data[i - 5] = xbeeReceiveBuffer[i];
					}
				}
				xbeeDataUpdated = false;				//Mark packet as read
				break;
			case RECIEVE_PACKET:
				//SEND_SERIAL_MSG("PACKET_RECEIVED...\r\n");

				i = 1;	//Remember that "Frame type" byte was the first one
				uint8_t eightByteSourceAdress[8]; //64 bit

				for(; i < 9; i++){	//Read address from received packet
					eightByteSourceAdress[i-1] = xbeeReceiveBuffer[i];
				}
				for(n = 0; n < NUMBER_OF_NODES; n++){	//Find the matching node for the received address
					if(memcmp(&eightByteSourceAdress[4],&node[n].adress[4], 4) == 0 )
						tmpNode = n;
				}

				//After reading source address, comes 2 reserved bytes
				i = i + 2;
				//In this case comandStatus actually is receive options
				commandStatus = xbeeReceiveBuffer[i++];
				//Read the typeOfMessurement
				node[tmpNode].state = xbeeReceiveBuffer[i++] - 0x30;	//Calculate the integer from ASCII by subtracting 0x30

				/*
				 * State (type of measurement) from another node is separated
				 * with whitespace from actual measurement data
				 */
				i++;

				n = 0;
				while (xbeeReceiveBuffer[i] != '\0'){
					stringOfMessurement[n++] = xbeeReceiveBuffer[i];
					i++;
				}
				stringOfMessurement[n] = xbeeReceiveBuffer[i];

				//Measure the time when packet was received
				node[tmpNode].packetTime = globalCounter;

/*				SEND_SERIAL_BYTE(node[tmpNode].state + 0x30);
				SEND_SERIAL_BYTE(' ');
				SEND_SERIAL_MSG(stringOfMessurement);
				SEND_SERIAL_MSG(":Received\r\n");*/

				switch(node[tmpNode].state){
					case ACC_STATE_CASE:
						node[tmpNode].measurment[ACC_MEASUREMENT] = atoi(stringOfMessurement);
						receiverNode.measurment[ACC_MEASUREMENT] = returnX_axis();

						if(abs(receiverNode.measurment[ACC_MEASUREMENT] - node[tmpNode].measurment[ACC_MEASUREMENT]) > 120){
							node[tmpNode].errorByte |= 0x01;
							//add sd card accelerometer error code
							if(((state&0x04) >> 2)){
								writingSD = true;
								appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
								writingSD = false;
							}
						}
						else {
							node[tmpNode].errorByte &= 0x06;
							//if sd card is in use, log to it
							if(((state&0x04) >> 2)){
								writingSD = true;
								appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
								writingSD = false;
							}
						}

						//after receiving acc measurement, ask module for last packets rssi
						askModuleParams('D','B',tmpNode+1);
						break;

					case GPS_STATE_CASE:
						node[tmpNode].velocity = stof(stringOfMessurement);
						if(gpsDataUpdated){
							gpsDataUpdated = false;
							gps_parseGPVTG(gpsReceiveString,velocityString);
							receiverNode.velocity = stof(velocityString);
						}

						if(abs(receiverNode.velocity - node[tmpNode].velocity) > 5){
							node[tmpNode].errorByte |= 0x04;

							//Error was measured - log it to sd card
							if(((state&0x04) >> 2)){
								writingSD = true;
								appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
								writingSD = false;
							}
						}
						else{
							node[tmpNode].errorByte &= 0x03;

							//If sd card is in use, log data to it
							if(((state&0x04) >> 2)){
									writingSD = true;
									appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									writingSD = false;
								}
						}
						//after receiving gps measurement, ask module for last packets rssi

						askModuleParams('D','B',tmpNode+1);
						break;
					}
				break;
			case MODEM_STATUS:
				SEND_SERIAL_MSG("MODEM STATUS...\r\n");
				//blinkAllRed();
				if(xbeeReceiveBuffer[1] == 0x00){
					SEND_SERIAL_MSG("HARDWARE RESET...\r\n");
				}
				break;
			default:
				SEND_SERIAL_MSG("UNKNOWN PACKET...\r\n");
				break;
    		}
    		xbeeDataUpdated = false;
    	}

    	/*
    	 * If the data was not read in the interrupt (sd in use for ex.)
    	 * Then read it in while loop after the interrupt
    	 */
    	else if(xbeeReading == false  && !GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4) && xbeeDataUpdated == false){

    		errorTimer, cheksum = 0;
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

    	if(moduleStatus == MODULE_IDLE_READY){
    		if(turnOffTimer > 0){
				turnOffTimer--;
				blinkGreenLeds(state);
				redStartup(DELAY);
			}
			else{
				blinkGreenLeds(state);
				ADC_value = (ADC_GetConversionValue(ADC1));
				ADC_value = (ADC_value * 330) / 128;
				batteryIndicationStartup(ADC_value);
			}
    	}
    	else if(moduleStatus == MODULE_TURNING_OFF){
    		if((state&0x04) >> 2){
    			sdIdleState();
    		}
    		if((state&0x02) >> 1){
    			hibernateGps();
    		}
    		errorOnSomeNode = false;
			GPIOC->ODR &=~ 7 << 6;
			GPIOB->ODR &=~ (1 << (5+tmpNode -1));
			PWM_PULSE_LENGHT(0);

    		state = 0;
    		turnOffTimer = 0;
    		moduleStatus = MODULE_IDLE_READY;
    	}

    	/*
    	 * Routine for checking timer error
    	 *  (if node isnt disconected)
    	 */
 /*   	if(counterIncremented == true ){
    		for(nodeCounter = 0; nodeCounter < NUMBER_OF_NODES; nodeCounter++){
    			//check if node has received any packet
    			if(node[nodeCounter].packetTime > 0){
    				//check for packet receiving timeout
    				if(globalCounter - node[nodeCounter].packetTime > 2){
    					//indicate timer error
						GPIOC->ODR |= 7 << 6;
						GPIOB->ODR |= (1 << (5+tmpNode -1));
						PWM_PULSE_LENGHT(0);
						errorOnSomeNode = true;
    					break;
    				}
    				errorOnSomeNode = false;
    			}
    		}
			if(errorOnSomeNode == true && moduleStatus != MODULE_IDLE_READY){
				//Fire up the speakaaaaaaaaaaaaaar...
				PWM_PULSE_LENGHT(1000);
			}
			else if(moduleStatus != MODULE_IDLE_READY){
				PWM_PULSE_LENGHT(0);
			}
    		counterIncremented = false;
    	}*/
    }
}

void USART2_IRQHandler(void){
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		if((gpsReceiveString[gpsReadIterator++] = USART_ReceiveData(USART2)) == '\n'){
			gpsDataUpdated = true;
			gpsReadIterator = 0;
		}
	}
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button2 pin interrupt

		if(moduleStatus == MODULE_NOT_RUNNING){
			moduleStatus = MODULE_SETUP_STATE;
		}
		else if(moduleStatus == MODULE_SETUP_STATE){
			if(state++ > 7){
				state = 0;
			}
			turnOnGreenLeds(state);
		}
		else if(moduleStatus == MODULE_IDLE_READY){
			if(turnOffTimer > 0)
				moduleStatus = MODULE_TURNING_OFF;
			else
				moduleStatus = MODULE_RUNNING;
		}
		else if(moduleStatus == MODULE_RUNNING){
			turnOffTimer = 10;
			moduleStatus = MODULE_IDLE_READY;
		}
		EXTI_ClearITPendingBit(EXTI_Line8);
	}

	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt (data ready indicator)

		if(xbeeReading == false && writingSD == false){
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

		counterIncremented = true;
		globalCounter++;
	}
}






