#include <i2c.h>
#include <mcpi2c.h>

void mcpInit()
{
    mcpWrite(IODIR,0); // all pin output...
    mcpWrite(GPIO,0);  // ... and low
}

void mcpWrite(UINT8 iReg,UINT8 iPort)
{
    UINT8 iTmp[] = {iReg,iPort};

    writeI2C(MCP23_1,iTmp,sizeof(iTmp));
}
UINT8 mcpRead(UINT8 iReg)
{
    UINT8 iTmp = iReg;

    readI2C(MCP23_1,&iTmp,1,&iTmp,1);
    return iTmp;
}
