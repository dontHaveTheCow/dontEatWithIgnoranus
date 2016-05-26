#include "SPI1.h"
#include "SysTickDelay.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"

#include <stdio.h>
#include <semihosting.h>

int main(void)
{

	InitialiseSPI_GPIO();
	InitialiseSPI();
	//Configure_SPI_interrupt();
	initialiseSysTick();
	initializeIndicationGPIO();
	initializeADXL362();

	int8_t x = 0;
	int8_t y = 0;
	int16_t z = 0;
	uint8_t status = 0x01;

	int16_t z_low = 0;
	int16_t z_high = 0;

    while(1)
    {
    	blinkIndicationLedOnce();
    	delay_400ms();
    	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    	SPI_TransRecieve(READ_REGISTER);
    	//GPIO_SetBits(GPIOA,GPIO_Pin_4);
    	SPI_TransRecieve(0x12);
    	z_low=SPI_TransRecieve(0x00);
    	z_high=SPI_TransRecieve(0x00);
    	GPIO_SetBits(GPIOA,GPIO_Pin_4);

    	z = (z_high << 8) + z_low;
    	//printf("x:%d y:%d z:%d\n",x,y,z);
    	printf("z_low: %d, z_high: %d\n", z_low, z_high);

    	//printf("x:%d y:%d\n",x,y);
    	printf("z:%d\n",z);
        printf("status %d\n",status);
    }
}


