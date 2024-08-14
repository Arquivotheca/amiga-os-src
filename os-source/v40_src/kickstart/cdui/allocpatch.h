#ifndef ALLOCPATCH_H
#define ALLOCPATCH_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


struct AllocPatch
{
    APTR ap_OldAllocMem;
    APTR ap_PatchCodeStart;
};


VOID PatchStart(VOID);
extern ULONG __far patchSize;


/*****************************************************************************/


#endif  /* ALLOCPATCH_H */
