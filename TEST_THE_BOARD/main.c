#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
#include "IndicationGPIOs.h"
#include "USART1.h"
#include "stdbool.h"
#include "Button.h"
#include "MyStringFunctions.h"
#include "SPI2.h"
#include "ADXL362.h"
#include "USART2.h"
#include "A2035H.h"

#define BAUD_9600 9600
#define BAUD_4800 4800
#define RXBUFFERSIZE 0x20
#define TXBUFFERSIZE 0x20

uint8_t RxBuffer[RXBUFFERSIZE];
uint8_t TxBuffer[TXBUFFERSIZE];
uint8_t RxLenght = 0;
bool stringRecieved;
int DELAY = 50;

int main(void){

	initializeUserButton();;
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
	Usart1_Init(BAUD_9600);
	Usart2_Init(BAUD_4800);
	ConfigureUsart2Interrupt();

	//initializeA2035H();
	//hibernate_da_A2035H();

	GPIOB->ODR ^= GPIO_Pin_5;
	GPIOB->ODR ^= GPIO_Pin_6;
	GPIOB->ODR ^= GPIO_Pin_7;
	GPIOB->ODR ^= GPIO_Pin_8;
	GPIOB->ODR ^= GPIO_Pin_9;
	delay_400ms();
	GPIOB->ODR ^= GPIO_Pin_5;
	GPIOB->ODR ^= GPIO_Pin_6;
	GPIOB->ODR ^= GPIO_Pin_7;
	GPIOB->ODR ^= GPIO_Pin_8;
	GPIOB->ODR ^= GPIO_Pin_9;

	while(1){
		GPIOB->ODR ^= GPIO_Pin_5;
		delayMs(DELAY);
		GPIOB->ODR ^= GPIO_Pin_5;
		GPIOB->ODR ^= GPIO_Pin_6;
		delayMs(DELAY);
		GPIOB->ODR ^= GPIO_Pin_6;
		GPIOB->ODR ^= GPIO_Pin_7;
		delayMs(DELAY);
		GPIOB->ODR ^= GPIO_Pin_7;
		GPIOB->ODR ^= GPIO_Pin_8;
		delayMs(DELAY);
		GPIOB->ODR ^= GPIO_Pin_8;
		GPIOB->ODR ^= GPIO_Pin_9;
		delayMs(DELAY);
		GPIOB->ODR ^= GPIO_Pin_9;
		GPIOC->ODR ^= GPIO_Pin_6;
		delayMs(DELAY);
		GPIOC->ODR ^= GPIO_Pin_6;
		GPIOC->ODR ^= GPIO_Pin_7;
		delayMs(DELAY);
		GPIOC->ODR ^= GPIO_Pin_7;
		GPIOC->ODR ^= GPIO_Pin_8;
		delayMs(DELAY);
		GPIOC->ODR ^= GPIO_Pin_8;
    }
}

/*void USART1_IRQHandler(void){
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
		//blink a led
    	GPIOB->ODR ^= GPIO_Pin_5;

    	RxBuffer[RxLenght] = USART_ReceiveData(USART1);

    	//Send response when carriage return received
		if(RxBuffer[RxLenght++] == '\r'){
			RxBuffer[RxLenght] = '\0';
			Usart1_SendString("Received ");
			Usart1_SendString((char*)RxBuffer);

			RxLenght = 0;
		}
		//if buffer is full, overwrite that sonOfA...
		else if(RxLenght == RXBUFFERSIZE)
			RxLenght = 0;
	}
}*/

void USART2_IRQHandler(void){
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		//blink a led
    	//GPIOB->ODR ^= GPIO_Pin_5;
		Usart1_Send(USART_ReceiveData(USART2));
	}
}

void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button pin interrupt
		DELAY += 50;
		if(DELAY > 300)
			DELAY  = 50;
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}


