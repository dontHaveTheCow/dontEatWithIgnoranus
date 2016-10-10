//Stm32 includes and C libraries
#include<stm32f0xx.h>
#include <string.h>
#include <stdbool.h>

//Debugging
#include "debug.h"

//My library includes
#include "SPI1.h"
#include "SPI2.h"
#include "SysTickDelay.h"
#include "USART1.h"
#include "USART2.h"
#include "MyStringFunctions.h"
#include "Timer.h"
#include "ADC.h"

//Device libraries
#include "Xbee.h"
#include "Button.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"
#include "A2035H.h"
#include "sdCard.h"

//Dewi defines
#define RSSI_MESSAGE_0 "1 0"
#define RSSI_MESSAGE_1 "1 1"
#define BAUD_9600 9600
#define BAUD_4800 4800

//ModuleStatus defines
#define MODULE_NOT_RUNNING 0x04
#define MODULE_SETUP_STATE 0x05
#define MODULE_APPLYING_PARAMS 0x06
#define MODULE_IDLE_READY 0x0C
#define MODULE_RUNNING 0x08
#define MODULE_TURNING_OFF 0x10

//Xbee globals
static char recievePacket[64];
static bool dataUpdeted = false;
volatile uint8_t length;
bool readingPacket = false;
uint8_t state = 1;
// last 4 lsb -> |ready|idle|applying params|setup|
uint8_t moduleStatus= MODULE_NOT_RUNNING;
uint8_t turnOffTimer = 0;

//Gps globals
volatile bool gpsIsOn = false;
char gpsReceiveString[80];
uint8_t gpsReadIterator;
volatile bool readingNMEA = false;
volatile bool readingGPGGA = false;
volatile bool gpsDataUpdated = false;

//DEWI globals
uint32_t globalCounter = 0;

int main(void){

	//Local variables for xbee
	char transmitString[8];
	uint8_t errorTimer = 20;
	//Locals for ADXL
	int16_t z = 0;
	int16_t z_low = 0;
	int16_t z_high = 0;
	int16_t x = 0;
	int16_t x_low = 0;
	int16_t x_high = 0;
	char messurementString[6];
	//variable for iterations
	int i;
	//variables for gps
	char* gpsPtr;
	char* tmpGpsPtr;
    char ts[11] = " ";
    char lat[11] = " ";
    char latd[2]= " ";
    char lon[11]= " ";
    char lond[2]= " ";
    char fix[2]= " ";
    char sats[3]= " ";
    char *ptrToNMEA[] = {ts, lat, latd, lon, lond, fix, sats};
	uint8_t messageIterator;
	//variables for sd card
	//initialize sdcard (it uses the same SPI as Xbee)
	uint8_t sdBuffer[512];
	uint16_t sdBufferCurrentSymbol = 0;
	uint8_t sector;
	uint32_t mstrDir;
	uint32_t cluster;
	uint32_t filesize;
	uint16_t fatSect, fsInfoSector;
	uint8_t sdStatus = 0x00;
	//Periph variable
	uint16_t ADC_value;
	//Timer string
	char timerString[8];

	//Leds and buttons
	initializeUserButton();
	initializeEveryGreenLed();
	initializeEveryRedLed();
	//SPI attention pin for incoming data alert
	initializeXbeeATTnPin();
	//Usart1 for debugging and serial communication
	Usart1_Init(9600);
	//System clock for delays
	initialiseSysTick();
	//SPI2 for ADXL
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	//SPI1 for xbee
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	//Timer for second counter
	Initialize_timer();
	Timer_interrupt_enable();
	//ADC for voltage control
	adcPinConfig();
	adcConfig();

	//Xbee initialization (check if there arent unread data)
	//... transmitter shouldn't have any received message from others
	XBEE_CS_LOW();
	while(errorTimer--){
		SPI1_TransRecieve(0x00);
	}
	XBEE_CS_HIGH();

	//Dewi Module wont start until the button is pressed
	while(moduleStatus == MODULE_NOT_RUNNING){
    	ADC_value = (ADC_GetConversionValue(ADC1));
    	ADC_value = (ADC_value * 330) / 128;
    	batteryIndicationStartup(ADC_value);
    	blinkGreenLeds(7);
	}


	//12 seconds to choose the modules to use
	//One redStartup take about 500ms, so let it spin for twenty times
	//Default state is 1 (only acc is on)
	turnOnGreenLeds(state);
	for(errorTimer = 0 ; errorTimer < 6 ; errorTimer++){
		redStartup(REAL_REAL_SLOW_DELAY);
	}
	//Setup state is cleared
	moduleStatus = MODULE_APPLYING_PARAMS;

	//ADXL362Z
	if(state&0x01){
		initializeADXL362();
		blinkRedLed3();
		while(!return_ADXL_ready()){
			//wait time for caps to discharge
			delayMs(2000);
			initializeADXL362();
			delayMs(1000);
			blinkRedLed3();
		}
	}

	//GPS
	if((state&0x02) >> 1){
		//turnGpsOn();
		//while(gpsReceiveString[])
		blinkRedLed4();
	}

	//SD
	if((state&0x04) >> 2){

		errorTimer = 10;
		while(!initializeSD() && errorTimer-- > 1){
			delayMs(300);
			blinkRedLed5();
		}
		if(!errorTimer){
			//If sd card doesnt turn on, dont log anything to it
			state &= 0xFB;
		}
		else{
			blinkGreenLed1();

			findDetailsOfFAT(sdBuffer,&fatSect,&mstrDir, &fsInfoSector);

			findDetailsOfFile("LOGFILE",sdBuffer,mstrDir,&filesize,&cluster,&sector);

			findLastClusterOfFile("LOGFILE",sdBuffer, &cluster,fatSect,mstrDir);

			if(filesize < 512)
				filesize = 512;

			appendTextToTheSD("\nNEW LOG", '\n', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
		}
	}

	moduleStatus = MODULE_IDLE_READY;
	turnOnGreenLeds(state);
	while(moduleStatus == MODULE_IDLE_READY){
		blinkGreenLeds(state);
		redStartup(DELAY);
	}

    while(1){

    	switch(moduleStatus){

    	case MODULE_RUNNING:
    		delayMs(600);
    		//Transmit acc data
    		if(state&0x01){
    		  	getX(&x,&x_low,&x_high);
    		    itoa(x, messurementString);
    		    transmitString[0] = '0';
    		    transmitString[1] = ' ';
    		    strcpy(&transmitString[2],&messurementString[0]);
    			transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
    			//if sd card is active, log data to it
    			if((state&0x04) >> 2){
    				itoa(globalCounter,timerString);
    				appendTextToTheSD(timerString, '\t', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
    				appendTextToTheSD(transmitString, '\n', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
    				xorGreenLed(2);
    			}
    			xorGreenLed(0);
    		}
    		delayMs(600);
    		//Transmit gps data
    		if((state&0x02) >> 1){
    			strcpy(transmitString, "1 1234");
    			transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
    			xorGreenLed(1);
    		}
    		break;

    	case MODULE_IDLE_READY:
    		if(turnOffTimer > 0){
    			turnOffTimer--;
        		blinkGreenLeds(state);
        		redStartup(DELAY);
    		}
    		else{
        		blinkGreenLeds(state);
            	ADC_value = (ADC_GetConversionValue(ADC1));
            	ADC_value = (ADC_value * 330) / 128;
            	batteryIndicationStartup(ADC_value);
    		}
    		break;

    	case MODULE_TURNING_OFF:
    		if((state&0x04) >> 2){
    			goToIdleState();
    		}
    		if((state&0x02) >> 1){
    			hibernateGps();
    		}
    		state = 0;
    		turnOffTimer = 0;
    		moduleStatus = MODULE_IDLE_READY;
    		break;
    	}
   	}
}

/*void USART2_IRQHandler(void){
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		if(readingGPGGA == true){
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
		}
	}
}*/

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button2 pin interrupt

		if(moduleStatus == MODULE_NOT_RUNNING){
			moduleStatus = MODULE_SETUP_STATE;
		}
		else if(moduleStatus == MODULE_SETUP_STATE){
			if(state++ > 7){
				state = 0;
			}
			turnOnGreenLeds(state);
		}
		else if(moduleStatus == MODULE_IDLE_READY){
			if(turnOffTimer > 0)
				moduleStatus = MODULE_TURNING_OFF;
			else
				moduleStatus = MODULE_RUNNING;
		}
		else if(moduleStatus == MODULE_RUNNING){
			turnOffTimer = 10;
			moduleStatus = MODULE_IDLE_READY;
		}
		EXTI_ClearITPendingBit(EXTI_Line8);
	}

	if(EXTI_GetITStatus(EXTI_Line4) == SET){	//Handler for Radio ATTn pin interrupt

		if(!readingPacket){

			uint8_t numberOfDumpBytes = 0;
			uint8_t cheksum = 0;
			GPIO_ResetBits(GPIOA,GPIO_Pin_4);

			while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
				numberOfDumpBytes++;
				if(numberOfDumpBytes >5)			//Exit loop if there is no start delimiter
					break;
			}
			readingPacket = true;
			SPI1_TransRecieve(0x00);
			length = SPI1_TransRecieve(0x00);
			uint8_t i = 0;

			for(; i < length; i ++ ){				//Read data based on packet length
				recievePacket[i] = SPI1_TransRecieve(0x00);
				cheksum += recievePacket[i];
			}
			cheksum += SPI1_TransRecieve(0x00);
			if(cheksum == 0xFF)
				dataUpdeted = true;					//Data is updated if checksum is true

			readingPacket = false;
			GPIO_SetBits(GPIOA,GPIO_Pin_4);
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		globalCounter++;
	}
}








