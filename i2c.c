#define _PLIB_DISABLE_LEGACY

#include <i2c.h>
#include <plib.h>

static BOOL startI2C(BOOL restart);
static BOOL writeOneI2C(UINT8 data);
static void stopI2C(void);

static BOOL startI2C( BOOL restart )
{
    I2C_STATUS status;

    // Send the Start (or Restart) signal
    if(restart)
        I2CRepeatStart(I2C1);
    else
    {
        // Wait for the bus to be idle, then start the transfer
        while(!I2CBusIsIdle(I2C1));
        if(I2CStart(I2C1)!=I2C_SUCCESS)
            return FALSE;
    }
    // Wait for the signal to complete
    do {
        status = I2CGetStatus(I2C1);
    } while(!(status & I2C_START));
    return TRUE;
}

static void stopI2C()
{
    I2C_STATUS status;
    // Send the Stop signal
    I2CStop(I2C1);
    // Wait for the signal to complete
    do {
        status = I2CGetStatus(I2C1);
    }while(!(status & I2C_STOP));
}

static BOOL writeOneI2C(UINT8 data)
{
    // Wait for the transmitter to be ready
    while(I2C1STATbits.TBF);
    // Transmit the byte
    if(I2CSendByte(I2C1, data) == I2C_MASTER_BUS_COLLISION)
        return FALSE;
    // Wait for the transmission to finish
    while(I2C1STATbits.TRSTAT);
    // Verify that the byte was acknowledged
    if(I2C1STATbits.ACKSTAT)
        return FALSE;
    return TRUE;
}

/***********************************************************/
/********* PUBLIC FUNCTION *********************************/
/***********************************************************/

void initI2C(void)
{
    I2CConfigure(I2C1, I2C_ENABLE_SLAVE_CLOCK_STRETCHING|I2C_ENABLE_HIGH_SPEED);
    I2C1BRG = ((GetPeripheralClock()/I2C_CLOCK_FREQ)/2) - 2;
    I2C1CONbits.ON = 1;
}

BOOL writeI2C(UINT8 iAddr, UINT8 *aData, UINT8 iSize)
{
    I2C_7_BIT_ADDRESS addr;
    UINT8 iNdx = 0;
    BOOL bSucc = TRUE;

    if(!startI2C(FALSE))
        return FALSE;
    I2C_FORMAT_7_BIT_ADDRESS(addr, iAddr, I2C_WRITE);
    bSucc = writeOneI2C(I2C_GET_7_BIT_ADDRESS_BYTE(addr));
    while(bSucc && (iNdx < iSize))
    {
        bSucc = writeOneI2C(aData[iNdx]);
        iNdx++;
    }
    // End the transfer (hang here if an error occured)
    stopI2C();
    return bSucc;
}

BOOL readI2C(UINT8 iAddr, UINT8 *aData, UINT8 iSize, UINT8 *aReadData, UINT8 iByteToRead)
{
    I2C_7_BIT_ADDRESS addr;
    UINT8 iNdx = 0;
    BOOL bSucc = TRUE;

    if(!startI2C(FALSE))
        return FALSE;
    I2C_FORMAT_7_BIT_ADDRESS(addr, iAddr, I2C_WRITE);
    bSucc = writeOneI2C(I2C_GET_7_BIT_ADDRESS_BYTE(addr));
    while(bSucc && (iNdx < iSize))
    {
        bSucc = writeOneI2C(aData[iNdx]);
        iNdx++;
    }
    if(bSucc)
    {
        // Send a Repeated Started condition
        if(!startI2C(TRUE))
	    return FALSE;
        // Transmit the address with the READ bit set
        I2C_FORMAT_7_BIT_ADDRESS(addr, iAddr, I2C_READ);
        bSucc = writeOneI2C(I2C_GET_7_BIT_ADDRESS_BYTE(addr));
    }
    // Read the data from the desired address
    if(bSucc)
    {
        for(iNdx=0;iNdx<iByteToRead-1;iNdx++)
        {
            if(I2CReceiverEnable(I2C1, TRUE) == I2C_RECEIVE_OVERFLOW)
            {
                bSucc = FALSE;
                break;
            }
            while(!I2CReceivedDataIsAvailable(I2C1));
            I2CAcknowledgeByte(I2C1, TRUE);
            aReadData[iNdx] = I2CGetByte(I2C1);
            while(!I2CAcknowledgeHasCompleted(I2C1));
        }
        if(bSucc)
        {
            if(I2CReceiverEnable(I2C1, TRUE) == I2C_RECEIVE_OVERFLOW)
                bSucc = FALSE;
            else
            {
                while(!I2CReceivedDataIsAvailable(I2C1));
                I2CAcknowledgeByte(I2C1, FALSE);
                aReadData[iNdx] = I2CGetByte(I2C1);
                while(!I2CAcknowledgeHasCompleted(I2C1));
            }
        }
        if(!bSucc)
            I2C1STATbits.I2COV = 0;
    }
    stopI2C();
    return bSucc;
}
