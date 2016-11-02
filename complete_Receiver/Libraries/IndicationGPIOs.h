#ifndef GPIO_LIBRARY
#define GPIO_LIBRARY

//These are the Includes
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>

//These are the Define statements
#define DELAY 20
#define SLOW_DELAY 50
#define REAL_SLOW_DELAY 100
#define REAL_REAL_SLOW_DELAY 200

//These are the prototypes for the routines
void initializeEveryRedLed(void);
void initializeEveryGreenLed(void);

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
void redStartup(int delay);
void redGreenStartup(void);
void amazingRedGreenStartup(void);
void xorGreenLed1(void);
void xorRedLed1(void);
void turnOnGreenLeds(uint8_t pin);
void blinkGreenLeds(uint8_t pin);
void xorGreenLed(uint8_t pin);
void batteryIndicationStartup(uint16_t voltageLevel);

#endif

