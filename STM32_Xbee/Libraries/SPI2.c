#include "SPI2.h"

void InitialiseSPI2_GPIO(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

	//PB12 - CS // PB13 - SCK PB14 - MISO PB15 - MOSI

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void InitialiseSPI2()
{
	SPI_InitTypeDef SPI_InitStruct;

	//Set CS high
	GPIO_SetBits(GPIOB,GPIO_Pin_12);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);


	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low; //Clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge; //Data sampled  at rising edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Hard; //Was Soft
	SPI_InitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_16;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI2, &SPI_InitStruct);

	SPI_SSOutputCmd(SPI2,ENABLE);
	SPI_Cmd(SPI2, ENABLE);
}

void SPI2_SendByte(uint8_t byte)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	SPI_SendData8(SPI2,byte);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}


void SPI2_ManualSendByte(uint8_t byte)
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET); //wait buffer empty
	SPI_SendData8(SPI2,byte);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);

}

uint8_t SPI2_TransRecieve(uint8_t data){
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_SendData8(SPI2,data);
//	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	//SPI_I2S_ClearFlag(SPI2, SPI_I2S_FLAG_RXNE);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	return SPI_ReceiveData8(SPI2);


}

uint8_t SPI2_RecieveByte()
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET); //wait buffer empty
	SPI_SendData8(SPI2,0x00);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_ReceiveData8(SPI2);
}


void Configure_SPI2_interrupt(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_Init(&NVIC_InitStructure);
}


