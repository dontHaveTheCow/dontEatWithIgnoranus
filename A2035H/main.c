#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
#include "IndicationGPIOs.h"
#include "USART1.h"
#include "USART2.h"
#include "A2035H.h"
#include "stdbool.h"
#include <stm32f0xx_exti.h>
#include "Button.h"

#define BAUD_9600 9600
#define BAUD_4800 4800

volatile bool gpsIsOn = false;
char gpsReceiveString[80];
uint8_t gpsReadIterator;
volatile bool readingNMEA = false;
volatile bool readingGPGGA = false;
volatile bool gpsDataUpdated = false;

int main(void)
{
	char* ptr;
	char* tmpPtr;
    char ts[11] = " ";
    char lat[11] = " ";
    char latd[2]= " ";
    char lon[11]= " ";
    char lond[2]= " ";
    char fix[2]= " ";
    char sats[3]= " ";
    char *ptrToNMEA[] = {ts, lat, latd, lon, lond, fix, sats};
	uint8_t messageIterator;


	//Clock for GPS
	setupGpsGpio();
	initialiseSysTick();
	initializeRedLed1();
	initializeGreenLed1();
	initializeUserButton();
	//initializeDiscoveryLeds();
	Usart2_Init(BAUD_4800);
	Usart1_Init(BAUD_9600);
	ConfigureUsart2Interrupt();
	setupGpsTimer();
	setupGpsTimerInterrupt();

	turnGpsOn();
	gpsIsOn = true;


    while(1){
    	if(gpsDataUpdated){
    		gpsDataUpdated = false;
    		messageIterator = 0;
    		ptr = &gpsReceiveString[6];
    	    for(; messageIterator < 7; messageIterator ++){
    	    	tmpPtr = ptrToNMEA[messageIterator];
    	        while(*ptr++ != ','){
    	            *ptrToNMEA[messageIterator]++ = *(ptr-1);
    	        }
    	        ptrToNMEA[messageIterator] = tmpPtr;
    	    }
    	    if(fix[0] == '0'){
        		Usart1_SendString(ts);
        		Usart1_SendString(" No GPS fix\r\n");
    	    }
    	    else{
        		Usart1_SendString(gpsReceiveString);
        		Usart1_Send('\r');
        		Usart1_Send('\n');
    	    }
    	    TIM_Cmd(TIM2,ENABLE);
    	    GPIOC->ODR ^= GPIO_Pin_6;
    	}
    	if(!gpsIsOn)
    		hibernateGps();
    }
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){

		GPIOB->ODR ^= GPIO_Pin_5;
		if(gpsIsOn){
			USART_Cmd(USART2, DISABLE);
			gpsIsOn = false;
		}
		else{
			USART_Cmd(USART2, ENABLE);
			gpsIsOn = true;
		}


		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}

void USART2_IRQHandler(void){
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		if(readingGPGGA == true){
			gpsReceiveString[gpsReadIterator++] = USART_ReceiveData(USART2);
			if(gpsReceiveString[gpsReadIterator-1] == 0x0D){
				gpsDataUpdated = true;
				gpsReceiveString[gpsReadIterator-1] = '\0';
				readingNMEA = false;
				readingGPGGA = false;
				USART_Cmd(USART2, DISABLE);
			}
		}
		else if(readingNMEA == true){
			gpsReceiveString[gpsReadIterator++] = USART_ReceiveData(USART2);
			if(gpsReadIterator == 4 ){
				if(gpsReceiveString[3] == 'G'){
					readingGPGGA = true;
				}
				else{
					readingNMEA = false;
				}
			}
		}
		else if(USART_ReceiveData(USART2) == '$'){
			gpsReadIterator = 0;
			readingNMEA = true;
		}

	}
}

void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
      	GPIOC->ODR ^= GPIO_Pin_6;
      	TIM_Cmd(TIM2,DISABLE);
      	USART_Cmd(USART2, ENABLE);
	}


}
