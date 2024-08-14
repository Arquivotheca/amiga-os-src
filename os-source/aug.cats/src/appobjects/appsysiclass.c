/* appsysiclass.c
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <libraries/apshattrs.h>
#include <utility/tagitem.h>
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <string.h>

void kprintf(void *, ...);
#define	DB(x)	x

extern struct Library *SysBase, *DOSBase;
extern struct Library *IntuitionBase, *GfxBase, *UtilityBase;

#define IM(o)   ((struct Image *)(o))	/* transparent base class */

#define MYCLASSID       "appsysiclass"
#define SUPERCLASSID    (IMAGECLASS)

Class *initSysIClass (VOID);
ULONG freeSysIClass (Class * cl);
ULONG __saveds dispatchSysI (Class * cl, Object * o, Msg msg);
ULONG frameSysI (Class *cl, Object *o, struct impFrameBox *msg);
ULONG setSysIAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getSysIAttrs (Class * cl, Object * o, struct opGet * msg);
ULONG drawSysI (Class * cl, Object * o, struct impDraw * msg);
WORD aTextExtent (struct RastPort *, STRPTR, LONG, struct TextExtent *);
VOID getContentsExtent (Class *cl, Object *o, struct DrawInfo * drinfo);

/* Class prototypes */
ULONG DoMethod (Object * o, ULONG methodID,...);
ULONG DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CM (Class * cl, Object * o, Msg msg);
ULONG DM (Object * o, Msg msg);
ULONG DSM (Class * cl, Object * o, Msg msg);
ULONG SetSuperAttrs (Class * cl, Object * o, ULONG data,...);

#define	ASIZE	256

struct globalObjData
{
    struct AreaInfo god_AreaInfo;
    struct TmpRas god_TmpRas;
    WORD god_Buffer[ASIZE];
    UWORD god_W, god_H;
    PLANEPTR god_Plane;
};

#define	GSIZE	(sizeof(struct globalObjData))

Class *initSysIClass (VOID)
{
    struct globalObjData *god;
    ULONG hookEntry ();
    UWORD w, h;
    Class *cl;

    /* Create our global data */
    if (god = (struct globalObjData *) AllocVec (GSIZE, MEMF_CLEAR))
    {
	/* How big of a raster do we need? */
	god->god_W = w = ((struct GfxBase *)GfxBase)->NormalDisplayColumns;
	god->god_H = h = ((struct GfxBase *)GfxBase)->NormalDisplayRows;

	/* Allocate the raster */
	if (god->god_Plane = AllocRaster (w, h))
	{
	    /* Initialize the work areas */
	    InitArea (&(god->god_AreaInfo), god->god_Buffer, (ASIZE*2)/5);
	    InitTmpRas (&(god->god_TmpRas), god->god_Plane, RASSIZE(w,h));

	    /* Create our class */
	    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, 0, 0))
	    {
		/* Fill in the callback hook */
		cl->cl_Dispatcher.h_Entry = hookEntry;
		cl->cl_Dispatcher.h_SubEntry = dispatchSysI;
		cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;
		cl->cl_UserData = (ULONG) god;

		/* Make the class public */
		AddClass (cl);

		return (cl);
	    }

	    /* Free the raster */
	    FreeRaster(god->god_Plane, w, h);
	}

	/* Free the global data */
	FreeVec (god);
    }

    /* Return a pointer to the class */
    return (NULL);
}

ULONG freeSysIClass (Class * cl)
{
    struct globalObjData *god;
    ULONG retval = 0L;

    if (cl)
    {
	god = (struct globalObjData *) cl->cl_UserData;

	if (retval = (ULONG)FreeClass (cl))
	{
	    if (god)
	    {
		FreeRaster (god->god_Plane, god->god_W, god->god_H);
		FreeVec (god);
	    }
	}
    }

    /* Try to free the public class */
    return (retval);
}


ULONG __saveds dispatchSysI (Class * cl, Object * o, Msg msg)
{
    Object *newobj;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* Have our superclass create it */
	    if (newobj = (Object *) DSM (cl, o, msg))
	    {
		struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
		ULONG which;

		which = GetTagData (SYSIA_Which, NULL, attrs);

		switch (which)
		{
		    case UPIMAGE:
		    case DOWNIMAGE:
			IM(newobj)->Width = 8;
			IM(newobj)->Height = 4;
			break;

		    case RIGHTIMAGE:
		    case LEFTIMAGE:
			IM(newobj)->Width = 8;
			IM(newobj)->Height = 5;
			break;
		}

		/* Set the attributes */
		setSysIAttrs(cl, newobj, (struct opSet *) msg);
	    }
	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = getSysIAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    /* Do the superclass first */
	    retval = DSM (cl, o, msg);

	    /* Call our set routines */
	    retval += setSysIAttrs (cl, o, (struct opSet *) msg);
	    break;

	case IM_DRAW:		/* draw with state */
	case IM_DRAWFRAME:	/* special case of draw */
	    retval = drawSysI (cl, o, (struct impDraw *) msg);
	    break;

	/* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

/* Set attributes of an object */
ULONG setSysIAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case IA_Width:
		IM(o)->Width = (SHORT) tidata;
		break;

	    case IA_Height:
		IM(o)->Height = (SHORT) tidata;
		break;

	    case SYSIA_Which:
		IM(o)->ImageData = (USHORT *) tidata;
		break;
	}
    }

    return (1L);
}

/* Inquire attributes of an object */
ULONG getSysIAttrs (Class * cl, Object * o, struct opGet * msg)
{
    switch (msg->opg_AttrID)
    {
	case SYSIA_Which:
	    *msg->opg_Storage = (ULONG) IM(o)->ImageData;
	    break;

	/* Let the superclass try */
	default:
	    return ((ULONG) DSM (cl, o, msg));
    }

    return (1L);
}

ULONG drawSysI (Class *cl, Object *o, struct impDraw *msg)
{
    struct globalObjData *god = (struct globalObjData *) cl->cl_UserData;
    struct DrawInfo *di = msg->imp_DrInfo;
    struct RastPort rp = *(msg->imp_RPort);
    LONG state = msg->imp_State;
    UWORD *pens = di->dri_Pens;
    WORD left, top;
    WORD height;
    WORD width;
    ULONG type;
    WORD pen;

    /* Initialize the RastPort */
    rp.AreaInfo = &(god->god_AreaInfo);
    rp.TmpRas = &(god->god_TmpRas);

    /* Set the colors */
    pen = pens[TEXTPEN];
    if (state == IDS_SELECTED || state == IDS_INACTIVESELECTED)
    {
	pen = pens[FILLTEXTPEN];
    }
    SetAPen (&rp, pen);

    /* Set the drawing mode */
    SetDrMd (&rp, JAM1);

    /* Figure out our coordinates */
    top = msg->imp_Offset.Y + IM (o)->TopEdge;
    left = msg->imp_Offset.X + IM (o)->LeftEdge;
    width = IM(o)->Width;
    height = IM(o)->Height;

    /* See if we have frame information */
    if (msg->MethodID == IM_DRAWFRAME)
    {
	width = msg->imp_Dimensions.Width - 4;
	height = msg->imp_Dimensions.Height - 2;
	top += ((msg->imp_Dimensions.Height - height) / 2);
	left += ((msg->imp_Dimensions.Width - width) / 2);
    }

    /* Cast the type */
    type = (ULONG) IM(o)->ImageData;

    /* It's in the border... */
    if (type < 10)
    {
	left++;
	width -= 2;
    }

    switch (type)
    {
	/* Left */
	case 0:
	    /* Adjust width */
	    left += 2;
	    width -= 4;

	    /* Is height even? */
	    height = (height & 1) ? height : --height;

	    AreaMove (&rp, (left + width - 1),  top);
	    AreaDraw (&rp,  left,              (top + height / 2));
	    AreaDraw (&rp, (left + 3),         (top + height / 2));
	    AreaMove (&rp, (left + width - 1),  top);
	    AreaEnd  (&rp);

	    AreaMove (&rp, (left + width - 1), (top + height - 1));
	    AreaDraw (&rp,  left,              (top + height / 2));
	    AreaDraw (&rp, (left + 3),         (top + height / 2));
	    AreaMove (&rp, (left + width - 1), (top + height - 1));
	    AreaEnd  (&rp);

	    break;

	/* Up */
	case 1:
	    /* Is width even? (do the rest) */
	    width = (width & 1) ? width : (width - 1);

	    AreaMove (&rp,  left,               (top + height - 1));
	    AreaDraw (&rp, (left + (width / 2)), top);
	    AreaDraw (&rp, (left + (width / 2)),(top + 2));
	    AreaDraw (&rp, (left + 1),          (top + height - 1));
	    AreaDraw (&rp,  left,               (top + height - 1));
	    AreaEnd  (&rp);

	    AreaMove (&rp, (left + width - 1),  (top + height - 1));
	    AreaDraw (&rp, (left + (width / 2)), top);
	    AreaDraw (&rp, (left + (width / 2)),(top + 2));
	    AreaDraw (&rp, (left + width - 2),  (top + height - 1));
	    AreaDraw (&rp, (left + width - 1),  (top + height - 1));
	    AreaEnd  (&rp);

	    break;

	/* Right */
	case 2:
	    /* Adjust width */
	    left += 2;
	    width -= 4;

	    /* Is height even? */
	    height = (height & 1) ? height : --height;

	    AreaMove (&rp,  left,               top);
	    AreaDraw (&rp, (left + width - 1), (top + height / 2));
	    AreaDraw (&rp, (left + width - 4), (top + height / 2));
	    AreaMove (&rp,  left,               top);
	    AreaEnd  (&rp);

	    AreaMove (&rp,  left,              (top + height - 1));
	    AreaDraw (&rp, (left + width - 1), (top + height / 2));
	    AreaDraw (&rp, (left + width - 4), (top + height / 2));
	    AreaMove (&rp,  left,              (top + height - 1));
	    AreaEnd  (&rp);

	    break;

	/* Down */
	case 3:
	    /* Is width even? (do the rest) */
	    width = (width & 1) ? width : (width - 1);

	    AreaMove (&rp,  left,                top);
	    AreaDraw (&rp, (left + (width / 2)),(top + height - 1));
	    AreaDraw (&rp, (left + (width / 2)),(top + height - 3));
	    AreaDraw (&rp, (left + 1),           top);
	    AreaDraw (&rp,  left,                top);
	    AreaEnd  (&rp);

	    AreaMove (&rp, (left + width - 1),   top);
	    AreaDraw (&rp, (left + (width / 2)),(top + height - 1));
	    AreaDraw (&rp, (left + (width / 2)),(top + height - 3));
	    AreaDraw (&rp, (left + width - 2),   top);
	    AreaDraw (&rp, (left + width - 1),   top);
	    AreaEnd  (&rp);

	    break;


	case LEFTIMAGE:
	    /* Is height even? */
	    height = (height - 1) | 1;

	    Move (&rp, (left + width - 1), top);
	    Draw (&rp, left, (top + height / 2));
	    Draw (&rp, (left + width - 1), (top + height - 1));
	    break;

	case UPIMAGE:
	    /* Is width odd? (do the rest) */
	    width &= ~1;

	    Move (&rp, left, (top + height - 1));
	    Draw (&rp, (left + (width / 2) - 1), top);
	    Draw (&rp, (left + (width / 2)), top);
	    Draw (&rp, (left + width - 1), (top + height - 1));
	    Draw (&rp, (left + width - 2), (top + height - 1));
	    Draw (&rp, (left + (width / 2) - 1), top);
	    Draw (&rp, (left + (width / 2)), top);
	    Draw (&rp, (left + 1), (top + height - 1));
	    break;

	case RIGHTIMAGE:
	    /* Is height even? */
	    height = (height - 1) | 1;

	    Move (&rp, left, top);
	    Draw (&rp, (left + width - 1), (top + (height / 2)));
	    Draw (&rp, left, (top + height - 1));
	    break;

	case DOWNIMAGE:
	    /* Is width odd? */
	    width &= ~1;

	    Move (&rp, left, top);
	    Draw (&rp, (left + (width / 2) - 1), (top + height - 1));
	    Draw (&rp, (left + (width / 2)), (top + height - 1));
	    Draw (&rp, (left + width - 1), top);
	    Draw (&rp, (left + width - 2), top);
	    Draw (&rp, (left + (width / 2) - 1), (top + height - 1));
	    Draw (&rp, (left + (width / 2)), (top + height - 1));
	    Draw (&rp, (left + 1), top);
	    break;
    }

    return (1L);
}
