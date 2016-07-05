//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
//Debugging
#include <stdio.h>

//My library includes
#include "SysTickDelay.h"

//Device libraries
#include "ADXL362.h"
#include "Xbee.h"
#include "IndicationGPIOs.h"
#include "Button.h"

//Defines
#define AT_COMMAND_RESPONSE 0x88
#define RECIEVE_PACKET 0x90
#define MODEM_STATUS 0x8A

//Changeable defines
#define NUMBER_OF_NODES 4

//Xbee globals
static char receivePacket[128];
volatile bool dataUpdeted = false;
volatile uint8_t length;
bool readingPacket = false;

struct node{
	uint8_t adress[8];
	int16_t measurment[3];	//0 -> ACC 1 -> RSSI 2 -> GPS
	int8_t state;
};

//Dewi globals
struct node receiverNode;

int main(void)
{
	printf("Initializing!\n");
	receiverNode.adress[0] = 0x00;
	receiverNode.adress[1] = 0x13;
	receiverNode.adress[2] = 0xA2;
	receiverNode.adress[3] = 0x00;
	receiverNode.adress[4] = 0x40;
	receiverNode.adress[5] = 0xE0;
	receiverNode.adress[6] = 0x1B;
	receiverNode.adress[7] = 0xA6;
	receiverNode.state = 0;		//0 -> ACC 1 -> RSSI 2 -> GPS

	//Nodes
	struct node node[NUMBER_OF_NODES];
	node[0].adress[0] = 0x00;
	node[0].adress[1] = 0x13;
	node[0].adress[2] = 0xA2;
	node[0].adress[3] = 0x00;
	node[0].adress[4] = 0x40;
	node[0].adress[5] = 0xE3;
	node[0].adress[6] = 0xE1;
	node[0].adress[7] = 0x35;
	node[0].state = 0;

	//Our delay timer
	initialiseSysTick();
	//Leds and buttons
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

	//Xbee (radio)
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	initializeXbeeATTnPin();
	//packet variables
	uint8_t typeOfFrame;
	//variables for AT_COMMAND_RESPONSE
	uint8_t AT_command[2];
	uint8_t commandStatus;
	uint8_t AT_data[4];
	//Packet reading iterator
	int i = 0;
	//Node finding iterator
	int n = 0;
	//variables for RECIEVE_PACKET
	uint8_t currentNode;
	char stringOfMessurement[5];
	//variables for RSSI detection
	uint8_t RSSIdefaultCount = 0;
	uint8_t RSSIdefaultValues[5];

	//Restore default settings
	readModuleParams('R','E');

	redStartup();

	while(1)
    {
    	//Check Xbee Attention pin ( there might be unexpected data )
    	if(readingPacket == false  && !GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4) && dataUpdeted == false){
    		readingPacket = true;
    		blinkRedLed5();
			uint8_t numberOfDumpBytes, cheksum = 0;
			XBEE_CS_LOW();

			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				numberOfDumpBytes++;
				if(numberOfDumpBytes >5)			//Exit loop if there is no start delimiter
				break;
			}
			if(numberOfDumpBytes < 6){
				SPI1_TransRecieve(0x00);
				length = SPI1_TransRecieve(0x00);
				printf("Lenght: %d\n", length);
				uint8_t i = 0;
				for(; i < length; i ++ ){				//Read data based on packet length
					receivePacket[i] = SPI1_TransRecieve(0x00);
					cheksum += receivePacket[i];
				}
				cheksum += SPI1_TransRecieve(0x00);
				if(cheksum == 0xFF){
					dataUpdeted = true;
				}
				printf("Checksum:%d\n",cheksum);
				//Data is updated if checksum is true
				readingPacket = false;
			}
			XBEE_CS_HIGH();
    	}
    }
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt (data ready indicator)

		GPIOB->ODR ^= GPIO_Pin_9;
		if(readingPacket == false){
			uint8_t numberOfDumpBytes = 0;
			uint8_t cheksum = 0;
			GPIO_ResetBits(GPIOA,GPIO_Pin_4);
			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				numberOfDumpBytes++;
				if(numberOfDumpBytes >5)			//Exit loop if there is no start delimiter
					break;
			}
			readingPacket = true;
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
				dataUpdeted = true;					//Data is updated if checksum is true
			readingPacket = false;
			GPIO_SetBits(GPIOA,GPIO_Pin_4);
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}








