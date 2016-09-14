#include "USART2.h"

void Usart2_Init(int baudrate)
{
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* Enable USART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART2_InitStructure;

	/* Configure USART Tx, Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART2_InitStructure.USART_BaudRate = baudrate;
	USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART2_InitStructure.USART_Parity = USART_Parity_No;
	USART2_InitStructure.USART_StopBits = USART_StopBits_1;
	USART2_InitStructure.USART_WordLength = USART_WordLength_8b;

	/* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
    /* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

	USART_Init(USART2,&USART2_InitStructure);
	USART_Cmd(USART2, ENABLE);
}

void Usart2_Send(uint8_t data)
{
	USART_SendData(USART2,data);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC ) == RESET)
	{
	}
}

void Usart2_SendString(char* string){
	while(*string != 0){
		Usart2Send(*string++);
	}
}

uint8_t Usart2_Recieve(void)
{
	while(USART_GetFlagStatus(USART2,USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(USART2);
}

void ConfigureUsart2Interrupt(void)
{
	NVIC_InitTypeDef NVIC_structure;

	/* Enable the USART2 Interrupt */
	NVIC_structure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_structure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_structure.NVIC_IRQChannelPriority = 0;
	NVIC_Init(&NVIC_structure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}
