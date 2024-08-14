/*** itexticlass.c *******************************************************
 *
 *  IntuiText image class
 *
 *  $Id: itexticlass.c,v 40.0 94/02/15 17:35:38 davidj Exp $
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

#define IM(o)		((struct Image *)(o))		/* transparent base class */
#define ITEXT(ptr)	((struct IntuiText *)(ptr))

/*****************************************************************************/

extern UBYTE *ITextIClassName;
extern UBYTE *ImageClassName;

#define MYCLASSID	ITextIClassName
#define SUPERCLASSID	ImageClassName

/*****************************************************************************/

/* overrides pens and draw mode of IntuiText elements
 * uses its own PlanePick for fgpen, uses JAM1.
 * Each element may set its own font, though. */
static ULONG imDraw (Class *cl, Object *o, struct impDraw *msg)
{
    /* use PlanePick as FG_PEN	*/
    SetABPenDrMd (msg->imp_RPort, IM (o)->PlanePick, 0, JAM1);

    printIText (msg->imp_RPort, (struct IntuiText *) IM (o)->ImageData,
		msg->imp_Offset.X + IM (o)->LeftEdge,
		msg->imp_Offset.Y + IM (o)->TopEdge, FALSE);

    return 0;
}

/*****************************************************************************/

static ULONG dispatchITextI (Class *cl, Object *o, Msg msg)
{
    switch (msg->MethodID)
    {
	case IM_DRAW:		/* draw with state */
	case IM_DRAWFRAME:	/* special case of draw	*/
	    return (ULONG) imDraw (cl, o, (struct impDraw *) msg);

	default:
	    return (ULONG) SSM (cl, o, msg);
    }
}

/*****************************************************************************/

Class *initITextIClass (void)
{
    return (makePublicClass (MYCLASSID, SUPERCLASSID, 0, dispatchITextI));
}
