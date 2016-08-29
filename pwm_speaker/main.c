#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"

void InitializeTimer(void);
void InitializeLEDs(void);
void InitializePwm(void);

//Use pwm on pb4

int main(void){

	InitializeLEDs();
	InitializeTimer();
	InitializePwm();

	TIM3->CCR1 = 750;

    while(1){

    }
}

void InitializePwm()
{
	TIM_OCInitTypeDef  TIM_OCInitStructure ;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
	//TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	//TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	//TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_1);
}

void InitializeTimer(void)
{
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	  /* Time Base configuration */
	  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	  TIM_TimeBaseStructure.TIM_Prescaler = 48;
	  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	  TIM_TimeBaseStructure.TIM_Period = 2000;
	  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	  TIM_Cmd(TIM3, ENABLE);
}

void InitializeLEDs()
{
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
