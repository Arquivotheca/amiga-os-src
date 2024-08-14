#ifndef POOL_H
#define POOL_H


/*****************************************************************************/


#include <exec/types.h>


/****************************************************************************/


VOID FlushPools(VOID);
APTR AllocPoolMem(ULONG size);
VOID FreePoolMem(APTR memory);


/*****************************************************************************/


#endif /* POOL_H */
