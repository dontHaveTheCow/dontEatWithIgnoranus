#ifndef ADXL362_LIBRARY
#define ADXL362_LIBRARY

//These are the Includes
#include "SPI2.h"
#include "SysTickDelay.h"
#include <stdbool.h>

//These are the Define statements
//Read/writes
#define ADXL_READ 0x0B
#define ADXL_WRITE 0x0A
#define READ_FIFO 0x0D

//Registers
#define ADXL_RESET_REGISTER 0x1F
#define ADXL_BYTE_XAXIS 0x08
#define ADXL_FILTER_CTL_REGISTER 0x2C
#define ADXL_POWER_CTL_REGISTER 0x2D
#define ADXL_STATUS_REGISTER 0x0B
#define ADXL_STATUS_REGISTER_AWAKE_BIT 0x40
#define ADXL_STATUS_REGISTER_DATA_READY_BIT 0x01
//Command defines
#define ADXL_RESET_CMD 0x52
//These are the prototypes for the routines
void initializeADXL362(void);
void writeADXL326Register(uint8_t reg, uint8_t cmd);
uint8_t readADXL362Register(uint8_t reg);
int16_t SPIreadTwoRegisters(uint8_t regAddress);
void getZ(int16_t *z, int16_t *z_low, int16_t *z_high);
int16_t returnZ_axis(void);
int16_t returnX_axis(void);
bool return_ADXL_ready(void);


#endif
