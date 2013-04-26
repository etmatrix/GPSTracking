//#define TOUCH_EN bFlags.bits.b3
#define TOUCH_CAL bFlags.bits.b3

void convertByteToHex(UINT8,char *);
UINT8 convertHexToByte(char,char);
void delayUs(UINT16);
void delayWith5(const UINT16);
void delayWith5M(const UINT16);
void initDelayWith5M(const UINT16 iDelay);
void __inline restoreT1(void);
void DelayMs(WORD);
void delayUs(UINT16 iUs);
