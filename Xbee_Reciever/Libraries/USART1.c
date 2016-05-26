#include "USART1.h"

void Usart1Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	/* Connect pin to Periph */

	USART_DeInit(USART1);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	USART_InitTypeDef USART1_InitStructure;

	USART_ClockInitTypeDef USART_ClockInitStructure;
	USART_ClockStructInit(&USART_ClockInitStructure);
	USART_ClockInit(USART1, &USART_ClockInitStructure);

	USART1_InitStructure.USART_BaudRate = 9600;
	USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART1_InitStructure.USART_Parity = USART_Parity_No;
	USART1_InitStructure.USART_StopBits = USART_StopBits_1;
	USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART1_InitStructure);

	USART_Cmd(USART1, ENABLE);
}

void Usart1Send(uint8_t data)
{
	USART_SendData(USART1,data);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC ) == RESET)
	{
	}
}

void Usart1SendString(char* string){
	while(*string != 0){
		Usart1Send(*string++);
	}
}

char* Usart1RecieveString(void){
	char* String;
	while(*String != '/r'){
		while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == RESET);
		*String++ = USART_ReceiveData(USART1);
	}
	return String;
}

char* Usart1Recieve(void)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(USART1);
}

void ConfigureUsart1Interrupt(void)
{
	NVIC_InitTypeDef NVIC_structure;
	NVIC_structure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_structure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_structure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_Init(&NVIC_structure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void waitForOkResponse(void){
	while(!stringRecieved && strcmp(usartRecievedStringBuffer ,"OK/r") != 0);
	stringRecieved = false;
	memset(usartRecievedStringBuffer,0,sizeof(usartRecievedStringBuffer));
	usartRecievedStringLenght = 0;
	blinkIndicationLedOnce();
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		usartRecievedStringBuffer[usartRecievedStringLenght++] = USART_ReceiveData(USART1);
		if(USART_ReceiveData(USART1) == '\r')
			stringRecieved = true;
	}
}

