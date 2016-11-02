/*
 * STM32 and C libraries
 */
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_exti.h>

#include "stdbool.h"
/*
 * DEW| libraries
 */
#include "SPI1.h"
#include "SPI2.h"
#include "SysTickDelay.h"
#include "USART1.h"
#include "USART2.h"
#include "MyStringFunctions.h"
#include "Timer.h"
#include "ADC.h"

#include "Xbee.h"
#include "Button.h"
#include "IndicationGPIOs.h"
#include "ADXL362.h"
#include "A2035H.h"
#include "sdCard.h"
/*
 * Module state defines
 */
#define MODULE_NOT_RUNNING 0x04
#define MODULE_SETUP_STATE 0x05
#define MODULE_APPLYING_PARAMS 0x06
#define MODULE_IDLE_READY 0x0C
#define MODULE_RUNNING 0x08
#define MODULE_TURNING_OFF 0x10
/*
 * Any other defines
 */
#define RSSI_MESSAGE_0 "1 0"
#define RSSI_MESSAGE_1 "1 1"
#define GPS_MSG_INIT_DELAY 400
/*
 * XBEE globals
 */
static char recievePacket[64];
static bool dataUpdeted = false;
volatile uint8_t length;
bool readingPacket = false;
/*
 * Module globals
 */
volatile uint8_t state = 1;
volatile uint8_t moduleStatus = MODULE_NOT_RUNNING;
uint8_t turnOffTimer = 0;
uint32_t globalCounter = 0;
/*
 * GPS globals
 */
char gpsReceiveString[96];
uint8_t gpsReadIterator;
volatile bool gpsDataUpdated = false;

int main(void){
	/*
	 * Local variables for XBEE
	 */
	char transmitString[16];
	/*
	 * Local variables for Accelerometer
	 */
	int16_t z = 0;
	int16_t z_low = 0;
	int16_t z_high = 0;
	int16_t x = 0;
	int16_t x_low = 0;
	int16_t x_high = 0;
	char messurementString[6];
	/*
	 * Local variables for GPS
	 */
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
	/*
	 * Local variables for SD card
	 */
	uint8_t sdBuffer[512];
	uint16_t sdBufferCurrentSymbol = 0;
	uint8_t sector = 0;
	uint32_t mstrDir = 0;
	uint32_t cluster = 0;
	uint32_t filesize = 0;
	uint16_t fatSect, fsInfoSector;
	/*
	 * ADC, Timer and other loco's
	 */
	uint16_t ADC_value;
	uint8_t errorTimer = 20;
	/*
	 * Initializing gpio's
	 */
	initializeUserButton();
	initializeEveryRedLed();
	initializeEveryGreenLed();
	setupGpsGpio();
	adcPinConfig();
	initializeXbeeATTnPin();
	/*
	 * Initializing peripherals
	 */
	Usart2_Init(BAUD_4800);
	Usart1_Init(BAUD_9600);
	ConfigureUsart2Interrupt();
	//used for delayMs()
	//not meant for using in interrupt routines
	initialiseSysTick();
	//SPI1 for XBEE and SD card
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	//SPI2 used for accelerometer
	InitialiseSPI2_GPIO();
	InitialiseSPI2();
	//ADC is used for battery monitoring
	adcConfig();
	//Timer counter with i=100ms
	Initialize_timer();
	Timer_interrupt_enable();
	/*
	 * Initializing XBEE
	 */
	XBEE_CS_LOW();
	while(errorTimer--){
		SPI1_TransRecieve(0x00);
	}
	XBEE_CS_HIGH();
	/*
	 * Start module only when button is pressed
	 * Meanwhile, check the battery voltage
	 */
	while(moduleStatus == MODULE_NOT_RUNNING){
    	ADC_value = (ADC_GetConversionValue(ADC1));
    	ADC_value = (ADC_value * 330) / 128;
    	batteryIndicationStartup(ADC_value);
    	blinkGreenLeds(7);
	}
	/*
	 * LED blinking that indicates need to...
	 * choose the method for integrity detection
	 */
	turnOnGreenLeds(state);
	for(errorTimer = 0 ; errorTimer < 6 ; errorTimer++){
	redStartup(REAL_REAL_SLOW_DELAY);
	}
	moduleStatus = MODULE_APPLYING_PARAMS;
	/*
	 * Initialize Accelerometer if chosen
	 */
	if(state&0x01){
		//SPI2 for ADXL
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
	/*
	 * Initialize GPS if chosen
	 */
	//GPS module has 40 seconds to find enough satellites
	errorTimer = 40;
	if((state&0x02) >> 1){
		turnGpsOn();

		while(!GPIO_ReadInputDataBit(GPS_PORTC,WAKEUP_PIN)){
			blinkRedLed2();
			delayMs(1000);
		}

		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPGSA);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPGSV);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPRMC);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_dissableMessage($GPVTG);
		delayMs(GPS_MSG_INIT_DELAY);
		gps_setRate($GPGGA, 1);

		while(fix[0] == '0' && errorTimer > 0 ){
			if(gpsDataUpdated){
				errorTimer--;
				gpsDataUpdated = false;
				messageIterator = 0;
				 // Make sure that you are comparing GPGGA message
				 // $PSRF and $GPVTG messages are possable at the startup
				if(strncmp(gpsReceiveString,"$GPGGA", 6) == 0){
					ptr = &gpsReceiveString[7]; //This value could change whether the $ is used or not
					for(; messageIterator < 7; messageIterator ++){
						tmpPtr = ptrToNMEA[messageIterator];
						while(*ptr++ != ','){
							*ptrToNMEA[messageIterator]++ = *(ptr-1);
						}
						ptrToNMEA[messageIterator] = tmpPtr;
					}
				}
				SEND_SERIAL_MSG("Waiting for sats...\r\n");
				blinkRedLed4();
			}
		}
		//if not enough satellites are found, turn off gps
		if(!errorTimer){
			SEND_SERIAL_MSG("GPS timeout...\r\n");
			hibernateGps();
			state &= 0xFD;
		}
		else{
			//if satellites found -> turn on $GPVTG to monitor velocity
			delayMs(400);
			gps_setRate($GPVTG,1);
			delayMs(400);
			blinkRedLed4();
			gps_dissableMessage($GPGGA);
			SEND_SERIAL_MSG("SATs found...\r\n");
		}
	}
	/*
	 * Initialize SD if chosen
	 */
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

			findDetailsOfFAT(sdBuffer,&fatSect,&mstrDir, &fsInfoSector);
			findDetailsOfFile("LOGFILE",sdBuffer,mstrDir,&filesize,&cluster,&sector);
			/*
			 * Program halts during this function
			 */
			findLastClusterOfFile("LOGFILE",sdBuffer, &cluster,fatSect,mstrDir);

			if(filesize < 512)
				filesize = 512;

			appendTextToTheSD("\nNEW LOG", '\n', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
		}
	}
	/*
	 * Send message to coordinator for time synchronization
	 */

	/*
	 * Wait for input to start communication
	 */
	moduleStatus = MODULE_IDLE_READY;
	turnOnGreenLeds(state);
	while(moduleStatus == MODULE_IDLE_READY){
		blinkGreenLeds(state);
		redStartup(DELAY);
	}

    while(1){

    	/*
    	 * Check in what state module is running
    	 */
    	switch(moduleStatus){

    	case MODULE_RUNNING:
    		delayMs(600);
        	/*
        	 * Send Accelerometer data if chosen
        	 */
    		if(state&0x01){
    		  	getX(&x,&x_low,&x_high);
    		    itoa(x, messurementString);
    		    transmitString[0] = 'M';
    		    transmitString[1] = ' ';
    		    transmitString[2] = '0';
    		    transmitString[3] = ' ';
    		    strcpy(&transmitString[4],&messurementString[0]);
    			transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
    			/*
    			 * Log data to SD card if chosen
    			 */
/*    			if((state&0x04) >> 2){
    				itoa(globalCounter,timerString);
    				appendTextToTheSD(timerString, '\t', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
    				appendTextToTheSD(transmitString, '\t', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
    				xorGreenLed(2);
    			}*/
    			SEND_SERIAL_MSG(transmitString);
    			SEND_SERIAL_MSG(" ACC\r\n");
    			xorGreenLed(0);
    		}
    		delayMs(600);
        	/*
        	 * Send GPS data if chosen
        	 */
    		if((state&0x02) >> 1){
    			if(strncmp(gpsReceiveString,"$GPVTG" , 6) == 0){
    				gps_parseGPVTG(gpsReceiveString,velocity);
    			}
    			else{
    				strcpy(velocity,"999.9");
    			}
    		    transmitString[0] = 'M';
    		    transmitString[1] = ' ';
    		    transmitString[2] = '1';
    		    transmitString[3] = ' ';
        		strcpy(&transmitString[4],velocity);
    			transmitRequest(0x0013A200, 0x40E3E13C, TRANSOPT_DISACK, transmitString);
    			/*
    			 * Log data to SD card if chosen
    			 */
/*    			if((state&0x04) >> 2){
    				appendTextToTheSD(transmitString, '\n', &sdBufferCurrentSymbol, sdBuffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
    				xorGreenLed(2);
    			}*/
    			SEND_SERIAL_MSG(transmitString);
    			SEND_SERIAL_MSG(" GPS\r\n");
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

/*
 * Interrupt routines
 */
void USART2_IRQHandler(void){
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		if((gpsReceiveString[gpsReadIterator++] = USART_ReceiveData(USART2)) == '\n'){
			gpsDataUpdated = true;
			gpsReadIterator = 0;
		}
	}
}

void EXTI4_15_IRQHandler(void)					//External interrupt handlers
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET){	//Handler for Button2 pin interrupt

		if(moduleStatus == MODULE_NOT_RUNNING){
			moduleStatus = MODULE_SETUP_STATE;
		}
		else if(moduleStatus == MODULE_SETUP_STATE){
			if(++state > 7){
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


