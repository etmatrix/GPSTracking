#include "GenericTypeDefs.h"
#include "Compiler.h"

#define LCD_RS   PORTBbits.RB4
#define LCD_EN   PORTBbits.RB3

#define ROW1 0x80
#define ROW2 0xC0
#define ROW3 0x94
#define ROW4 0xD4

#define LCD_CLEAR       0x01
#define LCD_RETURN_HOME 0x02

#define CMD 0
#define DATA 1

void lcdInit(void);
void lcdPutCharXY(unsigned char, unsigned char, unsigned char);
void lcdPutChar(unsigned char);
void lcdPutStringXY(unsigned char, unsigned char, char* );
void lcdPutString(char*);
void lcdPutCGRAMCharPos(char*, char);
//void lcdSendCommand(UINT8);
void lcdSendByte(UINT8 iByte,UINT8 iType);

#define lcdSendCmd(x) lcdSendByte(x,CMD)
#define lcdSendData(x) lcdSendByte(x,DATA)

// Puts a charachter on LCD
#define lcdPutChar(x) lcdSendData(x)
#define lcdClear() lcdSendCmd(LCD_CLEAR)
