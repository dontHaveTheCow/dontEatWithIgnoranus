//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

//My library includes
#include "SysTickDelay.h"
#include "myStringFunctions.h"

//debugging
#include "debug.h"

//Device libraries
#include "ADXL362.h"
#include "Xbee.h"
#include "IndicationGPIOs.h"
#include "Button.h"
#include "USART1.h"
#include "PWM.h"

//Xbee packet Defines
#define AT_COMMAND_RESPONSE 0x88
#define RECIEVE_PACKET 0x90
#define MODEM_STATUS 0x8A

//Changeable defines
#define NUMBER_OF_NODES 4
#define ERROR_TIMER_COUNT 30
#define STARTUP_ERROR_TIMER_COUNT 5

//Xbee globals
static char receivePacket[128];
volatile bool dataUpdated = false;
volatile uint8_t length,errorTimer,cheksum;
bool readingPacket = false;

struct node{
	uint8_t adress[8];
	int16_t measurment[3];	//0 -> ACC 1 -> RSSI 2 -> GPS
	uint8_t tmpRSSI;
	uint8_t avarageRSSIcount;
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

	//Usart for debugging and communication
	Usart1_Init(9600);

	//Our delay timer
	initialiseSysTick();

	//Led's and buttons
	initializeXbeeATTnPin();
	initializeUserButton();
	initializeGreenLed1();
	initializeGreenLed2();
	initializeGreenLed3();
	initializeRedLed1();
	initializeRedLed2();
	initializeRedLed3();
	initializeRedLed4();
	initializeRedLed5();
	//Turn on first status led ( ACC )
	turnOnGreenLed(receiverNode.state);

	InitializePwmPin();
	InitializeTimer();
	InitializePwm();

	//blink these 3 leds to indicate that gpio,pwm,usart and systick is initialzied
	blinkRedLed1();
	blinkRedLed2();
	blinkRedLed3();

	//Xbee (radio)
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
	char stringOfMessurement[5];

	//read dump
	readingPacket = true;
	XBEE_CS_LOW();
	while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
	errorTimer++;
	if(errorTimer >STARTUP_ERROR_TIMER_COUNT)//Exit loop if there is no start delimiter
			break;
	}
	if(errorTimer < STARTUP_ERROR_TIMER_COUNT){
		SPI1_TransRecieve(0x00);
		length = SPI1_TransRecieve(0x00);
		//printf("Lenght: %d\n", length);
		uint8_t i = 0;
		for(; i < length; i ++ ){				//Read data based on packet length
			receivePacket[i] = SPI1_TransRecieve(0x00);
			cheksum += receivePacket[i];
		}
		cheksum += SPI1_TransRecieve(0x00);
		if(cheksum == 0xFF){
			dataUpdated = true;
		}
		readingPacket = false;
		XBEE_CS_HIGH();
	}
	else
	readingPacket = false;

	blinkRedLed4();

	//ADXL362Z (acc)
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	initializeADXL362();
	while(!return_ADXL_ready()){
		//wait time for caps to discharge
		delayMs(2000);
		initializeADXL362();
		delayMs(1000);
		blinkRedLed5();
	}
	redStartup();

	while(1)
    {
    	if(dataUpdated == true){
    		typeOfFrame = receivePacket[0];
    		switch(typeOfFrame){
    			case AT_COMMAND_RESPONSE:

    				frameID = receivePacket[1];
    				//receivePacket[2];	//TYPE OF AT COMMAND
					//receivePacket[3];
    		        //receivePacket[4];	//PACKET STATUS (FAIL OR NOT)

    		        if(receivePacket[2] == 'D' && receivePacket[3] == 'B'){
    		        	if(node[frameID-1].avarageRSSIcount == 6){
    		        		node[frameID-1].measurment[node[frameID-1].state] = receivePacket[5];
    		        			if(abs(node[frameID-1].tmpRSSI - node[frameID-1].measurment[node[frameID-1].state]) > 10)
    		        				node[frameID-1].errorByte |= 0x02;
    		        			else {
    		        				node[frameID-1].errorByte &= 0x01;
								}
    		        		}
    		        	//calculate average
    		        	else if(node[frameID-1].avarageRSSIcount++ < 6){
    		        		node[frameID-1].tmpRSSI = receivePacket[5];
    		        	}
    		        }
    		        else{
        		        i = 5;
        		        for(; i < length; i++){				//Read AT data
        		          	AT_data[i - 5] = receivePacket[i];
        		          	DEBUG_SEND_BYTE(AT_data[i - 5]);
        		        }
    		        }

    		    	dataUpdated = false;				//Mark packet as read
    				break;
    			case RECIEVE_PACKET:

    				i = 1;	//Remember that "Frame type" byte was the first one
    				uint8_t eightByteSourceAdress[8]; //64 bit

    				for(; i < 9; i++){	//Read address from received packet
    					eightByteSourceAdress[i-1] = receivePacket[i];
    				}
    				for(n = 0; n < NUMBER_OF_NODES; n++){	//Find the matching node for the received address
    					if(memcmp(&eightByteSourceAdress[4],&node[n].adress[4], 4) == 0 )
    						tmpNode = n;
}

    				//After reading source address, comes 2 reserved bytes
    				i = i + 2;
    				//In this case comandStatus actually is receive options
    				commandStatus = receivePacket[i++];
    				//Read the typeOfMessurement
    				node[tmpNode].state = receivePacket[i++] - 0x30;	//Calculate the integer from ASCII by subtracting 0x30

    				i++;
    				//Read the recievedMessurement(data from sensors)
    				n = 0;
    				while (receivePacket[i] != '\0'){
    					stringOfMessurement[n++] = receivePacket[i];
    					i++;
    				}
    				stringOfMessurement[n] = '\0';
    				node[tmpNode].measurment[node[tmpNode].state] = atoi(stringOfMessurement);
    				//printf("node:%d\tFriendState:%d\tdata:%d\n", tmpNode, node[tmpNode].state, node[tmpNode].measurment[node[tmpNode].state]);

    					switch(node[tmpNode].state){
							case 0:
								receiverNode.measurment[0] = returnZ_axis();

								if(abs(receiverNode.measurment[0] - node[tmpNode].measurment[0]) > 80)
									node[tmpNode].errorByte |= 0x01;
								else {
									node[tmpNode].errorByte &= 0x02;
								}
								//after receiving acc measurement, ask module for last packets rssi
								askModuleParams('D','B',tmpNode+1);
								break;
							case 1:

								/*
								 * read the gps value of receiver
								 * compare it with received value from node
								 * decide for errorByte 3lsb value
								 */
								askModuleParams('D','B',tmpNode+1);
								break;
    					}
    				//check if two or more measurments were correct
    				if((node[tmpNode].errorByte & 0x01)+((node[tmpNode].errorByte >> 1)&0x01)+(node[tmpNode].errorByte >> 3)){
    					PWM_PULSE_LENGHT(1000);
    				}
    				else{
    					PWM_PULSE_LENGHT(0);
    				}
    				break;
    			case MODEM_STATUS:
    				blinkAllRed();
    				if(receivePacket[1] == 0x00){
						//DEBUG_MESSAGE("*ModemStatus - Hardware reset*\n");
    				}
					break;
    			default:
    				//FUCK: There was another type of packet
    				break;
    		}
    		dataUpdated = false;
    		blinkRedLed(tmpNode);
    	}
    	//Check Xbee Attention pin ( there might be unexpected data )
    	else if(readingPacket == false  && !GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4) && dataUpdated == false){

    		errorTimer, cheksum = 0;
    		readingPacket = true;
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
					receivePacket[i] = SPI1_TransRecieve(0x00);
					cheksum += receivePacket[i];
				}
				cheksum += SPI1_TransRecieve(0x00);
				if(cheksum == 0xFF){
					dataUpdated = true;
				}
				//printf("Checksum:%d\n",cheksum);
				//Data is updated if checksum is true
				readingPacket = false;
				XBEE_CS_HIGH();
			}
			else{
				readingPacket = false;
			}
    	}
    }
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt (data ready indicator)

		if(readingPacket == false){
			errorTimer, cheksum = 0;
			readingPacket = true;
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
					receivePacket[i] = SPI1_TransRecieve(0x00);
					cheksum += receivePacket[i];
				}
				receivePacket[i] = '\0';
				cheksum += SPI1_TransRecieve(0x00);
				if(cheksum == 0xFF)
					dataUpdated = true;					//Data is updated if checksum is true
				readingPacket = false;
				GPIO_SetBits(GPIOA,GPIO_Pin_4);
			}
			else{
				readingPacket = false;
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}








