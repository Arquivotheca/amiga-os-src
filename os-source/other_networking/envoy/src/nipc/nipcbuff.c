/**************************************************************************
**
** memory.c     - NIPC Memory management routines
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: nipcbuff.c,v 1.3 94/02/08 14:21:40 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include "externs.h"

/*------------------------------------------------------------------------*/

#define MIN(x, y) ((x) < (y) ? (x):(y))
#define MAX(x, y) ((x) > (y) ? (x):(y))

/****** nipc.library/FreeNIPCBuffEntry ***************************************
*
*   NAME
*     FreeNIPCBuffEntry -- Free a NIPCBuffEntry structure
*
*   SYNOPSIS
*     FreeNIPCBuffEntry(entry)
*                       A0
*
*     VOID FreeNIPCBuffEntry(struct NIPCBuffEntry *);
*
*   FUNCTION
*     This function is used to free a NIPCBuffEntry structure.  Note that
*     the data area referenced by the NIPCBuffEntry structure will not be
*     freed.
*
*   INPUTS
*     entry - Pointer to the NIPCBuffEntry structure to be freed.
*
*   RESULT
*     None
*
*   EXAMPLE
*
*   NOTES
*     You MUST use this function for freeing NIPCBuffEntry structures, or
*     memory loss will occur.
*
*   BUGS
*
*   SEE ALSO
*       AllocNIPCBuffEntry(), <envoy/nipc.h>
*
******************************************************************************
*
*/

VOID __asm FreeNIPCBuffEntry(register __a0 struct NIPCBuffEntry *entry)
{
    EFreePooled(gb->MemoryPool,entry,sizeof(struct NIPCBuffEntryI));
}

/****** nipc.library/FreeNIPCBuff ********************************************
*
*   NAME
*     FreeNIPCBuff -- Free a NIPCBuff structure
*
*   SYNOPSIS
*     FreeNIPCBuff(buff)
*                  A0
*
*     VOID FreeNIPCBuff(struct NIPCBuff *);
*
*   FUNCTION
*     This function is used to free a NIPCBuff structure and any
*     NIPCBuffEntry structures contained in it.  Note that the data areas
*     refereced by the NIPCBuffEntry's will not be freed.
*
*   INPUTS
*     buff - Pointer to the NIPCBuf structure to be freed.
*
*   RESULT
*     None
*
*   EXAMPLE
*
*   NOTES
*     You MUST use this function for freeing NIPCBuff structures, or
*     memory loss will occur.
*
*   BUGS
*
*   SEE ALSO
*       AllocNIPCBuff(), <envoy/nipc.h>
*
******************************************************************************
*
*/

VOID __asm FreeNIPCBuff(register __a0 struct NIPCBuff *buff)
{
    struct NIPCBuffEntry *nbe;

    while(nbe = (struct NIPCBuffEntry *)RemHead((struct List *)&buff->nbuff_Entries))
        FreeNIPCBuffEntry(nbe);

    EFreePooled(gb->MemoryPool,buff,sizeof(struct NIPCBuffI));
}


/****** nipc.library/AllocNIPCBuffEntry **************************************
*
*   NAME
*     AllocNIPCBuffEntry -- Allocate and initialize a NIPCBuffEntry structure
*
*   SYNOPSIS
*     entry = AllocNIPCBuffEntry()
*     D0
*
*     struct NIPCBuffEntry *AllocNIPCBuffEntry();
*
*   FUNCTION
*     This function is used to allocate an NIPCBuffEntry structure for use
*     with NIPCBuff structures. Note that this function only allocates the
*     NIPCBuffEntry structure, and clears it.  No data area is allocated.
*
*   INPUTS
*     None
*
*   RESULT
*     entry - Pointer to a newly allocated and initialized NIPCBuffEntry
*             structure or NULL if the NIPCBuffEntry couldn't be allocated.
*
*   EXAMPLE
*
*   NOTES
*     You MUST use this function for allocating NIPCBuffEntry structures, or
*     innocent memory will get overwritten.
*
*   BUGS
*
*   SEE ALSO
*       FreeNIPCBuffEntry(), <envoy/nipc.h>
*
******************************************************************************
*
*/

__stdargs struct NIPCBuffEntry *AllocNIPCBuffEntry(VOID)
{
    struct NIPCBuffEntryI *nbei;

    if(nbei = (struct NIPCBuffEntryI *)EAllocPooled(gb->MemoryPool,sizeof(struct NIPCBuffEntryI)));
    {
        nbei->nbei_NBuffEntry.nbe_Offset = 0;
        nbei->nbei_NBuffEntry.nbe_Length = 0;
        nbei->nbei_NBuffEntry.nbe_PhysicalLength = 0;
        nbei->nbei_NBuffEntry.nbe_Data = 0;
//        nbei->nbei_cookie = 0xC0EDBEEF;
    }
    return((struct NIPCBuffEntry *)nbei);
}

/****** nipc.library/AllocNIPCBuff *******************************************
*
*   NAME
*     AllocNIPCBuff -- Allocate and initialize a NIPCBuff structure
*
*   SYNOPSIS
*     nipcbuff = AllocNIPCBuff(entries)
*     D0                       D0
*
*     struct NIPCBuff *AllocNIPCBuff(entries);
*
*   FUNCTION
*     This function is used to allocate an NIPCBuff structure for use with
*     transaction data areas.  Note that this function only allocates the
*     NIPCBuff structure, initializes it, and possibly adds a number of
*     NIPCBuffEntry's to it.  No data area is allocated.
*
*   INPUTS
*     entries - The number of NIPCBuffEntry's that should be placed in the
*               NIPCBuff structure.
*
*   RESULT
*     nipcbuff - Pointer to a newly allocated and initialized NIPCBuff
*                structure or NULL if the NIPCBuff or the number of
*                NIPCBuffEntry structures couldn't be allocated.
*
*   EXAMPLE
*
*   NOTES
*     You MUST use this function for allocating NIPCBuff structures, or
*     innocent memory will get overwritten.
*
*   BUGS
*
*   SEE ALSO
*       FreeNIPCBuff(), <envoy/nipc.h>
*
******************************************************************************
*
*/
struct NIPCBuff * __asm AllocNIPCBuff(register __d0 ULONG entries)
{
    struct NIPCBuffI *nbuff;
    struct NIPCBuffEntry *nbe;

    if(nbuff = (struct NIPCBuffI *)EAllocPooled(gb->MemoryPool,sizeof(struct NIPCBuffI)))
    {
        NewList((struct List *)&(nbuff->nbi_NBuff.nbuff_Entries));
        nbuff->nbi_Timeout = 0;
        nbuff->nbi_RefCnt = 0;

        while(entries--)
        {
            if(nbe = AllocNIPCBuffEntry())
            {
            	AddTail((struct List *)&(nbuff->nbi_NBuff.nbuff_Entries),(struct Node *)nbe);
            }
            else
            {
            	FreeNIPCBuff((struct NIPCBuff *)nbuff);
            	nbuff = NULL;
            	break;
            }
        }
    }
//    nbuff->nbi_cookie = 0xDEADC0ED;
    return((struct NIPCBuff *)nbuff);
}


/****** nipc.library/CopyNIPCBuff ********************************************
*
*   NAME
*     CopyNIPCBuff -- Copy a NIPCBuff structure
*
*   SYNOPSIS
*     actual = CopyNIPCBuff(src, dest, srcoffset, dstoffset, length)
*     D0                    A0   A1    D0         D1	     D2
*
*     ULONG CopyNIPCBuff(struct NIPCBuff *, struct NIPCBuff *,
*			 ULONG, ULONG, ULONG);
*
*   FUNCTION
*     This function is used to copy one NIPCBuff to another.  The number of
*     bytes actually copied will be returned.  If there isn't enough space
*     in the destination buffer, then actual will be less than length.
*
*   INPUTS
*     src - Pointer to the NIPCBuf structure to be copied from.
*     dest - Pointer to the NIPCBuff structure to be copied to.
*     srcoffset - Offset into src buffer.
*     dstoffset - Offset into dest buffer.
*     length - Number of bytes to copy.
*
*   RESULT
*     actual - Number of bytes actually copied.
*
*   EXAMPLE
*
*   NOTES
*     This function does not modify any of the fields on the src or data
*     NIPCBuff structures or their NIPCBuffEntries.  It will abide by
*     the current values of nbe_Offset and nbe_Length on both the source
*     and desitnation buffers.
*
*     The upshot of this is that if you want the entire data area referenced
*     by a NIPCBuffEntry to be copied from/to, you should do the following:
*
*     ...
*     nbe->nbe_Offset = 0;
*     nbe->nbe_Length = nbe->nbe_PhysicalLength;
*     ...
*
*
*   BUGS
*
*     Before version 40.7, this function may not work correctly due to a
*     bug in NIPCBuffPointer, except when both source and destination offsets
*     are zero.
*
*   SEE ALSO
*     <envoy/nipc.h>
*
******************************************************************************
*
*/

ULONG __asm CopyNIPCBuff(register __a0 struct NIPCBuff *srcbuff,
			 register __a1 struct NIPCBuff *dstbuff,
			 register __d0 ULONG srcoffset,
			 register __d1 ULONG dstoffset,
			 register __d2 ULONG length)
{
    struct NIPCBuffEntry *srcbe,*dstbe;

    UBYTE *src,*dst;
    ULONG srclen, dstlen, tlen;

    ULONG actual = 0;

    /* Make sure that each list at LEAST has something in it */
    if (IsListEmpty((struct List *) & srcbuff->nbuff_Entries) ||
        IsListEmpty((struct List *) & dstbuff->nbuff_Entries))
        return (0);

    /* Find start offset */

    if(src = BuffPointer((struct Buffer *)srcbuff, srcoffset, (struct BuffEntry **)&srcbe))
    {
    	if(dst = BuffPointer((struct Buffer *)dstbuff, dstoffset, (struct BuffEntry **)&dstbe))
    	{

            /* Determine how much data we can copy from the current source NIPCBuffEntry */
            /* Note: This is a special case because we might not be starting right at    */
            /*       nbe_Data + nbe_Offset.						 */
            srclen = srcbe->nbe_Length - (src - (srcbe->nbe_Data + srcbe->nbe_Offset));

    	    while(length)
    	    {
    	    	length -= srclen;

                /* Again, this is to handle the special case of dst not starting right
                   at nbe_Data + nbe_Offset. */

                dstlen = dstbe->nbe_Length - (dst - (dstbe->nbe_Data + dstbe->nbe_Offset));

    	    	while(srclen)
    	    	{
                    /* Okay, at this point we have srclen contiguous bytes at src.  Break
                       this across the boundaries that the obe has. */

                    if(!dstbe->nbe_Link.mln_Succ)
                        return(actual);

                    tlen = MIN(srclen,dstlen);
                    CopyMem(src,dst,tlen);
                    actual += tlen;
                    dst += tlen;
                    src += tlen;
                    srclen -= tlen;
                    dstlen -= tlen;

	            if(!dstlen)
	            {
	            	dstbe = (struct NIPCBuffEntry *)dstbe->nbe_Link.mln_Succ;
	            	if(dstbe->nbe_Link.mln_Succ)
	            	{
	            	    dst = dstbe->nbe_Data + dstbe->nbe_Offset;
	            	    dstlen = dstbe->nbe_Length;
	            	}
	            }
	        }
	        srcbe = (struct NIPCBuffEntry *)srcbe->nbe_Link.mln_Succ;
	        if(!srcbe->nbe_Link.mln_Succ)
	            return(actual);

		src = srcbe->nbe_Data + srcbe->nbe_Offset;
		srclen = MIN(srcbe->nbe_Length,length);
	    }
	}
    }
    return(actual);
}

/****** nipc.library/CopyToNIPCBuff ******************************************
*
*   NAME
*     CopyToNIPCBuff -- Copy block of memory into a NIPCBuff structure
*
*   SYNOPSIS
*     actual = CopyToNIPCBuff(src, dest, dstoffset, length)
*     D0                      A0   A1    D0         D1
*
*     ULONG CopyToNIPCBuff(UBYTE *, struct NIPCBuff *, ULONG, ULONG);
*
*   FUNCTION
*     This function is used to copy data from a block of memory into a
*     NIPCBuff.  If there isn't enough space in the destination buffer,
*     then actual will be less than length.
*
*   INPUTS
*     src - Pointer to the block of memory to be copied from.
*     dest - Pointer to the NIPCBuff structure to be copied to.
*     dstoffset - Offset into destination buffer.
*     length - Number of bytes to copy.
*
*   RESULT
*     actual - Number of bytes actually copied.
*
*   EXAMPLE
*
*   NOTES
*     This function does not modify any of the fields in the destination
*     NIPCBuff structure or its NIPCBuffEntries.  It will abide by the
*     existing values of nbe_Offset and nbe_Length.
*
*     The upshot of this is that if you want the entire data area referenced
*     by a NIPCBuffEntry to be copied to, you should do the following:
*
*     ...
*     nbe->nbe_Offset = 0;
*     nbe->nbe_Length = nbe->nbe_PhysicalLength;
*     ...
*
*
*   BUGS
*
*     Before version 40.7, this function may not work correctly due to a
*     bug in NIPCBuffPointer, except when the destination offset is zero.
*
*   SEE ALSO
*     <envoy/nipc.h>
*
******************************************************************************
*
*/

ULONG __asm CopyToNIPCBuff(register __a0 UBYTE *src,
			   register __a1 struct NIPCBuff *dstbuff,
			   register __d0 ULONG dstoffset,
			   register __d1 ULONG length)
{
    struct NIPCBuffEntry *dstbe;
    ULONG dstlen,tlen,actual=0;
    UBYTE *dst;

    if(dst = BuffPointer((struct Buffer *)dstbuff, dstoffset, (struct BuffEntry **)&dstbe))
    {
        dstlen = dstbe->nbe_Length - (dst - (dstbe->nbe_Data + dstbe->nbe_Offset));

        while(length)
        {
            tlen = MIN(length,dstlen);
            CopyMem(src, dst, tlen);
            length -= tlen;
            dstlen -= tlen;
            actual += tlen;
            src = (UBYTE *)(ULONG)src + tlen;
            dst = (UBYTE *)(ULONG)dst + tlen;

            if(length)
            {
                while(!dstlen)
                {
                    dstbe = (struct NIPCBuffEntry *)dstbe->nbe_Link.mln_Succ;
                    if(!dstbe->nbe_Link.mln_Succ)
                        return(actual);

                    dst = (UBYTE *)(ULONG)dstbe->nbe_Data + dstbe->nbe_Offset;
                    dstlen = dstbe->nbe_Length;
                }
            }
        }
    }
    return(actual);
}

/****** nipc.library/CopyFromNIPCBuff ******************************************
*
*   NAME
*     CopyFromNIPCBuff -- Copy block of memory out of a NIPCBuff structure
*
*   SYNOPSIS
*     actual = CopyFromNIPCBuff(src, dest, srcoffset, length)
*     D0                        A0   A1    D0         D1
*
*     ULONG CopyFromNIPCBuff(struct NIPCBuff *, UBYTE *, ULONG, ULONG);
*
*   FUNCTION
*     This function is used to copy one NIPCBuff to another.  The number of
*     bytes actually copied will be returned.  If there isn't enough space
*     in the destination buffer, then actual will be less than length.
*
*   INPUTS
*     src - Pointer to the NIPCBuf structure to be copied from.
*     dest - Pointer to the NIPCBuff structure to be copied to.
*     dstoffset - Offset into dest buffer.
*     length - Number of bytes to copy.
*
*   RESULT
*     actual - Number of bytes actually copied.
*
*   EXAMPLE
*
*   NOTES
*     This function does not modify any of the fields in the destination
*     NIPCBuff structure or its NIPCBuffEntries.  It will abide by the
*     existing values of nbe_Offset and nbe_Length.
*
*     The upshot of this is that if you want the entire data area referenced
*     by a NIPCBuffEntry to be copied to, you should do the following:
*
*     ...
*     nbe->nbe_Offset = 0;
*     nbe->nbe_Length = nbe->nbe_PhysicalLength;
*     ...
*
*
*   BUGS
*
*     Before version 40.7, this function may not work correctly due to a
*     bug in NIPCBuffPointer, except when the source offset is zero.
*
*   SEE ALSO
*     <envoy/nipc.h>
*
******************************************************************************
*
*/

ULONG __asm CopyFromNIPCBuff(register __a0 struct NIPCBuff *srcbuff,
			     register __a1 UBYTE *dst,
			     register __d0 ULONG srcoffset,
			     register __d1 ULONG length)
{
    struct NIPCBuffEntry *srcbe;
    ULONG srclen,tlen,actual=0;
    UBYTE *src;

    if(src = BuffPointer((struct Buffer *)srcbuff, srcoffset, (struct BuffEntry **)&srcbe))
    {
        srclen = srcbe->nbe_Length - (src - (srcbe->nbe_Data + srcbe->nbe_Offset));

        while(length)
        {
            tlen = MIN(length,srclen);
            CopyMem(src, dst, tlen);
            length -= tlen;
            srclen -= tlen;
            actual += tlen;
            src = (UBYTE *)(ULONG)src + tlen;
            dst = (UBYTE *)(ULONG)dst + tlen;

            if(length)
            {
                while(!srclen)
                {
                    srcbe = (struct NIPCBuffEntry *)srcbe->nbe_Link.mln_Succ;
                    if(!srcbe->nbe_Link.mln_Succ)
                        return(actual);

                    src = (UBYTE *)(ULONG)srcbe->nbe_Data + srcbe->nbe_Offset;
                    srclen = srcbe->nbe_Length;
                }
            }
        }
    }
    return(actual);
}

/****** nipc.library/NIPCBuffLength ******************************************
*
*   NAME
*     NIPCBuffLength -- Determine the amount of data stored in a NIPCBuff
*
*   SYNOPSIS
*     length = NIPCBuffLength(buff)
*     D0                      A0
*
*     ULONG NIPCBuffLength(struct NIPCBuff *);
*
*   FUNCTION
*     This function may be used to determine the total amount of data stored
*     in the NIPCBuff structure.  Note that this does not return the total
*     capacity of all of the NIPCBuffEntry's.
*
*   INPUTS
*     buff - Pointer to a NIPCBuf structure
*
*   RESULT
*     length - Number of bytes stored in buff.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     <envoy/nipc.h>
*
******************************************************************************
*
*/

ULONG __asm NIPCBuffLength(register __a0 struct NIPCBuff *buffer)
{
    /* Lame way to do this, but hey... */

    return(DataLength((struct Buffer *)buffer));
}

/****** nipc.library/NIPCBuffPointer *****************************************
*
*   NAME
*     NIPCBuffPointer -- Get a pointer to data inside a NIPCBuff
*
*   SYNOPSIS
*     data = NIPCBuffPointer(buff, offset, buffentry)
*     D0                     A0    D0      A1
*
*     UBYTE *NIPCBuffPointer(struct NIPCBuff *, ULONG,
*		struct NIPCBuffEntry **);
*
*   FUNCTION
*     This function may be used to find a particular piece of data within a
*     NIPCBuff structure.  Returns a pointer to the data item, and a pointer
*     to the NIPCBuffEntry that contains that item.
*
*   INPUTS
*     buff - Pointer to a NIPCBuf structure
*     offset - Offset into the NIPCBuff
*     buffentry - Pointer to a struct NIPCBuffEntry * to be modified
*
*   RESULT
*     data - Points to the byte at offset bytes into buff.
*     buffentry - Points to the NIPCBuffEntry that contains "data".
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*     Versions of nipc.library prior to 40.7 had a problem with offsets
*     that exactly matched the length of a BuffEntry, and returned a pointer
*     past the end of the BuffEntry previous to the one it should have.
*
*   SEE ALSO
*     <envoy/nipc.h>
*
******************************************************************************
*
*/

UBYTE * __asm NIPCBuffPointer(register __a0 struct NIPCBuff *buff,
			      register __d0 ULONG offset,
			      register __a1 struct NIPCBuffEntry **entry)
{
    return(BuffPointer((struct Buffer *)buff,offset,(struct BuffEntry **)entry));
}

/****** nipc.library/AppendNIPCBuff ******************************************
*
*   NAME
*     AppendNIPCBuff -- Append one NIPCBuff structure to another
*
*   SYNOPSIS
*     result = CopyNIPCBuff(first, second)
*     D0                    A0     A1
*
*     struct NIPCBuff *AppendNIPCBuff(struct NIPCBuff *, struct NIPCBuff *);
*
*   FUNCTION
*     This function may be used to append the data in one NIPCBuff to another
*     NIPCBuff.  Essentially, it just removes off the of NIPCBuffEntry's from
*     one NIPCBuff and adds them to the other NIPCBuff (in order).
*
*     After this function completes, the second NIPCBuff will be empty.
*
*   INPUTS
*     first- Pointer to the NIPCBuf structure to be appended to.
*     second - Pointer to the NIPCBuff structure to be appended.
*
*   RESULT
*     result - Pointer to the first NIPCBuff
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     <envoy/nipc.h>
*
******************************************************************************
*
*/

struct NIPCBuff * __asm AppendNIPCBuff(register __a0 struct NIPCBuff *first,
			               register __a1 struct NIPCBuff *second)
{
	return((struct NIPCBuff *)MergeBuffer((struct Buffer *)first,(struct Buffer *)second));
}

