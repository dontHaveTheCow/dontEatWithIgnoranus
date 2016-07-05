#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
#include "IndicationGPIOs.h"
#include "USART1.h"
#include "stdbool.h"
#include "ADC.h"

#define BAUD_9600 9600
#define RXBUFFERSIZE 0x20
#define TXBUFFERSIZE 0x20

#define Toggle_green_led() GPIOC->ODR ^= GPIO_Pin_9;
#define Toggle_blue_led()  GPIOC->ODR ^= GPIO_Pin_8;

uint8_t RxBuffer[RXBUFFERSIZE];
uint8_t TxBuffer[TXBUFFERSIZE];
uint8_t RxLenght = 0;
bool stringRecieved;
uint8_t ADC_value;
char value_of_adc[4];

int main(void){

	//Initialize board
	initialiseSysTick();
	initializeDiscoveryLeds();
	//Initialize UART
	Usart1Init(BAUD_9600);
	ConfigureUsart1Interrupt();
	//Initialize ADC
	adcConfig();
	adcInterruptConfig();

	//Indicate startup
	Toggle_green_led();
	delayMs(100);
	Toggle_blue_led();
	delayMs(100);
	Toggle_green_led();
	delayMs(100);
	Toggle_blue_led();
	delayMs(100);

    while(1){
    	Toggle_green_led();
    	Toggle_blue_led();
    	//ADC_value = (ADC_value * 33) / 256;
    	itoa(ADC_value, value_of_adc);
    	Usart1SendString(value_of_adc);
    	Usart1SendString("\n");
    	delayMs(500);
    }
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		//blink a led
		Toggle_green_led();

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

void ADC1_COMP_IRQHandler(void)
{
	if (ADC_GetITStatus(ADC1,ADC_IT_EOC) != RESET  )
	{
	ADC_value = (ADC_GetConversionValue(ADC1) / 4095.)* 3300;
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
	}
}




