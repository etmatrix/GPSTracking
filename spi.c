#include <spi.h>

void __inline setupSpi()
{
    // Assign SPI with RP
    RPOR0bits.RP1R = 8;  // SPI CLKO to Pin 5 / RP1 RB1
    RPOR6bits.RP13R = 7;  // SPI SDO to Pin 24 / RP13 RB13
    RPINR20bits.SDI1R = 3;  // SPI SDI to Pin 7 / RP3 RB3

    // Init SPI module
    SPI1CON1 = 0x013B;
    SPI1CON2 = 0x0000;
    SPI1STAT = 0x8000;
}

UINT8 SpiWrite(const UINT8 Data)
{
    while(SPI1STATbits.SPITBF){}
    // Send the data byte
    SPI1BUF = Data;
    // Wait for a data byte reception
    while(!SPI1STATbits.SPIRBF){}
    return SPI1BUF;
}

// Fa anche la read
/*BYTE SpiWrite(BYTE Data)
{
    BYTE Tmp;

    //Controllo che non ci sia in corso una trasmissione.
    while(SPI1STATbits.SPITBF);
    //Vuoto il buffer di lettura.
    Tmp = SPI1BUF;
   //Scrivo il dato da mandare
    SPI1BUF = Data;
   //Aspetto che venga mandato
    while(SPI1STATbits.SPITBF);
   //Aspetto che sia pronto il dato in lettura.
    while(!SPI1STATbits.SPIRBF);
 
    //Restituisco il valore letto.
    return SPI1BUF;
}*/
