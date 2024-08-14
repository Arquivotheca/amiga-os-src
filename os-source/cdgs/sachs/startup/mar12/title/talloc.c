/* :ts=4
*
*	talloc.c -- Trace Allocator.
*
*	William A. Ware							9005.18
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright (C) 1990, Silent Software Incorporated.						*
*   All Rights Reserved.													*
****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <proto/exec.h>

#include <ss/talloc.h>

#define MEMALLOC_SIZE		(sizeof(struct MemAlloc))

struct MinList AllocList = 
{
	(struct MinNode *)&AllocList.mlh_Tail,
	0,
	(struct MinNode *)&AllocList.mlh_Head
};

/****** ss.lib/TAlloc/varables/TAllocEmgFree **************************
*
*   NAME
*	TAllocEmgFree - Pointer to a routine that might free up memory
*
*   SYNOPSIS
*	VOID 	(*TAllocEmgFree)( struct MinList *, ULONG, ULONG );
*
*   FUNCTION
*	If not NULL this routine is called when the program fails to allocate
*	memory.  It passes the TAllocList which is a MinList with each.
*	also the size and flags of the requested memory block.
*
*	On returning from this program the memory will tryied to be allocated
*	again.
*
*
*************************
*/
VOID 	(*TAllocEmgFree)( struct MinList *, ULONG, ULONG ) = NULL; 

/****** ss.lib/TAlloc/TAllocMem ************************************
*
*   NAME
*	TAllocMem - Track allocate memory.
*
*   SYNOPSIS
*	memoryBlock = TAllocMem(byteSize, attributes)
*
*	VOID * __regargs AllocMem(ULONG, ULONG);
*
*   FUNCTION
*	Allocate memory just like AllocMem() except that this memory is
*	put into a general memory list.  At the end of an applicataion
*	a call to FreeTAllocALL() can be made and all memory will be freed
*	regardless.
*
*	If memory is filled it will try calling an optional routine 
*	(see TAllocEmgFree) which might free up enough memory to make the
*	request.
*
*	The Memory MUST be freed with FreeTAlloc or a call must be made
*	to FreeTAllocALL.
*
*   INPUTS
*	Same as AllocMem.
*
*   RESULT
*	Same as AllocMem.
*
*   NOTES
*	A good practice is to call FreeTAllocAll() at the very end of the.
*	program.
*
*   SEE ALSO
*	FreeTAlloc() FreeTAllocALL() AllocMem().
*
*********************
*/
VOID * __regargs TAllocMem( register ULONG size, register ULONG flags )
{
	register struct MemAlloc *ptr;
	extern ULONG MemThreshold;
	
	size += MEMALLOC_SIZE;
	if ( !(ptr = (struct MemAlloc *)AllocMem( size, flags))) 
	{
		if (TAllocEmgFree) TAllocEmgFree( &AllocList, size, flags );
		if (!(ptr = (struct MemAlloc *)AllocMem( size, flags )))
			return(NULL);
	}
	ptr->size = size;
	AddHead( (struct List *) &AllocList, (struct Node *) ptr );
	return( (VOID *)(&ptr[1]) );
}

/****** ss.lib/TAlloc/GetTSize ************************************
*
*   NAME
*	GetTSize - Return the size of a TAlloc'ed memory segment.
*
*   SYNOPSIS
*	size = GetTSize( data )
*
*	ULONG __regargs GetTSize( register VOID * )
*
*   FUNCTION
*	GetTSize() will return the size of the memory segment that was
*	allocated with TAllocMem().  Only use this with segments that are
*	allocated with TAllocMem()!
*
*   INPUTS
*	data - talloc data.
*
*   RESULT
*	size - size of the allocated memory as passed to TAllocMem()
*
*   NOTES
*	The size is stored in the long word before the data.
*
*   SEE ALSO
*	TAllocMem()
*
*********************
*/
ULONG __regargs GetTSize( register VOID *data )
{ 
	register struct MemAlloc *ptr; 
	if (!data) return(0L); 
	ptr = (struct MemAlloc *)(&((UBYTE *)data)[ -MEMALLOC_SIZE ]); 
	return( (ULONG)(ptr->size - sizeof(struct MemAlloc)) );
}

/****** ss.lib/TAlloc/FreeTAlloc ************************************
*
*   NAME
*	FreeTAlloc - Free TAlloc Memory.
*
*   SYNOPSIS
*	FreeTAlloc( data )
*
*	VOID __regargs FreeTAlloc( register VOID * );
*
*   FUNCTION
*	Frees memory allocated with TAllocMem().
*
*   INPUTS
*	data - a TAllocMem() allocated memory block.
*
*   SEE ALSO
*	TAllocMem()
*
*********************
*/
VOID __regargs FreeTAlloc( register VOID *data )
			/* Frees one entry in alloc table */
{
	register struct MemAlloc *ptr;
	
	if (!data) return;
	ptr = (struct MemAlloc *)(&((UBYTE *)data)[ -MEMALLOC_SIZE ]);
	Remove( (struct Node *)ptr );
	FreeMem( ptr, ptr->size );
}

/****** ss.lib/TAlloc/FreeTAllocALL ************************************
*
*   NAME
*	FreeTAllocALL - Frees all the TAllocated Memory for this task.
*
*   SYNOPSIS
*	FreeTAllocALL()
*
*	VOID __regargs FreeTAllocALL( VOID );
*
*   FUNCTION
*	Frees all the memory allocated with TAllocMem().
*
*   SEE ALSO
*	FreeTAlloc(), TAllocMem()
*
*********************
*/
VOID __regargs FreeTAllocALL( VOID )
{
	register struct MemAlloc *ptr;
	
	while ( (ptr = (struct MemAlloc *)
			RemHead( (struct List *)&AllocList )))	
		FreeMem( ptr, ptr->size );
}