#ifndef MCPI2C_H
#define	MCPI2C_H

#include "GenericTypeDefs.h"
#include "Compiler.h"

#define MCP23_1 0x20

// Register Address MCP23008
#define IODIR 0x00
#define GPPU  0x06
#define GPIO  0x09

void mcpInit(void);
void mcpWrite(UINT8,UINT8);
UINT8 mcpRead(UINT8);

#endif
