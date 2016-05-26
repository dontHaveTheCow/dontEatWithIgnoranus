//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>

//My library includes
#include "USART1.h"
#include "USART2.h"
#include "SPI1.h"
#include "IndicationGPIOs.h"
#include "SysTickDelay.h"

//Device libraries
#include "Xbee.h"

int main(void)
{
	//USART
	Usart1Init();
	ConfigureUsart1Interrupt();
	//GPIO
	initializeIndicationGPIO();
	initialiseSysTick();
	initializeXbee();

    while(1)
    {
    	delay_1s();
    	blinkIndicationLedOnce();
    	Usart1SendString("StringSent \n");
    }
}







