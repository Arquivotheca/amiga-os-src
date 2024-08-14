
/*** itexticlass.c *******************************************************
 *
 *  IntuiText image class
 *
 *  $Id: itexticlass.c,v 38.1 91/09/23 12:28:03 peter Exp $
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

#define D(x)	;
#define IM(o)	((struct Image *)(o))	/* transparent base class */
#define ITEXT(ptr) ((struct IntuiText *)(ptr))

extern UBYTE	*ITextIClassName;
extern UBYTE	*ImageClassName;
#define MYCLASSID	ITextIClassName
#define SUPERCLASSID	ImageClassName


Class	*
initITextIClass()
{
    ULONG	dispatchITextI();
    Class	*makePublicClass();

    D( printf("init ITextIClass\n"));
    return (  makePublicClass( MYCLASSID, SUPERCLASSID, 0, dispatchITextI));
}


ULONG 
dispatchITextI( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct localObjectData  *lob;

    switch ( msg->MethodID )
    {
    case IM_DRAW:			/* draw with state */
    case IM_DRAWFRAME:			/* special case of draw	*/
	return ( (ULONG) drawITextI( cl, o, msg ) );

    default:
	return ( (ULONG) SSM( cl, o, msg ) );
    }
}

/*
 * overrides pens and draw mode of IntuiText elements
 * uses its own PlanePick for fgpen, uses JAM1.
 * Each element may set its own font, though.
 */
drawITextI( cl, o, msg )
Class		*cl;
Object		*o;
struct impDraw	*msg;	/* superset of impDraw	*/
{
    /* use PlanePick as FG_PEN	*/
    SetABPenDrMd( msg->imp_RPort, IM(o)->PlanePick, 0, JAM1 );

    printIText( msg->imp_RPort, IM(o)->ImageData,
	msg->imp_Offset.X + IM(o)->LeftEdge,
	msg->imp_Offset.Y + IM(o)->TopEdge,  FALSE );
}
