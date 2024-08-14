/* misc.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

struct Window *openwindowtags (struct AmigaGuideLib * ag, Tag tag1,...)
{
    return (OpenWindowTagList (NULL, (struct TagItem *) & tag1));
}

/*****************************************************************************/

VOID setwindowpointer (struct AmigaGuideLib * ag, struct Window * win, Tag tag1,...)
{
    return (SetWindowPointerA (win, (struct TagItem *) & tag1));
}

/*****************************************************************************/

BOOL layoutmenus (struct AmigaGuideLib * ag, VOID * vi, struct Menu * menus, ULONG tag1,...)
{
    return (LayoutMenusA (menus, vi, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG easyrequest (struct AmigaGuideLib * ag, struct Window * win, struct EasyStruct * es, ULONG data,...)
{
    return ((ULONG) EasyRequestArgs (win, es, NULL, (ULONG *) & data));
}

/*****************************************************************************/

BOOL aslrequesttags (struct AmigaGuideLib * ag, struct FileRequester * fr, Tag tag1,...)
{
    return (AslRequest (fr, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG setattrs (struct AmigaGuideLib *ag, Object *o, Tag tag1, ...)
{
    return (SetAttrsA (o, (struct TagItem *)&tag1));
}

/*****************************************************************************/

VOID CloseWindowSafely (struct AmigaGuideLib * ag, struct Window * win)
{
    struct IntuiMessage *msg, *succ;

    Forbid ();

    msg = (struct IntuiMessage *) win->UserPort->mp_MsgList.lh_Head;
    while (succ = (struct IntuiMessage *) msg->ExecMessage.mn_Node.ln_Succ)
    {
	if (msg->IDCMPWindow == win)
	{
	    Remove ((struct Node *) msg);
	    ReplyMsg ((struct Message *) msg);
	}
	msg = succ;
    }
    win->UserPort = NULL;
    ModifyIDCMP (win, NULL);

    Permit ();

    /* clear the menu strip */
    if (win->MenuStrip)
	ClearMenuStrip (win);

    /* close the window */
    CloseWindow (win);
}

/*****************************************************************************/

struct MsgPort *CreatePort (struct AmigaGuideLib * ag, STRPTR name, BYTE pri)
{
    struct MsgPort *mp;

    if (mp = CreateMsgPort ())
    {
	mp->mp_Node.ln_Name = name;
	mp->mp_Node.ln_Pri = pri;

	Forbid ();
	if (!FindPort (name))
	{
	    AddPort (mp);
	}
	else
	{
	    DeleteMsgPort (mp);
	    mp = NULL;
	}
	Permit ();
    }
    return (mp);
}

/*****************************************************************************/

VOID DeletePort (struct AmigaGuideLib * ag, struct MsgPort * mp)
{

    if (mp)
    {
	RemPort (mp);
	DeleteMsgPort (mp);
    }
}

/*****************************************************************************/

VOID *AllocPVec (struct AmigaGuideLib * ag, void *pool, ULONG byteSize)
{
#if 1
    return (AllocVec (byteSize, MEMF_CLEAR));
#else
    ULONG realSize;
    ULONG *mem;

    realSize = byteSize + sizeof (ULONG);
    if (mem = AllocPooled (pool, realSize))
	*mem++ = realSize;
    return ((void *) mem);
#endif
}

/*****************************************************************************/

VOID FreePVec (struct AmigaGuideLib * ag, void *pool, void *memoryBlock)
{
#if 1
    FreeVec (memoryBlock);
#else
    ULONG *mem;

    if (memoryBlock)
    {
	mem = ((ULONG *) memoryBlock) - 1;
	FreePooled (pool, mem, *mem);
    }
#endif
}

/*****************************************************************************/

/* Martin Taillefer's block pointer */
static UWORD chip Block_sdata[] =
{
    0x0000, 0x0000,		/* reserved, must be NULL */

    0x0000, 0x0100,
    0x0100, 0x0280,
    0x0380, 0x0440,
    0x0100, 0x0280,
    0x0100, 0x0ee0,
    0x0000, 0x2828,
    0x2008, 0x5834,
    0x783c, 0x8002,
    0x2008, 0x5834,
    0x0000, 0x2828,
    0x0100, 0x0ee0,
    0x0100, 0x0280,
    0x0380, 0x0440,
    0x0100, 0x0280,
    0x0000, 0x0100,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,		/* reserved, must be NULL */
};

VOID SetBlockPointer (struct AmigaGuideLib * ag, struct Window * win)
{
    SetPointer (win, Block_sdata, 32, 16, -8, -7);
}

/*****************************************************************************/

VOID PrintF (struct AmigaGuideLib * ag, struct Client * cl, LONG mode, LONG id, STRPTR arg1,...)
{
    struct EasyStruct es;
    UBYTE namebuffer[64];
    STRPTR ermsg = NULL;

    if (id == 0)
	return;

    if (id < 990)
    {
	if (Fault (id, NULL, cl->cl_WorkText, 510))
	{
	    ermsg = cl->cl_WorkText;
	}
    }
    else if ((id >= 2000) && (id <= 2999))
    {
	ermsg = GetDTString (id);
    }
    else
    {
	ermsg = GetAmigaGuideStringLVO (ag, id);
    }

    if (ermsg)
    {
	if ((mode == 2) && cl->cl_WinObject)
	{
	    sprintf (namebuffer, ermsg, (LONG *) arg1);
	    setattrs (ag, cl->cl_WinObject, WOA_Title, (ULONG) namebuffer, TAG_DONE);
	}
	else if (cl->cl_Process->pr_CLI && (mode == 0))
	{
	    VPrintf (ermsg, (LONG *) & arg1);
	    VPrintf ("\n", NULL);
	}
	else
	{
	    es.es_StructSize = sizeof (struct EasyStruct);
	    es.es_Flags = 0;
	    es.es_Title = GetAmigaGuideStringLVO (ag, TITLE_AMIGAGUIDE);
	    es.es_TextFormat = ermsg;
	    es.es_GadgetFormat = GetAmigaGuideStringLVO (ag, LABEL_CONTINUE);

	    EasyRequestArgs (cl->cl_Window, &es, NULL, &arg1);
	}
    }
}
