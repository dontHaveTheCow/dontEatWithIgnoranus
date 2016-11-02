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
#include "SPI2.h"
#include "SysTickDelay.h"
#include "USART1.h"
#include "USART2.h"
#include "MyStringFunctions.h"
#include "Timer.h"
#include "ADC.h"

#include "Xbee.h"
#include "Button.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"
#include "A2035H.h"
#include "sdCard.h"
/*
 * Module state defines
 */
#define MODULE_NOT_RUNNING 0x04
#define MODULE_SETUP_STATE 0x05
#define MODULE_APPLYING_PARAMS 0x06
#define MODULE_IDLE_READY 0x0C
#define MODULE_RUNNING 0x08
#define MODULE_TURNING_OFF 0x10
/*
 * Any other defines
 */
#define RSSI_MESSAGE_0 "1 0"
#define RSSI_MESSAGE_1 "1 1"
#define GPS_MSG_INIT_DELAY 400
#define ACC_MEASUREMENT 0x00
#define RSSI_MEASUREMENT 0x01
#define ERROR_TIMER_COUNT 30
#define STARTUP_ERROR_TIMER_COUNT 5
#define ACC_STATE_CASE 0x00
#define GPS_STATE_CASE 0x01

#define NUMBER_OF_NODES 4
/*
 * XBEE globals
 */
char xbeeReceiveBuffer[255];
volatile bool xbeeDataUpdated = false;
volatile uint8_t length,errorTimer,cheksum;
bool xbeeReading = false;
/*
 * Module globals
 */
volatile uint8_t state = 1;
volatile uint8_t moduleStatus = MODULE_NOT_RUNNING;
uint8_t turnOffTimer = 0;
struct node receiverNode;
bool SPI1_Busy = false;
uint32_t globalCounter = 0;
/*
 * GPS globals
 */
char gpsReceiveString[96];
uint8_t gpsReadIterator;
volatile bool gpsDataUpdated = false;
/*
 * Struct for node measurements, id's...
 */
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

int main(void){
	/*
	 * Initialize addresses of nodes
	 */
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
	node[0].tmpRSSI = 0;
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
	node[1].tmpRSSI = 0;
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
	node[2].tmpRSSI = 0;
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
	node[3].tmpRSSI = 0;
	node[3].avarageRSSIcount = 0;
	node[4].packetTime = 0;
	/*
	 * Local variables for XBEE
	 */
	uint8_t typeOfFrame;
	uint8_t commandStatus;
	uint8_t AT_data[4];
	uint8_t frameID;
	/*
	 *--- Accelerometer does not
	 *--- have any variables
	 *--- for coordinator
	 */
	/*
	 * Local variables for GPS
	 */
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
	/*
	 * Local variables for SD card
	 */
	uint8_t sdBuffer[512];
	uint16_t sdBufferCurrentSymbol = 0;
	uint8_t sector;
	uint32_t mstrDir;
	uint32_t cluster;
	uint32_t filesize;
	uint16_t fatSect, fsInfoSector;
	char sdTmpWriteString[20];
	/*
	 * ADC, Timer and other loco's
	 */
	uint16_t ADC_value;
	uint8_t errorTimer = 20;
	int i = 0;//Packet reading iterator
	int n = 0;//Node finding iterator
	uint8_t tmpNode;//
	//String for temporarily storing received data
	char stringOfMessurement[6];
	/*
	 * Initializing gpio's
	 */
	initializeUserButton();
	initializeEveryRedLed();
	initializeEveryGreenLed();
	setupGpsGpio();
	adcPinConfig();
	initializeXbeeATTnPin();
	/*
	 * Initializing peripherals
	 */
	Usart2_Init(BAUD_4800);
	Usart1_Init(BAUD_9600);
	ConfigureUsart2Interrupt();
	//used for delayMs()
	//not meant for using in interrupt routines
	initialiseSysTick();
	//SPI2 used for accelerometer
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	//SPI for xbee and sdcard
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	//ADC is used for battery monitoring
	adcConfig();
	Initialize_timer();
	Timer_interrupt_enable();
	/*
	 * Initializing XBEE
	 */
	xbeeDataUpdated = xbeeStartupParamRead(ERROR_TIMER_COUNT,(uint8_t*)xbeeReceiveBuffer);
	/*
	 * Start module only when button is pressed
	 * Meanwhile, check the battery voltage
	 */
	SEND_SERIAL_MSG("Waiting for input...\r\n");
	while(moduleStatus == MODULE_NOT_RUNNING){
   	ADC_value = (ADC_GetConversionValue(ADC1));
    	ADC_value = (ADC_value * 330) / 128;
    	batteryIndicationStartup(ADC_value);
    	blinkGreenLeds(i++);
    	if(i > 7)
    		i = 1;
	}
	/*
	 * LED blinking that indicates need to...
	 * choose the method for integrity detection
	 */
	turnOnGreenLeds(state);
	for(errorTimer = 0 ; errorTimer < 6 ; errorTimer++){
	redStartup(REAL_REAL_SLOW_DELAY);
	}
	moduleStatus = MODULE_APPLYING_PARAMS;
	/*
	 * Initialize Accelerometer if chosen
	 */
	if(state&0x01){
		//SPI2 for ADXL
		initializeADXL362();
		blinkRedLed1();
		while(!return_ADXL_ready()){
			//wait time for caps to discharge
			delayMs(2000);
			initializeADXL362();
			delayMs(1000);
			blinkRedLed3();
		}
	}
	/*
	 * Initialize GPS if chosen
	 */
	//GPS module has 40 seconds to find enough satellites
	errorTimer = 40;
	if((state&0x02) >> 1){
		turnGpsOn();

		while(!GPIO_ReadInputDataBit(GPS_PORTC,WAKEUP_PIN)){
			blinkRedLed4();
			delayMs(1000);
		}

		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPGSA);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPGSV);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPRMC);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPVTG);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_setRate($GPGGA, 1);

		while(fix[0] == '0' && errorTimer > 0 ){
			if(gpsDataUpdated){
				errorTimer--;
				gpsDataUpdated = false;
				messageIterator = 0;
				 // Make sure that you are comparing GPGGA message
				 // $PSRF and $GPVTG messages are possible at the startup
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
				SEND_SERIAL_MSG("Waiting for sats...\r\n");
				blinkRedLed2();
			}
		}
		//if not enough satellites are found, turn off gps
		if(!errorTimer){
			SEND_SERIAL_MSG("GPS timeout...\r\n");
			hibernateGps();
			state &= 0xFD;
		}
		else{
			//if satellites found -> turn on $GPVTG to monitor velocity
			delayMs(400);
			gps_setRate($GPVTG,1);
			delayMs(400);
			blinkRedLed2();
			gps_dissableMessage($GPGGA);
			SEND_SERIAL_MSG("SATs found...\r\n");
		}
	}
	/*
	 * Initialize SD if chosen
	 */
	if((state&0x04) >> 2){

		errorTimer = 10;
		while(!initializeSD() && errorTimer-- > 1){
			delayMs(300);
			blinkRedLed3();
		}
		if(!errorTimer){
			//If sd card doesnt turn on, dont log anything to it
			state &= 0xFB;
		}
		else{

			findDetailsOfFAT(sdBuffer,&fatSect,&mstrDir, &fsInfoSector);
			findDetailsOfFile("LOGFILE",sdBuffer,mstrDir,&filesize,&cluster,&sector);
			/*
			 * Program halts during this function
			 */
			findLastClusterOfFile("LOGFILE",sdBuffer, &cluster,fatSect,mstrDir);

			if(filesize < 512)
				filesize = 512;

			appendTextToTheSD("\nNEW LOG", '\n', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
		}
	}

	/*
	 * Wait for input to start communication
	 */
	moduleStatus = MODULE_IDLE_READY;
	turnOnGreenLeds(state);
	while(moduleStatus == MODULE_IDLE_READY){
		blinkGreenLeds(state);
		redStartup(DELAY);
	}

    while(1){

    	/*
    	 * Check whether the XBEE has new packet
    	 */
    	if(xbeeDataUpdated == true){
    		typeOfFrame = xbeeReceiveBuffer[0];
    		switch(typeOfFrame){
				/*
				 * Parse AT command packet
				 */
    			case AT_COMMAND_RESPONSE:

				frameID = xbeeReceiveBuffer[1];
    			/*
    			 * Parse RSSI packet
    			 */
				if(xbeeReceiveBuffer[2] == 'D' && xbeeReceiveBuffer[3] == 'B'){
					if(node[frameID-1].avarageRSSIcount == 6){
						node[frameID-1].measurment[RSSI_MEASUREMENT] = xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX];
							/*
							 * If threshold was exceeded - raise an alarm bit
							 */
							if(abs(node[frameID-1].tmpRSSI - node[frameID-1].measurment[RSSI_MEASUREMENT]) > 10){
								node[frameID-1].errorByte |= 0x02;
								SEND_SERIAL_BYTE((xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] / 10) + ASCII_DIGIT_OFFSET);
								SEND_SERIAL_BYTE((xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] % 10) + ASCII_DIGIT_OFFSET);
								SEND_SERIAL_MSG(":RSSI_DANGER\r\n");
			    				/*
			    				 * Log to SD card if needed
			    				 */
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
									SPI1_Busy = true;
									appendTextToTheSD(sdTmpWriteString,'\n',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									SPI1_Busy = false;
								}
							}
							else{
								/*
								 * If threshold was not exceeded - clear an alarm bit
								 */
								node[frameID-1].errorByte &= 0x05;
								if((state&0x04) >> 2){
				    				/*
				    				 * Log to SD card if needed
				    				 */
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
									SPI1_Busy = true;
									appendTextToTheSD(sdTmpWriteString,'\n',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									SPI1_Busy = false;
								}
							}
					}
					/*
					 * From first couple of packets - calculate avarage rssi
					 */
					else if(node[frameID-1].avarageRSSIcount++ < 6){
						node[frameID-1].tmpRSSI =  (xbeeReceiveBuffer[AT_COMMAND_DATA_INDEX] + node[frameID-1].tmpRSSI) / 2 ;
						SEND_SERIAL_BYTE((node[frameID-1].tmpRSSI / 10) + ASCII_DIGIT_OFFSET);
						SEND_SERIAL_BYTE((node[frameID-1].tmpRSSI % 10) + ASCII_DIGIT_OFFSET);
						SEND_SERIAL_MSG(":AVR_RSSI\r\n");
						if((state&0x04) >> 2){
		    				/*
		    				 * Log to SD card if needed
		    				 */
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
							SPI1_Busy = true;
							appendTextToTheSD(sdTmpWriteString,'\n',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
							SPI1_Busy = false;
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

					if((node[frameID-1].errorByte & 0x01)+((node[frameID-1].errorByte >> 1)&0x01)+(node[frameID-1].errorByte >> 2) > 1){

						GPIOB->ODR |= (1 << (5+tmpNode -1));
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
			/*
			 * Parse RECEIVE command packet
			 */
			case RECIEVE_PACKET:
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
				/*
				 * Whether the command(C) or measurement(M) was received
				 */
				if(xbeeReceiveBuffer[i] == 'M'){
					/*
					 * Example
					 * M 0 45
					 * M 1 10.2
					 */
					i = i + 2;
					node[tmpNode].state = xbeeReceiveBuffer[i++] - 0x30;	//Calculate the integer from ASCII by subtracting 0x30

					n = 0;
					while (xbeeReceiveBuffer[i] != '\0'){
						stringOfMessurement[n++] = xbeeReceiveBuffer[i];
						i++;
					}
					stringOfMessurement[n] = xbeeReceiveBuffer[i];

					//Measure the time when packet was received
					node[tmpNode].packetTime = globalCounter;

					SEND_SERIAL_BYTE(node[tmpNode].state + 0x30);
					SEND_SERIAL_BYTE(' ');
					SEND_SERIAL_MSG(stringOfMessurement);
					SEND_SERIAL_MSG(":Received\r\n");

					switch(node[tmpNode].state){
						case ACC_STATE_CASE:
							node[tmpNode].measurment[ACC_MEASUREMENT] = atoi(stringOfMessurement);
							receiverNode.measurment[ACC_MEASUREMENT] = returnX_axis();

							if(abs(receiverNode.measurment[ACC_MEASUREMENT] - node[tmpNode].measurment[ACC_MEASUREMENT]) > 120){
								node[tmpNode].errorByte |= 0x01;
								SEND_SERIAL_MSG("ACC_DANGER\r\n");
								//add sd card accelerometer error code
								if(((state&0x04) >> 2)){
									SPI1_Busy = true;
									appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									SPI1_Busy = false;
								}
							}
							else {
								node[tmpNode].errorByte &= 0x06;
								//if sd card is in use, log to it
								if(((state&0x04) >> 2)){
									SPI1_Busy = true;
									appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									SPI1_Busy = false;
								}
							}
							//after receiving acc measurement, ask module for last packets RSSI
							askModuleParams('D','B',tmpNode+1);
							break;

						case GPS_STATE_CASE:
							node[tmpNode].velocity = stof(stringOfMessurement);
							if(gpsDataUpdated == true){
								gpsDataUpdated = false;
								gps_parseGPVTG(gpsReceiveString,velocityString);
								receiverNode.velocity = stof(velocityString);
								SEND_SERIAL_MSG(velocityString);
								SEND_SERIAL_MSG(":MyVel\r\n");
							}
							if(abs(receiverNode.velocity - node[tmpNode].velocity) > 5){
								node[tmpNode].errorByte |= 0x04;
								SEND_SERIAL_MSG("GPS_DANGER\r\n");
								//Error was measured - log it to sd card
								if(((state&0x04) >> 2)){
									SPI1_Busy = true;
									appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
									SPI1_Busy = false;
								}
							}
							else{
								node[tmpNode].errorByte &= 0x03;
								//If sd card is in use, log data to it
								if(((state&0x04) >> 2)){
										SPI1_Busy = true;
										appendTextToTheSD(stringOfMessurement, ' ',&sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
										SPI1_Busy = false;
									}
							}
							//after receiving gps measurement, ask module for last packets RSSI
							askModuleParams('D','B',tmpNode+1);
							break;
						}
				}
				else if(xbeeReceiveBuffer[i] == 'C'){
					/*
					 * Example
					 * C 0 -> Request for timestamp
					 * C 1
					 */
					i = i + 2;
					switch(xbeeReceiveBuffer[i]){

					case 0:
						//globalCounter;
						//transmitRequest(node[tmpNode].adress, node[tmpNode].adress, TRANSOPT_DISACK, "smth to transmit back");

						break;

					default:
						break;
					}

				}
				break;
				case MODEM_STATUS:
				SEND_SERIAL_MSG("MODEM STATUS...\r\n");
				//blinkAllRed();
				if(xbeeReceiveBuffer[1] == 0x00){
					SEND_SERIAL_MSG("HARDWARE RESET...\r\n");
				}
				break;
				/*
				 * Parse Any other XBEE packet
				 */
				default:
				SEND_SERIAL_MSG("UNKNOWN PACKET...\r\n");
				break;
    		}
    		xbeeDataUpdated = false;
    	}
    	/*
    	 * Check whether there aren't any unread XBEE data
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
    	/*
    	 * Check whether the state of coordinator hasn't changed
    	 */
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
			GPIOC->ODR &=~ 7 << 6;
			GPIOB->ODR &=~ (1 << (5+tmpNode -1));

    		state = 0;
    		turnOffTimer = 0;
    		moduleStatus = MODULE_IDLE_READY;
    	}

    	/*
    	 * Check timer error
    	 */

    	/*
    	 * Check whether the serial command was received
    	 */
    }
}

/*
 * INTERRUPT ROUTINES
 */

/*
 * USB-Serial interface USART interrupt
 */

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
			if(++state > 7){
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
