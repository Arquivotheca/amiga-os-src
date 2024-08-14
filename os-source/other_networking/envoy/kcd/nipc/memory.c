/**************************************************************************
**
** memory.c     - NIPC Memory management routines
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: memory.c,v 1.22 94/02/07 19:50:16 kcd Exp $
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

/*------------------------------------------------------------------------*/

/*
 * InitMemory()
 *
 * Allocates the NIPC memory pool's initial puddle.
 *
 *
 */
BOOL InitMemory(void)
{
    struct NBase *nb = gb;
    InitSemaphore(&nb->PoolLock);
    gb->MemoryPool = ECreatePool(0,25*1024,25*1024);
    if (gb->MemoryPool)
        return(TRUE);
    else
        return(FALSE);
}

/*------------------------------------------------------------------------*/

/*
 * DeinitMemory(void)
 *
 * Free NIPC's memory pool's puddle.
 *
 */
void DeinitMemory(void)
{
    EDeletePool(gb->MemoryPool);
}

/*------------------------------------------------------------------------*/

/*
 * AllocBuffEntry(size) UWORD size;
 *
 * Attempts to allocate an initialize a buffentry of the given size. If unable
 * to allocate this memory, this function will return a null pointer.
 *
 */
struct BuffEntry *AllocBuffEntry(size)
UWORD size;
{
    struct BuffEntry *buffe;
    ONTIMER(13);
    buffe = (struct BuffEntry *) EAllocPooled(gb->MemoryPool,sizeof(struct BuffEntry));
    if (buffe == 0L)
    {
        OFFTIMER(13);
        return ((struct BuffEntry *) 0L);
    }
    buffe->be_data = (UBYTE *) EAllocPooled(gb->MemoryPool,size);
    if (buffe->be_data == 0)
    {
        EFreePooled(gb->MemoryPool, buffe, sizeof(struct BuffEntry));
        OFFTIMER(13);
        return ((struct BuffEntry *) 0L);
    }

    buffe->be_physicallength = size;
    buffe->be_offset = 0;
    buffe->be_length = 0;
//    buffe->be_cookie = 0xC0EDBABE;
    OFFTIMER(13);
    return (buffe);
}

/*------------------------------------------------------------------------*/

/*
 * FreeBuffEntry(buffe) struct BuffEntry *buffe;
 *
 * Frees an old BuffEntry structure.
 *
 * Note:  Make sure that you've removed this buffentry from any buffer lists
 * before making this call.
 *
 */
void FreeBuffEntry(buffe)
struct BuffEntry *buffe;
{
    EFreePooled(gb->MemoryPool,buffe->be_data, buffe->be_physicallength);
    EFreePooled(gb->MemoryPool,buffe, sizeof(struct BuffEntry));
}

/*------------------------------------------------------------------------*/

/*
 * AllocBuffer(initial_size) UWORD initial_size;
 *
 * AllocBuffer allocates and creates a Buffer structure.  If you pass a non-zero
 * value for initial_size, AllocBuffer will allocate a BuffEntry of that
 * size, and insert it as the first BuffEntry on this Buffer's list.
 *
 * If AllocBuffer cannot allocate enough memory to meet the requirements you
 * have set, it will return a null pointer.
 *
 */
struct Buffer *AllocBuffer(i_size)
UWORD i_size;
{
    struct Buffer *buffer;
    struct BuffEntry *buffentry;
    ONTIMER(14);

    buffer = (struct Buffer *) EAllocPooled(gb->MemoryPool,sizeof(struct Buffer) );
    if (buffer == 0L)
    {
        OFFTIMER(14);
        return ((struct Buffer *) 0L);
    }
    NewList((struct List *) & (buffer->buff_list));
    buffer->buff_refcnt = 0;
    buffer->buff_timeout = 0;

    if (i_size)
    {
        buffentry = AllocBuffEntry(i_size);
        if (buffentry == 0L)
        {
            EFreePooled(gb->MemoryPool, buffer, sizeof(struct Buffer));
            OFFTIMER(14);
            return ((struct Buffer *) 0L);
        }
        AddHead((struct List *) & (buffer->buff_list), (struct Node *) buffentry);
    }
//    buffer->buff_cookie = 0xDEADF00D;
    OFFTIMER(14);
    return (buffer);
}

/*------------------------------------------------------------------------*/

/*
 * FreeBuffer(buff) struct Buffer *buff;
 *
 * FreeBuff will free up a Buffer structure, as well as free all BuffEntries
 * associated with it.  Make SURE that you haven't done anything crazy like
 * freed up a BuffEntry without getting it off of the Buffer list first.
 *
 */
void __asm FreeBuffer(register __a0 struct Buffer *buff)
{
    struct BuffEntry *tbe;

    if(!buff->buff_refcnt)
    {
        while (tbe = (struct BuffEntry *) RemHead((struct List *) & (buff->buff_list)))
            FreeBuffEntry(tbe);

        EFreePooled(gb->MemoryPool, buff, sizeof(struct Buffer));
    }
}

/*------------------------------------------------------------------------*/

/*
 * BuffPointer(buff,offset,buffe) struct Buffer *buff; ULONG offset; struct
 * BuffEntry **buffe;
 *
 * BuffPointer will return a UBYTE pointer to the specific data element that's
 * <offset> length into the Buffer, or return a null if that element doesn't
 * exist.
 *
 */

UBYTE *BuffPointer(buff, offset, buffe)
struct Buffer *buff;
ULONG offset;
struct BuffEntry **buffe;
{
    UBYTE *loc;
    struct BuffEntry *be;

    if (IsListEmpty((struct List *) & (buff->buff_list)))
        return ((UBYTE *) 0L);

    be = (struct BuffEntry *) buff->buff_list.mlh_Head;
    while (TRUE)
    {
        if (offset < be->be_length)
        {
            loc = (UBYTE *) (((ULONG) be->be_data + (ULONG) be->be_offset) + offset);
            *buffe = be;
            return (loc);
        }
        offset -= be->be_length;
        be = (struct BuffEntry *) be->be_link.mln_Succ;
        if (be->be_link.mln_Succ == 0)
            return ((UBYTE *) 0L);
    }
}

/*------------------------------------------------------------------------*/
#undef SysBase
#define SysBase (*(void **)4L)
/*
 * CopyToBuffer(targetbuff,source,length) ULONG length; UBYTE *source; struct
 * Buffer *targetbuff;
 *
 * Copies length bytes from source to the buffer, starting from location zero.
 * If successful, returns TRUE.  If the Buffer isn't large enough to hold the
 * data, the function will return FALSE.
 *
 */
BOOL CopyToBuffer(targetbuff, source, length)
ULONG length;
UBYTE *source;
struct Buffer *targetbuff;
{
    struct BuffEntry *be;
    ONTIMER(4);
    if (IsListEmpty((struct List *) & (targetbuff->buff_list)))
        return (FALSE);
    be = (struct BuffEntry *) targetbuff->buff_list.mlh_Head;

    while (length)
    {
        be->be_length = MIN(be->be_physicallength, length);
        CopyMem(source, be->be_data, be->be_length);
        be->be_offset = 0;
        length -= be->be_length;
        source = (UBYTE *) (source + be->be_length);
        be = (struct BuffEntry *) be->be_link.mln_Succ;
        if ((be->be_link.mln_Succ == 0) && (length != 0))
            return (FALSE);
    }
    OFFTIMER(4);
    return (TRUE);

}

/*------------------------------------------------------------------------*/

/*
 * CopyFromBuffer(target,sourcebuff,length) UBYTE *target; struct Buffer
 * *sourcebuff; ULONG length;
 *
 * Copies length bytes from the source buffer to contiguous memory.
 *
 * Returns nothing - you'd darned well better have length bytes free at
 * sourcebuff.
 *
 */

void __asm CopyFromBuffer(register __a1 UBYTE *target, register __a0 struct Buffer *sourcebuff,
                          register __d0 ULONG length)
{
    ULONG templength;
    struct BuffEntry *be;
    ONTIMER(5);
    if (IsListEmpty((struct List *) & (sourcebuff->buff_list)))
        return;
    be = (struct BuffEntry *) sourcebuff->buff_list.mlh_Head;

    while (length)
    {
        templength = MIN(be->be_length, length);
        CopyMem(&(((UBYTE *)be->be_data)[be->be_offset]), target, templength);
        length -= templength;
        target = (UBYTE *) &target[templength];
        be = (struct BuffEntry *) be->be_link.mln_Succ;
        if ((be->be_link.mln_Succ == 0) && (length != 0))
            return;
    }
    OFFTIMER(5);
}

#undef SysBase
#define SysBase        (NIPCBASE->nb_SysBase)

/*------------------------------------------------------------------------*/

/*
 * DataLength(buffer) struct Buffer *buffer;
 *
 * Returns a ULONG containing the length of data stored in the Buffer structure
 * given as an argument.
 *
 */
ULONG __asm DataLength(register __a0 struct Buffer *buffer)
{
    ULONG countup;
    struct BuffEntry *be;

    countup = 0L;
    be = (struct BuffEntry *) buffer->buff_list.mlh_Head;

    while (be->be_link.mln_Succ)
    {
        countup += be->be_length;
        be = (struct BuffEntry *) be->be_link.mln_Succ;
    }
    return (countup);
}

/*------------------------------------------------------------------------*/

/*
 * UWORD CalcChecksumBuffer(buffer) struct Buffer *buffer;
 *
 *
 * Does a checksum of a whole buffer.
 *
 */

UWORD CalcChecksumBuffer(buffer)
struct Buffer *buffer;
{

    UWORD chksum;
    struct BuffEntry *be;

    chksum = 0;
    be = (struct BuffEntry *) buffer->buff_list.mlh_Head;
    while (be->be_link.mln_Succ)
    {
        if (be->be_length)
            chksum = CalcChecksum(be->be_data, be->be_length, chksum);
        be = (struct BuffEntry *) be->be_link.mln_Succ;
    }
    return (chksum);

}

/*------------------------------------------------------------------------*/

/*
 * BOOL CopyBuffer(ibuff,offset,length,obuff) struct Buffer *ibuff; struct
 * Buffer *obuff; ULONG offset, length;
 *
 * This function allows data copying from one buffer to another - please note,
 * though, that it does NOT do sanity checks on the output buffer.  It's YOUR
 * responsibility to make sure it's large enough!
 *
 * BUGS:  I think the return value is always coming back FALSE. I need to check
 * this out.
 *
 */
BOOL CopyBuffer(ibuff, offset, length, obuff)
struct Buffer *ibuff;
struct Buffer *obuff;
ULONG offset, length;
{

    struct BuffEntry *ibe;
    struct BuffEntry *obe;

    UBYTE *iptr, *tptr;
    ULONG tlen, ilen;

    ONTIMER(3);

    /* Make sure that each list at LEAST has something in it */
    if (IsListEmpty((struct List *) & ibuff->buff_list) ||
        IsListEmpty((struct List *) & obuff->buff_list))
    {
        OFFTIMER(3);
        return (FALSE);
    }
    /* go in as far as to find the offset */

    iptr = (UBYTE *) BuffPointer(ibuff, offset, &ibe);
    if (!iptr)
    {
        OFFTIMER(3);
        return (FALSE);
    }
    ilen = (ULONG) ((ibe->be_length) - (iptr - (ibe->be_data + ibe->be_offset)));

    obe = (struct BuffEntry *) obuff->buff_list.mlh_Head;
    obe->be_length = 0;
    obe->be_offset = 0;

    while (length)
    {
        length -= ilen;
        while (ilen)
        {
            /*
             * we have ilen bytes at iptr.  Break this across the boundries
             * that the obe has.
             */
            if (!obe->be_link.mln_Succ)
            {
                OFFTIMER(3);
                return (FALSE);
            }
            tlen = MIN(ilen, (obe->be_physicallength - obe->be_length));
            tptr = (UBYTE *) (obe->be_data + obe->be_length);
            obe->be_length += tlen;
            CopyMem(iptr, tptr, tlen);
            if (tlen == obe->be_physicallength)
            {
                obe = (struct BuffEntry *) obe->be_link.mln_Succ;
                if (obe->be_link.mln_Succ)
                {
                    obe->be_length = 0;
                    obe->be_offset = 0;
                }
            }
            ilen -= tlen;
        }
        ibe = (struct BuffEntry *) ibe->be_link.mln_Succ;
        if (!ibe->be_link.mln_Succ)
        {
            OFFTIMER(3);
            return (FALSE);
        }
        iptr = ibe->be_data + ibe->be_offset;
        ilen = MIN(ibe->be_length, length);
    }
    OFFTIMER(3);
    return (TRUE);
}

/*------------------------------------------------------------------------*/

/*
 * struct Buffer *MergeBuffer(buff1, buff2); struct Buffer *buff1; struct
 * Buffer *buff2;
 *
 * MergeBuffer is of unusual usefulness.  Basically, it concatenates the data in
 * buff2 to the end of buff1, and procedes to delete the (now empty) buff2.
 * It will then return buff1 as the result.
 *
 * buff1={A B C D} buff2={E F G H} MergeBuffer(buff1,buff2) yields buff1={A B C
 * D E F G H}
 *
 */

struct Buffer *MergeBuffer(buff1, buff2)
struct Buffer *buff1;
struct Buffer *buff2;
{

    struct BuffEntry *tbe;

    while (tbe = (struct BuffEntry *) RemHead((struct List *) & (buff2->buff_list)))
    {
        AddTail((struct List *) & (buff1->buff_list), (struct Node *) tbe);
    }

    FreeBuffer(buff2);

    return (buff1);
}

/*------------------------------------------------------------------------*/

/*
 * struct Buffer *CloneBuffer(ibuff); struct Buffer *ibuff;
 *
 * This function creates a copy of the buffer pointed to by ibuff. There IS a
 * notable significant difference between the copy and the original, though -
 * the original may have it's data broken into a number of different
 * BuffEntry's.  The clone has one BuffEntry which encompasses the whole
 * thing.  If a clone cannot be created, the function returns a NULL.
 *
 */

struct Buffer *CloneBuffer(ibuff)
struct Buffer *ibuff;
{

    struct Buffer *obuff;
    ULONG len;

    len = DataLength(ibuff);
    obuff = AllocBuffer(len);
    if (!obuff)
        return (0);

    //if (!CopyBuffer(ibuff, 0, len, obuff))
        //return (0);
    CopyBuffer(ibuff, 0, len, obuff);


    return (obuff);

}

/*------------------------------------------------------------------------*/

/*
 * ULONG CountEntries(buffer) struct Buffer *buffer;
 *
 * Counts up the number of BuffEntrys in the given buffer, and returns the
 * total. The most obvious use for this is to find out whether you're dealing
 * with a truly contiguous Buffer or not.  Note that -empty- BuffEntrys are
 * not counted.
 *
 */

ULONG CountEntries(buffer)
struct Buffer *buffer;
{
    struct BuffEntry *be;
    ULONG becount;

    becount = 0L;

    be = (struct BuffEntry *) buffer->buff_list.mlh_Head;
    while (be->be_link.mln_Succ)
    {
        if (be->be_length)
            becount++;
        be = (struct BuffEntry *) be->be_link.mln_Succ;
    }
    return (becount);
}

/*------------------------------------------------------------------------*/

/* ULONG CopyBroken(sourcetable,desttable)
 *
 * Copies from source to destination.  Sourcetable and desttable are both
 * pointers to tables made up of:
 *      APTR    memloc
 *      ULONG   length
 *
 * A NULL memloc and length will delimit the table.
 *
 * The function returns the total number of bytes copied.
 *
 */


/*
ULONG CopyBroken(ULONG *st, ULONG *dt,ULONG soff, ULONG doff)
{

    ULONG bcopied=0L;
    UBYTE *src, *dest;
    ULONG srclen=0L, destlen=0L;
    ULONG t;

    while (soff)
    {

        ULONG ts;

        if (!srclen)
        {
            src = (UBYTE *) st[0];
            srclen = st[1];
            st += 2;
            if (!src)
                break;
        }

        ts = MIN(soff,srclen);
        src = &src[ts];
        srclen -= ts;
        soff -= ts;

    }

    while (doff)
    {

        ULONG ts;

        if (!destlen)
        {
            dest = (UBYTE *) dt[0];
            destlen = dt[1];
            dt += 2;
            if (!dest)
                break;
        }

        ts = MIN(doff,destlen);
        dest = &dest[ts];
        destlen -= ts;
        doff -= ts;
    }

    while (TRUE)
    {
        if (!srclen)
        {
            src = (UBYTE *) st[0];
            srclen = st[1];
            st += 2;
            if (!src)
                break;
        }
        if (!destlen)
        {
            dest = (UBYTE *) dt[0];
            destlen = dt[1];
            dt += 2;
            if (!dest)
                break;
        }

        t = MIN(srclen,destlen);
        CopyMem(src,dest,t);
        srclen -= t;
        destlen -=t;

        src = &src[t];
        dest = &dest[t];
        bcopied += t;
    }
    return(bcopied);

}
*/
