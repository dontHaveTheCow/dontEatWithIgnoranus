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
#include "sdCard.h"

#define BAUD_9600 9600
#define BAUD_4800 4800

volatile bool gpsIsOn = false;
volatile bool gpsTurnOff = false;
volatile bool gpsTurnOn = false;
char gpsReceiveString[96];
uint8_t gpsReadIterator = 0;
volatile bool gpsDataUpdated = false;

uint8_t errorTimer;

int main(void)
{
	char* ptr;
	char* tmpPtr;
    char ts[11] = " ";
    char lat[11] = " ";
    char latd[2]= " ";
    char lon[11]= " ";
    char lond[2]= " ";
    char fix[2]= "0";
    char sats[3]= " ";
    char velocity[6] = " ";
    char *ptrToNMEA[] = {ts, lat, latd, lon, lond, fix, sats};
	uint8_t messageIterator;

	//Clock for GPS
	setupGpsGpio();
	initialiseSysTick();
	initializeRedLed1();
	initializeRedLed2();
	initializeRedLed4();
	initializeGreenLed1();
	initializeUserButton();
	//initializeDiscoveryLeds();
	Usart2_Init(BAUD_4800);
	Usart1_Init(BAUD_9600);
	ConfigureUsart2Interrupt();
	//spi used for sd card

	delayMs(1000);
	turnGpsOn();

	while(!GPIO_ReadInputDataBit(GPS_PORTC,WAKEUP_PIN)){
		blinkRedLed2();
		delayMs(1000);
	}

	delayMs(400);
	gps_dissableMessage($GPGSA);
	delayMs(400);
	gps_dissableMessage($GPGSV);
	delayMs(400);
	gps_dissableMessage($GPRMC);
	delayMs(400);
	gps_dissableMessage($GPVTG);
	delayMs(400);
	gps_setRate($GPGGA, 1);

	gpsIsOn = true;

    while(1){
    	if(gpsDataUpdated){
    		gpsDataUpdated = false;
    		messageIterator = 0;
    		if(strncmp(gpsReceiveString,"$GPGGA", 6) == 0){
        		ptr = &gpsReceiveString[7]; //This value could change whether the $ is used or not
        	    for(; messageIterator < 7; messageIterator ++){
        	    	tmpPtr = ptrToNMEA[messageIterator];
        	        while(*ptr++ != ','){
        	            *ptrToNMEA[messageIterator]++ = *(ptr-1);
        	        }
        	        ptrToNMEA[messageIterator] = tmpPtr;
        	    }
        	    if(fix[0] == '0'){
        	    	Usart1_Send('\n');
            		Usart1_SendString(ts);
            		Usart1_SendString(" No GPS fix\r\n");
        	    }
        	    else{
        	    	Usart1_Send('\n');
            		Usart1_SendString(ts);
            		Usart1_SendString(" GPS Found\r\n");

            		gps_dissableMessage($GPGGA);
            		delayMs(400);
            		gps_setRate($GPVTG,1);
        	    }
    		}
    		else if(strncmp(gpsReceiveString,"$GPVTG" , 6) == 0){
    			gps_parseGPVTG(gpsReceiveString,velocity);
    			Usart1_SendString(" Velocity: ");
    			Usart1_SendString(velocity);
    		}
    		else{
    			Usart1_SendString(gpsReceiveString);
    		}
    	    GPIOC->ODR ^= GPIO_Pin_6;
    	}
    	if(gpsTurnOff == true){
    		hibernateGps();
    		gpsTurnOff = false;
    	}
    	else if(gpsTurnOn == true){
    		turnGpsOn();
    		gpsTurnOn = false;
    	}

    }
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){

		GPIOB->ODR ^= GPIO_Pin_5;
		if(gpsIsOn){
			gpsIsOn = false;
			gpsTurnOff = true;
		}
		else{
			gpsIsOn = true;
			gpsTurnOn = true;
		}

		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}

void USART2_IRQHandler(void){
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){

/*		Usart1_Send(USART_ReceiveData(USART2));
		GPIOC->ODR ^= GPIO_Pin_6;*/

		if((gpsReceiveString[gpsReadIterator++] = USART_ReceiveData(USART2)) == '\n'){
			gpsDataUpdated = true;
			gpsReadIterator = 0;
		}

/*		if(readingGPGGA == true){
			gpsReceiveString[gpsReadIterator++] = USART_ReceiveData(USART2);
			if(gpsReceiveString[gpsReadIterator-1] == 0x0D){
				gpsDataUpdated = true;
				gpsReceiveString[gpsReadIterator-1] = '\0';
				readingNMEA = false;
				readingGPGGA = false;
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
		}*/
	}
}

