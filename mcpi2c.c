#include <i2c.h>
#include <mcpi2c.h>

void mcpInit()
{
    UINT8 iTmp[] = {IODIR,0};
    writeI2C(MCP23_1,iTmp,sizeof(iTmp)); // all pin output
    mcpWrite(0); // all pin low
}

void mcpWrite(UINT8 iPort)
{
    UINT8 iTmp[] = {GPIO,iPort};

    writeI2C(MCP23_1,iTmp,sizeof(iTmp));
}
