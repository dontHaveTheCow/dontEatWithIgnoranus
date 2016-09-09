#include "IndicationGPIOs.h"
#include "SysTickDelay.h"

void initializeGreenLed1(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

void xorGreenLed1(void){
	GPIOC->ODR ^= GPIO_Pin_6;
}

void initializeGreenLed2(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

void initializeGreenLed3(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

void initializeRedLed1(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void xorRedLed1(void){
	GPIOB->ODR ^= GPIO_Pin_5;
}

void initializeRedLed2(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void initializeRedLed3(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void initializeRedLed4(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void initializeRedLed5(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void initializeDiscoveryLeds(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_structure;
	GPIO_structure.GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_structure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_structure.GPIO_OType = GPIO_OType_PP;
	GPIO_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_structure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOC, &GPIO_structure);
}

void xorDiscoveryGreen(void){
	GPIOC->ODR ^= GPIO_Pin_9;
}

void xorDiscoveryBlue(void){
	GPIOC->ODR ^= GPIO_Pin_8;
}

void blinkRedLed1(void){
		GPIOB->ODR |= GPIO_Pin_5;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_5;
    	delayMs(DELAY);
}

void blinkRedLed2(void){
		GPIOB->ODR |= GPIO_Pin_6;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_6;
    	delayMs(DELAY);
}

void blinkRedLed3(void){
		GPIOB->ODR |= GPIO_Pin_7;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_7;
    	delayMs(DELAY);
}

void blinkRedLed4(void){
		GPIOB->ODR |= GPIO_Pin_8;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_8;
    	delayMs(DELAY);
}

void blinkRedLed5(void){
		GPIOB->ODR |= GPIO_Pin_9;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_9;
    	delayMs(DELAY);
}



void blinkGreenLed1(void){
		GPIOC->ODR |= GPIO_Pin_6;
    	delayMs(DELAY);

    	GPIOC->ODR &=~ GPIO_Pin_6;
    	delayMs(DELAY);
}

void blinkGreenLed2(void){
		GPIOC->ODR |= GPIO_Pin_7;
    	delayMs(DELAY);

    	GPIOC->ODR &=~ GPIO_Pin_7;
    	delayMs(DELAY);
}

void blinkGreenLed3(void){
		GPIOC->ODR |= GPIO_Pin_8;
    	delayMs(DELAY);

    	GPIOC->ODR &=~ GPIO_Pin_8;
    	delayMs(DELAY);
}

void turnOnGreenLed1(void){
	GPIO_ResetBits(GPIOC,GPIO_Pin_All);
	GPIOC->ODR |= GPIO_Pin_6;
}
void turnOnGreenLed2(void){
	GPIO_ResetBits(GPIOC,GPIO_Pin_All);
	GPIOC->ODR |= GPIO_Pin_7;
}
void turnOnGreenLed3(void){
	GPIO_ResetBits(GPIOC,GPIO_Pin_All);
	GPIOC->ODR |= GPIO_Pin_8;
}
void turnOnRedLed1(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_All);
	GPIOB->ODR |= GPIO_Pin_5;
}
void turnOnRedLed2(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_All);
	GPIOB->ODR |= GPIO_Pin_6;
}
void turnOnRedLed3(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_All);
	GPIOB->ODR |= GPIO_Pin_7;
}
void turnOnRedLed4(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_All);
	GPIOB->ODR |= GPIO_Pin_8;
}
void turnOnRedLed5(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_All);
	GPIOB->ODR |= GPIO_Pin_9;
}

void blinkIndicationLedTwice(void){
	GPIOC->ODR |= GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR &=~ GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR |= GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR &=~ GPIO_Pin_8;
	delayMs(DELAY);
}

void blinkIndicationLedThrice(void){
	GPIOC->ODR |= GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR &=~ GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR |= GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR &=~ GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR |= GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR &=~ GPIO_Pin_8;
	delayMs(DELAY);
}



