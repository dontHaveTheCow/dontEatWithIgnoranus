//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>

//Debugging
#include "debug.h"

//My library includes
#include "SPI1.h"
#include "SPI2.h"
#include "SysTickDelay.h"
#include "USART1.h"
#include "MyStringFunctions.h"

//Device libraries
#include "Xbee.h"
#include "Button.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"

//Dewi defines
#define RSSI_MESSAGE_0 "1 0"
#define RSSI_MESSAGE_1 "1 1"

//Xbee globals
static char recievePacket[64];
static bool dataUpdeted = false;
volatile uint8_t length;
bool readingPacket = false;
bool onOff = false;
uint8_t state = 2;

int main(void){

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
	//Usart1 for debugging and serial communication
	Usart1_Init(9600);
	#ifdef DEBUG
	DEBUG_MESSAGE("*USART initialized *\n");
	#endif
	//System clock for delays
	initialiseSysTick();
	//blink these 3 leds to indicate that gpio,usart and systick is initialzied
	blinkRedLed1();
	blinkRedLed2();
	blinkRedLed3();
	#ifdef DEBUG
	DEBUG_MESSAGE("*Systick initialized *\n");
	#endif

	//Xbee initialization
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	//SPI attention pin for incoming data alert
	initializeXbeeATTnPin();
	char transmitString[8];

	blinkRedLed4();
	#ifdef DEBUG
	DEBUG_MESSAGE("*Xbee initialized *\n");
	#endif

	//ADXL362Z
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	initializeADXL362();
	while(!return_ADXL_ready()){
		//wait time for caps to discharge
		delayMs(2000);
		initializeADXL362();
		delayMs(1000);
		blinkRedLed5();
		#ifdef DEBUG
		DEBUG_MESSAGE("*ACC not initialized successfully *\n");
		#endif
	}
	blinkRedLed5();
	#ifdef DEBUG
	DEBUG_MESSAGE("*ACC  initialized successfully *\n");
	#endif

	int16_t z = 0;
	int16_t z_low = 0;
	int16_t z_high = 0;
	char messurementString[6];
	//variable for iterations
	int i;
	redStartup();
	#ifdef DEBUG
	DEBUG_MESSAGE("*DEWI module ready *\n");
	#endif

	//Erase comments to find out address of node
/*
	i = readModuleParams('S','H');
	itoa(i,transmitString);
	#ifdef DEBUG
	DEBUG_MESSAGE("*High Adress: *\n");
	DEBUG_MESSAGE(transmitString);
	DEBUG_MESSAGE("*\n");
	#endif
	i = readModuleParams('S','L');
	itoa(i,transmitString);
	#ifdef DEBUG
	DEBUG_MESSAGE("*Low Adress: *\n");
	DEBUG_MESSAGE(transmitString);
	DEBUG_MESSAGE("*\n");
	#endif
*/


    while(1){
        	delayMs(800);
        	switch(state){
        	//Transmit acc data
    			case 0:
    				getZ(&z,&z_low,&z_high);
    				itoa(z, messurementString);
    				transmitString[0] = '0';
    				transmitString[1] = ' ';
    				for(i = 2; i < sizeof(transmitString); i++){
    					transmitString[i] = messurementString[i-2];
    				}
    				transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
    				#ifdef DEBUG
    				DEBUG_MESSAGE("*String sent: ");
    				DEBUG_MESSAGE(transmitString);
    				DEBUG_MESSAGE("*\n");
    				#endif
    				blinkGreenLed1();
    				state++;
    				break;
    		//Transmit gps data
    			case 1:
    				strcpy(transmitString, "1 1234");
    				transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
    				#ifdef DEBUG
    				DEBUG_MESSAGE("*String sent: ");
    				DEBUG_MESSAGE(transmitString);
    				DEBUG_MESSAGE("*\n");
    				#endif
    				blinkGreenLed2();
    				state = 0;
    				break;
    			case 2:
    				redStartup();
    				break;
        	}
   		}
  }

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button2 pin interrupt
		if(state < 2 )
			state = 2;
		else
			state = 0;
		EXTI_ClearITPendingBit(EXTI_Line8);
	}

	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt

		if(!readingPacket){

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
				recievePacket[i] = SPI1_TransRecieve(0x00);
				cheksum += recievePacket[i];
			}
			cheksum += SPI1_TransRecieve(0x00);
			if(cheksum == 0xFF)
				dataUpdeted = true;					//Data is updated if checksum is true

			readingPacket = false;
			GPIO_SetBits(GPIOA,GPIO_Pin_4);
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}

}








