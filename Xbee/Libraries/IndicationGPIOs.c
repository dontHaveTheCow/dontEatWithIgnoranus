#include "IndicationGPIOs.h"
#include "SysTickDelay.h"

void initializeIndicationGPIO(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

void blinkIndicationLedOnce(void){
		GPIOC->ODR |= GPIO_Pin_8;
    	delay_400ms();
    	GPIOC->ODR &=~ GPIO_Pin_8;
    	delay_400ms();
}

void blinkIndicationLedTwice(void){
	GPIOC->ODR |= GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR &=~ GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR |= GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR &=~ GPIO_Pin_8;
	delay_400ms();
}

void blinkIndicationLedThrice(void){
	GPIOC->ODR |= GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR &=~ GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR |= GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR &=~ GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR |= GPIO_Pin_8;
	delay_400ms();
	GPIOC->ODR &=~ GPIO_Pin_8;
	delay_400ms();
}



