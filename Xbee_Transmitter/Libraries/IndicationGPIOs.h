#ifndef GPIO_LIBRARY
#define GPIO_LIBRARY

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>

//These are the Define statements

//These are the prototypes for the routines
void initializeGreenLed1(void);
void initializeGreenLed2(void);
void initializeGreenLed3(void);
void initializeRedLed1(void);
void initializeRedLed2(void);
void initializeRedLed3(void);
void initializeRedLed4(void);
void initializeRedLed5(void);

void turnOnGreenLed(uint8_t pin);

void blinkGreenLed1(void);
void blinkGreenLed2(void);
void blinkGreenLed3(void);
void blinkRedLed1(void);
void blinkRedLed2(void);
void blinkRedLed3(void);
void blinkRedLed4(void);
void blinkRedLed5(void);
void blinkAllRed(void);
void turnOnRedLed(uint8_t pin);
void blinkRedLed(uint8_t pin);
void redStartup(void);

#endif

