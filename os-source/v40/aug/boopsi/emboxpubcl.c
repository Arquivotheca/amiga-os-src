/* emboxclass.c -- :ts=8
 * Example of a PUBLIC image class.
 * This one does a raised embossed box.
 * Doesn't do resolution sensitive bevel widths yet,
 * nor correctly supports IA_LINEWIDTH yet.
 * But's it's an example of an image subclass
 * with some instance data.
 */

/*
Copyright (c) 1989, 1990 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#include "sysall.h"

#if 0	/* in sysall.h	*/
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/imageclass.h>
#endif

#include <intuition/classes.h>

#include <graphics/gfxmacros.h>

#define D(x)	;
#define DH(x)	;

struct EmbBData {
    /* right now, this is kept in pixels, and isn't adjusted
     * for resolution.  A fancier version might take this
     * parameter in "resolution ticks"
     */
    UWORD		ebd_LineWidth;
};

#define IM(o)	((struct Image *)(o))	/* transparent base class */

#define DEFAULTLINEWIDTH	1

/* public class	*/
#define PRIVATECLASS	FALSE

#if PRIVATECLASS
#define MYCLASSID	(NULL)
#else
#define MYCLASSID	"emboxclass"
#endif
extern struct Library	*IntuitionBase;

#define SUPERCLASSID	(IMAGECLASS)

Class	*
initEmbBClass()
{
    ULONG	__saveds dispatchEmbB();
    ULONG	hookEntry();
    Class	*cl;
    Class	*MakeClass();

    if ( cl =  MakeClass( MYCLASSID, 
		SUPERCLASSID, NULL,		/* superclass is public      */
 		sizeof (struct EmbBData),	/* my object's instance data */
		0 ) )
    {
	/* initialize the cl_Dispatcher Hook	*/
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchEmbB;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;	/* unused */

#if !PRIVATECLASS
	AddClass( cl );			/* make public and available	*/
#endif
    }
    return ( cl );
}

freeEmbBClass( cl )
Class	*cl;
{
    return ( FreeClass( cl )  );
}

ULONG  __saveds
dispatchEmbB( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct EmbBData 	*ebd;

    switch ( msg->MethodID )
    {
    case OM_NEW:
	/* let superclass do it's creation routine first	*/
	if ( newobj = (Object *) DSM( cl, o, msg ) )
	{
	    /* init my instance data	*/

	    /* get pointer to it, like a good fellow	*/
	    ebd = INST_DATA( cl, newobj );

	    /* mandatory default init */
	    ebd->ebd_LineWidth = DEFAULTLINEWIDTH;

	    /* may override ebd_LineWidth	*/
	    setEmbBAttrs( cl, newobj, msg );
	}

	return ( (ULONG) newobj );

    case OM_GET:
	return ( (ULONG) getEmbBAttrs( cl, o, msg  ) );

    case OM_SET:
	DSM( cl, o, msg );		/* let the superclass see the atts */
	setEmbBAttrs( cl, o, msg  );	/* set the ones I care about	   */
	return ( (ULONG) 1 );		/* i'm happy			   */

    case IM_DRAW:			/* draw with state */
	return ( (ULONG) drawEmbB( cl, o, msg ) );

    /* use superclass defaults */
    case IM_HITTEST:
    case IM_ERASE:
    case OM_DISPOSE:
    default:
	return ( (ULONG) DSM( cl, o, msg ) );
    }
}

drawEmbB( cl, o, msg )
Class		*cl;
Object		*o;
struct impDraw	*msg;
{
    struct EmbBData 	*ebd = INST_DATA( cl, o );
    struct IBox		box;

    UWORD		*pens;		/* pen spec array */
    UWORD		ulpen;		/* upper left	*/
    UWORD		lrpen;		/* lower right	*/
    UWORD		fillpen;	/* filled area	*/

    /* let's be sure that we were passed a DrawInfo	*/
    pens =  ( msg->imp_DrInfo )?  msg->imp_DrInfo->dri_Pens: NULL;

    box = *IM_BOX( IM(o) );		/* get Image.Left/Top/Width/Height */
    box.Left += msg->imp_Offset.X;
    box.Top += msg->imp_Offset.Y;

    switch ( msg->imp_State )
    {
    case IDS_SELECTED:
    case IDS_INACTIVESELECTED:
	ulpen = pens? pens[ shadowPen ]: 2;
	lrpen = pens? pens[ shinePen ]: 1;
	fillpen = pens? pens[ hifillPen ]: 3;
	break;

    case IDS_NORMAL:	/* doesn't use activefill in borders now */
    case IDS_DISABLED:	/* I don't have a ghosted version yet	 */
    case IDS_INACTIVENORMAL:	/* doesn't use activefill in borders now */
    default:
	ulpen = pens? pens[ shinePen ]: 2;
	lrpen = pens? pens[ shadowPen ]: 1;
	fillpen = pens? pens[ backgroundPen ]: 0;
	break;
    }

    embossedBoxTrim( msg->imp_RPort, &box,
	ebd->ebd_LineWidth, ebd->ebd_LineWidth,
	ulpen, lrpen );

    /* interior */
    interiorBox( msg->imp_RPort, &box,
	ebd->ebd_LineWidth,	/* inset in x dim */
	ebd->ebd_LineWidth,	/* inset in y dim */
	fillpen );
}

/* return requested attribute value	*/
getEmbBAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opGet	*msg;
{
    struct EmbBData 	*ebd = INST_DATA( cl, o );

    if ( msg->opg_AttrID == IA_LINEWIDTH )
    {
    	*msg->opg_Storage = ebd->ebd_LineWidth;
	return ( 1 );
    }
    else
    	return ( DSM( cl, o, msg ) );

}

/* set specified attribute value.
 * Technique provided is a little more general than we need ...
 */
setEmbBAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tag;
    ULONG		tidata;

    struct EmbBData 	*ebd = INST_DATA( cl, o );

    while ( tag = NextTagItem( &tags ) )
    {
	tidata = tag->ti_Data;
	switch ( tag->ti_Tag )
	{
	case IA_LINEWIDTH:
	    ebd->ebd_LineWidth = tidata;
	    break;
	}
    }
}

/* fill region centered in a box */
interiorBox( rp, b, xw, yw, pen )
struct RastPort	*rp;
struct IBox	*b;
{
    if ( (b->Width > (xw<<1)) && (b->Height > (yw<<1)) )
    {
	rp->Mask = -1;
	BNDRYOFF( rp );
	SetAfPt(rp, NULL, 0);
	SetDrMd(rp, JAM2);
	SetAPen(rp, pen);
	RectFill( rp,
	    b->Left + xw,
	    b->Top + yw,
	    b->Left + b->Width - 1 - xw,
	    b->Top + b->Height - 1 - yw );
    }
}

/* suggestion of Talin: don't draw upper-right and lower-left points */
/* ignoring thickness for now	*/
embossedBoxTrim( rp, b, hthick, vthick, ulpen, lrpen )
struct RastPort	*rp;
struct IBox	*b;
{
    int	bottom, right;

    bottom = b->Top + b->Height - 1;
    right = b->Left + b->Width - 1;

    /* upper right edges	*/
    SetAPen( rp, ulpen );

    Move( rp, b->Left, bottom - 1 );
    Draw( rp, b->Left, b->Top );
    Draw( rp, right - 1, b->Top );

    /* lower right edges	*/
    SetAPen( rp, lrpen );

    Move( rp, right, b->Top + 1 );
    Draw( rp, right, bottom );
    Draw( rp, b->Left + 1, bottom );
}

