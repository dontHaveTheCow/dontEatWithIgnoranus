//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>

//My library includes
#include "SPI2.h"
#include "IndicationGPIOs.h"
#include "SysTickDelay.h"

//Device libraries
#include "Xbee.h"


int main(void)
{
	//SPI
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	//GPIO
	initializeIndicationGPIO();
	initialiseSysTick();
	//initializeXbeeATTnPin();

	restoreDefaults();
	delay_1s();
	//queue4Param('S','H', 0x01,0x02,0x03,0x04);
	delay_10ms();
	//readModuleParams('A','C');

    while(1)
    {
    	delay_1s();
    	blinkIndicationLedOnce();
    	transmitRequest(0x00,0x13,0xA2,0x00,0x40,0xE0,0x1B,0xA6, "TxData0A");
    }
}

void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET) //Handler for Xbee ATTn pin interrupt
	{
		GPIOC->ODR ^= GPIO_Pin_8; //blinky
		SPI1_TransRecieve(0x00);
		SPI1_TransRecieve(0x00);
		SPI1_TransRecieve(0x00);

	    EXTI_ClearITPendingBit(EXTI_Line4);
	}
}








