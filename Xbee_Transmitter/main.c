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

//Dewi globals
static uint8_t state = 1;	//0 -> ACC 1 -> RSSI 2 -> GPS
bool RSSI_previouslySent = false;

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
	//System clock for delays
	initialiseSysTick();
	//Turn on first status led
	GPIOC->ODR = (1 << (state+6));

	//Xbee initialization
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	//SPI attention pin for incoming data alert
	initializeXbeeATTnPin();
	char transmitString[8];

	//ADXL362Z
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	initializeADXL362();
	int16_t z = 0;
	int16_t z_low = 0;
	int16_t z_high = 0;
	char messurementString[6];
	//variable for iterations
	int i;

    while(1){

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
				delayMs(300);
				break;

		//Transmit rssi look-a-like
			case 1:
				if(RSSI_previouslySent){
					strcpy(transmitString,RSSI_MESSAGE_1);
					transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
					#ifdef DEBUG
					DEBUG_MESSAGE("*String sent: ");
					DEBUG_MESSAGE(transmitString);
					DEBUG_MESSAGE("*\n");
					#endif
				}
				else{
					strcpy(transmitString,RSSI_MESSAGE_0);
					transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
					#ifdef DEBUG
					DEBUG_MESSAGE("*String sent: ");
					DEBUG_MESSAGE(transmitString);
					DEBUG_MESSAGE("*\n");
					#endif
					RSSI_previouslySent = true;
				}
				blinkGreenLed2();
				delayMs(300);
				break;

		//Transmit gps data
			case 2:
				strcpy(transmitString, "2");
				transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
				#ifdef DEBUG
				DEBUG_MESSAGE("*String sent: ");
				DEBUG_MESSAGE(transmitString);
				DEBUG_MESSAGE("*\n");
				#endif
				blinkGreenLed3();
				delayMs(300);
				break;
    	}
    }
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button2 pin interrupt
		state++;
		if(state > 2)
			state = 0;
		GPIOC->ODR = (1 << (state+6));			//State 0 -> ACC | State 1 -> RSSI | State 2 -> GPS
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








