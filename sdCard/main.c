/*
 *
 * Sd card's SPI speed should have a frequency in the range of 100 to 400 kHz at initialization process.
 * Xbee's SPI transfer speed
 *
 *
 *
 */
#include "stm32f0xx_gpio.h"
#include "sdCard.h"
#include "SPI1.h"
#include "SysTickDelay.h"

int main(void){
	uint8_t buffer[512];
	uint8_t sector;
	uint32_t mstrDir;
	uint32_t cluster;
	uint32_t filesize;
	uint16_t fatSect, fsInfoSector;

	initialiseSysTick();
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	delayMs(100);
	initializeSD();

	findDetailsOfFAT(buffer,&fatSect,&mstrDir, &fsInfoSector);

	findDetailsOfFile("LOGFILE",buffer,mstrDir,&filesize,&cluster,&sector);

	cluster = findLastClusterOfFile("LOGFILE",buffer,fatSect,mstrDir);

	writeNextSectorOfFile(buffer,"LOGFILE",&filesize,mstrDir,fatSect,&cluster,&sector);

	delayMs(100);
	while(!goToIdleState());

	//Debug with SPI ->>>>>>> CooCox debugger sucks dick
	SDSELECT();
	spi_rw(cluster);
	spi_rw(cluster >> 8);
	spi_rw(cluster >> 16);
	spi_rw(cluster >> 24);

	SDDESELECT();

	while(1){
    }
}
