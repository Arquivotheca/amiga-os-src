/*
 * $Id: errcalls.c,v 38.2 92/07/28 13:46:31 mks Exp $
 *
 * $Log:	errcalls.c,v $
 * Revision 38.2  92/07/28  13:46:31  mks
 * Now uses the buffered Read/Write routines
 * 
 * Revision 38.1  91/06/24  19:01:19  mks
 * Changed to V38 source tree - Trimmed Log
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <workbench/workbench.h>

#include "support.h"
#include "internal.h"

/*
******i icon.library/FreeAlloc ***********************************************
*
*   NAME
*       FreeAlloc - allocate memory and add it to a free list.
*
*   SYNOPSIS
*       status = FreeAlloc( free, len, type )
*         D0                  A0   A1   A2
*	APTR FreeAlloc(struct FreeList *, ULONG, ULONG);
*
*   FUNCTION
*       This routine allocates the amount of memory specified and
*	then adds it to the free list.  The free list will be extended
*	(if required).  If there is not enough memory to complete the call,
*	a null is returned.
*
*	Note that FreeAlloc not only allocates the requested memory
*	but also records the memory in the free list.
*
*   INPUTS
*	free -- a pointer to a FreeList structure
*	len -- the length of the memory to be recorded
*	type -- the type of memory to be allocated
*
*   RESULTS
*	memory - a pointer to the newly allocated memory chunk
*		 or zero if the call failed.
*
*   SEE ALSO
*	AllocEntry, FreeEntry, FreeFreeList
*
*   BUGS
*	None
*
******************************************************************************
*/
APTR IFreeAlloc( struct FreeList *free, ULONG len, ULONG type )
{
    APTR mem;

    DP(("IFreeAlloc: enter, free=$%lx len=$%lx, type=$%lx\n",free, len, type));
    if (mem = AllocMemStub(len,type))
    {
	if (!AddFreeListStub(free,mem,len))
	{
	    FreeMemStub(mem,len);
	    mem=NULL;
	}
    }
    DP(("IFreeAlloc: exit, returning $%lx\n", mem));
    return(mem);
}

#define NUMELEMENTS 10
replenishMem(struct FreeList *free)
{
    struct MemList *ml;

    /*
	The first MemEntry structure is included in a 'sizeof(struct MemList)'
	that is why we need to subtract one from NUMELEMENTS.
    */

    if (ml=AllocMemStub(((NUMELEMENTS-1) * sizeof(struct MemEntry))+sizeof(struct MemList),MEMF_PUBLIC|MEMF_CLEAR))
    {
        ml->ml_NumEntries=NUMELEMENTS;
        free->fl_NumFree=NUMELEMENTS;

        AddTail(&free->fl_MemList,ml);
    }

    return((ml!=NULL));
}


IRead( BPTR file, VOID *ptr, LONG len )
{
    if (FReadStub( file, ptr, 1, len ) != len) len=0;
    return( len );
}

IWrite( BPTR file, VOID *ptr, LONG len )
{
    if (FWriteStub( file, ptr, 1, len ) != len) len=0;
    return( len );
}
