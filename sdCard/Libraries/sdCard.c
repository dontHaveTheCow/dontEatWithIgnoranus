#include "sdCard.h"

uint8_t initializeSD(void){

	/* Declaration des variables ----------------------------- */
	uint8_t i, ocr[4], SDType;


	SDDESELECT();

	//80 clk cycles
	for (i = 10; i; i--) {
		spi_rw(0xff);
	}

	//Toggle cs line down and start sending commands
	/* Initialize sd card in spi mode ---------------------- */
	if (send_cmd(CMD0, 0) == 1) {

		/* Determine what commands/version the card is.  -------------- */
		if (send_cmd(CMD8, 0x1AA) == 1) {
			for (i = 0; i < 4; i++)
				ocr[i] = spi_r();

			/* Verify voltage and check pattern --------------- */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
				/* Activate cards initialization process ------------ */
				while (send_cmd(ACMD41, 0x40000000));

				/* Read OCR ----------------- */
				if (send_cmd(CMD58, 0) == 0) {
					for (i = 0; i < 4; i++)
						ocr[i] = spi_r();
				}
			}
		}
	}
	else {
		SDType = 0;
	}
	release_spi();
	return SDType;
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
	//Find the first cluster of file
	return *(buff + FIRST_FILE_CLUSTER_LOW1)
		   + (*(buff + FIRST_FILE_CLUSTER_LOW2) << 8)
		   + (*(buff + FIRST_FILE_CLUSTER_HIGH1) << 16)
		   + (*(buff + FIRST_FILE_CLUSTER_HIGH2) << 24);
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

uint32_t findLastClusterOfFile(char* filename, uint8_t *buff, uint16_t _fat, uint32_t _mstrDir){

	uint32_t tmpCluster, lastCluster;

	tmpCluster = findFirstClusterOfFile(filename,buff,_mstrDir);
	/* Refer to the comment in findNextClusterOfFile() function */

	//Right now, the buffer is filled with master directory sector
	//When you have found the first cluster, fill the buffer with FAT sector
	read_datablock(buff,_fat);

	while(tmpCluster != 0x0FFFFFFF){
	//while(tmpCluster != 0x00000080){
		lastCluster = tmpCluster;
		tmpCluster = buff[(lastCluster%0x80)*4]
		       	       + ((buff[(lastCluster%0x80)*4+1]) << 8)
		       	       + ((buff[(lastCluster%0x80)*4+2]) << 16)
		       	       + ((buff[(lastCluster%0x80)*4+3]) << 24);

		if(tmpCluster % 0x80 == 0)
			read_datablock(buff, ++_fat);
	}
	return lastCluster;

	/* Just a remainder!
	 * Each FAT sector contains of 128 (0x80) entries
	 * So after each 0x80 entries buffer is filled again with a next sector
	 * */
}


uint32_t findNextFreeCluster(uint8_t *buff, uint16_t _fsInfoSector){

	/* ...Next free cluster is located in fs dginfo sector */

	read_datablock(buff,_fsInfoSector);
	return buff[NEXT_FREE_CLUSTER_LOW1]
	       + ((buff[NEXT_FREE_CLUSTER_LOW2]) << 8)
	       + ((buff[NEXT_FREE_CLUSTER_HIGH1]) << 16)
	       + ((buff[NEXT_FREE_CLUSTER_HIGH2]) << 24);
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

	/* ...Assign value to 32 bit unsigned integer and decrement it by 1 */

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
uint32_t writeFileSize(uint8_t *buff, char* filename, uint32_t sizeInBytes, uint32_t _mstrDir){

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
	*(buff + FILE_SIZE_OFFSET_LOW1) = sizeInBytes;
	*(buff + FILE_SIZE_OFFSET_LOW2) = sizeInBytes >> 8;
	*(buff + FILE_SIZE_OFFSET_HIGH1) = sizeInBytes >> 16;
	*(buff + FILE_SIZE_OFFSET_HIGH2) = sizeInBytes >> 24;

	if(xmit_datablock(buff, _mstrDir)){
		return (*(buff + FILE_SIZE_OFFSET_LOW1))
		       + (*(buff + FILE_SIZE_OFFSET_LOW2) << 8)
		       + (*(buff + FILE_SIZE_OFFSET_HIGH1) << 16)
		       + (*(buff + FILE_SIZE_OFFSET_HIGH2) << 24);
	}
	else{
		return 0;
	}
}

void writeSectorToFile(uint8_t *buff, char* filename, uint32_t _mstrDir, uint16_t fatSect, uint32_t lastClusterOfFile){

	//find first cluster of file

	//find last cluster of file based on FAT

	//find last sector of file based on File size

	//write to the file

	//update file size

	//update FAT if necessary

	//update free cluster size if necessary

	//update next cluster if necessary


}

uint8_t startsWith(const char *pre, const char *str)
{
    uint8_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}






