#include "A2035H.h"

void initializeA2035H(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF,  ENABLE);
	//Initialize GPS ON PIN
	GPIO_InitStructure.GPIO_Pin = ON_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	//Initialize GPS RESET PIN
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,  ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPS_RESET_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//Initialize input for WAKEUP_PIN
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,  ENABLE);
	GPIO_InitStructure.GPIO_Pin = WAKEUP_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	//Set reset pin
	GPIO_ResetBits(GPIOC, GPS_RESET_PIN);
	delayMs(200);
	GPIO_SetBits(GPIOC, GPS_RESET_PIN);
	//Set on pin
	delay_1s();
    GPIO_SetBits(GPIOF, ON_PIN);
    delayMs(200);
	GPIO_ResetBits(GPIOF, ON_PIN);
}
void wake_da_A2035H(void){
	GPIO_ResetBits(GPIOC, GPS_RESET_PIN);
	delayMs(200);
	GPIO_SetBits(GPIOC, GPS_RESET_PIN);
	delay_1s();
	//Set on pin
    GPIO_SetBits(GPIOF, ON_PIN);
    delayMs(200);
	GPIO_ResetBits(GPIOF, ON_PIN);
}
void hibernate_da_A2035H(void){
	//Set on pin
    GPIO_SetBits(GPIOF, ON_PIN);
    delayMs(200);
	GPIO_ResetBits(GPIOF, ON_PIN);
}
