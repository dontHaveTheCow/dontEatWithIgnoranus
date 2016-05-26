//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>

//My library includes
#include "SPI1.h"
#include "IndicationGPIOs.h"
#include "SysTickDelay.h"

//Device libraries
#include "Xbee.h"

int main(void){

	InitialiseSPI_GPIO();
	InitialiseSPI();
	initialiseSysTick();
	initializeIndicationGPIO();
	initializeIndicationGPIO();
	initialiseSysTick();
	initializeXbee();

    while(1)
    {
    	delay_400ms();
    	/*
    	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    	// Send SPI_TransRecieve(READ_REGISTER);
    	// Recieve variable=SPI_TransRecieve(0x00);
    	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    	*/
    }
}







