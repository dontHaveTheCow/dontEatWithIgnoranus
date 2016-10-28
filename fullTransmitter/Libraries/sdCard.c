#include "sdCard.h"

uint8_t initializeSD(void){

	/* Declaration des variables ----------------------------- */
	uint8_t i, ocr[4];
	uint16_t timeout = 200;

	SDDESELECT();

	//80 clk cycles
	for (i = 10; i; i--) {
		spi_rw(0xff);
	}

	//Toggle cs line down and start sending commands
	/* Initialize sd card in spi mode ---------------------- */
	/* Firstly put sd card in idle mode ---------------------- */
	if (send_cmd(CMD0, 0) == 1) {

		/* Determine what commands/version the card is.  -------------- */
		if (send_cmd(CMD8, 0x1AA) == 1) {
			for (i = 0; i < 4; i++)
				ocr[i] = spi_r();

			/* Verify voltage and check pattern --------------- */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
				/* Activate cards initialization process ------------ */
				while (send_cmd(ACMD41, 0x40000000) && timeout--){
					//5 ~ 1000 attempts could be needed
					//increment error timer
					if(!timeout)
						return 0;
				}
				/* Read OCR ----------------- */
				if (send_cmd(CMD58, 0) == 0) {
					for (i = 0; i < 4; i++)
						ocr[i] = spi_r();
				}
			}
		}
	}
	else {
		return 0;
	}
	release_spi();

	//interface_speed(INTERFACE_FAST);

	return 1;
}

static uint8_t wait_ready(void) {
	uint8_t res;
	uint16_t timeout = 100; //try 100 times max.

	do {
		res = spi_r();
	} while ((res != 0xFF && timeout--)); // && Timer2);

	return res;
}

uint8_t send_cmd(uint8_t cmd, uint32_t arg){

	/* res - response, n - crc and timeout counter */
	uint8_t n;
	uint8_t res;

	/* Check whether the next command is application specific or the standard command */
	/* MSB signals whether the command is application specific  */
	if (cmd & 0x80) {
		//Every sd command is 6 bits long, so drop the first bit off
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);	//Response should be 0x01
		if (res > 1)
			return res;
	}

	/* Select the card --------------------------------------- */
	//  SDDESELECT();
	SDSELECT();

	/* wait for ready ---------------------------------------- */
	if (wait_ready() != 0xFF) {
		return 0xFF;
	}

	/* Send command packet ----------------------------------- */
	spi_rw(cmd); // Send command index

	spi_rw((uint8_t)(arg >> 24)); // Send argument[31..24]
	spi_rw((uint8_t)(arg >> 16)); // Send argument[23..16]
	spi_rw((uint8_t)(arg >> 8)); // Send argument[15..8]
	spi_rw((uint8_t) arg); // Send argument[7..0]

	n = 0x01; // Stop : Dummy CRC
	if (cmd == CMD0)
		n = 0x95; // Valid CRC for CMD0(0)
	if (cmd == CMD8)
		n = 0x87; // Valid CRC for CMD8(0x1AA)
	spi_rw(n); // Send CRC

	/* Receive command response ------------------------------ */
	if (cmd == CMD12)
		spi_r(); // Skip a stuff byte when stop reading

	/* Wait for a valid response in timeout of 10 attempts --- */
	n = 10;
	do
		res = spi_r();
	while ((res & 0x80) && --n);
	/* Return with the response value ------------------------ */
	return res;
}

uint8_t goToIdleState(void){
	if(send_cmd(CMD0, 0) == 1){
		release_spi();
		return 1;
	}

	else{
		release_spi();
		return 0;
	}
}

uint8_t xmit_datablock(uint8_t *buff, uint32_t sector){

	uint8_t resp;
	uint8_t wc;
	/* Single block write -------------------------------- */
	if (wait_ready() != 0xFF)
		return 0;
	if (send_cmd(CMD24, sector + SECTOR_OFFSET) == 0){
		spi_rw(DATA_TOKEN);
		wc = 0;
		do { /* transmit the 512 byte data block to MMC */
			spi_rw(*buff++);
			spi_rw(*buff++);
		} while (--wc);

		spi_rw(0xFF); /* CRC (Dummy) */
		spi_rw(0xFF);

		resp = spi_r(); /* Receive data response */
		if ((resp & 0x1F) != 0x05){	/* If not accepted, return with error */
			release_spi();
			return 0;
		}
	}
	else{
		release_spi();
		return 0;
	}
	release_spi();
	return 1;
}


uint8_t read_datablock(uint8_t *buff, uint32_t sector){

	uint8_t wc = 100;
	uint8_t res;

	if (wait_ready() != 0xFF)
		return 0;
	if(send_cmd(CMD17, sector + SECTOR_OFFSET) == 0){

		//Receive and check data token
		do {
			res = spi_r();
		} while ((res != DATA_TOKEN && wc--));
		wc = 0;
		do { /* transmit the 512 byte data block to MMC */
			spi_r_m(buff++);
			spi_r_m(buff++);
		} while (--wc);

		spi_r(); /* Discard CRC */
		spi_r();

	}
	else{
		release_spi();
		return 0;
	}
	release_spi();
	return 1;
}

void findDetailsOfFAT(uint8_t *buff, uint16_t* _fat, uint32_t* _mstrDir, uint16_t* _fsInfoSector){
	//read boot sector (sector 0) to find reserved sectors, fat size and number of fat's
	//fat_begin_lba = Number_of_Reserved_Sectors;
	//cluster_begin_lba =Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	delayMs(1);
	read_datablock(buff,0);
	*_fat = buff[RESERVED_SECTORS_1] + (buff[RESERVED_SECTORS_2] << 8);
	*_mstrDir = *_fat +	buff[NUMBER_OF_FATS] *
			(buff[SECTORS_PER_FAT_1] +
			(buff[SECTORS_PER_FAT_2] << 8 ) +
			(buff[SECTORS_PER_FAT_3] << 16 )+
			(buff[SECTORS_PER_FAT_4] << 24 ));
	*_fsInfoSector = buff[FS_INFO_SECTOR_LOW] + (buff[FS_INFO_SECTOR_HIGH] << 8);
}

//Use this only if root directory is less then 1 cluster long
uint32_t findFirstClusterOfFile(char* filename, uint8_t *buff, uint32_t _mstrDir){

	/* ...You need to extend this functions if master directory is longer then 1 sector */

	uint8_t directoryCount = 0;

	while(!read_datablock(buff,_mstrDir));

	while(!startsWith(filename,(char*)buff)){
		//Each byte directory is 32 bytes long
		//If Short Filename doesn't match, jump to next directory
		buff = buff + 0x20;
		directoryCount++;
		//Check whether you reached the end of the sector
		if(directoryCount == 16)
			return 0;
	}
	//Find the first cluster of file
	return *(buff + FIRST_FILE_CLUSTER_LOW1)
		   + (*(buff + FIRST_FILE_CLUSTER_LOW2) << 8)
		   + (*(buff + FIRST_FILE_CLUSTER_HIGH1) << 16)
		   + (*(buff + FIRST_FILE_CLUSTER_HIGH2) << 24);
}

uint8_t findDetailsOfFile(char* filename, uint8_t *buff, uint32_t _mstrDir, uint32_t *_filesize, uint32_t *cluster, uint8_t *_sector){
	uint8_t directoryCount = 0;
	delayMs(1);
	while(!read_datablock(buff,_mstrDir));
	delayMs(1 );
	while(!startsWith(filename,(char*)buff)){
		//Each byte directory is 32 bytes long
		//If Short Filename doesn't match, jump to next directory
		buff = buff + 0x20;
		directoryCount++;
		//Check whether you reached the end of the sector
		if(directoryCount == 16)
			return 0;
	}

	*cluster = *(buff + FIRST_FILE_CLUSTER_LOW1)
				   + (*(buff + FIRST_FILE_CLUSTER_LOW2) << 8)
				   + (*(buff + FIRST_FILE_CLUSTER_HIGH1) << 16)
				   + (*(buff + FIRST_FILE_CLUSTER_HIGH2) << 24);

	*_filesize = *(buff + FILE_SIZE_OFFSET_LOW1)
			   + (*(buff + FILE_SIZE_OFFSET_LOW2) << 8)
			   + (*(buff + FILE_SIZE_OFFSET_HIGH1) << 16)
			   + (*(buff + FILE_SIZE_OFFSET_HIGH2) << 24);


	*_sector = ((*_filesize % 4096) + 511)/512;

	return 1;
}

uint32_t findNextClusterOfFile(uint32_t currentCluster,uint8_t *buff, uint16_t _fat){
/*
 *
 * Bits 7-31 of the current cluster tell you which sectors to read from the FAT,
and bits 0-6 tell you which of the 128 integers in that sector contain is the
number of the next cluster of your file (or if all ones, that the current cluster is the last)
 *
 *
 *
*/
	read_datablock(buff,_fat);
	return buff[currentCluster*4]
	       + ((buff[currentCluster*4+1]) << 8)
	       + ((buff[currentCluster*4+2]) << 16)
	       + ((buff[currentCluster*4+3]) << 24);
}

void findLastClusterOfFile(char* filename, uint8_t *buff, uint32_t *cluster, uint16_t _fat, uint32_t _mstrDir){

	uint32_t tmpCluster, lastCluster;
	delayMs(1);
	tmpCluster = *cluster;
	lastCluster = tmpCluster;
	/* Refer to the comment in findNextClusterOfFile() function */

	//Right now, the buffer is filled with master directory sector
	//The first cluster should be found, so
	//fill the buffer with FAT sector
	delayMs(1);
	read_datablock(buff,_fat);

	//old value -> 0x0FFFFFFF
	while(tmpCluster != 0xFFFFFFF){
	//while(tmpCluster != 0x00000080){
		lastCluster = tmpCluster;
		tmpCluster = buff[(lastCluster%0x80)*4]
		       	       + ((buff[(lastCluster%0x80)*4+1]) << 8)
		       	       + ((buff[(lastCluster%0x80)*4+2]) << 16)
		       	       + ((buff[(lastCluster%0x80)*4+3]) << 24);

		if(tmpCluster % 0x80 == 0){
			delayMs(1);
			read_datablock(buff, ++_fat);
		}

	}
	*cluster = lastCluster;

	/* Just a remainder!
	 * Each FAT sector contains of 128 (0x80) entries
	 * So after each 0x80 entries buffer is filled again with a next sector
	 * */
}


uint32_t findNextFreeCluster(uint8_t *buff, uint16_t _fsInfoSector){

	/* ...Next free cluster is located in fs dginfo sector */

	if(read_datablock(buff,_fsInfoSector))
	return buff[NEXT_FREE_CLUSTER_LOW1]
	       + ((buff[NEXT_FREE_CLUSTER_LOW2]) << 8)
	       + ((buff[NEXT_FREE_CLUSTER_HIGH1]) << 16)
	       + ((buff[NEXT_FREE_CLUSTER_HIGH2]) << 24);
	else
		return 0;
}

uint32_t findFreeClusterCount(uint8_t *buff, uint16_t _fsInfoSector){

	/* ...Free cluster count is located in fs info sector */

	read_datablock(buff,_fsInfoSector);
	return (buff[FREE_CLUSTER_COUNT_LOW1] )
	       + ((buff[FREE_CLUSTER_COUNT_LOW2]) << 8)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH1]) << 16)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH2]) << 24);
}
uint32_t decrementFreeClusterCount(uint8_t *buff, uint16_t _fsInfoSector){

	uint32_t clusterValue;

	/* ...First read free cluster count */

	read_datablock(buff,_fsInfoSector);

	/* ...Assign value to 32 bit unsigned integer and decrement it by 1 */

	clusterValue = (buff[FREE_CLUSTER_COUNT_LOW1])
	       + ((buff[FREE_CLUSTER_COUNT_LOW2]) << 8)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH1]) << 16)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH2]) << 24) - 1;

	/* ...Write new value to buffer */

	*(buff+FREE_CLUSTER_COUNT_LOW1) = clusterValue;
	*(buff+FREE_CLUSTER_COUNT_LOW2) = clusterValue >> 8;
	*(buff+FREE_CLUSTER_COUNT_HIGH1) = clusterValue >> 16;
	*(buff+FREE_CLUSTER_COUNT_HIGH2) = clusterValue >> 24;

	/* ...Write new value to memory*/
	/* ...Check whether the new value was written  */
	if(xmit_datablock(buff, _fsInfoSector)){
		return (buff[FREE_CLUSTER_COUNT_LOW1] )
		       + ((buff[FREE_CLUSTER_COUNT_LOW2]) << 8)
		       + ((buff[FREE_CLUSTER_COUNT_HIGH1]) << 16)
		       + ((buff[FREE_CLUSTER_COUNT_HIGH2]) << 24);
	}
	else
		return 0;


}
uint32_t incrementFreeClusterCount(uint8_t *buff, uint16_t _fsInfoSector){

	uint32_t clusterValue;

	/* ...First read free cluster count */

	read_datablock(buff,_fsInfoSector);

	/* ...Assign value to 32 bit unsigned integer and increment it by 1 */

	clusterValue = (buff[FREE_CLUSTER_COUNT_LOW1])
	       + ((buff[FREE_CLUSTER_COUNT_LOW2]) << 8)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH1]) << 16)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH2]) << 24) + 1;

	/* ...Write new value to buffer */

	*(buff+FREE_CLUSTER_COUNT_LOW1) = clusterValue;
	*(buff+FREE_CLUSTER_COUNT_LOW2) = clusterValue >> 8;
	*(buff+FREE_CLUSTER_COUNT_HIGH1) = clusterValue >> 16;
	*(buff+FREE_CLUSTER_COUNT_HIGH2) = clusterValue >> 24;

	/* ...Write new value to memory*/
	/* ...Check whether the new value was written  */

	if(xmit_datablock(buff, _fsInfoSector)){
		return (buff[FREE_CLUSTER_COUNT_LOW1] )
		       + ((buff[FREE_CLUSTER_COUNT_LOW2]) << 8)
		       + ((buff[FREE_CLUSTER_COUNT_HIGH1]) << 16)
		       + ((buff[FREE_CLUSTER_COUNT_HIGH2]) << 24);
	}
	else{
		return 0;
	}
}
uint32_t changeNextFreeCluster(uint8_t *buff, uint16_t _fsInfoSector, uint32_t clusterValue){

	//store old fs info sector values in buffer
	read_datablock(buff,_fsInfoSector);

	/* ...Free cluster count is located in fs info sector */
	*(buff+NEXT_FREE_CLUSTER_LOW1) = clusterValue;
	*(buff+NEXT_FREE_CLUSTER_LOW2) = clusterValue >> 8;
	*(buff+NEXT_FREE_CLUSTER_HIGH1) = clusterValue >> 16;
	*(buff+NEXT_FREE_CLUSTER_HIGH2) = clusterValue >> 24;

	/* ...Write new value to memory*/
	/* ...Check whether the new value was written  */

	if(xmit_datablock(buff, _fsInfoSector)){
		return (buff[NEXT_FREE_CLUSTER_LOW1] )
		       + ((buff[NEXT_FREE_CLUSTER_LOW2]) << 8)
		       + ((buff[NEXT_FREE_CLUSTER_HIGH1]) << 16)
		       + ((buff[NEXT_FREE_CLUSTER_HIGH2]) << 24);
	}
	else{
		return 0;
	}
}

uint32_t update_fsInfo(uint8_t *buff, uint16_t _fsInfoSector, uint32_t nextFreeCluster){

	uint32_t clusterValue;
	//store old fs info sector values in buffer
	while(!read_datablock(buff,_fsInfoSector));

	/* ...Assign value to 32 bit unsigned integer and decrement it by 1 */

	clusterValue = (buff[FREE_CLUSTER_COUNT_LOW1])
	       + ((buff[FREE_CLUSTER_COUNT_LOW2]) << 8)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH1]) << 16)
	       + ((buff[FREE_CLUSTER_COUNT_HIGH2]) << 24) - 1;

	/* ...Write new value to buffer */

	*(buff+FREE_CLUSTER_COUNT_LOW1) = clusterValue;
	*(buff+FREE_CLUSTER_COUNT_LOW2) = clusterValue >> 8;
	*(buff+FREE_CLUSTER_COUNT_HIGH1) = clusterValue >> 16;
	*(buff+FREE_CLUSTER_COUNT_HIGH2) = clusterValue >> 24;

	/* ...Next update free cluster count */
	*(buff+NEXT_FREE_CLUSTER_LOW1) = nextFreeCluster;
	*(buff+NEXT_FREE_CLUSTER_LOW2) = nextFreeCluster >> 8;
	*(buff+NEXT_FREE_CLUSTER_HIGH1) = nextFreeCluster >> 16;
	*(buff+NEXT_FREE_CLUSTER_HIGH2) = nextFreeCluster >> 24;


	if(xmit_datablock(buff, _fsInfoSector)){
		return (buff[NEXT_FREE_CLUSTER_LOW1] )
		       + ((buff[NEXT_FREE_CLUSTER_LOW2]) << 8)
		       + ((buff[NEXT_FREE_CLUSTER_HIGH1]) << 16)
		       + ((buff[NEXT_FREE_CLUSTER_HIGH2]) << 24);
	}
	else{
		return 0;
	}


}

uint32_t readFileSize(uint8_t *buff, char* filename, uint32_t _mstrDir){

	uint8_t directoryCount = 0;

	read_datablock(buff,_mstrDir);

	while(!startsWith(filename,(char*)buff)){
		//Each byte directory is 32 bytes long
		//If Short Filename doesn't match, jump to next directory
		buff = buff + 0x20;
		directoryCount++;
		//Check whether you reached the end of the sector
		if(directoryCount == 16)
			return 0;
	}
	//Find the size of a file
	return *(buff + FILE_SIZE_OFFSET_LOW1)
		   + (*(buff + FILE_SIZE_OFFSET_LOW2) << 8)
		   + (*(buff + FILE_SIZE_OFFSET_HIGH1) << 16)
		   + (*(buff + FILE_SIZE_OFFSET_HIGH2) << 24);

}

uint8_t findSectorToWrite(uint32_t filesize){
	return ((filesize % 4096) + 511)/512;
}

uint32_t writeFileSize(uint8_t *buff, char* filename, uint32_t sizeInBytes, uint32_t _mstrDir){

	uint8_t directoryCount = 0;

	while(!read_datablock(buff,_mstrDir));

	//Each byte directory is 32 bytes long
	//If Short Filename doesn't match, jump to next directory
	while(!startsWith(filename,(char*)buff+directoryCount*0x20)){
		//buff = buff + 0x20;
		//Check whether you reached the end of the sector
		if(++directoryCount == 15)
			return 0;
	}

	*(buff + FILE_SIZE_OFFSET_LOW1+directoryCount*0x20) = sizeInBytes;
	*(buff + FILE_SIZE_OFFSET_LOW2+directoryCount*0x20) = sizeInBytes >> 8;
	*(buff + FILE_SIZE_OFFSET_HIGH1+directoryCount*0x20) = sizeInBytes >> 16;
	*(buff + FILE_SIZE_OFFSET_HIGH2+directoryCount*0x20) = sizeInBytes >> 24;

	while(!xmit_datablock(buff, _mstrDir));
		return (*(buff + FILE_SIZE_OFFSET_LOW1))
		       + (*(buff + FILE_SIZE_OFFSET_LOW2) << 8)
		       + (*(buff + FILE_SIZE_OFFSET_HIGH1) << 16)
		       + (*(buff + FILE_SIZE_OFFSET_HIGH2) << 24);
}

/*uint32_t writeLastSectorOfFile(uint8_t *writeBuff, char* filename, uint32_t _mstrDir, uint16_t _fatSect){

 *
 * FUCKING MALLOC NOT WORKING


	uint32_t fileSize;
	uint32_t cluster;
	void *readBuff = (uint8_t*)malloc(512*sizeof(uint8_t));
	if(readBuff==NULL)
		return 0;

	//find first cluster of file
	//find last cluster of file based on FAT
	cluster = findLastClusterOfFile(filename,readBuff,_fatSect,_mstrDir);
	//find last sector of file based on File size
	fileSize = readFileSize(readBuff,filename,_mstrDir);
	//Check whether the current sector is not the last one (4096-512=3584)
	if((fileSize % 4096) < 3585){
		//write to the file in the current cluster

		 * if size 0         -> write to 0 sector
		 * if size 1-512     -> write to 1 sector
		 * if size 513-1024  -> write to 2 sector
		 * if size 1025-1536 -> write to 3 sector
		 * if size 1037-2048 -> write to 4 sector
		 * if size 1037-2048 -> write to 4 sector
		 * if size 3072-3584 -> write to 7 sector

		xmit_datablock(writeBuff, cluster*8 + (_mstrDir - 16) + ((fileSize % 4096) + 511)/512);

	}
	else{
		//if the current sector is the last one, write to next cluster
		cluster = findNextFreeCluster(readBuff,0x01);
		xmit_datablock(writeBuff, cluster*8 + (_mstrDir - 16));
		//update next free cluster
		allocateNewCluster(readBuff,_fatSect,cluster);
	}
	//update file size
	writeFileSize(writeBuff,filename,fileSize + 512,_mstrDir);

	free(readBuff);

	return cluster;
}*/

/*
 * filesize / 512  = sector count
 * filesize / 4096 = cluster count
 *
 */

uint8_t writeNextSectorOfFile(uint8_t *writeBuff, char* filename, uint32_t *filesize, uint32_t _mstrDir, uint16_t fatSect, uint32_t *cluster, uint8_t *sector){

	//uint32_t fileSize = readFileSize(readBuff,filename,_mstrDir);
	delayMs(1);
	if(!xmit_datablock(writeBuff, (*cluster)*8 + (_mstrDir - 16) + *sector))
		return 0;
	delayMs(1);
	if(!writeFileSize(writeBuff,filename,*filesize += 512,_mstrDir))
		return 0;

	if(++(*sector) > 7){
		delayMs(1);
		*sector = 0;
		*cluster = allocateNewCluster(writeBuff,fatSect,*cluster);
	}
	return 1;
}

void appendTextToTheSD(char* text, char endofLineSymbol, uint16_t *currentSymbolToWrite, uint8_t *writeBuff, char* filename, uint32_t *filesize, uint32_t _mstrDir, uint16_t fatSect, uint32_t *cluster, uint8_t *sector){

	//Lenght of the write buff plus the endOfTheLineSymbol
	uint8_t writeBufferLenght = strlen(text) + 1;
	uint16_t symbolsLeftToWrite = 512  - *currentSymbolToWrite;

	if(symbolsLeftToWrite < writeBufferLenght){

        memcpy(writeBuff + *currentSymbolToWrite, text, symbolsLeftToWrite);
        //Buffer right now is full, write it to sd
        writeNextSectorOfFile(writeBuff,filename,filesize,_mstrDir,fatSect,cluster,sector);
        //write rest of the string to new buff
        strcpy((char*)&writeBuff[0], (text+symbolsLeftToWrite));
        //add end symbol and calculate lenght of owerwritten buffer
        *currentSymbolToWrite = writeBufferLenght - symbolsLeftToWrite;
        *(writeBuff + *currentSymbolToWrite-1) = endofLineSymbol;
	}
	else if(symbolsLeftToWrite == writeBufferLenght){

		memcpy((writeBuff + *currentSymbolToWrite), text, symbolsLeftToWrite-1);
		writeBuff[511] = endofLineSymbol;
		writeNextSectorOfFile(writeBuff,filename,filesize,_mstrDir,fatSect,cluster,sector);
		*currentSymbolToWrite = 0;
	}
	else{
        strcpy((char*)(writeBuff + *currentSymbolToWrite) , text);
        //add endOffLine symbol
        *currentSymbolToWrite +=writeBufferLenght;
        *(writeBuff + *currentSymbolToWrite -1) = endofLineSymbol;
	}
}

uint32_t allocateNewCluster(uint8_t *buff, uint16_t fatSect, uint32_t cluster){

	uint32_t nextFreeCluster = findNextFreeCluster(buff,0x01);

	//Read FAT sector to overwrite old FAT entries
	while(!read_datablock(buff,fatSect+(cluster / 0x80)));

	//Write pointer next cluster in place of 0x0FFFFFFF
	buff[(cluster % 0x80)*4] = nextFreeCluster;
	buff[(cluster % 0x80)*4+1] = nextFreeCluster >> 8;
	buff[(cluster % 0x80)*4+2] = nextFreeCluster >> 16;
	buff[(cluster % 0x80)*4+3] = nextFreeCluster >> 24;

	if((cluster / 0x80) != (nextFreeCluster / 0x80)){
		//If next free cluster is located in the next sector
		//Overwrite the last sector data
		while(!xmit_datablock(buff,fatSect+(cluster / 0x80)));
		//Read next sector
		while(!read_datablock(buff,fatSect+(nextFreeCluster / 0x80)));
		buff[(nextFreeCluster % 0x80)*4] = 0xFF;;
		buff[(nextFreeCluster % 0x80)*4+1] = 0xFF;
		buff[(nextFreeCluster % 0x80)*4+2] = 0xFF;
		buff[(nextFreeCluster % 0x80)*4+3] = 0x0F;
		//Write 0xF0 FF FF FF to new next free cluster
		while(!xmit_datablock(buff,fatSect+(nextFreeCluster / 0x80)));
	}
	else{
		buff[(cluster % 0x80)*4+4] = 0xFF;
		buff[(cluster % 0x80)*4+5] = 0xFF;
		buff[(cluster % 0x80)*4+6] = 0xFF;
		buff[(cluster % 0x80)*4+7] = 0x0F;
		while(!xmit_datablock(buff,fatSect+(cluster / 0x80)));
	}
	//in the place of 0x0FFFFFFF write pointer to last cluster
	//write 0x0FFFFFFF in next free cluster if it is in the same sector


	//else read the next sector and write the 0xF0000000 value

	//Next free cluster is next to the last cluster
	nextFreeCluster++;
	//Put it in while cycle, cause f**k knows why there is an error all the time
	while(update_fsInfo(buff,0x01,nextFreeCluster) != nextFreeCluster);

	return nextFreeCluster-1;
}

uint8_t startsWith(const char *pre, const char *str)
{
    uint8_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}
