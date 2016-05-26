#ifndef SDCARD_LIBRARY
#define SDCARD_LIBRARY

//These are the Includes
#include <SPI1.h>
#include <string.h>

//These are the Define statements
#define SDSELECT()      GPIOB->BRR = (1<<1)  // pin low, MMC CS = L
#define SDDESELECT()    GPIOB->BSRR = (1<<1) // pin high,MMC CS = H

#define DATA_TOKEN		0xFE				 /* Single/multiple block read or single-block write*/
#define SECTOR_OFFSET	0x2000				 /* Sector counting starts from offset 0x2000*/

/* Definitions for SDC command --------------------------------------------*/
#define CMD0    (0x40+0)    /* GO_IDLE_STATE */
#define CMD1    (0x40+1)    /* SEND_OP_COND (MMC) */
#define ACMD41  (0xC0+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (0x40+8)    /* SEND_IF_COND */
#define CMD9    (0x40+9)    /* SEND_CSD */
#define CMD10   (0x40+10)   /* SEND_CID */
#define CMD12   (0x40+12)   /* STOP_TRANSMISSION */
#define ACMD13  (0xC0+13)   /* SD_STATUS (SDC) */
#define CMD16   (0x40+16)   /* SET_BLOCKLEN */
#define CMD17   (0x40+17)   /* READ_SINGLE_BLOCK */
#define CMD18   (0x40+18)   /* READ_MULTIPLE_BLOCK */
#define CMD23   (0x40+23)   /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0xC0+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (0x40+24)   /* WRITE_BLOCK */
#define CMD25   (0x40+25)   /* WRITE_MULTIPLE_BLOCK */
#define CMD55   (0x40+55)   /* APP_CMD */
#define CMD58   (0x40+58)   /* READ_OCR -> 0x7A */

/* Definitions for FAT system (Offsets)--------------------------------*/
#define SECTORS_PER_FAT_1 0x24
#define SECTORS_PER_FAT_2 0x25
#define SECTORS_PER_FAT_3 0x26
#define SECTORS_PER_FAT_4 0x27
#define NUMBER_OF_FATS 0x10
#define RESERVED_SECTORS_1 0x0E
#define RESERVED_SECTORS_2 0x0F
#define FIRST_FILE_CLUSTER_HIGH1 0x14
#define FIRST_FILE_CLUSTER_HIGH2 0x15
#define FIRST_FILE_CLUSTER_LOW1 0x1A
#define FIRST_FILE_CLUSTER_LOW2 0x1B
#define FS_INFO_SECTOR_HIGH 0x2F
#define FS_INFO_SECTOR_LOW 0x30
#define FREE_CLUSTER_COUNT_LOW1 0x1E8
#define FREE_CLUSTER_COUNT_LOW2 0x1E9
#define FREE_CLUSTER_COUNT_HIGH1 0x1EA
#define FREE_CLUSTER_COUNT_HIGH2 0x1EB
#define NEXT_FREE_CLUSTER_LOW1 0x1EC
#define NEXT_FREE_CLUSTER_LOW2 0x1ED
#define NEXT_FREE_CLUSTER_HIGH1 0x1EE
#define NEXT_FREE_CLUSTER_HIGH2 0x1EF
#define FILE_SIZE_OFFSET_LOW1 0x1C
#define FILE_SIZE_OFFSET_LOW2 0x1D
#define FILE_SIZE_OFFSET_HIGH1 0x1E
#define FILE_SIZE_OFFSET_HIGH2 0x1F

//SD layer routines
uint8_t initializeSD(void);
uint8_t send_cmd(uint8_t cmd, uint32_t arg);
uint8_t goToIdleState(void);

//FAT layer functions
uint8_t xmit_datablock(uint8_t *buff, uint32_t sector);
uint8_t read_datablock(uint8_t *buff, uint32_t sector);

void findDetailsOfFAT(uint8_t *buff, uint16_t* _fat, uint32_t* _mstrDir, uint16_t *_fsInfoSector);

/*--FAT and master directory functions--*/
uint32_t findFirstClusterOfFile(char* filename, uint8_t *buff, uint32_t _mstrDir);
uint32_t findNextClusterOfFile(uint32_t currentCluster,uint8_t *buff, uint16_t _fat);

/*--FS  functions--*/
uint32_t findNextFreeCluster(uint8_t *buff, uint16_t _fsInfoSector);
uint32_t findFreeClusterCount(uint8_t *buff, uint16_t _fsInfoSector);
uint32_t decrementFreeClusterCount(uint8_t *buff, uint16_t _fsInfoSector);
uint32_t incrementFreeClusterCount(uint8_t *buff, uint16_t _fsInfoSector);
uint32_t changeNextFreeCluster(uint8_t *buff, uint16_t _fsInfoSector, uint32_t clusterValue);

/*--File interaction functions--*/
void writeSectorToFile(uint8_t *buff, char* filename, uint32_t _mstrDir);
uint32_t readFileSize(uint8_t *buff, char* filename, uint32_t _mstrDir);
uint32_t writeFileSize(uint8_t *buff, char* filename, uint32_t sizeInBytes, uint32_t _mstrDir);

//String functions
uint8_t startsWith(const char *pre, const char *str);

#endif
