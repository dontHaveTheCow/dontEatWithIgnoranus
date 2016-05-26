//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>

//Debugging
#include <stdio.h>

//My library includes
#include "SPI1.h"
#include "SPI2.h"
#include "SysTickDelay.h"

//Device libraries
#include "Xbee.h"
#include "Button.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"

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
	//packet variables
/*	uint8_t typeOfFrame;
	uint8_t AT_command[2];
	uint8_t commandStatus;
	uint8_t AT_data[4];*/
	char transmitString[] = "         ";

	//ADXL362Z
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	initializeADXL362();
	int16_t z = 0;
	int16_t z_low = 0;
	int16_t z_high = 0;

    while(1){
    	switch(state){
			case 0:		//Transmit acc data
				getZ(&z,&z_low,&z_high);
				sprintf(transmitString,"0 %d", z);
				transmitRequest(0x00,0x13,0xA2,0x00,0x40,0xE0,0x1B,0xA6, transmitString);
				blinkGreenLed1();
				delayMs(300);
				break;

			case 1:		//Transmit rssi look-a-like
				sprintf(transmitString, "1 %d", RSSI_previouslySent);
				transmitRequest(0x00,0x13,0xA2,0x00,0x40,0xE0,0x1B,0xA6, transmitString);
				RSSI_previouslySent = true;
				blinkGreenLed2();
				delayMs(300);
				break;

			case 2:
/*				delayMs(100);
				blinkGreenLed3();
				transmitRequest(0x00,0x13,0xA2,0x00,0x40,0xE0,0x1B,0xA6, "2 ");*/
				break;
    	}
    }
}

void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line9) == SET){	//Handler for Button1 pin interrupt
		GPIOC->ODR = 0;
		--state;
		if(state > 2){
			state = 2;
			GPIOC->ODR |= GPIO_Pin_8;
		}
		RSSI_previouslySent = false;
		EXTI_ClearITPendingBit(EXTI_Line9);
	}

	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button2 pin interrupt
		GPIOC->ODR = 0;
		++state;
		if(state > 2)
			state = 0;
		else if(state == 2){
			GPIOC->ODR |= GPIO_Pin_8;
		}
		RSSI_previouslySent = false;
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








