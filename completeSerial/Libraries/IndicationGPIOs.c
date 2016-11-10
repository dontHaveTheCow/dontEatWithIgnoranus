#include "IndicationGPIOs.h"
#include "SysTickDelay.h"

void initializeEveryRedLed(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void initializeEveryGreenLed(void){
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8| GPIO_Pin_7 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

void turnOnGreenLed(uint8_t pin){
	GPIO_ResetBits(GPIOC,GPIO_Pin_All);
	GPIOC->ODR |= (1 << (6+pin));
}

void blinkRedLed1(void){
		GPIOB->ODR |= GPIO_Pin_5;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_5;
    	delayMs(DELAY);
}

void blinkRedLed2(void){
		GPIOB->ODR |= GPIO_Pin_6;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_6;
    	delayMs(DELAY);
}

void blinkRedLed3(void){
		GPIOB->ODR |= GPIO_Pin_7;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_7;
    	delayMs(DELAY);
}

void blinkRedLed4(void){
		GPIOB->ODR |= GPIO_Pin_8;
    	delayMs(REAL_SLOW_DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_8;
    	delayMs(REAL_SLOW_DELAY);
}

void blinkRedLed5(void){
		GPIOB->ODR |= GPIO_Pin_9;
    	delayMs(DELAY);

    	GPIOB->ODR &=~ GPIO_Pin_9;
    	delayMs(DELAY);
}

void blinkAllRed(void){
	GPIOB->ODR |= GPIO_Pin_5;
	GPIOB->ODR |= GPIO_Pin_6;
	GPIOB->ODR |= GPIO_Pin_7;
	GPIOB->ODR |= GPIO_Pin_8;
	GPIOB->ODR |= GPIO_Pin_9;
	delayMs(DELAY);

	GPIOB->ODR &=~ GPIO_Pin_5;
	GPIOB->ODR &=~ GPIO_Pin_6;
	GPIOB->ODR &=~ GPIO_Pin_7;
	GPIOB->ODR &=~ GPIO_Pin_8;
	GPIOB->ODR &=~ GPIO_Pin_9;
	delayMs(DELAY);
}

void turnOnRedLed(uint8_t pin){
	GPIOB->ODR |= (1 << (5+pin));
}
void blinkRedLed(uint8_t pin){
	//5 to 9
	GPIOB->ODR |= (1 << (5+pin));
	delayMs(DELAY);

	GPIOB->ODR &=~ (1 << (5+pin));
	delayMs(DELAY);
}

void blinkGreenLed1(void){
		GPIOC->ODR |= GPIO_Pin_6;
    	delayMs(DELAY);

    	GPIOC->ODR &=~ GPIO_Pin_6;
    	delayMs(DELAY);
}

void blinkGreenLed2(void){
		GPIOC->ODR |= GPIO_Pin_7;
    	delayMs(DELAY);

    	GPIOC->ODR &=~ GPIO_Pin_7;
    	delayMs(DELAY);
}

void blinkGreenLed3(void){
		GPIOC->ODR |= GPIO_Pin_8;
    	delayMs(DELAY);

    	GPIOC->ODR &=~ GPIO_Pin_8;
    	delayMs(DELAY);
}

void redStartup(int delay){

	int i = 5;
	for(; i > 0; i--){
		GPIOB->ODR |= (1 << (i+4));
		delayMs(delay);
	}
	for(; i < 5; i++){
		GPIOB->ODR &=~ (1 << (i+5));
		delayMs(delay);
	}
}

void redGreenStartup(void){
	int i = 5;
	//turn on red leds
	for(; i > 0; i--){
		GPIOB->ODR |= (1 << (i+4));
		delayMs(SLOW_DELAY);	}
	//turn on green leds
	for(; i < 3; i++){
		GPIOC->ODR |= (1 << (i+6));
		delayMs(SLOW_DELAY);
		//turn off green leds
	}
	for(; i > 0; i--){
		GPIOC->ODR &=~ (1 << (i+5));
		delayMs(SLOW_DELAY);
	}
	//turn off red leds
	for(; i < 5; i++){
		GPIOB->ODR &=~ (1 << (i+5));
		delayMs(SLOW_DELAY);
	}
}

void amazingRedGreenStartup(void){

	//green 6 to 8
	//red 5 to 9

	int i = 5;
	//turn on red leds
	for(; i < 10; i++){
		GPIOB->ODR |= (1 << (i));
		delayMs(SLOW_DELAY);
	}
	//turn on green leds
	i = 6;
	for(; i < 9; i++){
		GPIOC->ODR |= (1 << (i));
		delayMs(SLOW_DELAY);
	}
	//turn off red leds
	i = 5;
	for(; i < 10; i++){
		GPIOB->ODR &=~ (1 << (i));
		delayMs(SLOW_DELAY);
	}
	//turn off green leds
	i = 6;
	for(; i < 9; i++){
		GPIOC->ODR &=~ (1 << (i));
		delayMs(SLOW_DELAY);
	}
}

void xorGreenLed1(void){
	GPIOC->ODR ^= GPIO_Pin_6;
}

void xorRedLed1(void){
	GPIOB->ODR ^= GPIO_Pin_5;
}

void turnOnGreenLeds(uint8_t pin){
	GPIOC->BSRR = 7 << (6+16);
	GPIOC->BSRR = pin << 6;
}

void blinkGreenLeds(uint8_t pin){

	GPIOC->BSRR = pin << 6;
	delayMs(REAL_REAL_SLOW_DELAY);
	GPIOC->BSRR = pin << (6+16);
	delayMs(REAL_REAL_SLOW_DELAY);
}

void xorGreenLed(uint8_t pin){
	GPIOC->ODR ^= (1 << (pin+6));
}

void batteryIndicationStartup(uint16_t voltageLevel){

	if(voltageLevel > 419){
		GPIOB->ODR |= (1 << 9);
		GPIOB->ODR |= (1 << 8);
		GPIOB->ODR |= (1 << 7);
		GPIOB->ODR |= (1 << 6);
		GPIOB->ODR |= (1 << 5);
		delayMs(REAL_REAL_SLOW_DELAY);
		GPIOB->ODR &=~ (1 << 9);
		GPIOB->ODR &=~ (1 << 8);
		GPIOB->ODR &=~ (1 << 7);
		GPIOB->ODR &=~ (1 << 6);
		GPIOB->ODR &=~ (1 << 5);
	}
	else if(voltageLevel > 394){
		GPIOB->ODR |= (1 << 8);
		GPIOB->ODR |= (1 << 7);
		GPIOB->ODR |= (1 << 6);
		GPIOB->ODR |= (1 << 5);
		delayMs(REAL_REAL_SLOW_DELAY);
		GPIOB->ODR &=~ (1 << 8);
		GPIOB->ODR &=~ (1 << 7);
		GPIOB->ODR &=~ (1 << 6);
		GPIOB->ODR &=~ (1 << 5);
	}
	else if(voltageLevel > 379){
		GPIOB->ODR |= (1 << 7);
		GPIOB->ODR |= (1 << 6);
		GPIOB->ODR |= (1 << 5);
		delayMs(REAL_REAL_SLOW_DELAY);
		GPIOB->ODR &=~ (1 << 7);
		GPIOB->ODR &=~ (1 << 6);
		GPIOB->ODR &=~ (1 << 5);
	}
	else if(voltageLevel > 369){
		GPIOB->ODR |= (1 << 6);
		GPIOB->ODR |= (1 << 5);
		delayMs(REAL_REAL_SLOW_DELAY);

		GPIOB->ODR &=~ (1 << 6);
		GPIOB->ODR &=~ (1 << 5);
	}
	else if(voltageLevel > 360){
		GPIOB->ODR |= (1 << 5);
		delayMs(REAL_REAL_SLOW_DELAY);
		GPIOB->ODR &=~ (1 << 5);
	}
}



