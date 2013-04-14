#ifndef SDCARD_H
#define	SDCARD_H

#include "GenericTypeDefs.h"

#define SDCARD_CS PORTAbits.RA2

#define SD_Read()    SpiWrite(0xFF)
#define SD_Clock()   SpiWrite(0xFF)
#define SD_Disable() SDCARD_CS = 1; SD_Clock()
#define SD_Enable()  SDCARD_CS = 0; SD_Clock()

/* Definitions for MMC/SDC command */
#define CMD0   (0)         /* GO_IDLE_STATE */
#define CMD1   (1)         /* SEND_OP_COND */
#define ACMD41 (41|0x80)   /* SEND_OP_COND (SDC) */
#define CMD8   (8)         /* SEND_IF_COND */
#define CMD9   (9)         /* SEND_CSD */
#define CMD10  (10)        /* SEND_CID */
#define CMD12  (12)        /* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)   /* SD_STATUS (SDC) */
#define CMD16  (16)        /* SET_BLOCKLEN */
#define CMD17  (17)        /* READ_SINGLE_BLOCK */
#define CMD18  (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)        /* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)        /* WRITE_BLOCK */
#define CMD25  (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)        /* SEND_OP_COND (ACMD) */
#define CMD55  (55)        /* APP_CMD */
#define CMD58  (58)        /* READ_OCR */

#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */

typedef struct SDCard_t
{
    UINT8 type;
    UINT32 size;
}SDCard;

void sdCardInit(void);
void sdCap(BYTE *buff);
BOOL sdDiskRead(UINT8 *,UINT32,UINT8);
BOOL sdDiskWrite(const UINT8 *,UINT32,UINT8);

#endif	/* SDCARD_H */
