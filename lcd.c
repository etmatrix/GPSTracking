#include "lcd.h"
#include <mcpi2c.h>
#include <util.h>

static void lcdClockEn(void);
static void lcdGotoXY(unsigned char, unsigned char);

// LCD Function
void lcdInit(void)
{
    UINT8 iNdx,iTmp;

    DelayMs(5);

    // Reset Sequence
    LCD_EN = 0;
    LCD_RS = 0;

    // Enter in 4 bit mode
    iTmp = mcpRead(GPIO);
    iTmp = (iTmp & 0xF0) | 0x03;
    mcpWrite(GPIO,iTmp);
    for(iNdx=1;iNdx<=3;iNdx++)
    {
        lcdClockEn();
        DelayMs(5);
    }

    iTmp = (iTmp & 0xF0) | 0x02;
    mcpWrite(GPIO,iTmp);
    lcdClockEn();

    lcdSendCmd(0x28); // 4-bit mode, 2 lines display, font 5x8 pixels
    lcdSendCmd(0x06); // Increment, no shift

    // Display ON, Cursor OFF, Blink OFF
    lcdSendCmd(0x0C);
    lcdClear();
    lcdSendCmd(0x03);
}

// Moves the cursor to ("row", "column") position
static void lcdGotoXY(UINT8 x, UINT8 y)
{
    UINT8 chByte;
    if(y == 0)
        chByte = ROW1;
    else if(y ==1 )
        chByte = ROW2;
    else if(y ==2 )
        chByte = ROW3;
    else
        chByte = ROW4;

    chByte+=x;
    lcdSendCmd(chByte);
}

// Puts a charachter on LCD starting from ("row", "column") position
void lcdPutCharXY(UINT8 x, UINT8 y, UINT8 charachter)
{
    lcdGotoXY(x,y);
    lcdPutChar(charachter);
}

// Puts a string on LCD starting from ("row", "column") position
void lcdPutStringXY(UINT8 x, UINT8 y, char* string)
{
    lcdGotoXY(x,y);
    lcdPutString(string);
}

// Puts a string on LCD
void lcdPutString(char* string)
{
    UINT8 iNdx;

    for(iNdx=0;string[iNdx]!='\0';iNdx++)
        lcdPutChar(string[iNdx]);
}

static void lcdClockEn(void)
{
    LCD_EN = 1;
    Nop();
    LCD_EN = 0;
}

// Sends command to LCD
/*void lcdSendCommand(UINT8 Command)
{
    LCD_RS = 0;
    lcdSendByte(Command);
    DelayMs(1);
}*/

// Sends data to LCD
/*static void lcdSendData(UINT8 Data)
{
    LCD_RS = 1;
    lcdSendByte(Data);
}*/

// Sends byte to LCD
void lcdSendByte(UINT8 iByte,UINT8 iType)
{
    UINT8 iTmp;

    LCD_RS = iType;
    // MSbit 8-4
    iTmp = mcpRead(GPIO);
    iTmp = (iTmp & 0xF0) | ((iByte & 0xF0)>>4);
    mcpWrite(GPIO,iTmp);
    lcdClockEn();

    // LSbit 4-0
    iTmp = mcpRead(GPIO);
    iTmp = (iTmp & 0xF0) | (iByte & 0x0F);
    mcpWrite(GPIO,iTmp);
    lcdClockEn();
    delayUs(50);
}

void lcdPutCGRAMCharPos(char* GraphicChar, char DisplayPosition)
{
    UINT8 iNdx;

    lcdSendCmd(0x40+8*DisplayPosition);
    for(iNdx=0;iNdx<8;iNdx++)
        lcdPutChar(GraphicChar[iNdx]);
}

