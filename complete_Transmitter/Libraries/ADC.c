#include "ADC.h"

void adcConfig(void){

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_DeInit(ADC1);
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Backward;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_ChannelConfig(ADC1, ADC_Channel_0 , ADC_SampleTime_239_5Cycles);

	ADC_GetCalibrationFactor(ADC1);
	ADC_Cmd(ADC1, ENABLE);

	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN));
	ADC_StartOfConversion(ADC1);
}

void adcInterruptConfig(void){

  NVIC_InitTypeDef NVIC_ADC1;

  NVIC_ADC1.NVIC_IRQChannelPriority   = 0x0F;
  NVIC_ADC1.NVIC_IRQChannel = ADC1_COMP_IRQn ;
  NVIC_ADC1.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_ADC1);
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
}

void adcPinConfig(void){

 GPIO_InitTypeDef GPIO_InitStructure;
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
 GPIO_Init(GPIOA, &GPIO_InitStructure);
}

