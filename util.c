#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"

void convertByteToHex(UINT8 bByte,char *sBuf)
{
    UINT8 bTemp = bByte>>4;
    if ( bTemp > 9 ) bTemp+=7;      //jump to letters in ASCII table
    sBuf[0] = bTemp + 0x30;

    bTemp = bByte & 0x0F;
    if ( bTemp > 9 ) bTemp+=7;      //jump to letters in ASCII table
    sBuf[1] = bTemp + 0x30;
}

UINT8 convertHexToByte(char cHi,char cLo)
{
    UINT8 bRet = 0;
    if(cHi>='A' && cHi<='F')
        cHi -= 7;
    bRet = (cHi - 0x30) << 4;
    if(cLo>='A' && cLo<='F')
        cLo -= 7;
    bRet += (cLo - 0x30);
    return bRet;
}

/*// Require the TIMER2 enabled
void delayUs(UINT16 iDelay)
{
    TMR2 = 0;
    PR2 = iDelay;
    IFS0bits.T2IF = 0;
    while(!IFS0bits.T2IF){}
}*/
// Each unit 500 ns 16MHz 62,5 ns * 8 (prescaler 01 1:8) = 500 ns
void delayWith5(const UINT16 iDelay)
{
    T5CON = 0b1000000000010000;
    TMR5 = 0;
    PR5 = iDelay;
    #if defined(__PIC24FJ64GB002__)
    IFS1bits.T5IF = 0;
    while(!IFS1bits.T5IF){}
    #endif
    T5CON = 0;
}
// Each unit 16 us 16MHz 62,5 ns * 256 (prescaler 11 1:256) = 16 us
void delayWith5M(const UINT16 iDelay)
{
    T5CON = 0b1000000000110000;
    TMR5 = 0;
    PR5 = iDelay;
    #if defined(__PIC24FJ64GB002__)
    IFS1bits.T5IF = 0;
    while(!IFS1bits.T5IF){}
    #endif
    T5CON = 0;
}
// Each unit 16 us 16MHz 62,5 ns * 256 (prescaler 11 1:256) = 16 us
void initDelayWith5M(const UINT16 iDelay)
{
    T5CON = 0b1000000000110000;
    TMR5 = 0;
    PR5 = iDelay;
    #if defined(__PIC24FJ64GB002__)
    IFS1bits.T5IF = 0;
    #endif
}

void DelayMs(WORD delay)
{
    unsigned int int_status;
    while( delay-- )
    {
        int_status = INTDisableInterrupts();
        OpenCoreTimer(GetSystemClock() / 2000);
        INTRestoreInterrupts(int_status);
        mCTClearIntFlag();
        while( !mCTGetIntFlag() );
    }
    mCTClearIntFlag();
}

void delayUs(UINT16 iUs)
{
    // CP0Count counts at half the CPU rate
    UINT8 Fcp0 = GetSystemClock() / 1000000 / 2;
    // get start ticks
    UINT32 start = _CP0_GET_COUNT();
    // calculate last tick number for the given number of microseconds
    UINT32 stop = start + iUs * Fcp0;

    // wait till count reaches the stop value
    if (stop > start)
        while (_CP0_GET_COUNT() < stop);
    else
        while (_CP0_GET_COUNT() > start || _CP0_GET_COUNT() < stop);
}
