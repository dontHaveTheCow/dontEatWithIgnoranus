#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
#include "IndicationGPIOs.h"
#include "USART1.h"
#include "USART2.h"

#define WAKEUP_PIN GPIO_Pin_3					//PC3
#define RESET_PIN GPIO_Pin_2					//PC2
#define ON_PIN GPIO_Pin_4						//PF4

#define BAUD_9600 9600
#define BAUD_4800 4800


int main(void)
{

	//Clock for GPS
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
	GPIO_InitStructure.GPIO_Pin = RESET_PIN;
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

	initialiseSysTick();
	//initializeDiscoveryLeds();
	Usart2_Init(BAUD_4800);
	Usart1_Init(BAUD_9600);
	ConfigureUsart2Interrupt();

	//Set reset pin
/*	GPIO_ResetBits(GPIOC, RESET_PIN);
	delayMs(200);
	GPIO_SetBits(GPIOC, RESET_PIN);*/
	//Set on pin
	delay_1s();
    GPIO_SetBits(GPIOF, ON_PIN);
    delayMs(200);
	GPIO_ResetBits(GPIOF, ON_PIN);

/*	GPIOC->ODR ^= GPIO_Pin_8;
	delayMs(DELAY);
	GPIOC->ODR ^= GPIO_Pin_8;
	GPIOC->ODR ^= GPIO_Pin_9;
	delayMs(DELAY);
	GPIOC->ODR ^= GPIO_Pin_9;*/
    while(1){

    }
}

void USART2_IRQHandler(void){
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		//blink a led
    	//GPIOB->ODR ^= GPIO_Pin_5;
		Usart1_Send(USART_ReceiveData(USART2));
	}
}
