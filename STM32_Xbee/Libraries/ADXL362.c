#include "ADXL362.h"

void initializeADXL362(void){
	//Sofware reset
	writeADXL326Register(RESET,0x52);
	delay_10ms();
	////Set FIFO size LSB bits
	/*writeADXL326Register(0x29,0x03);
	//Set stream mode and FIFO buffer MSB 0
	writeADXL326Register(0x28,0b00000010); //FIFO enabled (stream) 0b0000001 */
	//Set as measurement speed 12.5hz
	writeADXL326Register(0x2C,0b00010011);
	//Set as ultra-low noise and measurement mode
	writeADXL326Register(0x2D,0b00100010);
}

void writeADXL326Register(uint8_t reg, uint8_t cmd){
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI_ManualSendByte(WRITE_REGISTER);
	SPI_ManualSendByte(reg);
	SPI_ManualSendByte(cmd);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

uint8_t readADXL362Register(uint8_t reg){
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI_ManualSendByte(READ_REGISTER);
	SPI_ManualSendByte(reg);
	uint8_t val = SPI_RecieveByte();
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	return val;
}

int16_t SPIreadTwoRegisters(uint8_t regAddress){
	int16_t twoRegValue = 0;
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI_ManualSendByte(READ_REGISTER);
	SPI_ManualSendByte(regAddress);
	twoRegValue = SPI_RecieveByte();
	twoRegValue = twoRegValue + (SPI_RecieveByte() << 8);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	return twoRegValue;
}
