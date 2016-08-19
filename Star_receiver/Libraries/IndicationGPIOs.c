#include "IndicationGPIOs.h"
#include "SysTickDelay.h"

#define DELAY 20
#define SLOW_DELAY 50
#define REAL_SLOW_DELAY 100

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

void turnOnGreenLed(uint8_t pin){
	GPIO_ResetBits(GPIOC,GPIO_Pin_All);
	GPIOC->ODR |= (1 << (6+pin));
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
    	delayMs(REAL_SLOW_DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_8;
    	delayMs(REAL_SLOW_DELAY);
}

void blinkRedLed5(void){
		GPIOB->ODR |= GPIO_Pin_9;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_9;
    	delayMs(DELAY);
}

void blinkAllRed(void){
	GPIOB->ODR |= GPIO_Pin_5;
	GPIOB->ODR |= GPIO_Pin_6;
	GPIOB->ODR |= GPIO_Pin_7;
	GPIOB->ODR |= GPIO_Pin_8;
	GPIOB->ODR |= GPIO_Pin_9;
	delayMs(DELAY);

	GPIOB->ODR &=~ GPIO_Pin_5;
	GPIOB->ODR &=~ GPIO_Pin_6;
	GPIOB->ODR &=~ GPIO_Pin_7;
	GPIOB->ODR &=~ GPIO_Pin_8;
	GPIOB->ODR &=~ GPIO_Pin_9;
	delayMs(DELAY);
}

void turnOnRedLed(uint8_t pin){
	GPIOB->ODR |= (1 << (5+pin));
}
void blinkRedLed(uint8_t pin){
	//5 to 9
	GPIOB->ODR |= (1 << (5+pin));
	delayMs(DELAY);

	GPIOB->ODR &=~ (1 << (5+pin));
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

void redStartup(void){

	int i = 5;
	for(; i > 0; i--){
		GPIOB->ODR |= (1 << (i+4));
		delayMs(SLOW_DELAY);
	}
	for(; i < 5; i++){
		GPIOB->ODR &=~ (1 << (i+4));
		delayMs(SLOW_DELAY);
	}
	for(; i > 0; i--){
		GPIOB->ODR |= (1 << (i+4));
		delayMs(SLOW_DELAY);
	}
	for(; i < 6; i++){
		GPIOB->ODR &=~ (1 << (i+4));
		delayMs(SLOW_DELAY);
	}
}


