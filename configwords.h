#ifndef CONFIGWORDS_H
#define	CONFIGWORDS_H

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
    #pragma config JTAGEN = OFF             // JTAG Disable for use RB5, RB7, RB8, RB9
    #pragma config ICESEL = ICS_PGx1        // ICE/ICD Comm Channel Select (Communicate on PGEC1/PGED1)
    #pragma config PWP = OFF                // Program Flash Write Protect (Disable)
    #pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
    #pragma config CP = OFF                 // Code Protect (Protection Disabled)
#endif

#endif	/* CONFIGWORDS_H */
