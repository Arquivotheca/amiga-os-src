/*
 * Amiga Grand Wack
 *
 * validmem.c -- Determine valid ranges of memory to read/write.
 *
 * $Id: validmem.c,v 39.2 93/04/27 14:36:40 peter Exp $
 *
 * These routines build a linked list describing what the valid ranges
 * of memory in the target Amiga are.  That information is then used
 * to protect read or write operations against invalid memory accesses.
 *
 * The definitions of the valid regions were cribbed from our 68040.library.
 */

#include "validmem.h"
#include "validmem_proto.h"
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/configvars.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/expansion_pragmas.h>

extern struct ExecBase *SysBase;

#define GRANULARITY	(256-1)

long store_invalid_address = TRUE;
void *invalid_address;

long
validAddresses( struct MinList *validlist, void *address, unsigned long length )
{
    return( ( ( address == (void *) 4 ) && ( length <= 4 ) ) ||
	( validAddress( validlist, address ) &&
	validAddress( validlist, (void *) (((unsigned long) address)+length-1 ) ) ) );
}

/* Checks the validity of a given address.  If an address is invalid,
 * check if (global) store_invalid_address is non-zero.  If so, zero it
 * and save the invalid address in (global) invalid_address.
 */

long
validAddress( struct MinList *validlist, void *address )
{
    struct MemoryBlock *mb;
    struct MemoryBlock *succ;
    long found = 1;

    if ( validlist )
    {
	mb = (struct MemoryBlock *)validlist->mlh_Head;
	found = ( address == (void *)4 );
	while ( ( !found ) && ( succ = (struct MemoryBlock *)mb->mb_Node.mln_Succ ) )
	{
	    if ( ( ((unsigned long)address) >= mb->mb_Start ) &&
		( ((unsigned long)address) <= mb->mb_End ) )
	    {
		found = 1;
	    }
	    mb = succ;
	}
	if ( ( !found ) && ( store_invalid_address ) )
	{
	    store_invalid_address = FALSE;
	    invalid_address = address;
	}
    }
    return( found );
}
    

struct MinList *
allocValidMem( void )
{
    struct MinList *validlist = AllocMem( sizeof( struct MinList ), MEMF_CLEAR );
    if ( validlist )
    {
	NewList( (struct List *)validlist );
    }
    return( validlist );
}

void
freeValidMem( struct MinList *validlist )
{
    struct MemoryBlock *mb;
    struct MemoryBlock *succ;

    if ( validlist )
    {
	for ( mb = (struct MemoryBlock *)validlist->mlh_Head;
	    succ = (struct MemoryBlock *)mb->mb_Node.mln_Succ;
	    mb = succ )
	{
	    FreeMem( mb, sizeof( struct MemoryBlock ) );
	}
	FreeMem( validlist, sizeof( struct MinList ) );
    }
}

struct MinList *
addMemoryBlock( struct MinList *validlist, ULONG start, unsigned long length )
{
    struct MemoryBlock *mb;
    if ( validlist )
    {
	if ( mb = AllocMem( sizeof( struct MemoryBlock ), MEMF_ANY ) )
	{
	    /* Mike says: round the start down to nearest 256-byte boundary
	     * because that's the smallest granularity of the MMU.  Likewise,
	     * round the end UP to the nearest 256-byte boundary.
	     */
	    mb->mb_Start = start & ~GRANULARITY;
	    mb->mb_End = ( ( start + length + GRANULARITY ) & ~GRANULARITY ) - 1;
	    AddTail( (struct List *)validlist, (struct Node *)mb );
	}
	else
	{
	    freeValidMem( validlist );
	    validlist = NULL;
	}
    }
    return( validlist );
}


struct MinList *
buildValidMem( void )
{
    struct Library *ExpansionBase;
    struct MinList *validlist;
    struct MemHeader *mem;

    validlist = allocValidMem();

    /*
     * Now for the control areas...
     */
    validlist = addMemoryBlock( validlist,0x00BC0000,0x00040000 );
    validlist = addMemoryBlock( validlist,0x00D80000,0x00080000 );

    /*
     * Now for F-Space...
     */
    validlist = addMemoryBlock( validlist,0x00F00000,0x00080000 );

    /*
     * Now for the ROM...
     */
    validlist = addMemoryBlock( validlist, 0x00F80000,0x00080000 );

    /*
     * If the credit card resource, make the addresses valid...
     */
    if (OpenResource("card.resource"))
    {
	validlist = addMemoryBlock( validlist,0x00600000,0x00440002 );
    }

    /*
     * If CDTV, make CDTV hardware valid...
     */
    if (FindResident("cdstrap"))
    {
	validlist = addMemoryBlock( validlist,0x00E00000,0x00080000 );
    }

    /*
     * Check for ReKick/ZKick/KickIt
     */
    if ((((ULONG)(SysBase->LibNode.lib_Node.ln_Name)) >> 16) == 0x20)
    {
	validlist = addMemoryBlock( validlist,0x00200000,0x00080000 );
    }

    /*
     * Now, put in the free memory
     * Note: The first K of chip RAM isn't in the memory list, so
     * it'll be automatically excluded from readability.  However,
     * location 4 itself needs to be readable (up to a longword's
     * worth, but locations 5-7 are not legitimate addresses to _start_
     * reading from.  So we'll special case that up in validAddresses().
     */

    Forbid();
    mem=(struct MemHeader *)SysBase->MemList.lh_Head;
    while (mem->mh_Node.ln_Succ)
    {
	validlist = addMemoryBlock( validlist, (ULONG)(mem->mh_Lower),
	    (ULONG)(mem->mh_Upper)-(ULONG)(mem->mh_Lower) );
	mem=(struct MemHeader *)(mem->mh_Node.ln_Succ);
    }
    Permit();

    /*
     * Map in the autoconfig boards
     */
    if (ExpansionBase=OpenLibrary("expansion.library",0))
    {
	struct ConfigDev *cd=NULL;

	while (cd=FindConfigDev(cd,-1L,-1L))
	{
	    /* Skip memory boards... */
	    if (!(cd->cd_Rom.er_Type & ERTF_MEMLIST))
	    {
		validlist = addMemoryBlock( validlist,(ULONG)(cd->cd_BoardAddr),cd->cd_BoardSize );
	    }
	}
	CloseLibrary(ExpansionBase);
    }

    return( validlist );
}
