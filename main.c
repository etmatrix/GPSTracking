/* PIN MAPPING
 * 
 * RA0 ADC Batteria
 * RA1 Rx2 Bluetooth
 * RB0 Tx2 Bluetooth
 * RB1 MOSI SD
 * RB2 ADC Luminosit�
 * RB3 Pulsante 1
 * RA2 Quarzo
 * RA3 Quarzo
 * RB4 Pulsante 2
 * RA4 DHT11
 * RB5 MISO SD
 * RB7 Pulsante 3
 * RB8 SDA1 I2C
 * RB9 SCL1 I2C
 * RB10 D+ USB Clock ICSP
 * RB11 D- USB Data ICSP
 * RB13 Rx1 GPS
 * RB14 SCK SD
 * RB15 Tx1 GPS
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
 * - delay in util
 *
 */
/*
2 I2C
3 bottoni
2 usb
2 gps
2 bluetooth
1 adc batteria
1 adc luminosit� fotoresistenza
3 spi per SD senza CS c'� solo lui
1 DHT11

I2C frequenza minima 400KHz valutare per pi�
- MCP23
- Barometro e temperatura
- digital potter

MCP
6 LCD
1 bluetooth power
 */

/** I N C L U D E S **********************************************************/
#include "HardwareProfile.h"
#include "usb_config.h"

#include "USB/usb.h"
#include "USB/usb_device.h"
#include "USB/usb_function_cdc.h"

#include "GenericTypeDefs.h"
#include "Compiler.h"

#include <gps.h>
#include <i2c.h>
//#include <spi.h>
//#include <sdcard.h>

#if defined(__PIC24FJ64GB002__)
    _CONFIG1(WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & ICS_PGx2 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
    _CONFIG2(POSCMOD_NONE & I2C1SEL_PRI & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_FRCPLL & PLL96MHZ_OFF & PLLDIV_NODIV & IESO_ON)
    _CONFIG3(WPFP_WPFP0 & SOSCSEL_IO & WUTSEL_LEG & WPDIS_WPDIS & WPCFG_WPCFGDIS & WPEND_WPENDMEM)
    _CONFIG4(DSWDTPS_DSWDTPS3 & DSWDTOSC_LPRC & RTCOSC_SOSC & DSBOREN_OFF & DSWDTEN_OFF)
#elif defined(__32MX250F128B__)
    #pragma config PMDL1WAY = ON            // Peripheral Module Disable Configuration (Allow only one reconfiguration)
    #pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow only one reconfiguration)
    #pragma config FUSBIDIO = OFF           // USB USID Selection (Controlled by Port Function)
    #pragma config FVBUSONIO = OFF          // USB VBUS ON Selection (Controlled by Port Function)

    // external 20  / 5              * 20             / 2 = 40
    // FPLLIDIV = DIV_5, FPLLMUL = MUL_20, FPLLODIV = DIV_2, FWDTEN = OFF
    // DEVCFG2
    #pragma config FPLLIDIV = DIV_5         // PLL Input Divider (5x Divider)
    #pragma config FPLLMUL = MUL_20         // PLL Multiplier (20x Multiplier)
    #pragma config UPLLIDIV = DIV_5         // USB PLL Input Divider (5x Divider)
    #pragma config UPLLEN = ON              // USB PLL Enable (Enabled)
    #pragma config FPLLODIV = DIV_2         // System PLL Output Clock Divider (PLL Divide by 2)

    // DEVCFG1
    #pragma config FNOSC = PRIPLL           // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
    #pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
    #pragma config IESO = ON                // Internal/External Switch Over (Enabled)
    #pragma config POSCMOD = HS             // Primary Oscillator Configuration (HS osc mode)
    #pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
    #pragma config FPBDIV = DIV_1           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/1)
    #pragma config FCKSM = CSECME           // Clock Switching and Monitor Selection (Clock Switch Enable, FSCM Enabled)
    #pragma config WDTPS = PS1              // Watchdog Timer Postscaler (1:1)
    #pragma config WINDIS = OFF             // Watchdog Timer Window Enable (Watchdog Timer is in Non-Window Mode)
    #pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
    #pragma config FWDTWINSZ = WISZ_25      // Watchdog Timer Window Size (Window Size is 25%)

    // DEVCFG0
    #pragma config JTAGEN = ON              // JTAG Enable (JTAG Port Enabled)
    #pragma config ICESEL = ICS_PGx1        // ICE/ICD Comm Channel Select (Communicate on PGEC1/PGED1)
    #pragma config PWP = OFF                // Program Flash Write Protect (Disable)
    #pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
    #pragma config CP = OFF                 // Code Protect (Protection Disabled)
#endif

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
        // Se la USB � pronta per trasmettere e ci sono dati li scrivo su USB
        // Se arrivano dati ed il buffer � pieno c'� il rischio di perdere i dati
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
    TRISB = 0b0010000000000000; // RB13 RX1 input

    LATA = 0;
    LATB = 0;

    #if defined(__PIC24FJ64GB002__)
    #elif defined(__32MX250F128B__)
    SYSKEY = 0x0; // Lock
    // Unlock
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    // TODO guardare il PMD per disabilitare le periferiche non usate
    CFGCONbits.IOLOCK = 0;
    //RPB14Rbits.RPB14R = 5; // OC3 RB14
    RPB15Rbits.RPB15R = 1; // RB15 => TX1
    U1RXRbits.U1RXR = 3; // RX1 => RB13
    CFGCONbits.IOLOCK = 1;
    SYSKEY = 0x0; // Lock
    #endif

    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, GetPeripheralClock(), 9600);
    UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    initI2C();

    {
        UINT8 i[] = {0x1,0x2};
    writeI2C(0x11,i,2);
    }
    initGPS();

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

// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Call back that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/
void USBCBSuspend(void)
{
}


/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *					
 *					This call back is invoked when a wakeup from USB suspend 
 *					is detected.
 *
 * Note:            None
 *****************************************************************************/
void USBCBWakeFromSuspend(void)
{
    // If clock switching or other power savings measures were taken when
    // executing the USBCBSuspend() function, now would be a good time to
    // switch back to normal full power run mode conditions.  The host allows
    // a few milliseconds of wakeup time, after which the device must be
    // fully back to normal, and capable of receiving and processing USB
    // packets.  In order to do this, the USB module must receive proper
    // clocking (IE: 48MHz clock must be available to SIE for full speed USB
    // operation).
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/
void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/
void USBCBErrorHandler(void)
{
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

    // Typically, user firmware does not need to do anything special
    // if a USB error occurs.  For example, if the host sends an OUT
    // packet to your device, but the packet gets corrupted (ex:
    // because of a bad connection, or the user unplugs the
    // USB cable during the transmission) this will typically set
    // one or more USB error interrupt flags.  Nothing specific
    // needs to be done however, since the SIE will automatically
    // send a "NAK" packet to the host.  In response to this, the
    // host will normally retry to send the packet again, and no
    // data loss occurs.  The system will typically recover
    // automatically, without the need for application firmware
    // intervention.

    // Nevertheless, this callback function is provided, such as
    // for debugging purposes.
}

/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 * 					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.  For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.  This
 *					is not a standard USB chapter 9 request, and 
 *					therefore not handled by usb_device.c.  Instead
 *					this request should be handled by class specific 
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *******************************************************************/
void USBCBCheckOtherReq(void)
{
    USBCheckCDCRequest();
}//end


/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *******************************************************************/
void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}//end


/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This 
 *					callback function should initialize the endpoints 
 *					for the device's usage according to the current 
 *					configuration.
 *
 * Note:            None
 *******************************************************************/
void USBCBInitEP(void)
{
    CDCInitEP();
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *					
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes 
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function will only be able to wake up the host if
 *                  all of the below are true:
 *					
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET 
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.   
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior, 
 *                  as a USB device that has not been armed to perform remote 
 *                  wakeup must not drive remote wakeup signalling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *                  
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are 
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex: 
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup. 
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in 
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
void USBCBSendResume(void)
{
    static UINT16 delay_count;
    
    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager 
    //properties page for the USB device, power management tab, the 
    //"Allow this device to bring the computer out of standby." checkbox 
    //should be checked).
    if(USBGetRemoteWakeupStatus() == TRUE) 
    {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signalling.
        if(USBIsBusSuspended() == TRUE)
        {
            USBMaskInterrupts();
            
            //Clock switch to settings consistent with normal USB operation.
            USBCBWakeFromSuspend();
            USBSuspendControl = 0; 
            USBBusIsSuspended = FALSE;  //So we don't execute this code again, 
                                        //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signalling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at 
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
            //5ms+ total delay since start of idle).
            delay_count = 3600U;        
            do
            {
                delay_count--;
            }while(delay_count);
            
            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1;       // Start RESUME signaling
            delay_count = 1800U;        // Set RESUME line for 1-13 ms
            do
            {
                delay_count--;
            }while(delay_count);
            USBResumeControl = 0;       //Finished driving resume signalling

            USBUnmaskInterrupts();
        }
    }
}


/*******************************************************************
 * Function:        void USBCBEP0DataReceived(void)
 *
 * PreCondition:    ENABLE_EP0_DATA_RECEIVED_CALLBACK must be
 *                  defined already (in usb_config.h)
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called whenever a EP0 data
 *                  packet is received.  This gives the user (and
 *                  thus the various class examples a way to get
 *                  data that is received via the control endpoint.
 *                  This function needs to be used in conjunction
 *                  with the USBCBCheckOtherReq() function since 
 *                  the USBCBCheckOtherReq() function is the apps
 *                  method for getting the initial control transfer
 *                  before the data arrives.
 *
 * Note:            None
 *******************************************************************/
#if defined(ENABLE_EP0_DATA_RECEIVED_CALLBACK)
void USBCBEP0DataReceived(void)
{
}
#endif

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        USB_EVENT event, void *pdata, UINT16 size)
 *
 * PreCondition:    None
 *
 * Input:           USB_EVENT event - the type of event
 *                  void *pdata - pointer to the event data
 *                  UINT16 size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/

BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, UINT16 size)
{
    switch(event)
    {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED:
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT
            //      endpoints).
            break;
        default:
            break;
    }
    return TRUE;
}

/** EOF main.c *************************************************/