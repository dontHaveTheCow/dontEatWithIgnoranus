#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_misc.h>

void Initialize_timer(void);
void initializeGreenLed1(void);
void Timer_interrupt_enable(void);

int main(void)
{
	Initialize_timer();
	initializeGreenLed1();
	Timer_interrupt_enable();

    for(;;)
    {
    }
}

void Initialize_timer(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef Timer_init_structure;
	Timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;
	Timer_init_structure.TIM_ClockDivision = 0;
	Timer_init_structure.TIM_Prescaler = 800;
	//Period equals 900ms
	Timer_init_structure.TIM_Period = 9000;
	Timer_init_structure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &Timer_init_structure);

}

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

void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
      	 GPIOC->ODR ^= GPIO_Pin_6;
	}
}


