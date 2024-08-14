#ifndef SERIALOUT_H
#define SERIALOUT_H


/****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/*****************************************************************************/


VOID ASM SerialPutStr(REG(a0) STRPTR str);
kprintf(STRPTR,...);


/*****************************************************************************/

#endif /* SERIALOUT_H */
