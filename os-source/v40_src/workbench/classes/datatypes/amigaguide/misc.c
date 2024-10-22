/* misc.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

APTR newdtobject (struct AGLib *ag, STRPTR name, Tag tag1,...)
{
    return (NewDTObjectA (name, (struct TagItem *)&tag1));
}

/*****************************************************************************/

struct NamedObject *ano (struct AGLib *ag, STRPTR name, Tag tag1, ...)
{
    return (AllocNamedObjectA (name, (struct TagItem *)&tag1));
}

/*****************************************************************************/

ULONG dogadgetmethod (struct AGLib *ag, struct Gadget *g, struct Window *w, struct Requester *r, ULONG data, ...)
{
    return (DoGadgetMethodA (g, w, r, (Msg)&data));
}

/*****************************************************************************/

ULONG notifyAttrChangesDM (struct AGLib *ag, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

ULONG notifyAttrChanges (struct AGLib *ag, Object *o, struct Window *w, struct Requester *r, ULONG flags, ULONG tag1,...)
{
    return (dogadgetmethod (ag, (struct Gadget *)o, w, r, OM_NOTIFY, &tag1, NULL, flags));
}

/*****************************************************************************/

struct Process *createnewproc (struct AGLib *ag, Tag tag1,...)
{
    return (CreateNewProc ((struct TagItem *)&tag1));
}

/*****************************************************************************/

ULONG setattrs (struct AGLib * ag, Object * o, Tag tag1,...)
{
    return (SetAttrsA (o, (struct TagItem *)&tag1));
}

/*****************************************************************************/

ULONG getdtattrs (struct AGLib * ag, Object * o, Tag tag1,...)
{
    return (GetDTAttrsA (o, (struct TagItem *)&tag1));
}

/*****************************************************************************/

Object *newobject (struct AGLib *ag, Class *cl, UBYTE *name, Tag tag1, ...)
{
    return (NewObjectA (cl, name, (struct TagItem *)&tag1));
}

/*****************************************************************************/

ULONG sam (struct AGLib * ag, struct ClientData *cd, Msg msg, ULONG size)
{
    struct SIPCMsg *sm;
    ULONG msize;

    msize = sizeof (struct SIPCMsg) + size + 1;
    if (sm = AllocVec (msize, MEMF_CLEAR))
    {
	sm->sm_Message.mn_Node.ln_Type = NT_MESSAGE;
	sm->sm_Message.mn_Length       = msize;
	sm->sm_Type                    = SIPCT_MESSAGE;
	sm->sm_Data                    = MEMORY_FOLLOWING (sm);
	CopyMem (msg, sm->sm_Data, size);
	PutMsg (cd->cd_SIPCPort, (struct Message *) sm);
	return (1L);
    }
    return (0L);
}

/*****************************************************************************/

void drawbevel (struct AGLib * ag, Object * o, struct RastPort * rp, struct DrawInfo * dri, WORD x, WORD y, WORD w, WORD h, BOOL raised)
{
    if (o)
    {
	struct impDraw imp;

	imp.MethodID              = IM_DRAWFRAME;
	imp.imp_RPort             = rp;
	imp.imp_Offset.X          = x;
	imp.imp_Offset.Y          = y;
	imp.imp_State             = (raised) ? IDS_NORMAL : IDS_SELECTED;
	imp.imp_DrInfo            = dri;
	imp.imp_Dimensions.Width  = w;
	imp.imp_Dimensions.Height = h;
	DoMethodA (o, &imp);
    }
    else
    {
	UBYTE ulpen, lrpen;
	register WORD i;
	WORD bx, by;

	if (raised)
	{
	    ulpen = dri->dri_Pens[SHINEPEN];
	    lrpen = dri->dri_Pens[SHADOWPEN];
	}
	else
	{
	    ulpen = dri->dri_Pens[SHADOWPEN];
	    lrpen = dri->dri_Pens[SHINEPEN];
	}

	/* Calculate the bottom */
	bx = x + w - 1;
	by = y + h - 1;

	/* Draw the vertical lines */
	for (i = 0; i < 2; i++)
	{
	    /* Left side */
	    SetAPen (rp, ulpen);
	    Move (rp, x + i, y);
	    Draw (rp, x + i, by - i);

	    /* Right side */
	    SetAPen (rp, lrpen);
	    Move (rp, bx - i, y + i);
	    Draw (rp, bx - i, by);
	}

	/* Top side */
	SetAPen (rp, ulpen);
	Move (rp, x, y);
	Draw (rp, bx - 1, y);

	/* Bottom side */
	SetAPen (rp, lrpen);
	Move (rp, x + 1, by);
	Draw (rp, bx, by);
    }
}

/*****************************************************************************/

WORD GetLabelsExtent (struct AGLib * ag, struct RastPort *rp, LONG min, LONG max, struct IBox * box)
{
    WORD retval = 0;
    STRPTR label;
    WORD w = 0;
    WORD len;
    LONG i;

    /* Clear the upper-left corner */
    box->Left = box->Top = 0;

    /* Compute the height of the label, plus the gadget border. */
    box->Height = rp->TxHeight + 4;

    /* Check each label */
    for (i = min; i <= max; i++)
    {
	/* Get the string to use */
	label = LVOGetAGString (ag, i);

	/* Remember the maximum width label */
	len = (WORD) TextLength (rp, label, strlen (label));
	w = MAX (w, len);

	/* Increment the label counter */
	retval++;
    }

    /* Add in a little extra space, plus the gadget border. */
    box->Width = w + TextLength (rp, "m", 1) + 8;

    return retval;
}

/*****************************************************************************/

VOID *allocpvec (struct AGLib * ag, void *pool, ULONG byteSize, ULONG attributes)
{
    ULONG realSize;
    ULONG *mem;

    realSize = byteSize + sizeof(ULONG);
    if (mem = AllocPooled (pool, realSize))
        *mem++ = realSize;
    return((void *)mem);
}

/*****************************************************************************/

VOID freepvec (struct AGLib * ag, void *pool, void *memoryBlock)
{
    ULONG *mem;

    if (memoryBlock)
    {
        mem = ((ULONG *)memoryBlock)-1;
        FreePooled (pool, mem, *mem);
    }
}

/*****************************************************************************/

void Strncpy (char *dst, char *src, int num)
{
    while (*src && num--)
	*dst++ = *src++;
    *dst = 0;
}

/*****************************************************************************/

void StrToUpper (struct AGLib *ag, char *dst)
{
    while (*dst)
    {
	*dst = ToUpper (*dst);
	*dst++;
    }
}

/*****************************************************************************/

WORD m_binSearch (STRPTR token, UWORD tokenLen, const struct Keyword *keywords, WORD numKeywords)
{
    WORD l,r,x;
    WORD comp;

    l = 0;
    r = numKeywords-1;
    do
    {
        x = (l+r) / 2;
        comp = strncmp (token, keywords[x].word, tokenLen);
        if (comp < 0)
            r = x-1;
        else if (comp > 0)
            l = x+1;
        else if (strlen(keywords[x].word) != tokenLen)
            r = x-1;
        else
            return((WORD)keywords[x].id);
    }
    while (l <= r);
    return(NOMATCH);
}

/*****************************************************************************/

LONG m_sendCmd (struct AGLib *ag, STRPTR cmd)
{
    struct MsgPort *rexxmp;
    struct RexxMsg *rm;
    struct MsgPort *mp;
    LONG sent = -1;

    if (mp = CreateMsgPort ())
    {
	if (rm = CreateRexxMsg (mp, NULL, NULL))
	{
	    rm->rm_Action = RXCOMM;
	    if (rm->rm_Args[0] = CreateArgstring (cmd, strlen (cmd)))
	    {
		Forbid ();
		if (rexxmp = FindPort (RXSDIR))
		{
		    PutMsg (rexxmp, (struct Message *) rm);
		    sent = 0;
		}
		Permit ();

		if (sent != -1)
		{
		    WaitPort (mp);
		    sent = rm->rm_Result1;
		    SetIoErr (rm->rm_Result2);
		    DB (kprintf ("%ld %ld\n", rm->rm_Result1, rm->rm_Result2));
		}

		DeleteArgstring (rm->rm_Args[0]);
	    }
	    DeleteRexxMsg (rm);
	}
	DeleteMsgPort (mp);
    }
    return (sent);
}
