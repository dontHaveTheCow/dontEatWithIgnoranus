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
    blinkGreenLed3();
}

void apply2Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2){

	int8_t cheksum = 0;

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
    blinkGreenLed3();

}
void apply3Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3){

	int8_t cheksum = 0;

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
    blinkGreenLed3();

}
void apply4Params(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4){

	int8_t cheksum = 0;

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
    blinkGreenLed3();
}

void queue1Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1){

	int8_t cheksum = 0;

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
    blinkGreenLed3();
}

void queue2Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2){

	int8_t cheksum = 0;

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
    blinkGreenLed3();

}
void queue3Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3){

	int8_t cheksum = 0;

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
    blinkGreenLed3();

}
void queue4Param(uint8_t MSbyte, uint8_t LSbyte, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4){

	int8_t cheksum = 0;

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
    blinkGreenLed3();
}

/*void readModuleParams(uint8_t MSbyte, uint8_t LSbyte){

	int8_t cheksum = 0;

	while(readingPacket);

	GPIO_ResetBits(GPIOA,GPIO_Pin_4);

	processByte(0x7E);
	processByte(0x00);
	processByte(0x04);
	processByte(0x08);
	cheksum += 0x08;
	processByte(0x52);
	cheksum += 0x52;
	processByte(MSbyte);
	cheksum += MSbyte;
	processByte(LSbyte);
	cheksum += LSbyte;
	processByte(0xFF - cheksum);

	processRemainingBytes();

	GPIO_SetBits(GPIOA,GPIO_Pin_4);

	int i = 0;

	if(readingPacket){
		for(; i < numberOfBytesRead; i++){
			printf("Symbol %d: %d \n", i, recievePacket[numberOfBytesRead]);
		}
		readingPacket = false;
		numberOfBytesRead = 0;

	}



	recievedByte = SPI1_TransRecieve(0x7E);
	recievedByte = SPI1_TransRecieve(0x00);
	recievedByte = SPI1_TransRecieve(0x04);
	//AT command
	recievedByte = SPI1_TransRecieve(0x08);
	cheksum += 0x08;
	recievedByte = SPI1_TransRecieve(0x52); //Frame ID
	cheksum += 0x52;
	//NH - Network Hops
	recievedByte = SPI1_TransRecieve(MSbyte); //Command
	cheksum += MSbyte;
	recievedByte = SPI1_TransRecieve(LSbyte);
	cheksum += LSbyte;
	recievedByte = SPI1_TransRecieve(0xFF - cheksum); //Chekcsum


}*/

void readModuleParams(uint8_t MSbyte, uint8_t LSbyte){

	int8_t cheksum = 0;

	//while(readingPacket); //Check if rf packets aren't incoming at the same time

	//readingPacket = true;
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);

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

	GPIO_SetBits(GPIOA,GPIO_Pin_4);

}

void restoreDefaults(){

	int8_t cheksum = 0;

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
    blinkGreenLed3();
}



void initializeXbeeAPI(void){
	delayMs(10);
	restoreDefaults();
	delayMs(10);
	//applyModuleParams();
}

void transmitRequest(uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, char* data){

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
	SPI1_TransRecieve(adrHigh >> 24);	//64bit adress
	cheksum += adrHigh >> 24;
	SPI1_TransRecieve(adrHigh >> 16);
	cheksum += adrHigh >> 16;
	SPI1_TransRecieve(adrHigh >> 8);
	cheksum += adrHigh >> 8;
	SPI1_TransRecieve(adrHigh);
	cheksum += adrHigh;
	SPI1_TransRecieve(adrLow >> 24);
	cheksum += adrLow >> 24;
	SPI1_TransRecieve(adrLow >> 16);
	cheksum += adrLow >> 16;
	SPI1_TransRecieve(adrLow >> 8);
	cheksum += adrLow >> 8;
	SPI1_TransRecieve(adrLow);
	cheksum += adrLow;
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

	//Response from slave
	//SPI1_TransRecieve(0x00);

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

/*void processByte(uint8_t byte){
	recievedByte = SPI1_TransRecieve(byte);
	printf("Recieved byte: %d NumberOfByte: %d\n", recievedByte, numberOfBytesRead);

	if(readingPacket)
		recievePacket[numberOfBytesRead++] = recievedByte;
	else if(recievedByte == 0x7E){
		readingPacket = true;
		recievePacket[numberOfBytesRead++] = recievedByte;
	}
}*/

/*void processRemainingBytes(void){

	uint8_t i = 0;

	if(readingPacket && numberOfBytesRead < 2){
		processByte(0x00);
		processByte(0x00);
	}
	if(readingPacket && numberOfBytesRead < 3){
		processByte(0x00);
	}

	if(readingPacket == true && numberOfBytesRead > 2){
		for(i = numberOfBytesRead; i < recievePacket[2]+4; i++){ //numberOfBytesRead include checksum, start delimeter and lenght bytes
			recievedByte = SPI1_TransRecieve(0x00);
			recievePacket[numberOfBytesRead++] = recievedByte;
			printf("Recieved byte: %d NumberOfByte: %d\n", recievedByte, numberOfBytesRead);

		}
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

	}
}*/
