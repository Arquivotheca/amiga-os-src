/* stubs.c
 *
 */

#include "multiview.h"

/*****************************************************************************/

/* Martin Taillefer's block pointer */
#if 1
static UWORD __chip blockData[30] =
{
    /* Plane 0 */
    0x0000,0x0100,0x0380,0x0100,0x0100,0x0000,0x2008,0x783C,
    0x2008,0x0000,0x0100,0x0100,0x0380,0x0100,0x0000,

    /* Plane 1 */
    0x0100,0x0280,0x0440,0x0280,0x0EE0,0x2828,0x5834,0x8002,
    0x5834,0x2828,0x0EE0,0x0280,0x0440,0x0280,0x0100
};
#else
static UWORD __chip blockData[] =
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
#endif

/*****************************************************************************/

Object *AllocBlockPointer (struct GlobalData *gd)
{
    struct BitMap *bm = &gd->gd_BlockBM;

    /* Initialize the bitmap */
    InitBitMap (bm, 2, 16, 15);
    bm->Planes[0] = (PLANEPTR) &blockData[0];
    bm->Planes[1] = (PLANEPTR) &blockData[15];

    /* Return a pointer object */
    return (newobject (gd, NULL, POINTERCLASS,
		    POINTERA_BitMap,		bm,
		    POINTERA_WordWidth,		(16 >> 4),
		    POINTERA_XOffset,		-8,
		    POINTERA_YOffset,		-7,
		    POINTERA_XResolution,	POINTERXRESN_HIRES,
		    POINTERA_YResolution,	POINTERYRESN_HIGH,
		    TAG_DONE));
}

/*****************************************************************************/

void FreeBlockPointer (struct GlobalData *gd, Object *po)
{
    DisposeObject (po);
}

/*****************************************************************************/

VOID SetBlockPointer (struct GlobalData * gd, struct Window * win)
{
#if 1
    setwindowpointer (gd, win, WA_Pointer, gd->gd_BlockO, TAG_DONE);
#else
    SetPointer (win, blockData, 32, 16, -8, -7);
#endif
}

/*****************************************************************************/

ULONG easyrequest (struct GlobalData * gd, struct EasyStruct * es, ULONG data,...)
{
    return ((ULONG) EasyRequestArgs (gd->gd_Window, es, NULL, (ULONG *) & data));
}

/*****************************************************************************/

struct FileRequester *allocaslrequest (struct GlobalData *gd, ULONG kind, Tag tag1, ...)
{
    return ((struct FileRequester *)AllocAslRequest (kind, (struct TagItem *)&tag1));
}

/*****************************************************************************/

BOOL aslrequesttags (struct GlobalData *gd, struct FileRequester *fr, Tag tag1, ...)
{
    return (AslRequest (fr, (struct TagItem *)&tag1));
}

/*****************************************************************************/

ULONG setgadgetattrs (struct GlobalData *gd, struct Gadget *g, struct Window *w, Tag tag1, ...)
{
    return (SetGadgetAttrsA (g, w, NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG setdtattrs (struct GlobalData *gd, Object *o, struct Window *w, Tag tag1, ...)
{
    return (SetDTAttrsA (o, w, NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG getdtattrs (struct GlobalData *gd, Object *o, Tag tag1, ...)
{
    return (GetDTAttrsA (o, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG setattrs (struct GlobalData *gd, Object *o, Tag tag1, ...)
{
    return (SetAttrsA (o, (struct TagItem *) &tag1));
}

/*****************************************************************************/

APTR newobject (struct GlobalData *gd, Class *cl, STRPTR name, Tag tag1, ...)
{
    return (NewObjectA (cl, name, (struct TagItem *)&tag1));
}

/*****************************************************************************/

struct Screen *openscreentags (struct GlobalData *gd, Tag tag1, ...)
{
    return (OpenScreenTagList (NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

struct Window *openwindowtags (struct GlobalData *gd, Tag tag1, ...)
{
    return (OpenWindowTagList (NULL, (struct TagItem *) &tag1));
}

/*****************************************************************************/

struct Menu *createmenus (struct GlobalData *gd, struct NewMenu *nm, Tag tag1, ...)
{
    return (CreateMenusA (nm, (struct TagItem *)&tag1));
}

/*****************************************************************************/

struct Menu *layoutmenus (struct GlobalData *gd, struct Menu *menu, Tag tag1, ...)
{
    return ((struct Menu *)LayoutMenusA (menu, gd->gd_VI, (struct TagItem *)&tag1));
}

/*****************************************************************************/

VOID setwindowpointer (struct GlobalData *gd, struct Window *win, Tag tag1, ...)
{
    if (win)
	SetWindowPointerA (win, (struct TagItem *)&tag1);
}

/*****************************************************************************/

VOID CloseWindowSafely (struct GlobalData *gd, struct Window * win)
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
