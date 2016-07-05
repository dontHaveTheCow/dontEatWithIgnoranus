//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>

//My library includes
#include "SPI1.h"
#include "SPI2.h"
#include "SysTickDelay.h"

//Device libraries
#include "Xbee.h"
#include "Button.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"

#define MASTER_NODE_HIGH 0x0013A200
#define MASTER_NODE_LOW 0x40E01BA6

//Xbee globals
static char recievePacket[64];
static bool dataUpdeted = false;
volatile uint8_t length;
bool readingPacket = false;

//Dewi globals
static uint8_t state = 0;	//0 -> ACC 1 -> RSSI 2 -> GPS
bool RSSI_previouslySent = false;

int main(void){

	//Leds and buttons
	initializeButton1();
	initializeButton2();
	initializeGreenLed1();
	initializeGreenLed2();
	initializeGreenLed3();
	initializeRedLed1();
	initializeRedLed2();
	initializeRedLed3();
	initializeRedLed4();
	initializeRedLed5();
	initialiseSysTick();				//System clock for delays
	//Turn on first status led
	GPIOC->ODR = (1 << (state+6));

	//Xbee
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	initializeXbeeATTnPin();			//SPI attention pin for incoming data alert

	char transmitString[] = "bestMessageEver";

    while(1){
			transmitRequest(MASTER_NODE_HIGH,MASTER_NODE_LOW, TRANSOPT_DIGIMESH, transmitString);
			toggleRedLed5();
			delayMs(300);
    }
}

void EXTI4_15_IRQHandler(void)
{
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








