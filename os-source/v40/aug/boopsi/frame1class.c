/* frameclass.c -- :ts=8
 * Example of a PUBLIC image class with FRAME capability.
 *
 * This one does a raised embossed box.
 * Doesn't do resolution sensitive bevel widths yet,
 * nor correctly supports IA_LINEWIDTH yet.
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
#define DI(x)	x

struct Frame1Data {
    /* right now, this is kept in pixels, and isn't adjusted
     * for resolution.  A fancier version might take this
     * parameter in "resolution ticks"
     */
    UWORD		f1d_LineWidth;
};

#define IM(o)	((struct Image *)(o))	/* transparent base class */

#define DEFAULTLINEWIDTH	1

/* public class	*/
#define PRIVATECLASS	FALSE

#if PRIVATECLASS
#define MYCLASSID	(NULL)
#else
#define MYCLASSID	"frame1class"
#endif
extern struct Library	*IntuitionBase;

#define SUPERCLASSID	(IMAGECLASS)

Class	*
initFrame1Class()
{
    ULONG __saveds	dispatchFrame1();
    ULONG	hookEntry();
    Class	*cl;
    Class	*MakeClass();

    DI( printf("init Frame1Class\n"));

    if ( cl =  MakeClass( MYCLASSID, 
		SUPERCLASSID, NULL,		/* superclass is public      */
 		sizeof (struct Frame1Data),	/* my object's instance data */
		0) )
    {
	/* initialize the cl_Dispatcher Hook	*/
	DI( printf("class at %lx\n", cl ));
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchFrame1;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;	/* unused */

#if !PRIVATECLASS
	AddClass( cl );			/* make public and available	*/
#endif
    }
    return ( cl );
}

freeFrame1Class( cl )
Class	*cl;
{
    return ( FreeClass( cl )  );
}

ULONG __saveds 
dispatchFrame1( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct Frame1Data 	*f1d;

    switch ( msg->MethodID )
    {
    case OM_NEW:
	/* let superclass do it's creation routine first	*/
	if ( newobj = (Object *) DSM( cl, o, msg ) )
	{
	    /* init my instance data	*/

	    /* get pointer to it, like a good fellow	*/
	    f1d = INST_DATA( cl, newobj );

	    /* mandatory default init */
	    f1d->f1d_LineWidth = DEFAULTLINEWIDTH;

	    /* may override f1d_LineWidth	*/
	    setFrame1Attrs( cl, newobj, msg );
	}

	return ( (ULONG) newobj );

    case OM_GET:
	return ( (ULONG) getFrame1Attrs( cl, o, msg  ) );

    case OM_SET:
	DSM( cl, o, msg );		/* let the superclass see the atts */
	setFrame1Attrs( cl, o, msg  );	/* set the ones I care about	   */
	return ( (ULONG) 1 );		/* i'm happy			   */

    case IM_DRAW:			/* draw with state */
    case IM_DRAWFRAME:			/* special case of draw	*/
	return ( (ULONG) drawFrame1( cl, o, msg ) );

    case IM_FRAMEBOX:
	frameBox( cl, o, msg );
    	return ( 1 );			/* let him know I support this	*/

    /* use superclass defaults */
    case IM_HITFRAME:
    case IM_ERASEFRAME:

    case IM_HITTEST:
    case IM_ERASE:
    case OM_DISPOSE:
    default:
	return ( (ULONG) DSM( cl, o, msg ) );
    }
}

/*
 * return surrounding box, with a little bit of space.
 * If caller wants more space, he can pass me a larger
 * box
 */
frameBox( cl, o, msg )
Class		*cl;
Object		*o;
struct impFrameBox	*msg;	/* superset of impDraw	*/
{
	struct Frame1Data *f1d;
	UWORD		surround_room;
	struct IBox	*fbox;

	f1d = INST_DATA( cl, o );

	fbox = msg->imp_FrameBox;

	D( printf("frameBox height %lx\n", msg->imp_FrameBox->Height ) );

	if ( ! ( msg->imp_FrameFlags & FRAMEF_SPECIFY ) )
	{
	    *fbox = *msg->imp_ContentsBox;
	    surround_room = (f1d->f1d_LineWidth << 1);
	    fbox->Width += (surround_room << 1);
	    fbox->Height += (surround_room << 1);

	    fbox->Left -= surround_room;
	    fbox->Top  -= surround_room;

	    D( kprintf("frame cont. height %ld query height %ld\n",
	    	msg->imp_ContentsBox->Height, fbox->Height ) );
	}
	else
	{
	    /* use given dimensions and center	*/
	    fbox->Left =  msg->imp_ContentsBox->Left -
		(fbox->Width - msg->imp_ContentsBox->Width )/2;
	    fbox->Top =  msg->imp_ContentsBox->Top - 
		(fbox->Height - msg->imp_ContentsBox->Height )/2;
	}
}

drawFrame1( cl, o, msg )
Class		*cl;
Object		*o;
struct impDraw	*msg;	/* superset of impDraw	*/
{
    struct Frame1Data 	*f1d = INST_DATA( cl, o );
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

    if ( msg->MethodID == IM_DRAWFRAME )
    {
	box.Width = msg->imp_Dimensions.Width;
	box.Height = msg->imp_Dimensions.Height;
    }

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
	f1d->f1d_LineWidth, f1d->f1d_LineWidth,
	ulpen, lrpen );

    /* interior */
    interiorBox( msg->imp_RPort, &box,
	f1d->f1d_LineWidth,	/* inset in x dim */
	f1d->f1d_LineWidth,	/* inset in y dim */
	fillpen );
}

/* return requested attribute value	*/
getFrame1Attrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opGet	*msg;
{
    struct Frame1Data 	*f1d = INST_DATA( cl, o );

    if ( msg->opg_AttrID == IA_LINEWIDTH )
    {
    	*msg->opg_Storage = f1d->f1d_LineWidth;
	return ( 1 );
    }
    else
    	return ( DSM( cl, o, msg ) );

}

/* set specified attribute value.
 * Technique provided is a little more general than we need ...
 */
setFrame1Attrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tag;
    ULONG		tidata;

    struct Frame1Data 	*f1d = INST_DATA( cl, o );

    while ( tag = NextTagItem( &tags ) )
    {
	tidata = tag->ti_Data;
	switch ( tag->ti_Tag )
	{
	case IA_LINEWIDTH:
	    f1d->f1d_LineWidth = tidata;
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

