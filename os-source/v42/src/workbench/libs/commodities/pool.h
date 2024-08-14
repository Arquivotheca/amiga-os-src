#ifndef POOL_H
#define POOL_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/****************************************************************************/


VOID ASM FlushPools(REG(a6) struct CxLib *CxBase);
APTR AllocPoolMem(struct CxLib *CxBase, ULONG size);
VOID FreePoolMem(struct CxLib *CxBase, APTR memory);


/*****************************************************************************/


#endif /* POOL_H */
