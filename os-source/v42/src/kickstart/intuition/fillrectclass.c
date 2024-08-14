/*** fillrect.c *******************************************************
 *
 *  Pattern filled rectangle image class
 *
 *  $Id: fillrectclass.c,v 40.0 94/02/15 18:02:34 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1990, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "imageclass.h"
#include <graphics/gfxmacros.h>

/*****************************************************************************/

#define GETATTRS	FALSE	/* don't spend the space	*/
#define INSTANCEDATA	FALSE	/* don't maintain anything	*/

/*****************************************************************************/

#define D(x)	;
#define DI(x)	;

/*****************************************************************************/

struct localObjectData
{
    UWORD lod_Mode;
    UWORD lod_APatSize;
};

#define IM(o)	((struct Image *)(o))	/* transparent base class */

/*****************************************************************************/

extern UBYTE *FillRectClassName;
extern UBYTE *ImageClassName;

/*****************************************************************************/

#define MYCLASSID	FillRectClassName
#define SUPERCLASSID	ImageClassName

/*****************************************************************************/

static ULONG drawFillRect (Class *cl, Object *o, struct impDraw *msg)
{
    struct localObjectData *lod = INST_DATA (cl, o);

    struct IBox box;

    box = *IM_BOX (IM (o));	/* get Image.Left/Top/Width/Height */
    box.Left += msg->imp_Offset.X;
    box.Top += msg->imp_Offset.Y;

    if (msg->MethodID == IM_DRAWFRAME)
    {
	box.Width = msg->imp_Dimensions.Width;
	box.Height = msg->imp_Dimensions.Height;
    }

    BlastPatternBox (msg->imp_RPort, &box,
		     IM (o)->ImageData, lod->lod_APatSize,	/* pattern in ImageData */
		     IM (o)->PlanePick, IM (o)->PlaneOnOff,
		     lod->lod_Mode);

    return 1;
}

/*****************************************************************************/

/* set specified attribute value.
 * Technique provided is a little more general than we need ... */
static ULONG setFillRectAttrs (Class *cl, Object *o, struct opSet *msg)
{
    struct localObjectData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tag;
    ULONG tidata;

    while (tag = NextTagItem (&tags))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case IA_APATTERN:
		IM (o)->ImageData = (UWORD *) tidata;
		break;
	    case IA_APATSIZE:
		lod->lod_APatSize = (UWORD) tidata;
		break;
	    case IA_MODE:
		lod->lod_Mode = (UWORD) tidata;
		break;
	}
    }
    return 1;
}

/*****************************************************************************/

static ULONG dispatchFillRect (Class *cl, Object *o, Msg msg)
{
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    /* let superclass do it's creation routine first	*/
	    if (newobj = (Object *) SSM (cl, o, msg))
	    {
		setFillRectAttrs (cl, newobj, (struct opSet *) msg);
	    }
	    return ((ULONG) newobj);

	case OM_SET:
	    SSM (cl, o, msg);	/* let the superclass see the atts */
	    return setFillRectAttrs (cl, o, (struct opSet *) msg);

	case IM_DRAW:		/* draw with state */
	case IM_DRAWFRAME:	/* special case of draw	*/
	    return drawFillRect (cl, o, (struct impDraw *) msg);

	default:
	    return ((ULONG) SSM (cl, o, msg));
    }
}

/*****************************************************************************/

Class *initFillRectClass (void)
{
    DI (printf ("init FillRectClass\n"));
    return (makePublicClass (MYCLASSID, SUPERCLASSID,
			     sizeof (struct localObjectData), dispatchFillRect));
}
