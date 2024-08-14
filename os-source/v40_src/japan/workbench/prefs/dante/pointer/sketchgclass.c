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

#include "pointer.h"

/* classface.asm */
ULONG __stdargs DoMethod (Object * o, ULONG methodID,...);
ULONG __stdargs DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG __stdargs CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG __stdargs CoerceMethodA (Class * cl, Object * o, Msg msg);
ULONG __stdargs DoMethodA (Object * o, Msg msg);
ULONG __stdargs DoSuperMethodA (Class * cl, Object * o, Msg msg);
ULONG __stdargs SetSuperAttrs (Class * cl, Object * o, ULONG data,...);

#define	DB(x)	;
#define G(o)		((struct Gadget *)(o))
#define	VM	1
#define	HM	1

struct localObjData
{
    struct BitMap	*lod_BMap;		/* Work bitmap */
    UWORD	 	 lod_Width;		/* Width of work bitmap */
    UWORD	 	 lod_Height;		/* Height of work bitmap */
    UBYTE		 lod_Depth;		/* Depth of work bitmap */
};

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

	    return ((ULONG) newobj);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Do the superclass first */
	    DoSuperMethodA (cl, o, msg);

	    /* Call our set routines */
	    setsketchGAttrs (cl, o, (struct opSet *) msg);

	    /* Get a pointer to the rastport */
	    if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
	    {
		struct gpRender gpr;

		/* Force a redraw */
		gpr.MethodID   = GM_RENDER;
		gpr.gpr_GInfo  = ((struct opSet *) msg)->ops_GInfo;
		gpr.gpr_RPort  = rp;
		gpr.gpr_Redraw = GREDRAW_REDRAW;

		/* Redraw... */
		DoMethodA (o, &gpr);

		/* Release the temporary rastport */
		ReleaseGIRPort (rp);
	    }
	    break;

	    /* Render the graphics */
	case GM_RENDER:
	    /* Render the button */
	    rendersketchG (cl, o, (struct gpRender *) msg, 0, 0, FALSE);
	    break;

	    /* Handle input */
	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    /* Handle the input */
	    return (handlesketchG (cl, o, (struct gpInput *) msg));
	    break;

	    /* Delete ourself */
	case OM_DISPOSE:
	    /* Free the bitmaps that we use */
	    WaitBlit ();
	    lod->lod_BMap->Depth = ed->ed_Depth;
	    FreeBitMap (lod->lod_BMap);

	    /* Pass it up to the superclass */
	    return ((ULONG) DoSuperMethodA (cl, o, msg));
	    break;

	    /* Let the superclass handle everything else */
	default:
	    return ((ULONG) DoSuperMethodA (cl, o, msg));
	    break;
    }

    return (0L);
}

/*****************************************************************************/

/* initialize attributes of an object */
ULONG initsketchGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;
    UWORD i;

    lod->lod_Width  = MAXWIDTH  * 12;
    lod->lod_Height = MAXHEIGHT * 12;

    if (lod->lod_BMap = AllocBitMap (lod->lod_Width, lod->lod_Height, ed->ed_Depth, BMF_DISPLAYABLE | BMF_STANDARD | BMF_CLEAR, NULL))
    {
	lod->lod_BMap->Depth = lod->lod_Depth = MAX (ed->ed_Depth, ed->ed_Screen->RastPort.BitMap->Depth);
	for (i = ed->ed_Depth; i < 8; i++)
	    lod->lod_BMap->Planes[i] = NULL;
	return TRUE;
    }
    return FALSE;
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
    WORD pen;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	switch (tag->ti_Tag)
	{
	    case SPA_Store:
		Store (ed);
		break;

	    case SPA_Restore:
		Restore (ed);
		break;

	    case SPA_Clear:
		pen = ed->ed_APen;
		if (pen == 4)
		    pen = 0;
		SetPrefsRast (ed, pen);
		break;

	    case SPA_Erase:
		SetPrefsRast (ed, 0);
		break;
	}
    }

    return (1L);
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
    WORD pen;

    pen = ed->ed_APen;
    if (pen == 4)
	pen = 0;

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
	if (((UWORD) ReadPixel (&ed->ed_PrefsWork.ep_RPort, (ed->ed_Which*MAXWIDTH) + x, y)) != pen)
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

			if (ed->ed_GSetPoint->Flags & GFLG_SELECTED)
			{
			    retval = (GMR_NOREUSE | GMR_VERIFY);
			    ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_X = -(x);
			    ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_Y = -(y);
			    fast = FALSE;
			}
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

    if (refresh)
    {
	/* Update the graphics to reflect the current state. */
	rendersketchG (cl, o, (struct gpRender *) msg, x, y, fast);
    }

    return (retval);
}

/*****************************************************************************/

VOID WriteChunkyPixel (Class * cl, Object * o, struct RastPort * srp, struct DrawInfo *dri, UWORD x, UWORD y, UWORD mode)
{
    EdDataPtr ed = (EdDataPtr) cl->cl_UserData;
    WORD echo_w = MAXWIDTH + 4;
    WORD echo_h = MAXHEIGHT + 4;
    UBYTE pen = ed->ed_APen;
    register WORD i, ox, oy;
    WORD tx, ty, bx, by;
    UBYTE rpen;

    if (pen == 4)
	pen = 0;

    /* Draw the magnified pixel */
    tx = G (o)->LeftEdge + (VM * 2) + (x * ed->ed_XMag);
    ty = G (o)->TopEdge + (HM * 2) + (y * ed->ed_YMag);
    bx = tx + ed->ed_XMag - 2;
    by = ty + ed->ed_YMag - 2;
    bx = MIN (bx, G (o)->LeftEdge + G (o)->Width - (VM * 2) - 1);
    by = MIN (by, G (o)->TopEdge + G (o)->Height - (HM * 2) - 1);

    if (mode == 0)
    {
	/* Draw the normal pixel */
	SetAPen (&ed->ed_PrefsWork.ep_RPort, pen);
	WritePixel (&ed->ed_PrefsWork.ep_RPort, (ed->ed_Which*MAXWIDTH) + x, y);

	ox = ed->ed_GSketch->LeftEdge + ed->ed_GSketch->Width + 7 + 2;
	oy = ed->ed_GSketch->TopEdge + 2 + 2;

	for (i = 0; i < 4; i++, ox += echo_w)
	{
	    if (i == 2)
	    {
		oy += echo_h;
		ox = ed->ed_GSketch->LeftEdge + ed->ed_GSketch->Width + 7 + 2;
	    }

	    if ((rpen = ed->ed_APen) == 0)
		rpen = i;

	    SetAPen (ed->ed_Window->RPort, rpen);
	    WritePixel (ed->ed_Window->RPort, ox + x, oy + y);
	}

	/* Draw the magnified pixel */
	SetAPen (srp, pen);
	RectFill (srp, tx, ty, bx, by);
    }
    else
    {
	SetAPen (srp, dri->dri_Pens[FILLPEN]);
	tx--; ty--;
	bx++; by++;
	Move (srp, tx, ty);
	Draw (srp, bx, ty);
	Draw (srp, bx, by);
	Draw (srp, tx, by);
	Draw (srp, tx, ty);
    }
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
	    WriteChunkyPixel (cl, o, rp, msg->gpr_GInfo->gi_DrInfo, x, y, 0);
	}
	else
	{
	    /* CHANGE MAGNIFY TO START AT A PARTICULAR PIXEL!!! */
	    /* Copy from the preference record to temporary record */
	    bltbm (ed->ed_PrefsWork.ep_BMap, (ed->ed_Which * MAXWIDTH), 0,
		   ed->ed_TempBM,
		   0, 0, MAXWIDTH, MAXHEIGHT,
		   0xC0, 0xFF, NULL, ed->ed_GfxBase);
	    WaitBlit ();

	    /* Magnify the bitmap */
	    lod->lod_BMap->Depth = ed->ed_Depth;
	    MagnifyBitMap (ed->ed_TempBM, lod->lod_BMap,
			   ed->ed_XMag, ed->ed_YMag,
			   MAXWIDTH, TRUE);
	    lod->lod_BMap->Depth = lod->lod_Depth;

	    /* Blit from the work BitMap to the window */
	    bltbmrp (lod->lod_BMap, 0, 0,
		     rp,
		     G (o)->LeftEdge + (VM * 2),
		     G (o)->TopEdge + (HM * 2),
		     G (o)->Width - (VM * 4),
		     G (o)->Height - (HM * 4),
		     0xC0, ed->ed_GfxBase);

	    /* Clear that blank area around the edges (just in case the hot spot indicator was there */
	    SetAPen (rp, 0);
	    Move (rp, G(o)->LeftEdge + 1, G(o)->TopEdge + 1);
	    Draw (rp, G(o)->LeftEdge + G(o)->Width - 2, G(o)->TopEdge + 1);
	    Draw (rp, G(o)->LeftEdge + G(o)->Width - 2, G(o)->TopEdge + G(o)->Height - 2);
	    Draw (rp, G(o)->LeftEdge + 1, G(o)->TopEdge + G(o)->Height - 2);
	    Draw (rp, G(o)->LeftEdge + 1, G(o)->TopEdge + 1);

	    RefreshRepeats (ed);
	}

	/* Write the hotspot */
	WriteChunkyPixel (cl, o, rp, msg->gpr_GInfo->gi_DrInfo, -(ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_X), -(ed->ed_PrefsWork.ep_PP[ed->ed_Which].pp_Y), 1);

	/* Indicate success */
	retval = 1L;

	/* Release the rastport */
	ReleaseGIRPort (rp);
    }

    return (retval);
}

VOID Store (EdDataPtr ed)
{
    /* Copy to the undo buffer */
    bltbm (ed->ed_PrefsWork.ep_BMap, (ed->ed_Which*MAXWIDTH), 0,
	   ed->ed_UndoBM, 0, 0, MAXWIDTH, MAXHEIGHT,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);
}

VOID Restore (EdDataPtr ed)
{
    /* Copy from the undo buffer */
    bltbm (ed->ed_UndoBM, 0, 0,
	   ed->ed_TempBM, 0, 0, MAXWIDTH, MAXHEIGHT,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);

    /* Copy from the undo buffer */
    bltbm (ed->ed_PrefsWork.ep_BMap, (ed->ed_Which*MAXWIDTH), 0,
	   ed->ed_UndoBM, 0, 0, MAXWIDTH, MAXHEIGHT,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);

    /* Copy from the undo buffer */
    bltbm (ed->ed_TempBM, 0, 0,
	   ed->ed_PrefsWork.ep_BMap, (ed->ed_Which*MAXWIDTH), 0, MAXWIDTH, MAXHEIGHT,
	   0xC0, 0xFF, NULL, ed->ed_GfxBase);
}
