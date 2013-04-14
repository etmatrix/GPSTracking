#include <sdcard.h>
#include <spi.h>
#include <util.h>

static UINT8 sdSendCmd(UINT8 iCmd,UINT32 iArg);
static BOOL sdWaitReady(void);
static BOOL sdEnable(void);

#define BLOCKSIZE 512

#pragma udata
static SDCard sd;

#pragma code
void sdCardInit()
{
    UINT8 cmd, ocr[4];
    UINT8 iNdx;

    SPI1STATbits.SPIEN = 0;
    SPI1CON1bits.PPRE = 0; // Switch clock to 125KHz
    SPI1STATbits.SPIEN = 1;

    // Switch to SPI MODE at least 74 Clocks with CS and CMD high
    for(iNdx=0;iNdx<10;iNdx++)
        SD_Clock(); /* 80 dummy clocks */

    sd.type = 0;
    if(sdSendCmd(CMD0, 0)==1) /* Enter Idle state */
    {
        initDelayWith5M(62500);                                 /* Initialization timeout of 1000 msec */
        if (sdSendCmd(CMD8, 0x1AA) == 1)                        /* SDv2? */
        {
            for(iNdx=0;iNdx<4;iNdx++)
                ocr[iNdx] = SD_Read();                          /* Get trailing return value of R7 resp */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA)               /* The card can work at vdd range of 2.7-3.6V */
            {				
                while (!IFS1bits.T5IF && sdSendCmd(ACMD41, 0x40000000)); /* Wait for leaving idle state (ACMD41 with HCS bit) */
                if (!IFS1bits.T5IF && sdSendCmd(CMD58, 0) == 0)          /* Check CCS bit in the OCR */
                {
                    for(iNdx=0;iNdx<4;iNdx++)
                        ocr[iNdx] = SD_Read();
                    sd.type = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;	 /* SDv2 */
                }
            }
        }
        else
        {							/* SDv1 or MMCv3 */
            if (sdSendCmd(ACMD41, 0) <= 1)
            {
                sd.type = CT_SD1;                                    /* SDv1 */
                cmd = ACMD41;	
                while (!IFS1bits.T5IF && sdSendCmd(cmd, 0));		/* Wait for leaving idle state */
                if (IFS1bits.T5IF || sdSendCmd(CMD16, 512) != 0)	/* Set read/write block length to 512 */
                    sd.type = 0;
            }
        }
    }
    T5CON = 0;
    SD_Disable();
    SPI1STATbits.SPIEN = 0;
    SPI1CON1bits.PPRE = 3; // Switch clock to 8MHz
    SPI1STATbits.SPIEN = 1;
}

static UINT8 sdSendCmd(UINT8 iCmd,UINT32 iArg)
{
    UINT8 iTemp, iRet;

    if (iCmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
        iCmd &= 0x7F;
        iRet = sdSendCmd(CMD55, 0);
        if (iRet > 1)
            return iRet;
    }

    SD_Disable();
    if (!sdEnable())
        return 0xFF;

    /* Send command packet */
    SpiWrite(0x40 | iCmd);            /* Start + Command index */
    SpiWrite((UINT8)(iArg >> 24));    /* Argument[31..24] */
    SpiWrite((UINT8)(iArg >> 16));    /* Argument[23..16] */
    SpiWrite((UINT8)(iArg >> 8));     /* Argument[15..8] */
    SpiWrite((UINT8)iArg);            /* Argument[7..0] */
    iTemp = 0x01;                     /* Dummy CRC + Stop */
    if (iCmd == CMD0)
        iTemp = 0x95;                 /* Valid CRC for CMD0(0) + Stop */
    if (iCmd == CMD8)
        iTemp = 0x87;                 /* Valid CRC for CMD8(0x1AA) + Stop */
    SpiWrite(iTemp);

    /* Receive command response */
    if (iCmd == CMD12)
        SD_Read();                    /* Skip a stuff byte on stop to read */
    iTemp = 10;                       /* Wait for a valid response in timeout of 10 attempts */
    do {
        iRet = SD_Read();
    } while((iRet & 0x80) && --iTemp);

    return iRet;                      /* Return with the response value */
}
static BOOL sdWaitReady()
{
    UINT8 iTemp;

    initDelayWith5M(31250); // 500 ms
    do {
        iTemp = SD_Read();
    } while(iTemp != 0xFF && !IFS1bits.T5IF);
    T5CON = 0;
    return (iTemp == 0xFF) ? TRUE : FALSE;
}
/* 1:Successful, 0:Timeout */
static BOOL sdEnable()
{
    SD_Enable();

    if (sdWaitReady())
        return TRUE;
    SD_Disable();
    return FALSE;
}

static BOOL readDataBlock(UINT8 *aiBuff,UINT16 iCount)
{
    UINT8 iTemp;
    UINT16 iNdx;

    initDelayWith5M(6250); // Timeout 100ms
    do {
        iTemp = SD_Read();
    } while ((iTemp == 0xFF) && !IFS1bits.T5IF);
    T5CON = 0;

    // If not valid data token, retutn with error
    if(iTemp != 0xFE)
        return FALSE;

    for(iNdx=0;iNdx<iCount;iNdx++)
        aiBuff[iNdx] = SD_Read();
    /* Discard CRC */
    SD_Read();
    SD_Read();

    return TRUE;
}

static BOOL writeDataBlock(const UINT8 *aiBuff,UINT8 iToken)
{
    UINT16 iNdx;

    if(!sdWaitReady())
        return FALSE;

    SpiWrite(iToken);
    if (iToken != 0xFD)
    {
        iNdx = BLOCKSIZE;
        do {
            SpiWrite(*aiBuff++);
        }while(iNdx--);

        SD_Read(); /* CRC (Dummy) */
        SD_Read();
        /* Receive a data response */
        if ((SD_Read() & 0x1F) != 0x05)
            return FALSE;
    }
    return TRUE;
}

/* ****************** */
/* External Functions */
/* ****************** */

void sdCap(BYTE *buff)
{
    UINT8 n;

    if (sdSendCmd(CMD58, 0) == 0) {	/* READ_OCR */
        for (n = 0; n < 4; n++)
            *((BYTE*)buff+n) = SD_Read();
    }
    SD_Disable();
}

BOOL sdDiskRead(UINT8 *aiBuff,UINT32 iSector,UINT8 iCount)
{
    if (!(sd.type & CT_BLOCK))
        iSector *= BLOCKSIZE;	/* Convert to byte address if needed */

    if(iCount == 1) /* Single block read */
    {		
        if ((sdSendCmd(CMD17, iSector) == 0) && readDataBlock(aiBuff, BLOCKSIZE))
            iCount = 0;
    }
    else
    {
        if(sdSendCmd(CMD18, iSector) == 0)/* READ_MULTIPLE_BLOCK */
        {
            do {
                if (!readDataBlock(aiBuff, BLOCKSIZE))
                    break;
                aiBuff += 512;
            } while (--iCount);
            sdSendCmd(CMD12, 0); /* STOP_TRANSMISSION */
        }
    }
    SD_Disable();
    return iCount;
}
BOOL sdDiskWrite(const UINT8 *aiBuff,UINT32 iSector,UINT8 iCount)
{
    if (!(sd.type & CT_BLOCK))
        iSector *= 512;	/* Convert to byte address if needed */

    if (iCount == 1) {		/* Single block write */
        if ((sdSendCmd(CMD24, iSector) == 0) && writeDataBlock(aiBuff, 0xFE))
            iCount = 0;
    }
    else
    {
        if (sd.type & CT_SDC)
            sdSendCmd(ACMD23, iCount);
        if (sdSendCmd(CMD25, iSector) == 0)
        {
            do {
                if (!writeDataBlock(aiBuff, 0xFC))
                    break;
                aiBuff += 512;
            } while (--iCount);
            if (!writeDataBlock(0, 0xFD))	/* STOP_TRAN token */
                iCount = 1;
        }
    }
    SD_Disable();
    return iCount;
}
