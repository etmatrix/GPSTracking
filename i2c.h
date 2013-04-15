#ifndef I2C_H
#define	I2C_H

#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"

void initI2C(void);
BOOL writeI2C(UINT8, UINT8 *, UINT8);
BOOL readI2C(UINT8, UINT8 *, UINT8, UINT8 *, UINT8);

#endif	/* I2C_H */
