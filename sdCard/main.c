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
#include "IndicationGPIOs.h"
#include <string.h>
#include <stdlib.h>

void reverse(char s[]);
void itoa(int n, char s[]);

int main(void){
	uint8_t buffer[512];
	uint8_t sector;
	uint32_t mstrDir;
	uint32_t cluster;
	uint32_t filesize;
	uint16_t fatSect, fsInfoSector;
	uint16_t sdBufferCurrentSymbol = 0;

	char FATinfoString[30] = "";
	char FATchar[8];

	uint16_t i;

	uint8_t sdStatus;

	initialiseSysTick();
	InitialiseSPI1_GPIO();
	InitialiseSPI1();
	initializeRedLed1();
	initializeGreenLed1();

	delayMs(100);
	sdStatus = initializeSD();

	if(sdStatus == 0x01){

		xorGreenLed1();

		findDetailsOfFAT(buffer,&fatSect,&mstrDir, &fsInfoSector);
		findDetailsOfFile("LOGFILE",buffer,mstrDir,&filesize,&cluster,&sector);
		findLastClusterOfFile("LOGFILE",buffer, &cluster,fatSect,mstrDir);

		xorGreenLed1();

		if(filesize < 512)
			filesize = 512;

		appendTextToTheSD("\nNEW LOG", '\n', &sdBufferCurrentSymbol, buffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);

		//180 for writing 6 sectors
		for(i; i < 180; i ++){
		strcpy(&FATinfoString[0], "SIZE");
		itoa(filesize,FATchar);
		strcpy(&FATinfoString[strlen(FATinfoString)], FATchar);
		strcpy(&FATinfoString[strlen(FATinfoString)], "SECT");
		itoa(sector,FATchar);
		strcpy(&FATinfoString[strlen(FATinfoString)], FATchar);
		strcpy(&FATinfoString[strlen(FATinfoString)], "CLUST");
		itoa(cluster,FATchar);
		strcpy(&FATinfoString[strlen(FATinfoString)], FATchar);
		appendTextToTheSD(FATinfoString, '\n', &sdBufferCurrentSymbol, buffer, "LOGFILE", &filesize, mstrDir, fatSect, &cluster, &sector);
		xorGreenLed1();
		}

		delayMs(100);
		while(!goToIdleState());
		xorGreenLed1();
		//xorGreenLed1();
		//Debug with SPI ->>>>>>> CooCox debugger sucks dick
/*		SDSELECT();
		spi_rw(cluster);
		spi_rw(cluster >> 8);
		spi_rw(cluster >> 16);
		spi_rw(cluster >> 24);
		spi_rw(0xFF);
		spi_rw(sector);
		spi_rw(sector >> 8);
		spi_rw(sector >> 16);
		spi_rw(sector >> 24);
		spi_rw(0xFF);
		spi_rw(filesize);
		spi_rw(filesize >> 8);
		spi_rw(filesize >> 16);
		spi_rw(filesize >> 24);
		SDDESELECT();*/
	}
	else{
		xorRedLed1();
	}

	while(1){

    }
}

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}
