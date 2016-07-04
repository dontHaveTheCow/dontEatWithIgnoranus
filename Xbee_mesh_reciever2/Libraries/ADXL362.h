#ifndef ADXL362_LIBRARY
#define ADXL362_LIBRARY

//These are the Includes
#include "SPI2.h"
#include "SysTickDelay.h"

//These are the Define statements
#define READ_REGISTER 0x0B
#define WRITE_REGISTER 0x0A
#define READ_FIFO 0x0D

#define RESET 0x1F
#define BYTE_XAXIS 0x08
//These are the prototypes for the routines
void initializeADXL362(void);
void writeADXL326Register(uint8_t reg, uint8_t cmd);
uint8_t readADXL362Register(uint8_t reg);
int16_t SPIreadTwoRegisters(uint8_t regAddress);
void getZ(int16_t *z, int16_t *z_low, int16_t *z_high);
int16_t getZV2(void);


#endif
