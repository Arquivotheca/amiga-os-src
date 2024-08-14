/* sketchgclass.c --- sketch boopsi object
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 * This gadget class is tightly weaved with the
 * Pointer preference editor.  It can not
 * standalone.
 *
 */

#include "wbpattern.h"

#define G(o)		((struct Gadget *)(o))
#define	VM	1
#define	HM	1

/*****************************************************************************/

/* classface.asm */
ULONG __stdargs DoMethod (Object * o, ULONG methodID, ...);
ULONG __stdargs DoSuperMethod (Class * cl, Object * o, ULONG methodID, ...);
ULONG __stdargs CoerceMethod (Class * cl, Object * o, ULONG methodID, ...);
ULONG __stdargs SetSuperAttrs (Class * cl, Object * o, ULONG data, ...);
ULONG __stdargs CoerceMethodA (Class * cl, Object * o, Msg msg);
ULONG __stdargs DoMethodA (Object * o, Msg msg);
ULONG __stdargs DoSuperMethodA (Class * cl, Object * o, Msg msg);

/*****************************************************************************/

extern UWORD ghost_pattern[];

/*****************************************************************************/

struct localObjData
{
    struct BitMap	 lod_Work;		/* Magnify work bitmap */
    UWORD		 lod_WWidth;		/* Width of work bitmap */
    UWORD		 lod_WHeight;		/* Height of work bitmap */
};

/*****************************************************************************/

#define SUPERCLASSID	"gadgetclass"

/*****************************************************************************/

Class *initsketchGClass (EdDataPtr ed)
{
    ULONG HookEntry ();
    Class *cl;

    /* Make the class */
    if (cl = MakeClass (SKETCHCLASSID, SUPERCLASSID, NULL,
			(LONG) sizeof (struct localObjData), 0L))
    {
	/* Fill in the callback hook */
	cl->cl_Dispatcher.h_Entry = HookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchsketchG;
	cl->cl_UserData = (ULONG) ed;
    }

    /* Return the public class */
    return (cl);
}

/*****************************************************************************/

ULONG freesketchGClass (EdDataPtr ed, Class * cl)
{

    /* Try to free the public class */
    return ((ULONG) FreeClass (cl));
}

/*****************************************************************************/

ULONG __stdargs dispatchsketchG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;
    struct RastPort *rp;
    ULONG retval = 0L;
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Allow our superclass to create the base */
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		if (initsketchGAttrs (cl, newobj, (struct opSet *) msg))
		{
		    /* Set the attributes */
		    setsketchGAttrs (cl, newobj, (struct opSet *) msg);
		}
		else
		{
		    /* Free what's there if failure */
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }

	    retval = (ULONG) newobj;
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Do the superclass first */
	    retval = DoSuperMethodA (cl, o, msg);

	    /* Call our set routines */
	    retval += setsketchGAttrs (cl, o, (struct opSet *) msg);

	    /* See if we should refresh */
	    if (retval && (OCLASS (o) == cl))
	    {
		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    struct gpRender gpr = {NULL};

		    /* Force a redraw */
		    gpr.MethodID   = GM_RENDER;
		    gpr.gpr_GInfo  = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort  = rp;
		    gpr.gpr_Redraw = GREDRAW_UPDATE;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}

		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	    /* Render the graphics */
	case GM_RENDER:
	    /* Render the button */
	    rendersketchG (cl, o, (struct gpRender *) msg, 0, 0, FALSE);

	    break;

	    /* Always respond to hit */
	case GM_HITTEST:
	    retval = GMR_GADGETHIT;
	    break;

	    /* Handle input */
	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    /* Handle the input */
	    retval = handlesketchG (cl, o, (struct gpInput *) msg);
	    break;

	    /* Delete ourself */
	case OM_DISPOSE:
	    /* Free the bitmaps that we use */
	    freebitmap (ed, &lod->lod_Work, lod->lod_WWidth, lod->lod_WHeight);

	    /* Pass it up to the superclass */
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

/* initialize attributes of an object */
ULONG initsketchGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;

    lod->lod_WWidth = ed->ed_Width * ed->ed_XMag;
    lod->lod_WHeight = ed->ed_Height * ed->ed_YMag;

    if (allocbitmap (ed, &lod->lod_Work, ed->ed_Depth, lod->lod_WWidth, lod->lod_WHeight))
    {
	return (TRUE);
    }
    freebitmap (ed, &lod->lod_Work, lod->lod_WWidth, lod->lod_WHeight);

    return (FALSE);
}

/*****************************************************************************/

/* CHANGE ALL THESE TO METHODS!!! */
/* Set attributes of an object */
ULONG setsketchGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG refresh = 0L;
    ULONG tidata;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	switch (tag->ti_Tag)
	{
	    case SPA_Store:
		Store (ed);
		refresh = 1;
		break;

	    case SPA_Restore:
		Restore (ed);
		refresh = 1;
		break;

	    case SPA_Clear:
		SetPrefsRast (ed, ed->ed_APen);
		refresh = 1;
		break;

	    case SPA_Erase:
		SetPrefsRast (ed, 0);
		refresh = 1;
		break;

	    case SPA_Update:
		refresh = 1;
		break;

	    case GA_Disabled:
		refresh = 1;
		break;
	}
    }

    return (refresh);
}

/*****************************************************************************/

ULONG handlesketchG (Class * cl, Object * o, struct gpInput * msg)
{
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;
    struct InputEvent *ie = msg->gpi_IEvent;
    ULONG retval = GMR_MEACTIVE;
    BOOL refresh = FALSE;
    BOOL valid = FALSE;
    BOOL fast = FALSE;
    BOOL draw = FALSE;
    WORD x, y;

    /* Figure out which cell the mouse is over */
    x = msg->gpi_Mouse.X - (VM * 2);
    y = msg->gpi_Mouse.Y - (HM * 2);

    if ((x < 0) || (x > (G (o)->Width - (VM * 4))))
    {
	x = -1;
    }
    else
    {
	x /= ed->ed_XMag;
    }

    if ((y < 0) || (y > (G (o)->Height - (HM * 4))))
    {
	y = -1;
    }
    else
    {
	y /= ed->ed_YMag;
    }

    /* Do we have a valid, new cell? */
    if ((x >= 0) && (y >= 0))
    {
	valid = TRUE;
	if (((UWORD) ReadPixel (&(ed->ed_PrefsWork.ep_RPort), ed->ed_Which + x, y)) != ed->ed_APen)
	{
	    draw = TRUE;
	}
    }

    /* Determine what to do... */
    switch (ie->ie_Class)
    {
	case IECLASS_RAWMOUSE:
	    switch (ie->ie_Code)
	    {
		    /* Select button pressed */
		case SELECTDOWN:
		    /* Select button */
		    if (valid)
		    {
			Store (ed);
			fast = refresh = TRUE;
		    }
		    else
		    {
			retval = GMR_NOREUSE;
		    }
		    break;

		    /* Menu button pressed */
		case MENUDOWN:
		    Restore (ed);
		    refresh = TRUE;

		    /* This event ends it */
		    retval = GMR_NOREUSE;
		    break;

		    /* Select button released */
		case SELECTUP:
		    retval = (GMR_NOREUSE | GMR_VERIFY);

		    /* Continue giving me input */
		default:
		    if (draw)
		    {
			fast = refresh = TRUE;
		    }
		    break;
	    }
	    break;
    }

    /* No change */
    G (o)->SpecialInfo = (APTR) 0xFFFFFFFFL;

    if (refresh)
    {
	/* Update the graphics to reflect the current state. */
	rendersketchG (cl, o, (struct gpRender *) msg, x, y, fast);

	/* Show where */
	G (o)->SpecialInfo = (APTR) ((((ULONG) x) << 16) | ((ULONG) y));
    }

    return (retval);
}

/*****************************************************************************/

VOID WriteChunkyPixel (Class * cl, Object * o, struct RastPort * srp, UWORD x, UWORD y, UWORD mode)
{
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;
    UBYTE pen = ed->ed_APen;
    WORD tx, ty, bx, by;

    /* Draw the magnified pixel */
    tx = G (o)->LeftEdge + (VM * 2) + (x * ed->ed_XMag);
    ty = G (o)->TopEdge + (HM * 2) + (y * ed->ed_YMag);
    bx = tx + ed->ed_XMag - 2;
    by = ty + ed->ed_YMag - 2;
    bx = MIN (bx, G (o)->LeftEdge + G (o)->Width - (VM * 2) - 1);
    by = MIN (by, G (o)->TopEdge + G (o)->Height - (HM * 2) - 1);
    SetAPen (srp, pen);
    RectFill (srp, tx, ty, bx, by);

    /* Draw the normal pixel */
    SetAPen (&(ed->ed_PrefsWork.ep_RPort), pen);
    WritePixel (&(ed->ed_PrefsWork.ep_RPort), ed->ed_Which + x, y);
}

/*****************************************************************************/

ULONG rendersketchG (Class * cl, Object * o, struct gpRender * msg, WORD x, WORD y, BOOL fast)
{
    struct localObjData *lod = INST_DATA (cl, o);
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;
    struct RastPort *rp;
    ULONG retval = 0L;

    /* Get the rastport that we're going to write into. */
    if (rp = ObtainGIRPort (msg->gpr_GInfo))
    {
	if (fast)
	{
	    WriteChunkyPixel (cl, o, rp, x, y, 0);
	}
	else
	{
	    /* CHANGE MAGNIFY TO START AT A PARTICULAR PIXEL!!! */
	    /* Copy from the preference record to temporary record */
	    bltbm (&(ed->ed_PrefsWork.ep_BMap), ed->ed_Which, 0,
		   &(ed->ed_Temp),
		   0, 0, ed->ed_Width, ed->ed_Height,
		   0xC0, 0xFF, NULL, ed->ed_GfxBase);
	    WaitBlit ();

	    /* Magnify the bitmap */
	    MagnifyBitMap (&(ed->ed_Temp), &lod->lod_Work,
			   ed->ed_XMag, ed->ed_YMag,
			   ed->ed_Width, TRUE);

	    /* Blit from the work BitMap to the window */
	    bltbmrp (&lod->lod_Work, 0, 0,
		     rp,
		     G (o)->LeftEdge + (VM * 2),
		     G (o)->TopEdge + (HM * 2),
		     G (o)->Width - (VM * 4),
		     G (o)->Height - (HM * 4),
		     0xC0, ed->ed_GfxBase);

	    if (G(o)->Flags & GFLG_DISABLED)
	    {
		SetAfPt (rp, ghost_pattern, 2);
		SetDrMd (rp, JAM1);
		SetAPen (rp, ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
		RectFill (rp,
			  G(o)->LeftEdge + 2,
			  G(o)->TopEdge + 2,
			  G(o)->LeftEdge + G(o)->Width - 3,
			  G(o)->TopEdge + G(o)->Height - 3);
		SetAfPt (rp, NULL, 0);
	    }
	}

	/* Indicate success */
	retval = 1L;

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }

    return (retval);
}

/*****************************************************************************/

VOID Store (EdDataPtr ed)
{
    /* Copy to the undo buffer */
    bltbm (&(ed->ed_PrefsWork.ep_BMap), ed->ed_Which, 0,
	   &ed->ed_Undo, 0, 0, ed->ed_Width, ed->ed_Height,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);
}

/*****************************************************************************/

VOID Restore (EdDataPtr ed)
{
    /* Copy from the undo buffer */
    bltbm (&ed->ed_Undo, 0, 0,
	   &(ed->ed_Temp), 0, 0, ed->ed_Width, ed->ed_Height,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);

    /* Copy from the undo buffer */
    bltbm (&ed->ed_PrefsWork.ep_BMap, ed->ed_Which, 0,
	   &(ed->ed_Undo), 0, 0, ed->ed_Width, ed->ed_Height,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);

    /* Copy from the undo buffer */
    bltbm (&ed->ed_Temp, 0, 0,
	   &(ed->ed_PrefsWork.ep_BMap), ed->ed_Which, 0, ed->ed_Width, ed->ed_Height,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);
}
