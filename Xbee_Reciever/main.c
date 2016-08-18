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

//Xbee packet Defines
#define AT_COMMAND_RESPONSE 0x88
#define RECIEVE_PACKET 0x90
#define MODEM_STATUS 0x8A

//Changeable defines
#define NUMBER_OF_NODES 4
#define ERROR_TIMER_COUNT 30

//Xbee globals
static char receivePacket[128];
volatile bool dataUpdated = false;
volatile uint8_t length,errorTimer,cheksum;
bool readingPacket = false;

struct node{
	uint8_t adress[8];
	int16_t measurment[3];	//0 -> ACC 1 -> RSSI 2 -> GPS
	uint8_t tmpRSSI;
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

	node[1].adress[0] = 0x00;
	node[1].adress[1] = 0x13;
	node[1].adress[2] = 0xA2;
	node[1].adress[3] = 0x00;
	node[1].adress[4] = 0x40;
	node[1].adress[5] = 0xEA;
	node[1].adress[6] = 0xEF;
	node[1].adress[7] = 0xE9;
	node[1].state = 0;

	//Usart for debugging and communication
	Usart1_Init(9600);
	#ifdef DEBUG
	DEBUG_MESSAGE("*USART initialized *\n");
	#endif

	//Our delay timer
	initialiseSysTick();
	#ifdef DEBUG
	DEBUG_MESSAGE("*Systick initialized *\n");
	#endif

	//Led's and buttons
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

	#ifdef DEBUG
	DEBUG_MESSAGE("*GPIO initialized *\n");
	#endif

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

	#ifdef DEBUG
	DEBUG_MESSAGE("*Xbee initialized *\n");
	#endif

	//ADXL362Z (acc)
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	initializeADXL362();
/*	while(!return_ADXL_ready()){
		//wait time for caps to discharge
		delayMs(3000);
		initializeADXL362();
		#ifdef DEBUG
		DEBUG_MESSAGE("*ACC not initialized successfully *\n");
		#endif
	}*/

	#ifdef DEBUG
	DEBUG_MESSAGE("*ACC  initialized successfully *\n");
	#endif

	redStartup();
	#ifdef DEBUG
	DEBUG_MESSAGE("*DEWI module ready *\n");
	#endif

	while(1)
    {
    	if(dataUpdated == true){
    		typeOfFrame = receivePacket[0];

    		switch(typeOfFrame){
    			case AT_COMMAND_RESPONSE:

					#ifdef DEBUG
					DEBUG_MESSAGE("*AT_COMMAND_RESPONSE received *\n");
					#endif

    		    	AT_command[0] = receivePacket[2];	//Understand the type of AT command received
    		        AT_command[1] = receivePacket[3];
    		        commandStatus = receivePacket[4];	//Understand whether there was a fail

					#ifdef DEBUG
					DEBUG_MESSAGE("*AT_COMMAND: ");
					DEBUG_SEND_BYTE(AT_command[0]);
					DEBUG_SEND_BYTE(AT_command[1]);
					DEBUG_MESSAGE("*\n");
					if(commandStatus == 0)
						DEBUG_MESSAGE("*COMMAND STATUS: OK\n");
					else
						DEBUG_MESSAGE("*COMMAND STATUS: ERROR\n");
					#endif

    		        //printf("typeOfFrame is %d \n",receivePacket[0]);
    		        //printf("AT command is %c%c \n",receivePacket[2],receivePacket[3]);
    		       // printf("commandStatus is %d \n",receivePacket[4]);

    		        i = 5;
    		        for(; i < length; i++){				//Read AT data
    		          	AT_data[i - 5] = receivePacket[i];
    		           	//printf("byte %d is %d \n",i,receivePacket[i]);
    		        }
    		    	dataUpdated = false;				//Mark packet as read
    				break;

    			case RECIEVE_PACKET:

					#ifdef DEBUG
					DEBUG_MESSAGE("*RECIEVE_PACKET_API_FRAME*\n");
					#endif

    				i = 1;	//Remember that "Frame type" byte was the first one
    				uint8_t eightByteSourceAdress[8]; //64 bit

    				for(; i < 9; i++){	//Read address from received packet
    					eightByteSourceAdress[i-1] = receivePacket[i];
    				}
    				for(n = 0; n < NUMBER_OF_NODES; n++){	//Find the matching node for the received address
    					if(memcmp(eightByteSourceAdress,node[n].adress, 8) == 0 )
    						currentNode = n;
    				}

					#ifdef DEBUG
					DEBUG_MESSAGE("*Node ");
					//Works if there are less then 10 nodes
					DEBUG_SEND_BYTE(currentNode+48);
					DEBUG_MESSAGE("*\n");
					#endif

    				//After reading source address, comes 2 reserved bytes
    				i = i + 2;
    				//In this case comandStatus actually is receive options
    				commandStatus = receivePacket[i++];
    				//Read the typeOfMessurement
    				node[currentNode].state = receivePacket[i++] - 0x30;	//Calculate the integer from ASCII by subtracting 0x30

					#ifdef DEBUG
    				if(node[currentNode].state == 0){
    					DEBUG_MESSAGE("*ACC measurement*\n");
    				}
    				else if(node[currentNode].state == 1){
    					DEBUG_MESSAGE("*RSSI measurement*\n");
    				}
    				else{
    					DEBUG_MESSAGE("*GPS measurement*\n");
    				}
					#endif

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

					#ifdef DEBUG
					DEBUG_MESSAGE("*Value: ");
					DEBUG_MESSAGE(stringOfMessurement);
					DEBUG_MESSAGE("*\n");
					#endif

    				if(receiverNode.state == node[currentNode].state){
    					switch(receiverNode.state){
							case 0:
								receiverNode.measurment[0] = returnZ_axis();
								//printf("My ACC:%d\n", receiverNode.measurment[0]);
								//printf("ACC difference:%d\n",abs(receiverNode.measurment[0] - node[currentNode].measurment[0]));

/*
								#ifdef DEBUG
								itoa(receiverNode.measurment[0],stringOfMessurement);
								DEBUG_MESSAGE("*My measurement: ");
								DEBUG_MESSAGE(stringOfMessurement);
								DEBUG_MESSAGE("*\n");
								#endif
*/

								if(abs(receiverNode.measurment[0] - node[currentNode].measurment[0]) < 80)
									turnOnRedLed(currentNode);
								else
									blinkRedLed(currentNode);
								break;
							case 1:
								if(node[currentNode].measurment[node[currentNode].state] == 0){ //gather the average RSSI from the first couple of RSSI packets
									RSSIdefaultCount = 0;
									node[currentNode].tmpRSSI = 0;
									#ifdef DEBUG
									DEBUG_MESSAGE("*Calculating average RSSI...*\n");
									#endif
								}
								else if(RSSIdefaultCount == 5){
									node[currentNode].measurment[node[currentNode].state] = readModuleParams('D','B') >> 24;
									if(abs(node[currentNode].tmpRSSI - node[currentNode].measurment[node[currentNode].state]) < 5)
										turnOnRedLed(currentNode);
									else
										blinkRedLed(currentNode);
									#ifdef DEBUG
									DEBUG_MESSAGE("*Average RSSI: ");
									itoa(node[currentNode].tmpRSSI, stringOfMessurement);
									DEBUG_MESSAGE(stringOfMessurement);
									DEBUG_MESSAGE(" Received packet's RSSI: ");
									itoa(node[currentNode].measurment[node[currentNode].state], stringOfMessurement);
									DEBUG_MESSAGE(stringOfMessurement);
									DEBUG_MESSAGE(" *\n");
									#endif
									//printf("Avarage RSSI:%d\n",receiverNode.measurment[1]);
									//printf("Friends RSSI:%d\n",node[currentNode].measurment[1]);
								}
								else if(RSSIdefaultCount++ < 5){
									/*
									 * Remember that result is in little endian, so one byte result is stored as msb in 32 bit integer
									 */
									node[currentNode].tmpRSSI = (node[currentNode].tmpRSSI +  (readModuleParams('D','B') >> 24)) / 2;
									blinkRedLed(currentNode);
								}
								break;
							case 2:
								blinkRedLed(currentNode);
								break;
    					}
    				}
    				else{
    					blinkRedLed(currentNode);
						#ifdef DEBUG
						DEBUG_MESSAGE("*ERROR in State compability*\n");
						#endif
    				}
    				dataUpdated = false;
    				break;
    			case MODEM_STATUS:
    				blinkAllRed();
    				if(receivePacket[1] == 0x00){
						#ifdef DEBUG
						DEBUG_MESSAGE("*ModemStatus - Hardware reset*\n");
						#endif
    				}
    				dataUpdated = false;
					break;
    			default:
    				//FUCK: There was another type of packet
					#ifdef DEBUG
					DEBUG_MESSAGE("*ERROR - unexpected packet*\n");
					#endif
    				dataUpdated = false;
    				break;
    		}
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
				#ifdef DEBUG
				DEBUG_MESSAGE("*Unexpected data read correctly*\n");
				#endif
			}
			else{
				readingPacket = false;
				#ifdef DEBUG
				DEBUG_MESSAGE("*There was timing error in unexpected data*\n");
				#endif
			}
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
		#ifdef DEBUG
		DEBUG_MESSAGE("*State changed*\n");
		#endif
		EXTI_ClearITPendingBit(EXTI_Line8);
	}


	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt (data ready indicator)

		if(readingPacket == false){
			errorTimer, cheksum = 0;
			GPIO_ResetBits(GPIOA,GPIO_Pin_4);
			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				if(errorTimer++ > ERROR_TIMER_COUNT)			//Exit loop if there is no start delimiter
					return;
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
				dataUpdated = true;					//Data is updated if checksum is true
			readingPacket = false;
			GPIO_SetBits(GPIOA,GPIO_Pin_4);
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}








