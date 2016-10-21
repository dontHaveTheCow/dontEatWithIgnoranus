#ifndef A2035H
#define A2035H

//These are the Includes
#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_rcc.h>
#include "SysTickDelay.h"
#include <stm32f0xx_tim.h>
#include <stm32f0xx_misc.h>
#include <stdio.h>

#include "USART2.h"

//These are the Define statements
#define WAKEUP_PIN GPIO_Pin_3					//PC3
#define RESET_PIN GPIO_Pin_2					//PC2
#define ON_PIN GPIO_Pin_4						//PF4

//NMEA msg. defines
#define $GPGGA 0
#define $GPGLL 1
#define $GPGSA 2
#define $GPGSV 3
#define $GPRMC 4
#define $GPVTG 5
#define ASCII_DIGIT_OFFSET 0x30

//These are the prototypes for the routines
void turnGpsOn(void);
void hibernateGps(void);
void setupGpsGpio(void);
void setupGpsTimer(void);
void setupGpsTimerInterrupt(void);

//Query/Rate Control
void gps_dissableMessage(uint8_t message);
void gps_setRate(uint8_t message, uint8_t rate);
void gps_enable5hz(void);
void gps_disable5hz(void);

#endif
