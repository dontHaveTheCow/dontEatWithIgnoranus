#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
#include "IndicationGPIOs.h"
#include "USART1.h"
#include "stdbool.h"

#define BAUD_9600 9600
#define RXBUFFERSIZE 0x20
#define TXBUFFERSIZE 0x20

uint8_t RxBuffer[RXBUFFERSIZE];
uint8_t TxBuffer[TXBUFFERSIZE];
uint8_t RxLenght = 0;
bool stringRecieved;

int main(void){

	//Initialize board
	initializeGreenLed1();
	initializeGreenLed2();
	initializeGreenLed3();
	initializeRedLed1();
	initializeRedLed2();
	initializeRedLed3();
	initializeRedLed4();
	initializeRedLed5();
	initialiseSysTick();

	//Initialize UART
	Usart1Init(BAUD_9600);
	ConfigureUsart1Interrupt();

	while(1){
		delayMs(1000);
		blinkGreenLed1();
    }
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		//blink a led
    	GPIOB->ODR ^= GPIO_Pin_5;

    	RxBuffer[RxLenght] = USART_ReceiveData(USART1);

    	//Send response when carriage return received
		if(RxBuffer[RxLenght++] == '\r'){
			RxBuffer[RxLenght] = '\0';
			Usart1SendString("Received ");
			Usart1SendString((char*)RxBuffer);
			Usart1Send('\n');
			RxLenght = 0;
		}
		//if buffer is full, overwrite that sonOfA...
		else if(RxLenght == RXBUFFERSIZE)
			RxLenght = 0;
	}
}
