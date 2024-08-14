/*** textbclass.c *******************************************************
 *
 * textbclass.c -- (repeating) command button gadget class
 *
 *  $Header: 
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

/*
Copyright (c) 1989 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#include "sysall.h"
#include <intuition/classes.h>
#include <graphics/gfxmacros.h>

#define D(x)	;

extern struct  IntuitionBase   *IntuitionBase;
extern struct  GfxBase         *GfxBase;
extern struct  Library         *UtilityBase;

/* transparent base class	*/
#define G(o)	((struct Gadget *)(o))

/* jimm's peculiarities */
#define testf(v,f) ((v) & (f))
#define setf(v,f)	((v) |= (f))
#define clearf(v,f)	((v) &= ~(f))

#define DH(x)	;

/* this useful to convert a point structure to a 32 bit vanilla
 * data type if you want to pass it via libcall to lattice.
 * probably can fudge it some other way ...
 */
/* #define POINTLONG( pt )	(((pt).X<<16) + (pt).Y) */

#define POINTLONG( pt )	(*( (ULONG *)  &(pt) ))

/* private class	*/
#define PRIVATECLASS	TRUE

#if PRIVATECLASS
#define MYCLASSID	(NULL)
#else
#define MYCLASSID	"textbclass"
#endif

#define SUPERCLASSID	"buttongclass"
extern struct Library	*IntuitionBase;

struct TextBData {

    /* this is the offset of the contents, relative
     * (0,0) of the gadget and its frame
     */
    struct {
	UWORD	X;
	UWORD	Y;
    }		tbd_Offset;
    ULONG	tbd_Flags;
#define TBDF_USEFRAME	(1<<0)
};


Class	*
initTextBClass()
{
    ULONG __saveds	dispatchTextB();
    ULONG	hookEntry();
    Class	*cl;
    Class	*MakeClass();

    if ( cl =  MakeClass( MYCLASSID, 
		SUPERCLASSID, NULL,		/* superclass is public      */
 		sizeof( struct TextBData ),
		0 ))
    {
	/* initialize the cl_Dispatcher Hook	*/
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchTextB;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;	/* unused */

#if !PRIVATECLASS
	AddClass( cl );			/* make public and available	*/
#endif
    }
    return ( cl );
}

freeTextBClass( cl )
Class	*cl;
{
    return ( FreeClass( cl )  );
}


/*
 * The main dispatcher for this class of custom gadgets
 */
ULONG __saveds 
dispatchTextB( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    ULONG		DoMethod();
    Object  		*newobj;
    struct  RastPort	*rp;
    int			refresh;
    struct TextBData 	*tbd;
    ULONG		SetSuperAttrs();

    struct  RastPort	*ObtainGIRPort();

    tbd = INST_DATA( cl, o );

    switch ( msg->MethodID )
    {
    case OM_NEW:
	D( kprintf("textbclass new\n") );
	if ( newobj = (Object *) DSM( cl, o, msg ) )
	{
	    tbd = INST_DATA( cl, newobj );	/* want it for newobject */

	    /* frame our text, if any text ... */
	    if ( G(newobj)->GadgetText )
	    {
		struct IBox	contents;
		struct IBox	framebox;
		struct DrawInfo	*drinfo;

		/* get the contents	*/
		drinfo = (struct DrawInfo *) GetTagData( GA_DRAWINFO, NULL, 
		    ((struct opSet *) msg)->ops_AttrList );
		getContentsExtent( newobj, drinfo, &contents );

		/* and if our image understands IM_FRAMEBOX	*/
		if ( DoMethod(  G(newobj)->GadgetRender, IM_FRAMEBOX,
			&contents, &framebox, drinfo, 0 ) )
		{
		    /* use the frame dimensions and offset	*/
		    tbd->tbd_Flags |= TBDF_USEFRAME;
		    D( kprintf("call SSA to set width/height %lx/%lx\n",
		    	framebox.Width, framebox.Height ));

		    SetSuperAttrs( cl, newobj, 
		    	GA_WIDTH, framebox.Width,
			GA_HEIGHT, framebox.Height,
			TAG_END );
		    D( kprintf(" SetSuperAttrs returns, gadget Height %\n"));

		    /* when we draw the frame at 0,0, we
		     * need to offset the text contents 
		     * in the opposite direction.
		     */
		    tbd->tbd_Offset.X = -framebox.Left;
		    tbd->tbd_Offset.Y = -framebox.Top;
		}
		/* ZZZ: else just fail if not a frame? */
	    }

	    /*  set attributes in general, which might force dims */
	    setTextBAttrs( cl, o, msg );
	}
	return ( (ULONG) newobj );

    case OM_UPDATE:	
    case OM_SET:	
	/* inherit atts, accumulated refresh	*/
	refresh = DSM( cl, o, msg );
	refresh += setTextBAttrs( cl, o, msg );

	/* take responsibility here for refreshing visuals
	 * if I am the "true class."  It's sort of too bad
	 * that every class has to do this itself.
	 */
	if ( refresh && ( OCLASS(o) == cl ) )
	{
	    rp = ObtainGIRPort( ((struct opSet *)msg)->ops_GInfo );
	    if ( rp ) 
	    {
		renderTextG( rp, ((struct opSet *)msg)->ops_GInfo, G(o), tbd,
			GREDRAW_UPDATE );
	    }
	    ReleaseGIRPort( rp );
	    return ( 0 );		/* don't need refresh any more */
	}
	/* else */
	return ( (ULONG) refresh );	/* return to subclass perhaps	*/

    case GM_HITTEST:


#if 1	/* use HITFRAME */
	/* ask our frame if we have one  */
	return ( DoMethod( G(o)->GadgetRender, 
		IM_HITFRAME,	/* even non-frames will understand this */
		POINTLONG( ((struct gpHitTest *) msg)->gpht_Mouse ),
		*((ULONG *) &G(o)->Width) ) );	/* trick	*/

#else
	return ( (ULONG) PointInImage( pointlong, G(o)->GadgetRender ) );
#endif  /* use HITFRAME */

    case GM_RENDER:
	renderTextG( ((struct gpRender *) msg)->gpr_RPort,
	    ((struct gpRender *) msg)->gpr_GInfo, o, tbd,
		((struct gpRender *)msg)->gpr_Redraw );
	break;

    OM_GET:	/* I'm not telling anyone about it yet	*/
    case GM_GOACTIVE:
    case GM_GOINACTIVE:
    case GM_HANDLEINPUT:	/* the big one	*/
    default:	/* let the superclass handle it */
	return ( (ULONG) DSM( cl, o, msg ) );
    }
    return ( 0 );
}


renderTextG( rp, gi, o, tbd, redraw_mode )
struct RastPort		*rp;
struct GadgetInfo	*gi;
Object			*o;
struct TextBData 	*tbd;
{
    int		state;

    if ( redraw_mode == GREDRAW_TOGGLE ) return;

    state = buttonDrawState( G(o), gi );

    /* draw frame	*/
    DoMethod( G(o)->GadgetRender, IM_DRAWFRAME, rp, 
    			*((ULONG *) &G(o)->LeftEdge),
			state,
			gi->gi_DrInfo,
    			*((ULONG *) &G(o)->Width) );	/* frame dimensions */
			
    displayContents( rp, o, gi->gi_DrInfo,
    	G(o)->LeftEdge + tbd->tbd_Offset.X,
    	G(o)->TopEdge + tbd->tbd_Offset.Y, state );
}

/*
 * WARNING: this might change!
 *
 * returns image drawstate suitable for
 * gadget active/border circumstances
 */

#define ALLBORDERS (RIGHTBORDER+LEFTBORDER+TOPBORDER+BOTTOMBORDER)
buttonDrawState( g, gi )
struct Gadget		*g;
struct GadgetInfo	*gi;
{
    if ( g && testf( g->Flags, SELECTED ) )
	return ( IDS_SELECTED );

    if ( g && gi && gi->gi_Window &&
	! testf( gi->gi_Window->Flags, WINDOWACTIVE ) &&
	( testf( g->Activation, ALLBORDERS ) ||
	  testf( g->GadgetType, 0xF0 ) ) )	/* a "system" gadget */
    {
	if ( testf( g->Flags, SELECTED ) )
	    return ( IDS_INACTIVESELECTED );
	/* else */
	    return ( IDS_INACTIVENORMAL );
    }

    else if ( g && testf( g->Flags, SELECTED ) )
    	return (IDS_SELECTED );
    /* else */
    return ( IDS_NORMAL );
}

setTextBAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tag;
    ULONG		tidata;
    struct TextBData 	*tbd;

    int			refresh = 0;
    struct DrawInfo	*drinfo;
    struct IBox		newframe;

    tbd = INST_DATA( cl, o );

    /* only interested in Width/Height fields */
    newframe.Width = G(o)->Width;
    newframe.Height = G(o)->Height;

    /* process rest */
    while ( tag = NextTagItem( &tags ) )
    {
	tidata = tag->ti_Data;
	switch ( tag->ti_Tag )
	{

	/* if forced dimension change, then
	 * center text, recalc offsets using FRAMEF_SPECIFY
	 */
 	case GA_WIDTH:
	    refresh = 1;
	    newframe.Width = tidata;
	    break;
 	case GA_HEIGHT:
	    refresh = 1;
	    newframe.Height = tidata;
	    break;
	/* could also reset for a new GA_IMAGE here, if
	 * you wanted
	 */
	}
    }

    /* fix up changes which require some work	*/
    if ( refresh )
    {
	/* get new offsets from the specified frame dimensions	*/

	if ( tbd->tbd_Flags & TBDF_USEFRAME )
	{
	    struct IBox	contents;

	    /* really need DrawInfo, if layout change	*/
	    drinfo = msg->ops_GInfo? msg->ops_GInfo->gi_DrInfo: NULL;

	    /* override with tags version */
	    drinfo = (struct DrawInfo *) GetTagData( GA_DRAWINFO, drinfo, tags);

	    /* get contents box again */
	    getContentsExtent( o, drinfo, &contents );

	    DoMethod(  G(o)->GadgetRender, IM_FRAMEBOX, &contents,
	    	&newframe, drinfo, FRAMEF_SPECIFY );

	    tbd->tbd_Offset.X = -newframe.Left;
	    tbd->tbd_Offset.Y = -newframe.Top;
	}

	/* superclass knows about the new dimensions, and I don't
	 * have any reason to disagree, so no need to 
	 * override the values now.
	 */
    }

    return ( refresh );
}

/*
 * This is going to be nifty.
 * Right now, we'll use GadgetText as (UBYTE *),
 * but we could conceivably code for this
 * to be either simple text, or a pointer to
 * a generic image Object which could be a text label,
 * or glyph, or what have you.
 *
 * Then this class becomes the "button with label which is
 * rendered by overdrawing some frame" class.
 */
getContentsExtent( o, drinfo, contents )
Object		*o;
struct DrawInfo	*drinfo;
struct IBox	*contents;
{
    UBYTE	*label;
    struct RastPort	rport;
    struct TextExtent	textent;

    /* maybe look at some flags to handle other types of text someday */
    if ( label = (UBYTE *) G(o)->GadgetText )
    {
	InitRastPort( &rport );
	if ( drinfo && drinfo->dri_Font )
	{
	    SetFont( &rport, drinfo->dri_Font );
	}
	TextExtent( &rport, label, strlen( label ), &textent );

	/* convert extent Rectangle to an IBox	*/
	contents->Left = textent.te_Extent.MinX;
	contents->Top = textent.te_Extent.MinY;
	contents->Width = textent.te_Extent.MaxX -  textent.te_Extent.MinX + 1;
	contents->Height = textent.te_Extent.MaxY -  textent.te_Extent.MinY + 1;
    }
    else
    {
	contents->Left = contents->Top =
	    contents->Width = contents->Height = 0;
    }
}

/*
 * show enclosed text label in correct color.
 * always use textpen for now
 */
displayContents( rp, o, drinfo, offsetx, offsety, state )
struct RastPort	*rp;
Object		*o;
struct DrawInfo	*drinfo;
WORD		offsetx;
WORD		offsety;
{
    UBYTE	*label;

    if ( label = (UBYTE *) G(o)->GadgetText )
    {
	SetDrMd( rp, JAM1 );
	SetAPen( rp, drinfo->dri_Pens[ textPen ] );
	Move( rp, offsetx, offsety );
	Text( rp, label, strlen( label ) );
    }
}
