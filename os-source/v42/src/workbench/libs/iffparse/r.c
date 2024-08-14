
#include <exec/types.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "iffprivate.h"
#include "iffparsebase.h"
#include "r.h"
#include "iffi.h"
#include "iff.h"


/*****************************************************************************/


/* Function reads id and size and pushes a new context for this chunk
 * on to the context stack, initializing byte count to zero.
 * Assumes that file is positioned at the start of a chunk.
 * Override switch used for reading the first chunk,
 * since this function normally assumes a chunk is already pushed.
 * Returns zero for successful, error otherwise.
 */
LONG PushChunkR(struct IFFHandleP *iffp, LONG override, struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP *top, *cnp;
LONG                 type, error;
LONG                 buf[2];

    top = (struct ContextNodeP *) CurrentChunk(iffp);
    if (!top && !override)
        return (IFFERR_EOF);

    ASSERT (top && !override  ||  !top && override);
    type = (top ? top->cnp_Type : 0);

    if (override)
    {
        /* Override means this is the first chunk, so we must
         * use the read function directly and should do no
         * checking on the size of the chunk
         **
         * Issue (ewhac):  Couldn't this be done utilizing
         * !CurrentChunk()?  Or do you want to make sure that, after
         * the last chunk is popped, the client can't push any more?
         * In other words, why the 'override' flag?
         */
        if (error = UserRead (iffp, (UBYTE *) buf, 8L))
            return (error);
    }
    else
    {
        /* Using ReadChunkBytes() causes these 8 bytes to go into the
         * parent's scan count
         */
        if (ReadChunkBytes(iffp, (UBYTE *) buf, 8L) != 8)
            return (IFFERR_READ);

        if (buf[1] > top->cnp_Size - top->cnp_Scan)
            return (IFFERR_MANGLED);
    }
    if (!GoodID(buf[0]))
    {
        if (iffp->iffp_Flags & IFFFP_NEWIO)
            return (IFFERR_NOTIFF);
        else
            return (IFFERR_SYNTAX);
    }

    if (buf[1] < 0)
        return (IFFERR_MANGLED);

    if (!(cnp = AllocMem((long) sizeof (*cnp), 0L)))
        return (IFFERR_NOMEM);

    cnp->cnp_Type       = type;
    cnp->cnp_ID         = buf[0];
    cnp->cnp_Size       = buf[1];
    cnp->cnp_Scan       = 0;
    cnp->cnp_UserScan   = 0;
    NewList((struct List *)&cnp->cnp_LocalItems);
    AddHead((struct List *)&iffp->iffp_Stack, (struct Node *)cnp);
    iffp->iffp_Depth++;

    return (0);
}


/*****************************************************************************/


/* Pops top chunk off stack and skips past any unread bytes in the chunk.
 * Updates the new top chunk with bytes read and purges local context.
 * Returns zero if successful, error otherwise.
 */
LONG PopChunkR(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP *top;
long                 rsize, error;

    /* Get top chunk and seek past anything un-scanned including
     * the optional pad byte.
     */
    if (!(top = (struct ContextNodeP *) CurrentChunk(iffp)))
        return (IFFERR_EOF);

    rsize = (top->cnp_Size + 1) & ~1;

    ASSERT (top->cnp_Scan <= rsize);

    if (top->cnp_Scan < rsize)
    {
        if (error = UserSeek (iffp, rsize - top->cnp_Scan, IFFParseBase))
            return (error);
    }

    /* Remove and deallocate this chunk and any stored properties */
    Remove ((struct Node *)top);
    iffp->iffp_Depth--;
    FreeContextNode (iffp, top, IFFParseBase);

    /* Update the new top chunk, if any. */
    if (top = (struct ContextNodeP *) CurrentChunk(iffp))
        top->cnp_UserScan = (top->cnp_Scan += rsize);

    return (0);
}


/*****************************************************************************/


LONG ASM ReadChunkRecordsLVO(REG(a0) struct IFFHandleP  *iffp,
                             REG(a1) UBYTE *buf,
                             REG(d0) LONG record_size,
                             REG(d1) LONG num_records,
                             REG(a6) struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP *top;
LONG                 size, maxrecs, error;

    if (!(top = (struct ContextNodeP *) CurrentChunk (iffp)))
        return (IFFERR_EOF);

    size = top->cnp_Size - top->cnp_Scan;

    /*  Stupid optimization.  */
    if (record_size == 1)
        maxrecs = size;
    else
        maxrecs = size / record_size;

    if (maxrecs < num_records)
        num_records = maxrecs;

    /* Another stupid optimization.  */
    if (record_size == 1)
        size = num_records;
    else
        size = num_records * record_size;

    if (!size)
        return (0);

    /* Read using user-supplied vector
     */
    if (error = UserRead(iffp, buf, size))
        return (error);

    top->cnp_UserScan = (top->cnp_Scan += size);

    return (num_records);
}


/*****************************************************************************/


LONG ASM ReadChunkBytesLVO(REG(a0) struct IFFHandleP *iffp,
                           REG(a1) UBYTE *buf,
                           REG(d0) LONG size,
                           REG(a6) struct IFFParseLib *IFFParseBase)
{
    return (ReadChunkRecords(iffp,buf,1L,size));
}
