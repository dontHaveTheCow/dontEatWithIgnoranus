#include "A2035H.h"

void turnGpsOn(void){
		GPIO_ResetBits(GPIOC, RESET_PIN);
		delayMs(200);
		GPIO_SetBits(GPIOC, RESET_PIN);
		//Set on pin
		delay_1s();
	    GPIO_SetBits(GPIOF, ON_PIN);
	    delayMs(200);
		GPIO_ResetBits(GPIOF, ON_PIN);
}
void hibernateGps(void){

	GPIO_ResetBits(GPIOC, RESET_PIN);
	delayMs(200);
	GPIO_SetBits(GPIOC, RESET_PIN);
}
void setupGpsGpio(void){

	//Clock for GPS
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF,  ENABLE);
	//Initialize GPS ON PIN
	GPIO_InitStructure.GPIO_Pin = ON_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	//Initialize GPS RESET PIN
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,  ENABLE);
	GPIO_InitStructure.GPIO_Pin = RESET_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOC, RESET_PIN);

	//Initialize input for WAKEUP_PIN
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,  ENABLE);
	GPIO_InitStructure.GPIO_Pin = WAKEUP_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

//RCC_APB1Periph_TIM3 is used for speaker pwm - dont use that!!!
//okkk i will use RCC_APB1Periph_TIM2
void setupGpsTimer(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef Timer_init_structure;
	Timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;
	Timer_init_structure.TIM_ClockDivision = 0;
	Timer_init_structure.TIM_Prescaler = 800;
	//Period equals 900ms
	Timer_init_structure.TIM_Period = 8500;
	Timer_init_structure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &Timer_init_structure);
}

void setupGpsTimerInterrupt(void){
	NVIC_InitTypeDef NVIC_structure;
	NVIC_structure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_structure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_structure.NVIC_IRQChannelPriority = 0x01;
	NVIC_Init(&NVIC_structure);
	//TIM_Cmd(TIM2,ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void gps_dissableMessage(uint8_t $MSG){

	/*
	 * Cheksum is an XOR of all the bytes between the $ and the *
	 * For $PSRF103,00,00,00,01* is 0x24
	 */
	char checksum_str[3];
	uint8_t checksum = $MSG^0x24;
	sprintf(checksum_str,"%x",checksum);

	Usart2_SendString("$PSRF103,0");
	Usart2_Send($MSG + ASCII_DIGIT_OFFSET);
	Usart2_SendString(",00,00,01*");
	Usart2_Send(checksum_str[0]);
	Usart2_Send(checksum_str[1]);
	Usart2_Send('\r');
	Usart2_Send('\n');

}
void gps_setRate(uint8_t $MSG, uint8_t rate){

	/*
	 * Note that the 5Hz mode works only in navigation mode
	 *  once a GPS fix has been obtained. Until it gets a fix,
	 *   the receiver will work in 1Hz mode.
	 */

	char checksum_str[3];
	uint8_t checksum = $MSG^0x24^rate;
	sprintf(checksum_str,"%x",checksum);

	Usart2_SendString("$PSRF103,0");
	Usart2_Send($MSG + ASCII_DIGIT_OFFSET);
	Usart2_SendString(",00,0");
	Usart2_Send(rate + ASCII_DIGIT_OFFSET);
	Usart2_SendString(",01*");
	Usart2_Send(checksum_str[0]);
	Usart2_Send(checksum_str[1]);
	Usart2_Send('\r');
	Usart2_Send('\n');
}

void gps_enable5hz(void){
	Usart2_SendString("$PSRF103,00,6,00,0*23\r\n");
}

void gps_disable5hz(void){
	Usart2_SendString("$PSRF103,00,7,00,0*22\r\n");
}

void gps_parseGPGGA(char* gpsString, char* ts, char* lat, char* lon, char* fix, char* sats){

	uint8_t stringIterator = 7;
	uint8_t messageIterator = 0;
	char delimiter = ',';

	while(*(gpsString+stringIterator) != delimiter)
		*ts++ = *(gpsString+stringIterator++);
    stringIterator++;
    while(*(gpsString+stringIterator) != delimiter)
        *lat++ = *(gpsString+stringIterator++);
    stringIterator++;
    while(*(gpsString+stringIterator++) != delimiter);
    while(*(gpsString+stringIterator) != delimiter)
        *lon++ = *(gpsString+stringIterator++);
    stringIterator++;
    while(*(gpsString+stringIterator++) != delimiter);
    messageIterator = 0;
    while(*(gpsString+stringIterator) != delimiter)
        fix[messageIterator++] = *(gpsString+stringIterator++);
    stringIterator++;
    while(*(gpsString+stringIterator) != delimiter)
        *sats++ = *(gpsString+stringIterator++);
}

void gps_parseGPVTG(char* gpsString, char* speed){
    //$GPVTG,,T,,M,,N,,K,N*2C
    //$GPVTG,189.45,T,,M,0.00,N,0.0,K,A*0C
    int commaCounter = 0;

    while(commaCounter !=7){
        if(*gpsString++ == ',')
            commaCounter++;
    }
    while(*gpsString != ','){
       *speed++ = *gpsString++;
    }

}

