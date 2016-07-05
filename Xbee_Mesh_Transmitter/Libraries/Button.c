#include "Button.h"

void initializeButton1(void){
	  //Interrupt pin - PC9

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

		GPIO_InitTypeDef GPIO_structure;
		GPIO_structure.GPIO_Pin=GPIO_Pin_9;
		GPIO_structure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_structure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Init(GPIOC, &GPIO_structure);

		RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, ENABLE);
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource9);

		EXTI_InitTypeDef   EXTI_InitStructure;
		EXTI_InitStructure.EXTI_Line= EXTI_Line9;
		EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd=ENABLE;
		EXTI_Init(&EXTI_InitStructure);

		NVIC_InitTypeDef   NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
		NVIC_Init(&NVIC_InitStructure);
}


void initializeButton2(void){
	  //Interrupt pin - PA8

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_InitTypeDef GPIO_structure;
		GPIO_structure.GPIO_Pin=GPIO_Pin_8;
		GPIO_structure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_structure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Init(GPIOA, &GPIO_structure);

		RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, ENABLE);
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource8);

		EXTI_InitTypeDef   EXTI_InitStructure;
		EXTI_InitStructure.EXTI_Line= EXTI_Line8;
		EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd=ENABLE;
		EXTI_Init(&EXTI_InitStructure);

		NVIC_InitTypeDef   NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
		NVIC_Init(&NVIC_InitStructure);
}
