#include "XBee.h"

void initializeXbeeATTnPin(void){
	//Interrupt pin - PC4
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

		GPIO_InitTypeDef GPIO_structure;
		GPIO_structure.GPIO_Pin=GPIO_Pin_4;
		GPIO_structure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOC, &GPIO_structure);

		RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, ENABLE);
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource4);

		EXTI_InitTypeDef   EXTI_InitStructure;
		EXTI_InitStructure.EXTI_Line= EXTI_Line4;
		EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd=ENABLE;
		EXTI_Init(&EXTI_InitStructure);

		NVIC_InitTypeDef   NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPriority = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}

void apply1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1){

	uint8_t cheksum;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x05);
	cheksum += 0x05;
	//AT command
	SPI1_TransRecieve(0x08);
	cheksum += 0x08;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;

	 //Parameters goes here
	SPI1_TransRecieve(param1);
	cheksum += param1;

	SPI1_TransRecieve(0xFF - cheksum); //Checksum

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    blinkIndicationLedOnce();
}

void apply2Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2){

	uint8_t cheksum;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x06);
	//AT command
	SPI1_TransRecieve(0x08);
	cheksum += 0x08;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;

	 //Parameters goes here
	SPI1_TransRecieve(param1);
	cheksum += param1;
	SPI1_TransRecieve(param2);
	cheksum += param1;

	SPI1_TransRecieve(0xFF - cheksum); //Checksum

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    blinkIndicationLedOnce();

}
void apply3Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3){

	uint8_t cheksum;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x07);
	//AT command
	SPI1_TransRecieve(0x08);
	cheksum += 0x08;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;

	 //Parameters goes here
	SPI1_TransRecieve(param1);
	cheksum += param1;
	SPI1_TransRecieve(param2);
	cheksum += param2;
	SPI1_TransRecieve(param3);
	cheksum += param3;

	SPI1_TransRecieve(0xFF - cheksum); //Checksum

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    blinkIndicationLedOnce();

}
void apply4Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4){

	uint8_t cheksum;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x08);
	//AT command
	SPI1_TransRecieve(0x08);
	cheksum += 0x08;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;

	 //Parameters goes here
	SPI1_TransRecieve(param1);
	cheksum += param1;
	SPI1_TransRecieve(param2);
	cheksum += param2;
	SPI1_TransRecieve(param3);
	cheksum += param3;
	SPI1_TransRecieve(param4);
	cheksum += param4;

	SPI1_TransRecieve(0xFF - cheksum); //Checksum

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    blinkIndicationLedOnce();
}

void queue1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1){
	uint8_t cheksum = 0;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x05);
	SPI1_TransRecieve(0x09);
	cheksum += 0x09;
	SPI1_TransRecieve(0x52);
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //BD - Baudrate
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;
	SPI1_TransRecieve(param1); // ATBD7 = 115200
	cheksum += param1;
	SPI1_TransRecieve(0xFF - cheksum);
	delayMs(10);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
   	delay_1s();
    blinkIndicationLedOnce();
}

void queue2Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2){

	uint8_t cheksum;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x06);
	//AT command
	SPI1_TransRecieve(0x09);
	cheksum += 0x09;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;

	 //Parameters goes here
	SPI1_TransRecieve(param1);
	cheksum += param1;
	SPI1_TransRecieve(param2);
	cheksum += param1;

	SPI1_TransRecieve(0xFF - cheksum); //Checksum

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    blinkIndicationLedOnce();

}
void queue3Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3){

	uint8_t cheksum;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x07);
	//AT command
	SPI1_TransRecieve(0x09);
	cheksum += 0x09;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;

	 //Parameters goes here
	SPI1_TransRecieve(param1);
	cheksum += param1;
	SPI1_TransRecieve(param2);
	cheksum += param2;
	SPI1_TransRecieve(param3);
	cheksum += param3;

	SPI1_TransRecieve(0xFF - cheksum); //Checksum

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    blinkIndicationLedOnce();

}
void queue4Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4){

	uint8_t cheksum;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x08);
	//AT command
	SPI1_TransRecieve(0x09);
	cheksum += 0x09;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;

	 //Parameters goes here
	SPI1_TransRecieve(param1);
	cheksum += param1;
	SPI1_TransRecieve(param2);
	cheksum += param2;
	SPI1_TransRecieve(param3);
	cheksum += param3;
	SPI1_TransRecieve(param4);
	cheksum += param4;

	SPI1_TransRecieve(0xFF - cheksum); //Checksum

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
    blinkIndicationLedOnce();
}

void readModuleParams(uint8_t MSbyte, uint8_t LSbyte){
	int8_t cheksum = 0;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x04);
	//AT command
	SPI1_TransRecieve(0x08);
	cheksum += 0x08;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	//NH - Network Hops
	SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;
	SPI1_TransRecieve(0xFF - cheksum); //Chekcsum

	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x00);

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void restoreDefaults(){
	uint8_t cheksum = 0;

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x04);
	SPI1_TransRecieve(0x08); 	//AT command
	cheksum += 0x08;
	SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	SPI1_TransRecieve('R'); // Restore Defaults
	cheksum += 'R';
	SPI1_TransRecieve('E');
	cheksum += 'E';
	/*
	 * Parameters goes here
	 */
	SPI1_TransRecieve(0xFF - cheksum); //Cheksum

	//Response from slave
	SPI1_TransRecieve(0x00);

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
   	delay_1s();
    blinkIndicationLedOnce();
}

void initializeXbeeWithUart(void){
	delay_1s();
	//Send the +++ (Entering command mode)
	Usart1SendString("+++");
	waitForOkResponse();
	//Send the ATRE<CR>(Restore defaults)
	Usart1SendString("ATRE\r");
	waitForOkResponse();
	//Setting RO if not using (GT + CC + GT)
	//Send the ATDL ... <CR>(Destination Address Low.)
	Usart1SendString("ATDL FFFF \r");
	waitForOkResponse();
	//Send the ATDH ... <CR>(Destination Address High.)
	Usart1SendString("ATDH 0\r");
	waitForOkResponse();
	//Send the ATSL ... <CR>(Serial Number Low.)
	Usart1SendString("ATSL AAAA\r");
	waitForOkResponse();
	//Send the ATSH ... <CR>(Serial Number High.)
	Usart1SendString("ATSH 0\r");
	waitForOkResponse();
	//Send the ATAC<CR> Apply Changes) command
	Usart1SendString("ATAC\r");
	waitForOkResponse();
	//Send the ATCN<CR>(Exit Command Mode) command
	Usart1SendString("ATCN\r");
	waitForOkResponse();
}

void initializeXbeeAPI(void){
	delayMs(10);
	restoreDefaults();
	delayMs(10);
	//applyModuleParams();
}

void transmitRequest(uint8_t adr1, uint8_t adr2, uint8_t adr3, uint8_t adr4, uint8_t adr5, uint8_t adr6, uint8_t adr7, uint8_t adr8, char* data){
	int8_t cheksum = 0;
	uint8_t  lenghtOfData, i = 0;
	lenghtOfData = strlen(data);

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);	//Lenght
	SPI1_TransRecieve(14 + lenghtOfData);
	SPI1_TransRecieve(0x10);	//FrameType
	cheksum += 0x10;
	SPI1_TransRecieve(0x01);	//Frame ID
	cheksum += 0x01;
	SPI1_TransRecieve(adr1);	//64bit adress
	cheksum += adr1;
	SPI1_TransRecieve(adr2);
	cheksum += adr2;
	SPI1_TransRecieve(adr3);
	cheksum += adr3;
	SPI1_TransRecieve(adr4);
	cheksum += adr4;
	SPI1_TransRecieve(adr5);
	cheksum += adr5;
	SPI1_TransRecieve(adr6);
	cheksum += adr6;
	SPI1_TransRecieve(adr7);
	cheksum += adr7;
	SPI1_TransRecieve(adr8);
	cheksum += adr8;
	SPI1_TransRecieve(0xFF);	//Reserved
	cheksum += 0xFF;
	SPI1_TransRecieve(0xFE);
	cheksum += 0xFE;
	SPI1_TransRecieve(0x00);	//Broadcast radius
	cheksum += 0x00;
	SPI1_TransRecieve(0x00);	//Transmit options (Disable ack)
	cheksum += 0x00;
	for(i; i < lenghtOfData; i++){	//RF data
		SPI1_TransRecieve(data[i]);
		cheksum += data[i];
	}

	SPI1_TransRecieve(0xFF - cheksum); //Cheksum

	//Response from slave
	//SPI1_TransRecieve(0x00);

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
   	delay_1s();
    blinkIndicationLedOnce();
}

