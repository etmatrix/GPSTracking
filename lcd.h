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

void LCD_Init(void);
void LCD_GotoXY(unsigned char, unsigned char);
void LCD_PutCharXY(unsigned char, unsigned char, unsigned char);
void LCD_PutChar(unsigned char);
void LCD_PutStringXY(unsigned char, unsigned char, char* );
void LCD_PutString(char*);
void LCD_PutCGRAMCharPos(char*, char);
