
#include <exec/types.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "iffprivate.h"
#include "iffparsebase.h"
#include "w.h"
#include "iffi.h"
#include "iff.h"


/*****************************************************************************/


/* Prototypes for functions defined in w.c */
static LONG DeferredWrite(struct IFFHandleP *,
                          UBYTE *,
                          LONG, struct IFFParseLib *IFFParseBase);
static LONG BackPatch(struct IFFHandleP *,
                      LONG ,
                      LONG, struct IFFParseLib *IFFParseBase);

static LONG WriteWholeSchmere(struct IFFHandleP *, struct IFFParseLib *IFFParseBase);


/*****************************************************************************/


/* Push a chunk onto the iff context stack for write.
 * Type must be specified for generic chunks.
 * (Size will eventually be an optional parameter, but for now, it's
 *  determined by the calls to WriteChunkRecords().)
 **
 * ewhac:  Size is now in, and it's mandatory.  EA did it this way; I
 * see no obviously elegant way around it.
 */
LONG PushChunkW(struct IFFHandleP *iffp, LONG type, LONG id, LONG size, struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP *cnp;
LONG		     error, parenttype = 0;
LONG		     buf[2];
char		     firstchunk = 0;

	if (cnp = (struct ContextNodeP *) CurrentChunk (iffp))
		parenttype = cnp->cnp_Type;
	else
		if (iffp->iffp_Flags & IFFFP_NEWIO) {
			firstchunk = 1;
			iffp->iffp_Flags &= ~IFFFP_NEWIO;
		} else
			return (IFFERR_EOF);

	/*
	 * Syntax checking.  Try and do all this before we write anything.
	 */
	if (!GoodID (id))
		/*
		 * Naughty, naughty.
		 */
		return (IFFERR_SYNTAX);

	else if (firstchunk) {
		/*
		 * First chunk must be FORM, CAT, or LIST.
		 */
		if (id != ID_FORM  &&  id != ID_CAT  &&  id != ID_LIST)
			return (IFFERR_NOTIFF);

	} else if (id == ID_PROP) {
		/*
		 * Check that the containing context for PROP is a LIST.
		 */
		if (cnp->cnp_ID != ID_LIST)
			return (IFFERR_SYNTAX);

	} else if (IsGenericID (id)) {
		/*
		 * Generic ID.  Check the validity of its subtype.
		 */
		if (!GoodType (type))
			return (IFFERR_NOTIFF);

	} else {
		/*
		 * Non-generic ID.  Containing context must be PROP or FORM.
		 */
		if (cnp->cnp_ID != ID_FORM  &&
		    cnp->cnp_ID != ID_PROP)
			return (IFFERR_SYNTAX);
	}

	/*
	 * We passed the test!
	 * Write the chunk header: ID and size (if IFFSIZE_UNKNOWN, it will
	 * be BackPatch()ed by PopChunkW()).
	 */
	buf[0] = id;
	buf[1] = size;
	if (firstchunk) {
		if (error = DeferredWrite (iffp, (UBYTE *) buf, 8L, IFFParseBase))
			return (error);
	} else {
		/*
		 * Update parent's count of bytes written.
		 */
		if (WriteChunkBytes (iffp, (UBYTE *) buf, 8L) != 8)
			return (IFFERR_WRITE);
	}

	/*
	 * Allocate and fill in a ContextNodeP for the new
	 * chunk.  Only "scan" is used internally.
	 **
	 * ewhac:  Both 'size' and 'scan' are now used.  You may wish to
	 * check the robustness and validity of their implementation.
	 */
	if (!(cnp = AllocMem ((LONG) sizeof (*cnp), MEMF_ANY)))
		return (IFFERR_NOMEM);

	cnp->cnp_ID		= id;
	cnp->cnp_Size		= size;
	cnp->cnp_Scan		= 0;
	cnp->cnp_UserScan	= 0;
	NewList ((struct List *)&cnp->cnp_LocalItems);
	AddHead ((struct List *)&iffp->iffp_Stack, (struct Node *)cnp);
	iffp->iffp_Depth++;

	/*
	 * For generic chunks, write the type out now that
	 * the chunk context node is initialized.
	 */
	if (IsGenericID (id)) {
		if (WriteChunkBytes (iffp, (UBYTE *) &type, 4L) != 4)
			return (IFFERR_WRITE);
		cnp->cnp_Type = type;
	} else
		cnp->cnp_Type = parenttype;

	return (0);
}


/*****************************************************************************/


/* Pop a chunk off the iff context stack.
 * Ends the chunk and sets the size (if it has not already been set)
 * by seeking back and fixing it up.  New top chunk gets size updated.
 */
LONG PopChunkW(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP	*top;
LONG			rsize, error, size;
BYTE			pad = 0;
char			sizeunknown = 0;

	/*
	 * Pop the top
	 */
	if (!(top = (struct ContextNodeP *) CurrentChunk (iffp)))
		return (IFFERR_EOF);

	if (top->cnp_Size == IFFSIZE_UNKNOWN) {
		/*
		 * Size is unknown.  Therefore, the size is however many
		 * bytes the client wrote.
		 */
		sizeunknown = 1;

		size = top->cnp_Scan;
	} else {
		/*
		 * Size is known.  Compare what the client said against
		 * what actually happened.
		 */
		size = top->cnp_Size;
		if (size != top->cnp_Scan)
			return (IFFERR_MANGLED);
	}

	/*
	 * Add a possible pad byte.
	 */
	rsize = (size + 1) & ~1;
	if (rsize > size)
		if (error = DeferredWrite (iffp, &pad, 1L, IFFParseBase))
			return (error);

	/*
	 * This chunk should have nothing in it's local items list.
	 * (ewhac: what if it does?)
	 */
	Remove ((struct Node *)top);
	ASSERT (!TEST (HEAD (&top->cnp_LocalItems)));
	FreeMem (top, (LONG) sizeof (*top));
	iffp->iffp_Depth --;

	/*
	 * Seek back and fix the chunk size.
	 */
	if (sizeunknown)
		if (error = BackPatch (iffp, rsize + 4, size, IFFParseBase))
			return (error);

	if (!(top = (struct ContextNodeP *) CurrentChunk (iffp))) {
		/*
		 * No more chunks, file write is done.
		 */
		if (error = WriteWholeSchmere (iffp, IFFParseBase))
			return (error);

	} else {
		/*
		 * Update parent's count.
		 */
		top->cnp_UserScan = (top->cnp_Scan += rsize);
		if (top->cnp_Size != IFFSIZE_UNKNOWN &&
		    top->cnp_Scan  >  top->cnp_Size)
			/*
			 * ewhac: We could wait until the client pops the
			 * chunk off before we check this, but I think it's
			 * better that we let the client know of a screwup
			 * as quickly as possible to aid the debugging.
			 */
			return (IFFERR_MANGLED);
	}
	return (0);
}


/*****************************************************************************/


LONG ASM WriteChunkBytesLVO(REG(a0) struct IFFHandleP *iffp,
                            REG(a1) UBYTE *buf,
                            REG(d0) LONG size,
                            REG(a6) struct IFFParseLib *IFFParseBase)
{
    return (WriteChunkRecords (iffp, buf, 1L, size));
}


/*****************************************************************************/


LONG ASM WriteChunkRecordsLVO(REG(a0) struct IFFHandleP	*iffp,
                              REG(a1) UBYTE *buf,
                              REG(d0) LONG item_size,
                              REG(d1) LONG num_items,
                              REG(a6) struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP	*top;
LONG			size, error;

	if (!(top = (struct ContextNodeP *) CurrentChunk (iffp)))
		return (IFFERR_EOF);

	if (top->cnp_Size != IFFSIZE_UNKNOWN) {
		LONG	maxrecs;

		size = top->cnp_Size - top->cnp_Scan;

		/*  Stupid run-time optimization.  */
		if (item_size == 1)
			maxrecs = size;
		else
			maxrecs = size / item_size;

		if (maxrecs < num_items)
			num_items = maxrecs;
	}

	/*  Another stupid run-time optimization.  */
	if (item_size == 1)
		size = num_items;
	else
		size = item_size * num_items;

	if (!size)
		return (0);

	if (error = DeferredWrite (iffp, buf, size, IFFParseBase))
		return (error);

	top->cnp_UserScan = (top->cnp_Scan += size);
	return (num_items);
}


/*****************************************************************************/


/* Internal write function.
 * For seekable streams, just send the data directly.  For un-seekable
 * streams, store the data into an internal buffer and link into main list.
 */
static LONG DeferredWrite (struct IFFHandleP *iffp, UBYTE *buf, LONG size,
                           struct IFFParseLib *IFFParseBase)
{
struct WriteBuffer *wb;
UBYTE              *bp;

    if (iffp->iffp_Flags & IFFF_RSEEK)
            return (UserWrite (iffp, buf, size));

    if (!(wb = AllocMem (sizeof (*wb) + size, MEMF_ANY)))
            return (IFFERR_NOMEM);
    bp = (UBYTE *) wb + sizeof (*wb);

    wb->wb_Size = size;
    wb->wb_Buf = bp;
    while (size--)
            *bp++ = *buf++;

    AddTail((struct List *)&iffp->iffp_WriteBuffers,(struct Node *)wb);

    return (0);
}


/*****************************************************************************/


/* Free writes buffered with DeferredWrite(). This function only called
 * in the event of catastrophic failure, or as part of the normal cleanup
 * for write mode files.
 */
VOID FreeBufferedStream(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase)
{
struct WriteBuffer *wb;

    while (wb = (struct WriteBuffer *)RemHead((struct List *)&iffp->iffp_WriteBuffers))
	FreeMem(wb, sizeof(*wb) + wb->wb_Size);
}


/*****************************************************************************/


/* BackPatch function patches a longword back earlier in the stream.
 * Either seeks backward on seekable streams, or patches the buffers
 * for non-seekable streams.  "last" is 1 if this is the last backpatch.
 *
 * ewhac:  This is the third time I've written this comment.  I've separated
 * these into two functions: BackPatch() and WriteWholeSchmere().
 * BackPatch() does what it always did.  WriteWholeSchmere() writes out all
 * buffered data.  The advantage of separation is that the decision to
 * invoke these actions is more appropriately made outside of them, namely,
 * in PopChunkW().
 */
static LONG BackPatch (struct IFFHandleP *iffp, LONG nbytes, LONG val,
                       struct IFFParseLib *IFFParseBase)
{
struct WriteBuffer *wb;
LONG		    error, pos;

    if (iffp->iffp_Flags & IFFF_RSEEK)
    {
        /* For seekable streams, patch back directly */
        if (error = UserSeek(iffp, - nbytes, IFFParseBase))
            return (error);

        if (error = UserWrite(iffp, (UBYTE *) &val, 4))
            return (error);

        if (error = UserSeek(iffp, nbytes - 4, IFFParseBase))
            return (error);
    }
    else
    {
        /* For non-seekable streams, look back nbytes in list */

        pos = 0;
        wb = TAIL(&iffp->iffp_WriteBuffers);
        while (1)
        {
            if (!PREV(wb))         /* TEST in reverse */
                return (IFFERR_SEEK);

            pos += wb->wb_Size;
            if (pos >= nbytes)
                break;

            wb = PREV(wb);
        }

        /* Insert value into data stream
         * (Guaranteed to be completely within buffer block)
         */
        * ((LONG*)(wb->wb_Buf + pos - nbytes)) = val;
}

    return(0);
}


/*****************************************************************************/


/* It is assumed that you will call this at the appropriate time, and at
 * no other time (i.e. at the very last moment).
 */
static LONG WriteWholeSchmere(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase)
{
struct WriteBuffer *wb;
LONG		    error;

    /* Save the whole buffer list out to the stream. */
    if (!(iffp->iffp_Flags & IFFF_RSEEK))
    {
        while (wb = (struct WriteBuffer *)RemHead((struct List *)&iffp->iffp_WriteBuffers))
        {
            if (error = UserWrite(iffp, wb->wb_Buf, wb->wb_Size))
                return (error);

            FreeMem(wb, sizeof (*wb) + wb->wb_Size);
        }
    }

    return(0);
}
