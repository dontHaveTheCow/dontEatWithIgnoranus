#include "XBee.h"

void initializeXbeeATTnPin(void){
	//Interrupt pin - PC4
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

		GPIO_InitTypeDef GPIO_structure;
		GPIO_structure.GPIO_Pin=GPIO_Pin_4;
		GPIO_structure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_structure.GPIO_PuPd = GPIO_PuPd_UP;
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
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0F;
		NVIC_Init(&NVIC_InitStructure);
}

void apply1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1){

	int8_t cheksum = 0;

	XBEE_CS_LOW();
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

	XBEE_CS_HIGH();
    blinkGreenLed3();
}

void queue1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1){

	int8_t cheksum = 0;

	XBEE_CS_LOW();
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
	XBEE_CS_HIGH();
   	delay_1s();
    blinkGreenLed3();
}


uint32_t  readModuleParams(uint8_t MSbyte, uint8_t LSbyte){

	uint32_t data = 0;
	uint8_t cheksum = 0;
	uint8_t length, i = 0;
	int timer = 0;

	while(readingPacket); //Check if rf packets aren't incoming at the same time

	readingPacket = true;
	XBEE_CS_LOW();
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x04); //Lenght
	SPI1_TransRecieve(0x08); //AT command
	cheksum += 0x08;
	SPI1_TransRecieve(0x52); //Response
	cheksum += 0x52;
	SPI1_TransRecieve(MSbyte);
	cheksum += MSbyte;
	SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;
	SPI1_TransRecieve(0xFF - cheksum);
	cheksum = 0;
	//Wait for xbee to make AT command response
	while(SPI1_TransRecieve(0x00) != 0x7E){
		//Iterate timer to prevent function of going into endless loop
		timer++;
		if(timer > 100)
			return 0xFF;
	}
	SPI1_TransRecieve(0x00);
	length = SPI1_TransRecieve(0x00);
	cheksum +=SPI1_TransRecieve(0x00);	//Type of packet
	cheksum +=SPI1_TransRecieve(0x00);	//Response
	cheksum +=SPI1_TransRecieve(0x00);	//AT
	cheksum +=SPI1_TransRecieve(0x00);	//Command
	cheksum +=SPI1_TransRecieve(0x00);	//Command status
	for(i=5; i < length; i ++ ){
		//Command response parameter wont be longer than 4 bytes
		data |= SPI1_TransRecieve(0x00) << (8*(8-i));
		cheksum += (data & (0xFF << 8*(8-i))) >> (8*(8-i));
	}
	cheksum += SPI1_TransRecieve(0x00); //Cheksum
	XBEE_CS_HIGH();
	if(cheksum == 0xFF){
		#ifdef DEBUG
		Usart1_SendString("*AT command read correct*\n");
		#endif
		readingPacket = false;
		return data;
	}
	readingPacket = false;
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	#ifdef DEBUG
	Usart1_SendString("*AT command read error - cheksum*\n");
	#endif
	return 0xFF;
}

void transmitRequest(uint8_t adr1, uint8_t adr2, uint8_t adr3, uint8_t adr4, uint8_t adr5, uint8_t adr6, uint8_t adr7, uint8_t adr8, char* data){

	int8_t cheksum = 0;
	uint8_t  lenghtOfData, i = 0;
	lenghtOfData = strlen(data);

	XBEE_CS_LOW();
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
	for(; i < lenghtOfData; i++){	//RF data
		SPI1_TransRecieve(data[i]);
		cheksum += data[i];
	}
	SPI1_TransRecieve(0xFF - cheksum); //Cheksum
	XBEE_CS_HIGH();
}

