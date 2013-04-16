#ifndef MCPI2C_H
#define	MCPI2C_H

#include "GenericTypeDefs.h"
#include "Compiler.h"

#define MCP23_1 0x20

#define IODIR 0x00
#define GPIO  0x09

void mcpInit(void);
void mcpWrite(UINT8 data);
UINT8 mcpRead();

#endif
