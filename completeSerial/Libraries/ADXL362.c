#include "ADXL362.h"

void initializeADXL362(void){
	//Sofware reset
	writeADXL326Register(ADXL_RESET_REGISTER,ADXL_RESET_CMD);
	delay_10ms();
	////Set FIFO size LSB bits
	/*writeADXL326Register(0x29,0x03);
	//Set stream mode and FIFO buffer MSB 0
	writeADXL326Register(0x28,0b00000010); //FIFO enabled (stream) 0b0000001 */
	//Set as measurement speed 12.5hz
	writeADXL326Register(ADXL_FILTER_CTL_REGISTER,0b00010011);
	//Set as ultra-low noise and measurement mode
	writeADXL326Register(ADXL_POWER_CTL_REGISTER,0b00100010);
}

void writeADXL326Register(uint8_t reg, uint8_t cmd){
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	SPI2_TransRecieve(ADXL_WRITE);
	SPI2_TransRecieve(reg);
	SPI2_TransRecieve(cmd);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

uint8_t readADXL362Register(uint8_t reg){
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	SPI2_TransRecieve(ADXL_READ);
	SPI2_TransRecieve(reg);
	uint8_t val = SPI2_TransRecieve(0x00);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	return val;
}

int16_t SPIreadTwoRegisters(uint8_t regAddress){
	int16_t twoRegValue = 0;
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	SPI2_TransRecieve(ADXL_READ);
	SPI2_TransRecieve(regAddress);
	twoRegValue = SPI2_TransRecieve(0x00);
	twoRegValue = twoRegValue + (SPI2_TransRecieve(0x00) << 8);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	return twoRegValue;
}

void getZ(int16_t *z, int16_t *z_low, int16_t *z_high){

	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	SPI2_TransRecieve(ADXL_READ);
	SPI2_TransRecieve(0x12);
	*z_low=SPI2_TransRecieve(0x00);
	*z_high=SPI2_TransRecieve(0x00);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);

	*z = (*z_high << 8) + *z_low;
}

int16_t returnZ_axis(void){

	int16_t z_low = 0;
	int16_t z_high = 0;

	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	SPI2_TransRecieve(ADXL_READ);
	SPI2_TransRecieve(0x12);
	z_low=SPI2_TransRecieve(0x00);
	z_high=SPI2_TransRecieve(0x00);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);

	return (z_high << 8) + z_low;
}

int16_t returnX_axis(void){

	int16_t x_low = 0;
	int16_t x_high = 0;

	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	SPI2_TransRecieve(ADXL_READ);
	SPI2_TransRecieve(0x0E);
	x_low=SPI2_TransRecieve(0x00);
	x_high=SPI2_TransRecieve(0x00);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);

	return (x_high << 8) + x_low;
}

bool return_ADXL_ready(void){

	if ((readADXL362Register(ADXL_STATUS_REGISTER) & ADXL_STATUS_REGISTER_DATA_READY_BIT) == ADXL_STATUS_REGISTER_DATA_READY_BIT)
		return true;

	else
		return false;
}
