#include "GenericTypeDefs.h"
#include "Compiler.h"

#define DHT11 PORTBbits.RB7
#define DHT11_D TRISBbits.TRISB7

static UINT8 readByte();

BOOL __inline startDHT11(void)
{
    DHT11_D = 0;
    DHT11 = 0;
    DelayMs(25);
    DHT11 = 1;
    delayUs(30);
    DHT11_D = 1;

    T5CON = 0b1000000001110000;
    IFS0bits.T5IF = 0;
    TMR5 = 0;
    PR5 = 40;
    while(!DHT11 && !IFS0bits.T5IF);
    if(IFS0bits.T5IF)
    {
        T5CON = 0;
        return FALSE;
    }
    else
    {
        TMR5 = 0;
        while(DHT11 && !IFS0bits.T5IF);
        T5CON = 0;
        if(IFS0bits.T5IF)
            return FALSE;
        else
            return TRUE;
    }
}

static UINT8 readByte()
{
    UINT8 iNdx, iByte;

    iByte = 0;
    DHT11_D = 1;
    for(iNdx=0;iNdx<8;iNdx++)
    {
        while(!DHT11);
        delayUs(40);
        if(DHT11)
            iByte |= 1<<(7-iNdx);
        TMR5 = 0;
        while(DHT11 && !IFS0bits.T5IF);
    }
    return iByte;
}

BOOL getTempDHT11(INT8 *iTemp,INT8 *iHum)
{
    UINT8 aiResult[5];
    UINT8 iNdx;

    T5CON = 0b1000000001110000;
    IFS0bits.T5IF = 0;
    TMR5 = 0;
    PR5 = 40;

    for(iNdx=0;iNdx<sizeof(aiResult);iNdx++)
        aiResult[iNdx] = readByte();
    T5CON = 0;
    if(aiResult[4] == ((aiResult[0] + aiResult[1] + aiResult[2] + aiResult[3]) & 0xFF))
    {
        // decimal are ignored
        *iHum = aiResult[0];
        *iTemp = aiResult[2];
        return TRUE;
    }
    else
        return FALSE;
}
