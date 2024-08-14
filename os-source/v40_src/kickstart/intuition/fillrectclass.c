
/*** fillrect.c *******************************************************
 *
 *  Pattern filled rectangle image class
 *
 *  $Id: fillrectclass.c,v 38.0 91/06/12 14:17:06 peter Exp $
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

#define GETATTRS	FALSE	/* don't spend the space	*/
#define INSTANCEDATA	FALSE	/* don't maintain anything	*/

#define D(x)	;
#define DI(x)	;

struct localObjectData {
    UWORD		lod_Mode;
    UWORD		lod_APatSize;
};

#define IM(o)	((struct Image *)(o))	/* transparent base class */

extern UBYTE	*FillRectClassName;
extern UBYTE	*ImageClassName;
#define MYCLASSID	FillRectClassName
#define SUPERCLASSID	ImageClassName

Class	*
initFillRectClass()
{
    ULONG	dispatchFillRect();
    Class	*makePublicClass();

    DI( printf("init FillRectClass\n"));
    return (  makePublicClass( MYCLASSID, SUPERCLASSID, 
	sizeof (struct localObjectData), dispatchFillRect));
}

ULONG 
dispatchFillRect( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;

    switch ( msg->MethodID )
    {
    case OM_NEW:
	/* let superclass do it's creation routine first	*/
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    setFillRectAttrs( cl, newobj, msg );
	}
	return ( (ULONG) newobj );

    case OM_SET:
	SSM( cl, o, msg );		/* let the superclass see the atts */
	setFillRectAttrs( cl, o, msg  );
	return ( (ULONG) 1 );		/* i'm happy			   */

    case IM_DRAW:			/* draw with state */
    case IM_DRAWFRAME:			/* special case of draw	*/
	return ( (ULONG) drawFillRect( cl, o, msg ) );

    default:
	return ( (ULONG) SSM( cl, o, msg ) );
    }
}

drawFillRect( cl, o, msg )
Class		*cl;
Object		*o;
struct impDraw	*msg;	/* superset of impDraw	*/
{
    struct localObjectData 	*lod = INST_DATA( cl, o );

    struct IBox		box;

    box = *IM_BOX( IM(o) );		/* get Image.Left/Top/Width/Height */
    box.Left += msg->imp_Offset.X;
    box.Top += msg->imp_Offset.Y;

    if ( msg->MethodID == IM_DRAWFRAME )
    {
	box.Width = msg->imp_Dimensions.Width;
	box.Height = msg->imp_Dimensions.Height;
    }

    BlastPatternBox( msg->imp_RPort, &box,
	IM(o)->ImageData, lod->lod_APatSize,	/* pattern in ImageData */
	IM(o)->PlanePick, IM(o)->PlaneOnOff,
	lod->lod_Mode );
}


/* set specified attribute value.
 * Technique provided is a little more general than we need ...
 */
setFillRectAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tag;
    ULONG		tidata;

    struct localObjectData 	*lod = INST_DATA( cl, o );

    while ( tag = NextTagItem( &tags ) )
    {
	tidata = tag->ti_Data;
	switch ( tag->ti_Tag )
	{
	case IA_APATTERN:
	    IM(o)->ImageData = (UWORD *) tidata;
	    break;
	case IA_APATSIZE:
	    lod->lod_APatSize = (UWORD) tidata;
	    break;
	case IA_MODE:
	    lod->lod_Mode = (UWORD) tidata;
	    break;
	}
    }
}

