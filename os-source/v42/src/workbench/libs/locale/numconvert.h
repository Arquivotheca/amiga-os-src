#ifndef NUMCONVERT_H
#define NUMCONVERT_H


/*****************************************************************************/


#include <exec/types.h>
#include "localebase.h"


/*****************************************************************************/


ULONG ASM ConvSigned(REG (d0) LONG number, REG(a0) STRPTR buffer);
ULONG ASM ConvUnsigned(REG(d0) ULONG number, REG(a0) STRPTR buffer);
ULONG ASM ConvHex(REG(d0) ULONG number, REG(a0) STRPTR buffer, REG(d3) BOOL asLong);
ULONG ASM ConvHexUpper(REG(d0) ULONG number, REG(a0) STRPTR buffer, REG(d3) BOOL asLong);


/*****************************************************************************/


#endif /* NUMCONVERT_H */
