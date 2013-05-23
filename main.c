/* PIN MAPPING
 * 
 * RA0 ADC Batteria
 * RA1 Rx2 Bluetooth
 * RB0 Tx2 Bluetooth
 * RB1 MOSI SD
 * RB2 ADC Luminosità
 * RB3 LCD E
 * RA2 Quarzo
 * RA3 Quarzo
 * RB4 LCD RS
 * RA4 CS SD
 * RB5 MISO SD
 * RB7 DHT11
 * RB8 SDA1 I2C
 * RB9 SCL1 I2C
 * RB10 D+ USB Clock ICSP
 * RB11 D- USB Data ICSP
 * RB13 Rx1 GPS
 * RB14 SCK SD
 * RB15 Tx1 GPS
 *
 * - Gestione 3 pulsanti
 * - accesso SD e scrittura FAT FS
 * - Gestione GPS
 * - gestione menù
 * - disposizione dati su LCD
 * - calcolo tramonto e sorgere sole e luna
 * - calcolo altezza con pressione
 * 
 */
/* Timers
 * 
 * Timer1
 *
 * Timer2
 *
 * Timer3
 *
 * Timer4
 *
 * Timer5
 * - timeout DHT11
 *
 */
/*
2 I2C
2 usb
2 gps
2 bluetooth
1 adc batteria
1 adc luminosità fotoresistenza
4 spi per SD
1 DHT11
2 segnali LCD

I2C frequenza minima 400KHz valutare per più
- MCP23008
- Barometro e temperatura
- digital potter (vediamo ok per regolare il contrasto ma per la luminosità serve PWM, magari NE555 e con digital potter si regola il duty)

MCP23008
4 data LCD
1 bluetooth power
3 input pulsanti

*/

#define _PLIB_DISABLE_LEGACY

/** I N C L U D E S **********************************************************/
#include "HardwareProfile.h"
#include "usb_config.h"

#include "USB/usb.h"
#include "USB/usb_device.h"
#include "USB/usb_function_cdc.h"

#include "GenericTypeDefs.h"
#include "Compiler.h"

#include <configwords.h>

#include <gps.h>
#include <lcd.h>
#include <mpl115A2.h>
#include <dht11.h>
//#include <spi.h>
//#include <sdcard.h>

/** V A R I A B L E S ********************************************************/
#pragma udata
char asInBuff[CDC_DATA_IN_EP_SIZE];
char asOutBuff[CDC_DATA_OUT_EP_SIZE];

// Buffer Heap di 128 byte
UINT8 aiBuff[128];
UINT8_BITS bFlags;
UINT8 bPosOut;

#define VERSION "GPS Tracking 0.1\r\n"

/** P R I V A T E  P R O T O T Y P E S ***************************************/
//void USBDeviceTasks(void);
void USBCBSendResume(void);
static __inline void __attribute__((always_inline)) initSystem(void);
static __inline void __attribute__((always_inline)) processData(char);

#pragma code
int main(void)
{
    initSystem();

    #ifdef USB_INTERRUPT
    USBDeviceAttach();
    #endif

    strcpy(asOutBuff,VERSION);
    bPosOut = strlen(asOutBuff);

    while(TRUE)
    {
        #ifdef USB_POLLING
        USBDeviceTasks();
        #endif

        if(USBDeviceState < CONFIGURED_STATE || USBSuspendControl==1)
            continue;
        if(getsUSBUSART(asInBuff, 1)!=0)
            processData(asInBuff[0]);
        // Se la USB è pronta per trasmettere e ci sono dati li scrivo su USB
        // Se arrivano dati ed il buffer è pieno c'è il rischio di perdere i dati
        if(USBUSARTIsTxTrfReady() && bPosOut>0)
        {
            putUSBUSART(asOutBuff, bPosOut);
            bPosOut = 0;
        }
        CDCTxService();
    }
}

static __inline void __attribute__((always_inline)) initSystem()
{
    #if defined(__PIC24FJ64GB002__)
    unsigned int pll_startup_counter = 600;
    CLKDIVbits.PLLEN = 1;
    while(pll_startup_counter--);

    AD1PCFGL = 0xFFFF;
    #elif defined(__32MX250F128B__)
    ANSELA = 0; // all digital
    ANSELB = 0; // all digital

    SYSTEMConfig(GetSystemClock(),SYS_CFG_ALL);
    #endif
    TRISA = 0;
    TRISB = 0b0010000010000000; // RB13 RX1, RB7 input

    LATA = 0;
    LATB = 0;

    #if defined(__PIC24FJ64GB002__)
    #elif defined(__32MX250F128B__)
    SYSKEY = 0x0; // Lock
    // Unlock
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    CFGCONbits.PMDLOCK = 0;
    // Disable peripherical...
    PMD1 = 0xFFFFFFFF;
    PMD2 = 0xFFFFFFFF;
    PMD3 = 0xFFFFFFFF;
    PMD4 = 0xFFFFFFFF;
    //PMD5 = 0xFFFFFFFF;
    PMD5bits.SPI2MD = 1;
    PMD5bits.I2C2MD = 1;
    PMD6 = 0xFFFFFFFF;

    // ... and enable
    PMD4bits.T5MD = 0;

    CFGCONbits.PMDLOCK = 1;
    CFGCONbits.IOLOCK = 0;
    //RPB14Rbits.RPB14R = 5; // OC3 RB14
    RPB15Rbits.RPB15R = 1; // RB15 => TX1
    U1RXRbits.U1RXR = 3; // RX1 => RB13
    CFGCONbits.IOLOCK = 1;
    SYSKEY = 0x0; // Lock
    #endif

    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED | I2C_STOP_IN_IDLE);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, GetPeripheralClock(), 9600);
    UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    // Configure UART1 RX Interrupt
    INTEnable(INT_SOURCE_UART_RX(UART1), INT_ENABLED);
    INTSetVectorPriority(INT_VECTOR_UART(UART1), INT_PRIORITY_LEVEL_3);
    INTSetVectorSubPriority(INT_VECTOR_UART(UART1), INT_SUB_PRIORITY_LEVEL_0);

    // configure for multi-vectored mode
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

    // enable interrupts
    INTEnableInterrupts();

    initI2C();
    mcpInit();

    initGPS();
    lcdInit();
    baroInit();
    /*T3CON = 0x8000;
    PR3 = 1;

    OC3CON=0;
    OC3R=1;
    OC3RS=1;
    OC3CON=0x800E;*/

    //SDCARD_CS = 1;

    //setupSpi();

    //sdCardInit();

    //RPOR3bits.RP7R = 21; // Output compare 4 to RP7 RB7

    bFlags.Val = 0;

    USBDeviceInit();
}

static __inline void __attribute__((always_inline)) processData(char cData)
{
    BOOL bErr = FALSE;

    switch(cData)
    {
        case 't':
        {
            #if 0
            UINT8 /*iTmp,iTmp2,*/iNdx,iNdx2;
            char asBuf[3];

            asBuf[2] = 0;
            /*mcpWrite(IODIR,0xFF);
            mcpWrite(GPPU,0xFF);
            Nop();
            iTmp = mcpRead(GPIO);*/
            for(iNdx=0;iNdx<0xFF;iNdx++)
            {
            convertByteToHex(iNdx,asBuf);
            /*strcpy(asOutBuff,asBuf);
            bPosOut = strlen(asOutBuff);*/
            lcdPutStringXY(0,0,asBuf);
            for(iNdx2=0;iNdx2<0xFF;iNdx2++)
            {
                convertByteToHex(iNdx2,asBuf);
                /*strcpy(asOutBuff,asBuf);
                bPosOut = strlen(asOutBuff);*/
                lcdPutStringXY(3,0,asBuf);
                DelayMs(100);
                }
            }
            #endif
            #if 0
            char sBuff[8];
            float fTemp,fPress;
            getPressTemp(&fTemp,&fPress);
            snprintf(sBuff,sizeof(sBuff),"%f",fTemp);
            lcdPutStringXY(0,0,sBuff);
            snprintf(sBuff,sizeof(sBuff),"%f",fPress);
            lcdPutStringXY(8,0,sBuff);
            #endif

            char sBuff[16];
            //PORTAbits.RA0 = 1;
            INT8 iTemp, iHum;
            if(getTempDHT11(&iTemp,&iHum))
            {
                snprintf(sBuff,sizeof(sBuff),"T:%dC H:%d%%",iTemp,iHum);
                lcdPutStringXY(0,0,sBuff);
            }
            //else
            //    lcdPutStringXY(0,0,"KO");
            //snprintf(asOutBuff,sizeof(asOutBuff),"%d %d %d %d\r\n",aiRes[0],aiRes[1],aiRes[2],aiRes[3]);
            //bPosOut = strlen(asOutBuff);
            //PORTAbits.RA0 = 0;
            break;
        }
        case 'c':
            lcdClear();
            break;
        case 'v':
            strcpy(asOutBuff,VERSION);
            bPosOut = strlen(asOutBuff);
            break;
        case 'x':
            Reset();
            break;
        default:
            bErr = TRUE;
            break;
    }
    if(bErr)
    {
        strcpy(asOutBuff,"E\r\n");
        bPosOut = 3;
        bErr = FALSE;
    }
}

void __ISR(_UART_1_VECTOR, IPL3SOFT) uart1Handler(void)
{
    // Is this an RX interrupt?
    if(INTGetFlag(INT_SOURCE_UART_RX(UART1)))
    {
        // Clear the RX interrupt Flag
        INTClearFlag(INT_SOURCE_UART_RX(UART1));
    }
    // We don't care about TX interrupt
    //if(INTGetFlag(INT_SOURCE_UART_TX(UART1)))
    //    INTClearFlag(INT_SOURCE_UART_TX(UART1));
}
