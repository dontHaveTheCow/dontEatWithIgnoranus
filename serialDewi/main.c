#include <stm32f0xx.h>
#include <stdlib.h>
#include "USART1.h"
#include "myStringFunctions.h"


//Defines
#define PACKET_HEADER_SIZE 7
#define PACKET_END_SYMBOL '\0'
#define PACKET_DATA_SYMBOL 21

typedef enum {SENDER, RECIEVER, TWO_WAY, NONE} mote_role;

char s_delimiter[2] = "#";

// Radio
char serialBuffer[250];
static uint8_t packetLenght = 0;
bool serialReceived = false;

// Variables for test
mote_role role = NONE;
int addressCount = 0;
char addresses[10][65];
int packetSize = 0;
int packetCount = 0;

// For listener
int recievedPackets = 0;

bool experimentStarted = false;



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

		strcpy(addresses[addressCount++],value);
		strcat(value," - TARGET ADDED");
		Usart1_SendString(value);
		//Usart1_Send(addressCount + 0x30);

	}else if(strcmp(key,"CLEAR_TARGETS") == 0){

		addressCount = 0;

	}else if(strcmp(key,"SET_PACKET_SIZE") == 0){

		int i_value = atoi(value);
		packetSize = i_value;

	}else if(strcmp(key,"SET_PACKET_COUNT") == 0){

		int i_value = atoi(value);
		packetCount = i_value;

	}else if(strcmp(key,"START_EXPERIMENT") == 0){

		experimentStarted = true;

	}else if(strcmp(key,"SET_POWER") == 0){

	}else if(strcmp(key,"GET_ADDRESS") == 0){

		// ADDR#<address>
	}else{
		// WORNG DATA
		Usart1_SendString("WRONG INPUT DATA");
	}
}

int main(void)
{
	char key[16] = "";
	char value[64];
	//Usart1 for serial communication
	Usart1_Init(BAUD_9600);
	ConfigureUsart1Interrupt();

    while(1){

    	if(serialReceived == true){
    		str_splitter(serialBuffer,key,value,s_delimiter);
			processSerial(key, value);
			//Usart1_SendString(key);
			//Usart1_SendString(value);
			serialReceived = false;
		}
    }
}

void USART1_IRQHandler(void){
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){

		serialBuffer[packetLenght] = USART_ReceiveData(USART1);
		Usart1_Send(serialBuffer[packetLenght]);
		if(serialBuffer[packetLenght++] == '\r'){
			serialReceived = true;
			packetLenght = 0;
		}
	}
}




