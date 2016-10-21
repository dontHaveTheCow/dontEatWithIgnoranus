#include "SPI1.h"

void InitialiseSPI1_GPIO(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

	// PA5 - SCK PA6 - MISO PA7 - MOSI

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PA4 - CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PB1 - CS (for SDcard)
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void InitialiseSPI1()
{
	SPI_InitTypeDef SPI_InitStruct;

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_0);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_0);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_0);

	//Set CS high
	GPIO_SetBits(GPIOA,GPIO_Pin_4);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);


	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low; //Clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge; //Data sampled  at rising edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Hard; //Was Soft
	SPI_InitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_16;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI1, &SPI_InitStruct);

	SPI_SSOutputCmd(SPI1,ENABLE);
	SPI_Cmd(SPI1, ENABLE);
}

void Configure_SPI1_interrupt(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
	NVIC_Init(&NVIC_InitStructure);
}

void SPI1_SendByte(uint8_t byte)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI_SendData8(SPI1,byte);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}



void SPI1_ManualSendByte(uint8_t byte)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); //wait buffer empty
	SPI_SendData8(SPI1,byte);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

}

uint8_t SPI1_RecieveByte()
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); //wait buffer empty
	SPI_SendData8(SPI1,0x00);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_ReceiveData8(SPI1);
}

uint8_t SPI1_TransRecieve(uint8_t data){
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_SendData8(SPI1,data);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_ReceiveData8(SPI1);

	SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_RXNE);
}

uint8_t spi_rw(uint8_t out) {

	/*!< Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {
	}

	/*!< Send the byte */
	SPI_SendData8(SPI1, out);

	/*!< Wait to receive a byte*/
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	/*!< Return the byte read from the SPI bus */
	return SPI_ReceiveData8(SPI1);
}

uint8_t spi_r(void) {
	uint8_t Data = 0;

	/*!< Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {
	}
	/*!< Send the byte */
	SPI_SendData8(SPI1, 0xFF);

	/*!< Wait until a data is received */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	/*!< Get the received data */
	Data = SPI_ReceiveData8(SPI1);

	/*!< Return the shifted data */
	return Data;
}

void interface_speed(enum speed_setting speed) {
	uint32_t tmp;

	/* Lecture du registre CR1 ------------------------------- */
	tmp = SPI1->CR1;

	/* Test la configuration --- ----------------------------- */
	if (speed == INTERFACE_SLOW) {
		/* Set slow clock (100k-400k) ------------------------ */
		tmp = (tmp | SPI_BaudRatePrescaler_256);
	} else {
		/* Set fast clock (depends on the CSD) --------------- */
		tmp = (tmp & ~SPI_BaudRatePrescaler_256) | SPI_BaudRatePrescaler_16;
	}

	/* Ecriture de la nouvelle config. sur le registre CR1 --- */
	SPI1->CR1 = tmp;
}

void release_spi(void){
	GPIOB->BSRR = (1<<1);
	spi_r();
}

void spi_r_m(uint8_t *byte){
	/*!< Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {
	}
	/*!< Send the byte */
	SPI_SendData8(SPI1, 0xFF);

	/*!< Wait until a data is received */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	/*!< Get the received data */
	*byte = SPI_ReceiveData8(SPI1);
}




