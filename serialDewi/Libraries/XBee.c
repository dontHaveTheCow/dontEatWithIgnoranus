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

void xbeeApplyParamter(char* atCommand, uint8_t parameter, uint8_t frameID){

	uint8_t cheksum = 0;

	XBEE_CS_LOW();
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x05);
	SPI1_TransRecieve(0x08); //AT command
	cheksum += 0x08;
	SPI1_TransRecieve(frameID); //Frame ID
	cheksum += frameID;
	SPI1_TransRecieve(atCommand[0]); //Command
	cheksum += atCommand[0];
	SPI1_TransRecieve(atCommand[1]);
	cheksum += atCommand[1];
	//Parameters goes here
	SPI1_TransRecieve(parameter);
	cheksum += parameter;
	SPI1_TransRecieve(0xFF - cheksum); //Checksum
	XBEE_CS_HIGH();
}

void askXbeeParam(char* atCommand, uint8_t frameID){

	int8_t cheksum = 0;

	XBEE_CS_LOW();
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve(0x00);
	SPI1_TransRecieve(0x04); //Lenght
	SPI1_TransRecieve(0x08); //AT command
	cheksum += 0x08;
	SPI1_TransRecieve(frameID);
	cheksum += frameID;
	SPI1_TransRecieve(atCommand[0]); //Command
	cheksum += atCommand[0];
	SPI1_TransRecieve(atCommand[1]);
	cheksum += atCommand[1];
	SPI1_TransRecieve(0xFF - cheksum);
	XBEE_CS_HIGH();
}

bool xbeeStartupParamRead(uint8_t _packetErrorLimit, uint8_t* _xbeeBuffer){

	uint8_t checksum = 0;
	uint8_t errorTimer = 0;
	uint8_t length;

	XBEE_CS_LOW();
	while(SPI1_TransRecieve(0x00) != 0x7E){	//Wait for start delimiter
	errorTimer++;
	if(errorTimer >_packetErrorLimit)//Exit loop if there is no start delimiter
			break;
	}
	if(errorTimer < _packetErrorLimit){
		SPI1_TransRecieve(0x00);
		length = SPI1_TransRecieve(0x00);
		//printf("Lenght: %d\n", length);
		uint8_t i = 0;
		for(; i < length; i ++ ){				//Read data based on packet length
			checksum += (_xbeeBuffer[i] = SPI1_TransRecieve(0x00));
		}
		checksum += SPI1_TransRecieve(0x00);
		XBEE_CS_HIGH();
		if(checksum == 0xFF){
			return true;
		}
	}
	XBEE_CS_HIGH();
	return false;
}

void transmitRequest(uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, uint8_t frameID, char* data){

	int8_t cheksum = 0;
	uint16_t  i = 0;
	uint16_t lenghtOfData = strlen(data);

	XBEE_CS_LOW();
	SPI1_TransRecieve(0x7E);
	SPI1_TransRecieve((lenghtOfData+14)  >> 8);	//Lenght, need to check if working
	SPI1_TransRecieve(lenghtOfData+14);
	SPI1_TransRecieve(0x10);	//FrameType
	cheksum += 0x10;
	SPI1_TransRecieve(frameID);	//Frame ID
	cheksum += frameID;
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
	SPI1_TransRecieve(transmitOption);	//Transmit options (Disable ack -> 0x00 Point-multipoint)
	cheksum += transmitOption;
	for(; i < lenghtOfData; i++){	//RF data
		SPI1_TransRecieve(data[i]);
		cheksum += data[i];
	}
	SPI1_TransRecieve(0xFF - cheksum); //Cheksum

	XBEE_CS_HIGH();
}

uint8_t fillTransmitPacket( uint32_t adrHigh, uint32_t adrLow, uint8_t transmitOption, uint8_t frameID, uint8_t* sendBuff, char* data){

	int8_t cheksum = 0;
	uint16_t  i = 0;
	uint16_t lenghtOfData = strlen(data);

	sendBuff[0] = 0x7E;
	sendBuff[1] = ((lenghtOfData+14)  >> 8);	//Lenght, need to check if working
	sendBuff[2] = lenghtOfData+14;
	sendBuff[3] = 0x10;							//FrameType
	cheksum += 0x10;
	sendBuff[4] = frameID;	//Frame ID
	cheksum += frameID;
	sendBuff[5] = (adrHigh >> 24);	//64bit adress
	cheksum += adrHigh >> 24;
	sendBuff[6] = (adrHigh >> 16);
	cheksum += adrHigh >> 16;
	sendBuff[7] = (adrHigh >> 8);
	cheksum += adrHigh >> 8;
	sendBuff[8] = (adrHigh);
	cheksum += adrHigh;
	sendBuff[9] = (adrLow >> 24);
	cheksum += adrLow >> 24;
	sendBuff[10] = (adrLow >> 16);
	cheksum += adrLow >> 16;
	sendBuff[11] = (adrLow >> 8);
	cheksum += adrLow >> 8;
	sendBuff[12] = (adrLow);
	cheksum += adrLow;
	sendBuff[13] = (0xFF);	//Reserved
	cheksum += 0xFF;
	sendBuff[14] = (0xFE);
	cheksum += 0xFE;
	sendBuff[15] = (0x00);	//Broadcast radius
	cheksum += 0x00;
	sendBuff[16] = (transmitOption);	//Transmit options (Disable ack -> 0x00 Point-multipoint)
	cheksum += transmitOption;
	for(; i < lenghtOfData; i++){	//RF data
		sendBuff[i + 17] = (data[i]);
		cheksum += data[i];
	}
	sendBuff[i + 17] = (0xFF - cheksum); //Cheksum

	return i + 18;
}

uint8_t fillATPacket(char* atCommand, uint8_t parameter, uint8_t AT_FRAME_ID, uint8_t* sendBuff){

	int8_t cheksum = 0;

	sendBuff[0] = 0x7E;	//Start delimiter
	sendBuff[1] = 0x00; //16bit msb
	if(AT_FRAME_ID == AT_FRAME_ID_APPLY){
		sendBuff[2] = 0x05; //16bit lsb
	}
	else{
		sendBuff[2] = 0x04; //16bit lsb
	}
	cheksum  += (sendBuff[3] = XBEE_AT_COMMAND);	  //AT
	cheksum  += (sendBuff[4] = AT_FRAME_ID); //Frame id
	cheksum  += (sendBuff[5] = atCommand[0]); //AT command msb
	cheksum  += (sendBuff[6] = atCommand[1]); //AT command lsb
	if(AT_FRAME_ID == AT_FRAME_ID_APPLY){
		cheksum  += (sendBuff[7] = parameter); //Parameter
		sendBuff[8] = 0xFF - cheksum; //Checksum
		return 9;
	}
	sendBuff[7] = 0xFF - cheksum; //Checksum
	return 8;
}

void transreceiveAPIPacket(uint8_t* sendBuff, uint8_t* xbeeReceiveBuffer, uint16_t xbeeBytesToSend, uint16_t* xbeeBytesSent){

	*xbeeBytesSent = 0;
	uint16_t xbeeBytesReceived = 0;
	uint16_t xbeeBytesToReceive = 0;
	XBEE_CS_LOW();
	xbeeSendingPacket = true;

	while(*xbeeBytesSent < xbeeBytesToSend){
		if(xbeeReceivingPacket == true){
			xbeeReceiveBuffer[xbeeBytesReceived++] = SPI1_TransRecieve(sendBuff[(*xbeeBytesSent)++]);

			/*
			 * External interrupt might miss the start delimiter (0x7E)
			 * by one byte, so include it manualy
			 */
			if(xbeeReceiveBuffer[0] != 0x7E){
				xbeeReceiveBuffer[xbeeBytesReceived++] = xbeeReceiveBuffer[0];
				xbeeReceiveBuffer[0] = 0x7E;
			}
			if(xbeeBytesReceived == 3){
				xbeeBytesToReceive = (xbeeReceiveBuffer[1] << 8);
				xbeeBytesToReceive |= xbeeReceiveBuffer[2];
/*				Usart1_Send(xbeeReceiveBuffer[1]/100 + 0x30);
				Usart1_Send((xbeeReceiveBuffer[1]%100)/10 + 0x30);
				Usart1_Send(xbeeReceiveBuffer[1]%10 + 0x30);
				Usart1_Send(xbeeReceiveBuffer[2]/100 + 0x30);
				Usart1_Send((xbeeReceiveBuffer[2]%100)/10 + 0x30);
				Usart1_Send(xbeeReceiveBuffer[2]%10 + 0x30);*/
			}
			else if(xbeeBytesReceived == xbeeBytesToReceive + 4){
				xbeeReceivingPacket = false;
				//xbeePacketReady = true;
			}
		}
		else{
			SPI1_TransRecieve(sendBuff[(*xbeeBytesSent)++]);
		}
	}
	xbeeSendingPacket = false;

	if(xbeeReceivingPacket == true){

		while(xbeeBytesReceived < 3){
			xbeeReceiveBuffer[xbeeBytesReceived++] = SPI1_TransRecieve(0x00);
		}
		xbeeBytesToReceive = xbeeReceiveBuffer[1] << 8;
		xbeeBytesToReceive |= xbeeReceiveBuffer[2];
		/*
		 * This time include 4 bytes (start delimiter, lenght and checksum)
		 */
		while(xbeeBytesReceived < xbeeBytesToReceive + 4){
			xbeeReceiveBuffer[xbeeBytesReceived++] = SPI1_TransRecieve(0x00);
		}
		xbeeReceivingPacket = false;
		//xbeePacketReady = true;
	}

	XBEE_CS_HIGH();
}



