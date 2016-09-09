#include "A2035H.h"

void turnGpsOn(void){
		GPIO_ResetBits(GPIOC, RESET_PIN);
		delayMs(200);
		GPIO_SetBits(GPIOC, RESET_PIN);
		//Set on pin
		delay_1s();
	    GPIO_SetBits(GPIOF, ON_PIN);
	    delayMs(200);
		GPIO_ResetBits(GPIOF, ON_PIN);
}
void hibernateGps(void){

	GPIO_ResetBits(GPIOC, RESET_PIN);
	delayMs(200);
	GPIO_SetBits(GPIOC, RESET_PIN);
}
void setupGpsGpio(void){

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
}

//RCC_APB1Periph_TIM3 is used for speaker pwm - dont use that!!!
//okkk i will use RCC_APB1Periph_TIM2
/*void setupGpsTimer(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef Timer_init_structure;
	Timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;
	Timer_init_structure.TIM_ClockDivision = 0;
	Timer_init_structure.TIM_Prescaler = 800;
	//Period equals 900ms
	Timer_init_structure.TIM_Period = 8500;
	Timer_init_structure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &Timer_init_structure);
}

void setupGpsTimerInterrupt(void){
	NVIC_InitTypeDef NVIC_structure;
	NVIC_structure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_structure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_structure.NVIC_IRQChannelPriority = 0x01;
	NVIC_Init(&NVIC_structure);
	//TIM_Cmd(TIM2,ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}*/
