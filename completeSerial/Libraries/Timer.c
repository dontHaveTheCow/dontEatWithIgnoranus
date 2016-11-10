#include "Timer.h"

void Initialize_timer(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef Timer_init_structure;
	Timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;
	Timer_init_structure.TIM_ClockDivision = 0;
	Timer_init_structure.TIM_Prescaler = PRESCALER_VALUE;
	Timer_init_structure.TIM_Period = PERIOD_VALUE;
	Timer_init_structure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &Timer_init_structure);

}

void Timer_interrupt_enable(void)
{
	NVIC_InitTypeDef NVIC_structure;
	NVIC_structure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_structure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_structure.NVIC_IRQChannelPriority = 0x01;
	NVIC_Init(&NVIC_structure);
	TIM_Cmd(TIM2,ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}


