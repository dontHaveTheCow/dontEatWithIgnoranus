#include "SysTickDelay.h"


void initialiseSysTick(void){
	RCC_ClocksTypeDef RCC_clocks;
	RCC_GetClocksFreq(&RCC_clocks);
	SysTick_Config(RCC_clocks.HCLK_Frequency/100000);
}

void delay_1ms(void){
	sysTickCounter = 100;
	while (sysTickCounter != 0)
	{
	}
}

void delay_80ms(void){
	sysTickCounter = 8000;
	while (sysTickCounter != 0)
	{
	}
}

void delay_400ms(void){
	sysTickCounter = 40000;
	while (sysTickCounter != 0)
	{
	}
}

void delay_10us(void){
	sysTickCounter = 1;
	while (sysTickCounter != 0)
	{
	}
}

void delay_1s(void){
	sysTickCounter = 100000;
	while (sysTickCounter != 0)
	{
	}
}

void delay_10ms(void){
	sysTickCounter = 1000;
	while (sysTickCounter != 0)
	{
	}
}


void SysTick_Handler(void)
{
	if(sysTickCounter != 0)
	sysTickCounter--;
}
