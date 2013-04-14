#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"

UINT8 SpiWrite(const UINT8);
#define SpiRead() SpiWrite(0)
void __inline setupSpi(void);
