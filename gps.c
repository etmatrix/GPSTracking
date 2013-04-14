#include <plib.h>
#include <gps.h>

__inline void __attribute__((always_inline)) initGPS()
{
    unsigned char data;

    data = 'a';

    if (UARTTransmitterIsReady(UART1))
    {
        UARTSendDataByte(UART1, data);
    }
}

