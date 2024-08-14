#ifndef DOSPECIAL_H
#define DOSPECIAL_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


LONG __saveds __stdargs DriverDoSpecial(UWORD *cmd, char *buf, BYTE *vline,
                                        BYTE *currentVMI, BYTE *crlfFlag,
                                        UBYTE *parms);

VOID DoCmd(UWORD cmd, UBYTE *parms, BYTE *vline, BYTE *currentVMI);
VOID FixedPointToStr(LONG value, STRPTR result);


/*****************************************************************************/


#endif /* DOSPECIAL_H */
