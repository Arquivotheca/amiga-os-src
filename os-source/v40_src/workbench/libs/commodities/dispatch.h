#ifndef DISPATCH_H
#define DISPATCH_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/*****************************************************************************/


VOID ASM CxDispatch(REG(d0) LONG func,
                    REG(a0) struct CxMsg *msg,
                    REG(a1) struct CxObj *co);


/****************************************************************************/


#endif /* DISPATCH_H */
