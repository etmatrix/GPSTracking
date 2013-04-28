#include <i2c.h>

#define BARO_ADDR 0x60

#define PRESS_MSB 0x00
#define PRESS_LSB 0x01
#define TEMP_MSB  0x02
#define TEMP_LSB  0x03
#define A0_MSB    0x04
#define A0_LSB    0x05
#define B1_MSB    0x06
#define B1_LSB    0x07
#define B2_MSB    0x08
#define B2_LSB    0x09
#define C12_MSB   0x0A
#define C12_LSB   0x0B
#define CONVERT   0x12

INT16 iCoeA0, iCoeB1, iCoeB2, iCoeC12;

static INT16 calculatePComp(UINT16,UINT16);

void baroInit()
{
    UINT8 aiResult[8];

    aiResult[0] = A0_MSB;
    readI2C(BARO_ADDR,aiResult,1,aiResult,sizeof(aiResult)/sizeof(UINT8));

    iCoeA0 = (aiResult[0] << 8) | aiResult[1];
    iCoeB1 = (aiResult[2] << 8) | aiResult[3];
    iCoeB2 = (aiResult[4] << 8) | aiResult[5];
    iCoeC12 = ((aiResult[6] << 8) | aiResult[7]) >> 2;
}
void getPressTemp(float *fTemp,float *fPress)
{
    UINT8 aiResult[4];
    UINT16 iTemp;
    INT16 iPComp;

    aiResult[0] = CONVERT;
    aiResult[1] = 0;
    writeI2C(BARO_ADDR,aiResult,2);
    DelayMs(3);
    aiResult[0] = PRESS_MSB;
    readI2C(BARO_ADDR,aiResult,1,aiResult,sizeof(aiResult)/sizeof(UINT8));

    iTemp = ((aiResult[2] << 8) | aiResult[3]) >> 6;
    iPComp = calculatePComp(((aiResult[0] << 8) | aiResult[1]) >> 6,iTemp);

    *fPress = iPComp * ((115.0 - 50.0) / 1023.0) + 50; //((65.0 / 1023.0) * iPComp) + 50.0;     // kPa
    *fTemp = ((float) iTemp - 498.0F) / -5.35F +25.0F; // C
}

static INT16 calculatePComp(UINT16 iPress, UINT16 iTemp)
{
    INT32 c12x2, a1, a1x1, y1, a2x2, PComp;

    c12x2 = (((INT32)iCoeC12) * iTemp) >> 11;  // c12x2 = c12 * Tadc
    a1 = (INT32)iCoeB1 + c12x2;                // a1    = b1  + c12x2
    a1x1 = a1 * iPress;                        // a1x1  = a1  * Padc
    y1 = (((INT32)iCoeA0) << 10) + a1x1;       // y1    = a0  + a1x1
    a2x2 = (((INT32)iCoeB2) * iTemp) >> 1;     // a2x2  = b2  * Tadc
    PComp = (y1 + a2x2) >> 9;                  // PComp = y1  + a2x2
    return (INT16)(PComp/16);
}
