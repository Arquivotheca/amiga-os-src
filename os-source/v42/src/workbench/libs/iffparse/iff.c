
#include <exec/types.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "iffparsebase.h"
#include "iffprivate.h"
#include "iffa.h"
#include "w.h"
#include "r.h"
#include "iff.h"


/*****************************************************************************/


/* Handler for stop-on-exit chunks. Returns IFFERR_EOC to client. */
static LONG ASM HandleExitStop(VOID)
{
    return (IFFERR_EOC);
}


/*****************************************************************************/


/* Handler for stop chunks.  Just returns control to client */
static LONG ASM HandleStopChunk(VOID)
{
    return (IFF_RETURN2CLIENT);
}


/*****************************************************************************/


/* Characters in [0x20 - 0x7e], no leading spaces (except for ID_NULL) */
LONG ASM GoodIDLVO(REG(d0) LONG id)
{
char *c;
WORD i;
LONG idcopy;

    idcopy = id;
    c = (char *) &idcopy;
    if (*c == ' '  &&  id != ID_NULL)
    {
        /* No leading spaces, except for NULL identifier */
        return (FALSE);
    }

    for (i = 4;  i--;  c++)
    {
        if (*c < ' '  ||  *c > '~')
            return (FALSE);
    }

    return (TRUE);
}


/*****************************************************************************/


/* No lower case or punctuation allowed.
 * (NOTE:  The character range comparisons will have to be changed if ever
 * this code is ported to an EBCDIC machine (Ack!  Phft!))
 */
LONG ASM GoodTypeLVO(REG(d0) LONG type,
                     REG(a6) struct IFFParseLib *IFFParseBase)
{
char	*c, cc;
short	i;
LONG	typecopy;

	if (!GoodID(type))
		return (FALSE);

	typecopy = type;
	c = (char *) &typecopy;
	for (i = 4;  i--;  c++) {
		cc = *c;
		if ((cc < 'A'  ||  cc > 'Z')  &&
		    (cc < '0'  ||  cc > '9')  &&
		    cc != ' ')
			return (FALSE);
	}
	return (TRUE);
}


/*****************************************************************************/


struct IFFHandleP * ASM AllocIFFLVO(REG(a6) struct IFFParseLib *IFFParseBase)
{
struct IFFHandleP   *iffp;
struct ContextNodeP *cnp;

    if (iffp = AllocMem(sizeof(struct IFFHandleP),MEMF_CLEAR))
    {
        NewList((struct List *)&iffp->iffp_Stack);
        NewList((struct List *)&iffp->iffp_WriteBuffers);

        /* Get default context node and initialize. */

        if (cnp = AllocMem(sizeof(*cnp),MEMF_CLEAR))
        {
            NewList((struct List *)&cnp->cnp_LocalItems);
            AddHead((struct List *)&iffp->iffp_Stack, (struct Node *)cnp);
            return(iffp);
        }
        FreeMem(iffp,sizeof(struct IFFHandleP));
    }

    return (NULL);
}


/*****************************************************************************/


VOID ASM FreeIFFLVO(REG(a0) struct IFFHandleP *iffp,
		    REG(a6) struct IFFParseLib *IFFParseBase)
{
    if (iffp)
    {
	/* Issue (ewhac):  It is unclear whether or not this function should
	 * call CloseIFF().  It would be a nice convenience, but it tends to
	 * go against current Amiga philosophy (namely, do it right or die).
	 **
	 * (shf) Problem is with CloseIFF().  CloseIFF() presently calls
	 * PopChunk() which requires the file to be open.  The file can't
	 * be closed until after CloseIFF() and must be closed by the
	 * user before freeing the structure with this function.  So the
	 * user must do CloseIFF(), close the file, then FreeIFF().
	 */
	FreeContextNode(iffp, (struct ContextNodeP *)RemHead((struct List *)&iffp->iffp_Stack),IFFParseBase);
	FreeMem(iffp, sizeof(*iffp));
    }
}


/*****************************************************************************/


/* Sets up a file for reading (or writing) by initializing
 * the NEWIO bit and the stack depth.  Must be done before
 * any attempt is made to do I/O on the stream.
 */
LONG ASM OpenIFFLVO(REG(a0) struct IFFHandleP *iffp,
                    REG(d0) LONG rwmode,
                    REG(a6) struct IFFParseLib *IFFParseBase)
{
struct IFFStreamCmd sc;

    if (!iffp)
        return(IFFERR_NOMEM);

    /* Initialize stack depth. */
    iffp->iffp_Depth = 0;

    /* Initialize file for new I/O. */
    iffp->iffp_Flags &= ~IFFF_RWBITS;
    iffp->iffp_Flags |= IFFFP_NEWIO | (rwmode & IFFF_RWBITS);

    sc.sc_Command = IFFSCC_INIT;
    sc.sc_Buf     = NULL;
    sc.sc_NBytes  = 0;

    return (CallAHook(iffp->iffp_StreamHook, iffp, (LONG *) &sc, IFFParseBase));
}


/*****************************************************************************/


/* All context nodes except the default are freed. */
VOID ASM CloseIFFLVO(REG(a0) struct IFFHandleP *iffp,
		     REG(a6) struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP *top;
LONG		     error;
struct IFFStreamCmd  sc;

    if (iffp)
    {
	/* Free anything left over (except default node).
	 **
	 * (shf) If there is a stream I/O error,
	 * PopChunk() may fail and may not pop the top chunk (as in the
	 * case of a seek error).  Try to do a good cleanup, but failing
	 * that, just free the memory.
	 */
	error = 0;
	while (CurrentChunk(iffp) && !error)
	    error = PopChunk(iffp);

	/* If error is true, PopChunk() failed at some point.
	 * Just do a minimal cleanup if that happens.
	 */
	if (error)
	{
            while (top = (struct ContextNodeP *) CurrentChunk(iffp))
            {
                Remove((struct Node *)top);
                iffp->iffp_Depth--;
                FreeContextNode(iffp, top, IFFParseBase);
            }
        }

	/* Write files may have buffered streams still to free. */

	if ((iffp->iffp_Flags & IFFF_RWBITS) == IFFF_WRITE)
	    FreeBufferedStream(iffp, IFFParseBase);

	sc.sc_Command	= IFFSCC_CLEANUP;
	sc.sc_Buf	= NULL;
	sc.sc_NBytes	= 0;

	CallAHook(iffp->iffp_StreamHook, iffp, (LONG *) &sc, IFFParseBase);
    }
}


/*****************************************************************************/


/* Read and test generic type ID.  Return error or zero. */
static LONG ReadGenericType(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP	*top;

    /* EOF if no top chunk (must have gotten popped off) */

    if (!(top = (struct ContextNodeP *) CurrentChunk (iffp)))
        return (IFFERR_EOF);

    if (ReadChunkBytes (iffp, (UBYTE *) &top->cnp_Type, 4L) != 4)
        return (IFFERR_READ);

    if (!GoodType (top->cnp_Type))
        return (IFFERR_MANGLED);

    return (0);
}


/*****************************************************************************/


/* Test current state and advance to next state.
 * Current state is either:
 * - start of file if NEWIO flag set.
 * - poised at end of chunk if PAUSE flag set.
 * - inside a new chunk (generic or non-generic).
 *
 * - If NEWIO flag set, push initial chunk, get its info and return.
 * - If PAUSE flag set, PopChunk() the current context.  The top context
 *   will then be a generic or end of file.  Return EOF if the latter.
 *   If this chunk is exhausted, PAUSE and return, otherwise push new
 *   chunk and return.
 * - If inside a new generic chunk, push a new context and return.
 * - If inside a non-generic chunk, PAUSE and return.
 *
 * Returns zero if successful or error code.
 */
static LONG NextState(struct IFFHandleP *iffp, struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP	*top;
long			error, topid;

	/* Deal with the case of the first chunk */

	if (iffp->iffp_Flags & IFFFP_NEWIO)
        {
		error = PushChunkR (iffp, 1, IFFParseBase);
		iffp->iffp_Flags &= ~(IFFFP_NEWIO|IFFFP_PAUSE);
		if (error)
			return (error);

		top = (struct ContextNodeP *) CurrentChunk (iffp);
		ASSERT(top);
		if (top->cnp_ID != ID_FORM  &&
		    top->cnp_ID != ID_CAT  &&
		    top->cnp_ID != ID_LIST)
			return (IFFERR_NOTIFF);

                if (top->cnp_Size & 1)
                    return (IFFERR_MANGLED);

		return (ReadGenericType (iffp, IFFParseBase));
	}

	/* In the PAUSE state, do the deferred PopChunk() */

	if (iffp->iffp_Flags & IFFFP_PAUSE)
        {
		if (error = PopChunk (iffp))
		    return (error);

		iffp->iffp_Flags &= ~IFFFP_PAUSE;
	}

	/*
	 * If no top context node then the file is exhausted.
	 */
	if (!(top = (struct ContextNodeP *) CurrentChunk (iffp)))
		return (IFFERR_EOF);

	if (IsGenericID (topid = top->cnp_ID))
        {
		/* If inside a generic chunk, and not exhausted, push a
		 * subchunk.
		 */
		if (top->cnp_Scan < top->cnp_Size)
                {
			if (error = PushChunkR (iffp, 0, IFFParseBase))
				return (error);

			top = (struct ContextNodeP *) CurrentChunk (iffp);
			ASSERT (top);

			/*
			 * If non-generic, we're done, but
			 * if the containing chunk is not FORM or PROP,
			 * it's an IFF syntax error.
			 */
			if (!IsGenericID (top->cnp_ID))
                        {
				if (topid != ID_FORM  &&  topid != ID_PROP)
					return (IFFERR_SYNTAX);
				return (0);
			}

			/*
			 * If this new chunk is generic, and is a PROP,
			 * test to see if it's in the right place --
			 * containing chunk should be a LIST.
			 */
			if (top->cnp_ID == ID_PROP  &&  topid != ID_LIST)
				return (IFFERR_SYNTAX);

			/* Since it's an ok generic, get its type and return */
			return (ReadGenericType (iffp, IFFParseBase));

		/*
		 * If not EXACTLY exhaused, this is a junky IFF file
		 */
		} else if (top->cnp_Scan != top->cnp_Size)
			return (IFFERR_MANGLED);
	}
	/*
	 * If the generic is exhaused, or this is a non-generic chunk,
	 * enter the pause state and return flag.
	 */
	iffp->iffp_Flags |= IFFFP_PAUSE;
	return (IFFERR_EOC);
}


/*****************************************************************************/


/* Driver for the parse engine.  Loops calling NextState() and acting
 * on the next parse state.  NextState() will always cause the parser
 * to enter or exit (really pause before exiting) a chunk.  In each case,
 * either an entry handler or an exit handler will be invoked on the
 * current context.  A handler can cause the parser to return control
 * to the calling program or it can return an error.
 */
LONG ASM ParseIFFLVO(REG(a0) struct IFFHandleP *iffp,
                     REG(d0) LONG control,
                     REG(a6) struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP	 *top;
struct LocalContextItem *lci;
struct ChunkHandler	 *ch;
long			 error, eoc;

	while (1)
        {
		/*
		 * Advance to next state and store end of chunk info.
		 */
		eoc = NextState (iffp, IFFParseBase);
		if (eoc  &&  eoc != IFFERR_EOC)
			return (eoc);

		/*
		 * User requesting manual override -- return immediately
		 */
		if (control == IFFPARSE_RAWSTEP)
			break;

		if (!(top = (struct ContextNodeP *) CurrentChunk (iffp)))
			return (IFFERR_EOF);

		/*
		 * Find chunk handlers for entering or exiting a context.
		 */
		if (lci = FindLocalItem
			   (iffp, top->cnp_Type, top->cnp_ID,
			   (eoc ? IFFLCI_EXITHANDLER : IFFLCI_ENTRYHANDLER)))
		{
			LONG	cmd;

			ch = (struct ChunkHandler *) LocalItemData (lci);
			cmd = eoc ? IFFCMD_EXIT : IFFCMD_ENTRY;
			error = CallAHook(ch->ch_HandlerHook,
					  ch->ch_Object,&cmd,IFFParseBase);

			if (error == IFF_RETURN2CLIENT)
				return (0);
			if (error)
				return (error);
		}

		if (control == IFFPARSE_STEP)
			break;
	}
	return (eoc);
}


/*****************************************************************************/


/* See PushChunkW() and PushChunkR() for more */
LONG ASM PushChunkLVO(REG(a0) struct IFFHandleP	*iffp,
                      REG(d0) LONG type,
                      REG(d1) LONG id,
                      REG(d2) LONG size,
                      REG(a6) struct IFFParseLib *IFFParseBase)
{
    if ((iffp->iffp_Flags & IFFF_RWBITS) == IFFF_WRITE)
	return (PushChunkW (iffp, type, id, size, IFFParseBase));

    return (PushChunkR (iffp, 0, IFFParseBase));
}


/*****************************************************************************/


/* See PopChunkR() and PopChunkW() for more */
LONG ASM PopChunkLVO(REG(a0) struct IFFHandleP *iffp,
		     REG(a6) struct IFFParseLib *IFFParseBase)
{
    if ((iffp->iffp_Flags & IFFF_RWBITS) == IFFF_WRITE)
	return (PopChunkW (iffp,IFFParseBase));

    return (PopChunkR (iffp,IFFParseBase));
}


/*****************************************************************************/


struct LocalContextItem * ASM AllocLocalItemLVO(REG(d0) LONG type,
                                                REG(d1) LONG id,
                                                REG(d2) LONG ident,
                                                REG(d3) LONG usize,
                                                REG(a6) struct IFFParseLib *IFFParseBase)
{
struct LocalContextItemP *lip;

    if (lip = AllocVec(sizeof (*lip) + usize, MEMF_CLEAR))
    {
        /* Purge and Stub vectors are already NULL-ed for
         * default delete action.
         */
        lip->lip_ID    = id;
        lip->lip_Type  = type;
        lip->lip_Ident = ident;
    }

    return ((struct LocalContextItem *)lip);
}


/*****************************************************************************/


VOID ASM FreeLocalItemLVO(REG(a0) struct LocalContextItemP *item,
                          REG(a6) struct IFFParseLib *IFFParseBase)
{
    FreeVec(item);
}


/*****************************************************************************/


VOID ASM SetLocalItemPurgeLVO(REG(a0) struct LocalContextItemP *lip,
                              REG(a1) struct Hook *hook)
{
    lip->lip_PurgeHook = hook;
}


/*****************************************************************************/


/* Free a LocalContextItem.  Call either its purge vector, or just free it. */
static LONG PurgeLocalContextItem(struct IFFHandleP *iffp,
                                  struct LocalContextItemP *lip,
                                  struct IFFParseLib *IFFParseBase)
{
struct Hook *hook;
LONG         cmd = IFFCMD_PURGELCI;

    /* If the purge vector is null, use FreeLocalItem() */
    if (hook = lip->lip_PurgeHook)
        return (CallAHook(hook, lip, &cmd, IFFParseBase));

    FreeLocalItem(lip);
    return (0);
}


/*****************************************************************************/


APTR ASM LocalItemDataLVO(REG(a0) struct LocalContextItemP *lip)
{
    if (!lip)
	return (NULL);

    return((APTR)(lip + 1));
}


/*****************************************************************************/


/* Deallocate memory associated with a ContextNodeP struct
 * calling free vector for local context items.  ContextNodeP must
 * have already been removed from stack.
 */
VOID FreeContextNode(struct IFFHandleP *iffp, struct ContextNodeP *cnp,
		     struct IFFParseLib *IFFParseBase)
{
struct LocalContextItemP *lip;

    /* Dispose of all local context items for this context node.
     * error return value is ignored for now.
     */
    while (lip = (struct LocalContextItemP *)RemHead ((struct List *)&cnp->cnp_LocalItems))
        PurgeLocalContextItem (iffp, lip, IFFParseBase);

    FreeMem(cnp, (LONG) sizeof (*cnp));
}


/*****************************************************************************/


/* Install a handler node into the local context.  Handler vector and
 * purge vector will use the same stub if provided.  Pos is one of
 * the IFFSLI codes for where to locate the handler.
 */
static LONG InstallHandler(struct IFFHandleP *iffp, LONG type, LONG id,
                           LONG ident, LONG pos, struct Hook *hook,
                           APTR object, struct IFFParseLib *IFFParseBase)
{
struct LocalContextItemP	*lci;
struct ChunkHandler		*ch;
LONG				error;

	if (!(lci = (struct LocalContextItemP *)
	      AllocLocalItem (type, id, ident, (long) sizeof (*ch))))
		return (IFFERR_NOMEM);

	ch = (struct ChunkHandler *) LocalItemData (lci);

	ch->ch_HandlerHook = hook;
	ch->ch_Object = object;

	if (error = StoreLocalItem (iffp, lci, pos))
	    FreeLocalItem (lci);

	return (error);
}


/*****************************************************************************/


LONG ASM EntryHandlerLVO(REG(a0) struct IFFHandleP *iffp,
                         REG(d0) LONG type,
                         REG(d1) LONG id,
                         REG(d2) LONG pos,
                         REG(a1) struct Hook *hook,
                         REG(a2) APTR object,
                         REG(a6) struct IFFParseLib *IFFParseBase)
{
    return (InstallHandler(iffp,type,id,IFFLCI_ENTRYHANDLER,pos,hook,object,IFFParseBase));
}


/*****************************************************************************/


LONG ASM ExitHandlerLVO(REG(a0) struct IFFHandleP *iffp,
                        REG(d0) LONG type,
                        REG(d1) LONG id,
                        REG(d2) LONG pos,
                        REG(a1) struct Hook *hook,
                        REG(a2) APTR object,
                        REG(a6) struct IFFParseLib *IFFParseBase)
{
    return (InstallHandler(iffp,type,id,IFFLCI_EXITHANDLER,pos,hook,object,IFFParseBase));
}


#if 0
/*
 * This takes a list of chunks and does func(iff,type,id) on all of them.
 * The format of the list is:
 *
 * LONG prop_list[NPROPS * 2] = {  <type>, <id>, [...]  };
 *
 * Returns zero if successful, error otherwise.
 */
static LONG __regargs
ListAction (iffp, list, n, func)
struct IFFHandleP	*iffp;
LONG		*list;
LONG		n;
LONG		(*func)();
{
	register long	type, id, error;
	register short	i = n;

	while (i--) {
		type = *list++;
		id   = *list++;
		if (error = (*func) (iffp, type, id))
			return (error);
	}
	return (0);
}
#endif


/*****************************************************************************/


/* Store contents of a chunk in a buffer and return 0 or error code */
static LONG BufferChunk(struct IFFHandleP *iffp, LONG size, void *buf,
                        struct IFFParseLib *IFFParseBase)
{
LONG rsize;

    /* Read and store the chunk data testing for errors. */

    if ((rsize = ReadChunkBytes (iffp, buf, size)) != size)
    {
        if (rsize > 0)
            return (IFFERR_READ);

        return (rsize);
    }

    return (0);
}


/*****************************************************************************/


/* Handler function for storing property chunks into local context.
 * The contents of the current chunk -- which is the property in
 * question -- get read into a buffer which is attached to a local
 * context item in the current "PROP" context.
 */
static LONG ASM HandleProperty(REG(a0) struct Hook *hook,
                               REG(a2) struct IFFHandleP *iffp,
                               REG(a1) LONG *cmd,
                               REG(a6) struct IFFParseLib *IFFParseBase)
{
struct LocalContextItem *lci;
struct StoredProperty	 *sp;
struct ContextNodeP	 *cnp;
LONG			 error, size;

	/* Get Local Item and memory buffer.
	 */
	cnp = (struct ContextNodeP *) CurrentChunk (iffp);
	size = cnp->cnp_Size;
	if (!(lci = AllocLocalItem (cnp->cnp_Type,
				    cnp->cnp_ID,
				    IFFLCI_PROP,
				    size + sizeof (*sp))))
		return (IFFERR_NOMEM);

	sp = (struct StoredProperty *) LocalItemData (lci);

	/*
	 * Store contents of property chunk in local item.
	 */
	if (error = BufferChunk (iffp, size, sp + 1, IFFParseBase)) {
		FreeLocalItem (lci);
		return (error);
	}

	/*
	 * Init the StoredProperty structure for this data item and
	 * store the item in the current PROP context.
	 */
	sp->sp_Size = size;
	sp->sp_Data = (UBYTE *) (sp + 1);

	if (error = StoreLocalItem (iffp, lci, IFFSLI_PROP)) {
		FreeLocalItem (lci);
		return (error);
	}
	return (0);
}


/*****************************************************************************/


LONG ASM PropChunkLVO(REG(a0) struct IFFHandleP	*iffp,
                      REG(d0) LONG type,
                      REG(d1) LONG id,
                      REG(a6) struct IFFParseLib *IFFParseBase)
{
static const struct Hook InternalPropHook = {
	{ NULL },
	HandleProperty,
	NULL,
	NULL
};

    return (EntryHandler(iffp,
			 type, id,
			 IFFSLI_TOP,
			 &InternalPropHook, iffp));
}


/*****************************************************************************/


LONG ASM PropChunksLVO(REG(a0) struct IFFHandleP	*iffp,
                       REG(a1) LONG *list,
                       REG(d0) LONG n,
                       REG(a6) struct IFFParseLib *IFFParseBase)
{
    return (ListAction(iffp,list,n,PropChunkLVO,IFFParseBase));
}


/*****************************************************************************/


LONG ASM StopChunkLVO(REG(a0) struct IFFHandleP	*iffp,
                      REG(d0) LONG type,
                      REG(d1) LONG id,
                      REG(a6) struct IFFParseLib *IFFParseBase)
{
static const struct Hook	InternalStopHook = {
	{ NULL },
	HandleStopChunk,
	NULL,
	NULL
};

    return (EntryHandler(iffp,
			 type, id,
			 IFFSLI_TOP,
			 &InternalStopHook, iffp));
}


/*****************************************************************************/


LONG ASM StopChunksLVO(REG(a0) struct IFFHandleP *iffp,
                       REG(a1) LONG *list,
                       REG(d0) LONG n,
                       REG(a6) struct IFFParseLib *IFFParseBase)
{
   return (ListAction(iffp,list,n,StopChunkLVO,IFFParseBase));
}


/*****************************************************************************/


/* Purge a collection list node and all collection items within its
 * scope (given by First, LastPtr).
 */
static LONG ASM PurgeCollectionList(REG(a0) struct Hook *hook,
                                    REG(a2) struct LocalContextItem *lci,
                                    REG(a1) LONG *cmd,
                                    REG(a6) struct IFFParseLib *IFFParseBase)
{
struct CollectionList	*cl;
struct CollectionItem	*ci, *nextci;

	cl = (struct CollectionList *) LocalItemData (lci);

	for (ci = cl->cl_First;
	     ci != cl->cl_LastPtr  &&  ci;
	     ci = nextci)
	{
		nextci = ci->ci_Next;
		FreeMem (ci, sizeof (*ci) + ci->ci_Size);
	}
	FreeLocalItem (lci);
	return (0);
}


/*****************************************************************************/


/* Encountered a collection property chunk.  Store this at the head of
 * the appropriate list.  If no CollectionList at this level, create one.
 */
static LONG ASM HandleCollectionChunk(REG(a0) struct Hook *hook,
                                      REG(a2) struct IFFHandleP *iffp,
                                      REG(a1) LONG *cmd,
                                      REG(a6) struct IFFParseLib *IFFParseBase)
{
struct CollectionList	*cltop;
struct CollectionItem	*ci;
LONG			size, error;
struct LocalContextItem		*lci = NULL;
struct CollectionList		*clast;
struct ContextNodeP		*cnp, *ctxt;
static const struct Hook		InternalPurgeHook = {
	{ NULL },
	PurgeCollectionList,
	NULL,
	NULL
};

	/* Read the chunk into a buffer and create a collectionitem for it */

	cnp = (struct ContextNodeP *) CurrentChunk (iffp);
	size = cnp->cnp_Size;
	if (!(ci = AllocMem (size + sizeof (*ci), 0L)))
		return (IFFERR_NOMEM);

	if (error = BufferChunk (iffp, size, ci + 1, IFFParseBase))
		goto ackphft;

	ci->ci_Size = size;
	ci->ci_Data = (UBYTE *) (ci + 1);

	/* Locate current PROP context (ala. StoreProp). */

	if (!(ctxt = (struct ContextNodeP *) FindPropContext (iffp)))
 	    return (IFFERR_NOSCOPE);

	/* Find the top collection list node for this context. */

	clast = (struct CollectionList *)
	 LocalItemData (FindLocalItem (iffp, cnp->cnp_Type, cnp->cnp_ID,
				       IFFLCI_COLLECTION));
	/* If this list item is at the same level as the current chunk,
	 * this is the node for the current chunk and the new buffer
	 * just needs to be linked in.  If this is at a different
	 * level, or there is none at all, a new cl node for this
	 * level needs to be created.
	 */
	if (clast  &&  (clast->cl_Context == (struct ContextNode *) ctxt))
	{
		cltop = clast;
	} else {
		if (!(lci = AllocLocalItem (cnp->cnp_Type,
					    cnp->cnp_ID,
					    IFFLCI_COLLECTION,
					    (LONG) sizeof (*cltop))))
		{
			error = IFFERR_NOMEM;
			goto ackphft;
		}
		SetLocalItemPurge (lci, &InternalPurgeHook);
		cltop = (struct CollectionList *) LocalItemData (lci);

		cltop->cl_Context = (struct ContextNode *) ctxt;
		cltop->cl_LastPtr = NULL;
		if (clast)
			cltop->cl_LastPtr = clast->cl_First;
		cltop->cl_First = cltop->cl_LastPtr;

		if (error = StoreLocalItem (iffp, lci, IFFSLI_PROP))
			goto ackphft;
	}

	/* Link new CollectionItem at head of list. */

	ci->ci_Next = cltop->cl_First;
	cltop->cl_First = ci;

	return (0);

ackphft:
	if (lci)
		FreeLocalItem (lci);
	FreeMem (ci, sizeof (*ci) + size);

	return (error);
}


/*****************************************************************************/


LONG ASM CollectionChunkLVO(REG(a0) struct IFFHandleP *iffp,
                            REG(d0) LONG type,
                            REG(d1) LONG id,
                            REG(a6) struct IFFParseLib *IFFParseBase)
{
static const struct Hook InternalCollHook = {
	{ NULL },
	HandleCollectionChunk,
	NULL,
	NULL
};

    return (EntryHandler(iffp,
			 type, id,
			 IFFSLI_TOP,
			 &InternalCollHook, iffp));
}


/*****************************************************************************/


LONG ASM CollectionChunksLVO(REG(a0) struct IFFHandleP *iffp,
                             REG(a1) LONG *list,
                             REG(d0) LONG n,
                             REG(a6) struct IFFParseLib *IFFParseBase)
{
    return (ListAction(iffp,list,n,CollectionChunkLVO,IFFParseBase));
}


/*****************************************************************************/


LONG ASM StopOnExitLVO(REG(a0) struct IFFHandleP *iffp,
                       REG(d0) LONG type,
                       REG(d1) LONG id,
                       REG(a6) struct IFFParseLib *IFFParseBase)
{
static const struct Hook InternalExitHook = {
	{ NULL },
	HandleExitStop,
	NULL,
	NULL
};

    return (ExitHandler(iffp,
			type, id,
			IFFSLI_TOP,
			&InternalExitHook, iffp));
}


/*****************************************************************************/


struct StoredProperty * ASM FindPropLVO(REG(a0) struct IFFHandleP *iffp,
                                        REG(d0) LONG type,
                                        REG(d1) LONG id,
                                        REG(a6) struct IFFParseLib *IFFParseBase)
{
    return ((struct StoredProperty *) LocalItemData(FindLocalItem(iffp,type,id,IFFLCI_PROP)));
}


/*****************************************************************************/


struct CollectionItem * ASM FindCollectionLVO(REG(a0) struct IFFHandleP *iffp,
                                              REG(d0) LONG type,
                                              REG(d1) LONG id,
                                              REG(a6) struct IFFParseLib *IFFParseBase)
{
struct CollectionList	*cl;

    if (!(cl = (struct CollectionList *) LocalItemData (
			FindLocalItem (iffp, type, id, IFFLCI_COLLECTION))))
		return (NULL);

    return (cl->cl_First);
}


/*****************************************************************************/


LONG ASM StoreLocalItemLVO(REG(a0) struct IFFHandleP *iffp,
                           REG(a1) struct LocalContextItem *li,
                           REG(d0) LONG mode,
                           REG(a6) struct IFFParseLib *IFFParseBase)
{
struct ContextNodeP	*cnp;

	switch (mode) {
	case IFFSLI_ROOT:
		cnp = TAIL (&iffp->iffp_Stack);
		break;

	case IFFSLI_TOP:
		cnp = HEAD (&iffp->iffp_Stack);
		break;

	case IFFSLI_PROP:
		if (!(cnp = (struct ContextNodeP *) FindPropContext (iffp)))
			return (IFFERR_NOSCOPE);
		break;
	}

	StoreItemInContext (iffp, li, cnp);
	return (0);
}


/*****************************************************************************/


VOID ASM StoreItemInContextLVO(REG(a0) struct IFFHandleP *iffp,
                               REG(a1) struct LocalContextItemP *item,
                               REG(a2) struct ContextNodeP *context,
                               REG(a6) struct IFFParseLib *IFFParseBase)
{
struct LocalContextItemP	*next;
LONG				ident, ID, type;

	AddHead ((struct List *)&context->cnp_LocalItems, (struct Node *)item);

	/*
	 * Search other elements in this list for an identical
	 * property, and delete it if found.
	 */
	ident	= item->lip_Ident;
	ID	= item->lip_ID;
	type	= item->lip_Type;

	/*
	 * Search for and delete duplicates.
	 */
	for (item = (struct LocalContextItemP *)NEXT (item);
                    (struct LocalContextItemP *)TEST (item);
                    item = next)
        {
		next = NEXT (item);
		if (item->lip_Ident == ident  &&
		    item->lip_ID == ID  &&
		    item->lip_Type == type)
		{
			Remove ((struct Node *)item);
			PurgeLocalContextItem (iffp, item, IFFParseBase);
			break;
		}
	}
}


/*****************************************************************************/


#if 0
void PrintID(LONG id)
{
	char	buf[5];

	printf ("%s", IDtoStr (id, buf));
}

void PrintContext(struct IFFHandleP	*iffp)
{
struct ContextNodeP	*cnp;
struct LocalContextItem	*li;

	for (cnp = HEAD (&iffp->iffp_Stack);
	     TEST (cnp);
	     cnp = NEXT (cnp))
	{
		printf ("context node: ");
		PrintID (cnp->cnp_Type);
		printf (".");
		PrintID (cnp->cnp_ID);
		printf ("\n");
		for (li = HEAD (&cnp->cnp_LocalItems);
		     TEST (li);
		     li = NEXT (li))
		{
			printf ("  local item:  ");
			PrintID (li->lci_Ident);
			printf ("  ");
			PrintID (li->lci_Type);
			printf (".");
			PrintID (li->lci_ID);
			printf ("\n");
		}
	}
}
#endif
