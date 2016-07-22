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
#include "USART1.h"

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
	Usart1_Init(9600);
	//Leds and buttons
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
	uint32_t ATword;
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

	//ADXL362Z (acc)
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	initializeADXL362();

	ATword = readModuleParams('S','H');
	Usart1_Send('*');
	Usart1_Send(ATword >> 24);
	Usart1_Send(ATword >> 16);
	Usart1_Send(ATword >> 8);
	Usart1_Send(ATword);
	Usart1_Send('*');

	redStartup();

	while(1)
    {
    	if(readingPacket == false && dataUpdeted == true){
    		typeOfFrame = receivePacket[0];

    		switch(typeOfFrame){
    			case AT_COMMAND_RESPONSE:

    		    	AT_command[0] = receivePacket[2];	//Understand the type of AT command received
    		        AT_command[1] = receivePacket[3];
    		        commandStatus = receivePacket[4];	//Understand whether there was a fail

    		        //printf("typeOfFrame is %d \n",receivePacket[0]);
    		        //printf("AT command is %c%c \n",receivePacket[2],receivePacket[3]);
    		       // printf("commandStatus is %d \n",receivePacket[4]);

    		        i = 5;
    		        for(; i < length; i++){				//Read AT data
    		          	AT_data[i - 5] = receivePacket[i];
    		           	//printf("byte %d is %d \n",i,receivePacket[i]);
    		        }
    		    	dataUpdeted = false;				//Mark packet as read
    		    	//printf("After AT_COMMAND_RESPONSE:%d\n", receiverNode.state);
    		    	dataUpdeted = false;
    				break;

    			case RECIEVE_PACKET:
    				i = 1;	//Remember that "Frame type" byte was the first one
    				uint8_t eightByteSourceAdress[8]; //64 bit

    				for(; i < 9; i++){	//Read address from received packet
    					eightByteSourceAdress[i-1] = receivePacket[i];
    				}
    				for(n = 0; n < NUMBER_OF_NODES; n++){	//Find the matching node for the received address
    					if(memcmp(eightByteSourceAdress,node[n].adress, 8) == 0 )
    						currentNode = n;
    				}

    				//After reading source address, comes 2 reserved bytes
    				i = i + 2;
    				//In this case comandStatus actually is receive options
    				commandStatus = receivePacket[i++];
    				//Read the typeOfMessurement
    				node[currentNode].state = receivePacket[i++] - 0x30;	//Calculate the integer from ASCII by subtracting 0x30
    				i++;
    				//Read the recievedMessurement(data from sensors)
    				n = 0;
    				while (receivePacket[i] != '\0'){
    					stringOfMessurement[n++] = receivePacket[i];
    					i++;
    				}
    				stringOfMessurement[n] = '\0';
    				node[currentNode].measurment[node[currentNode].state] = atoi(stringOfMessurement);
    				//printf("node:%d\tFriendState:%d\tdata:%d\n", currentNode, node[currentNode].state, node[currentNode].measurment[node[currentNode].state]);

    				if(receiverNode.state == node[currentNode].state){
    					switch(receiverNode.state){
							case 0:
								receiverNode.measurment[0] = getZV2();
								//printf("My ACC:%d\n", receiverNode.measurment[0]);
								//printf("ACC difference:%d\n",abs(receiverNode.measurment[0] - node[currentNode].measurment[0]));
								if(abs(receiverNode.measurment[0] - node[currentNode].measurment[0]) < 50)
									turnOnRedLed(currentNode);
								else
									blinkRedLed(currentNode);
								break;
							case 1:
								if(node[currentNode].measurment[node[currentNode].state] == 0){ //gather the average RSSI from the first couple of RSSI packets
									RSSIdefaultCount = 0;
								}
								if(RSSIdefaultCount < 5){
									RSSIdefaultValues[RSSIdefaultCount] = readModuleParams('D','B');
									RSSIdefaultCount++;
									blinkRedLed(currentNode);
								}
								if(RSSIdefaultCount == 5){
									receiverNode.measurment[1]=(RSSIdefaultValues[0]+RSSIdefaultValues[1]+RSSIdefaultValues[2]+RSSIdefaultValues[3]+RSSIdefaultValues[4]) / 5;
									RSSIdefaultCount++;
								}
								if(RSSIdefaultCount == 6){
									node[currentNode].measurment[1] = readModuleParams('D','B');
									if(abs(receiverNode.measurment[1] - node[currentNode].measurment[1]) < 5)
										turnOnRedLed(currentNode);
									else
										blinkRedLed(currentNode);
									//printf("Avarage RSSI:%d\n",receiverNode.measurment[1]);
									//printf("Friends RSSI:%d\n",node[currentNode].measurment[1]);

								}
								break;
							case 2:
								blinkRedLed(currentNode);
								break;
    					}
    				}
    				else{
    					blinkRedLed(currentNode);
    					//printf("Error in state compatibility \n");
    				}
    				dataUpdeted = false;
    				break;
    			case MODEM_STATUS:
    				blinkRedLed3();
    				dataUpdeted = false;
    				//printf("At MODEM_STATUS of switch:%d\n", receiverNode.state);
    			default :
    				//FUCK: There was another type of packet
    				//printf("At DEFAULT of switch:%d\n", typeOfFrame);
    				dataUpdeted = false;
    				break;
    		}
    	}
    	//Check Xbee Attention pin ( there might be unexpected data )
    	else if(readingPacket == false  && !GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4) && dataUpdeted == false){
    		readingPacket = true;
    		blinkRedLed5();
			uint8_t numberOfDumpBytes, cheksum = 0;
			GPIO_ResetBits(GPIOA,GPIO_Pin_4);

			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				numberOfDumpBytes++;
				if(numberOfDumpBytes >5)			//Exit loop if there is no start delimiter
				break;
			}
			if(numberOfDumpBytes < 6){
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
					dataUpdeted = true;
				}
				//printf("Checksum:%d\n",cheksum);
				//Data is updated if checksum is true
				readingPacket = false;
			}
			GPIO_SetBits(GPIOA,GPIO_Pin_4);
    	}
    }
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button2 pin interrupt
		receiverNode.state++;
		if(receiverNode.state > 2)
			receiverNode.state = 0;
		GPIOC->ODR = (1 << (receiverNode.state+6));			//State 0 -> ACC | State 1 -> RSSI | State 2 -> GPS
		EXTI_ClearITPendingBit(EXTI_Line8);
	}


	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt (data ready indicator)

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








