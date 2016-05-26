#include "SysTickDelay.h"


void initialiseSysTick(void){
	RCC_ClocksTypeDef RCC_clocks;
	RCC_GetClocksFreq(&RCC_clocks);
	SysTick_Config(RCC_clocks.HCLK_Frequency/100000);
}


void delayMs(int ms){
	sysTickCounter = 100 * ms;
	while (sysTickCounter != 0)
	{
	}
}


void SysTick_Handler(void)
{
	if(sysTickCounter != 0)
	sysTickCounter--;
}
