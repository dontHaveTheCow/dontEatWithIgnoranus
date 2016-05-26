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
	uint8_t buffer[512]= "I am the Mattress Bitch I know if I can eat one I can eat 5";
	uint32_t mstrDir,first_file_cluster, next_file_cluster;
	uint32_t nextFreeCluster, freeClusterCount;
	uint32_t fileSize;
	uint16_t fatSect, fsInfoSector;

	initialiseSysTick();
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	delayMs(100);
	initializeSD();

	findDetailsOfFAT(buffer,&fatSect,&mstrDir, &fsInfoSector);
	first_file_cluster = findFirstClusterOfFile("HAMLET",buffer,mstrDir);
	next_file_cluster = findNextClusterOfFile(first_file_cluster,buffer,fatSect);
	freeClusterCount = decrementFreeClusterCount(buffer,fsInfoSector);
	delayMs(100);

	goToIdleState();

	//Debug with SPI ->>>>>>> CooCox debugger sucks dick
	SDSELECT();
	spi_rw(freeClusterCount);
	spi_rw(freeClusterCount >> 8);
	spi_rw(freeClusterCount >> 16);
	spi_rw(freeClusterCount >> 24);


	SDDESELECT();


	while(1){
    }
}
