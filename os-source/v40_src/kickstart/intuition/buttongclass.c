/*** buttongclass.c *******************************************************
 *
 * buttongclass.c -- (repeating) command button gadget class
 *
 *  $Id: buttongclass.c,v 38.3 92/08/02 12:41:08 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "imageclass.h"
#include "gadgetclass.h"
#include <graphics/gfxmacros.h>

/* transparent base class	*/
#define G(o)	((struct Gadget *)(o))

/* jimm's peculiarities */
#define testf(v,f) ((v) & (f))
#define setf(v,f)	((v) |= (f))
#define clearf(v,f)	((v) &= ~(f))

#define D(x)	;
#define D1(x)	;
#define DH(x)	;

/* this useful to convert a point structure to a 32 bit vanilla
 * data type if you want to pass it via libcall to lattice.
 * probably can fudge it some other way ...
 */
/*#define POINTLONG( pt )	(((pt).X<<16) + (pt).Y) */
#define POINTLONG( pt )	(*( (ULONG *)  &(pt) ))

Class	*
initButtonGClass()
{
    Class	*makePublicClass();
    ULONG 	dispatch();
    extern UBYTE	*ButtonGClassName;
    extern UBYTE	*GadgetClassName;

    /* no instance data for this subclass (uses Flags:SELECTED)	*/
    return (  makePublicClass( ButtonGClassName, GadgetClassName, 0, dispatch));
}


/*
 * The main dispatcher for this class of custom gadgets
 */
static ULONG 
dispatch( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct  RastPort	*rp;
    UWORD		overgadget;
    ULONG		pointlong;
    int			refresh;

    struct  RastPort	*ObtainGIRPort();

    switch ( msg->MethodID )
    {
    case OM_NEW:
	D( printf("buttongclass new\n") );
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    D( printf("got buttong\n") );
	    /* no instance data to set up	*/

	    setButtonAttrs( cl, newobj, msg );
	}
	return ( (ULONG) newobj );

    case OM_UPDATE:	
    case OM_SET:	
	/* inherit atts, accumulated refresh	*/
	D1( printf("buttongclass set/update\n") );
	refresh = SSM( cl, o, msg );
	D1( printf("SSM returned\n"));
	refresh += setButtonAttrs( cl, o, msg );
	D1( printf("sBA returned\n"));

	/* take responsibility here for refreshing visuals
	 * if I am the "true class."
	 */
	/*
	 * ZZZ: black box error ... does every gadget class have to do this?
	 */
	if ( refresh && ( OCLASS(o) == cl ) )
	{
	    D( printf("think it's time to refresh\n"));
	    rp = ObtainGIRPort( ((struct opSet *)msg)->ops_GInfo );
	    if ( rp ) 
	    {
		D( printf("refresh\n"));
		renderButtonG( rp,
			((struct opSet *)msg)->ops_GInfo, G(o),
			GREDRAW_UPDATE );
	    }
	    ReleaseGIRPort( rp );
	    return ( 0 );		/* don't need refresh any more */
	}
	/* else */
	D1( printf("returned refresh %lx\n", refresh));
	return ( (ULONG) refresh );	/* return to subclass perhaps	*/

    case GM_HITTEST:
	/* ask our image */
	D( printf("buttongclass hittest\n"));
	pointlong = 
		POINTLONG( ((struct gpHitTest *) msg)->gpht_Mouse );

	DH( printf("buttong %lx hittest: %lx\n", o,
	    PointInImage( pointlong, G(o)->GadgetRender ) ) );

	return ( (ULONG) PointInImage( pointlong, G(o)->GadgetRender )?
		GMR_GADGETHIT: 0);

    case GM_RENDER:
	D( printf("buttongclass render\n"));
	renderButtonG( ((struct gpRender *) msg)->gpr_RPort,
	    ((struct gpRender *) msg)->gpr_GInfo, o,
		((struct gpRender *)msg)->gpr_Redraw );
	break;

    case GM_GOACTIVE:
	D( printf("buttongclass goactive\n"));
	if ( ((struct gpInput *)msg)->gpi_IEvent )
	{
	    NotifyAttrChanges( o,
		((struct gpInput *)msg)->gpi_GInfo, OPUF_INTERIM, 
		GA_ID, G(o)->GadgetID,
		TAG_END );

	    return ( GMR_MEACTIVE );
	}

	return( GMR_NOREUSE );
	break;

    case GM_GOINACTIVE:
	/* leave g->Flags.SELECTED as is, and decide whether to notify */
	/* this doesn't handle abort: should turn off button,
	 * but works for the time being in the case where I
	 * said I was done.
	 */
	D( printf("buttongclass goinactive\n"));
	break;

    case GM_HANDLEINPUT:
	pointlong = POINTLONG(  ((struct gpInput *)msg)->gpi_Mouse );

	overgadget= SendMessage( o, GM_HITTEST,
			((struct gpInput*)msg)->gpi_GInfo,
			pointlong )? SELECTED: 0;

	D( printf("buttong HI, overgadget: %lx\n", overgadget ) );
	D( printf("gadget flag: %lx, SELECTED %lx, testf %lx\n",
		G(o)->Flags, SELECTED, testf(G(o)->Flags, SELECTED) ) );

	/* if visual state should change, make it so	*/
	if ( overgadget != testf( G(o)->Flags, SELECTED) )
	{
	    /* need to toggle state */
	    D( printf("buttong toggle state to %lx\n", overgadget) );
	    G(o)->Flags ^= SELECTED;

	    rp = ObtainGIRPort( ((struct gpInput *) msg)->gpi_GInfo);
	    if ( rp )
	    {
		/***  send message to self/subclasses	***/
		SendMessage( o, GM_RENDER,
		    ((struct gpInput *) msg)->gpi_GInfo,
		    rp,
		    GREDRAW_UPDATE );
	    }
	    ReleaseGIRPort( rp );
	}

	if ( isSelectup( ((struct gpInput *)msg)->gpi_IEvent ) )
	{
	    /* upbutton: swallow event, end gadget */
	    if ( overgadget )
	    {
		/* need to turn off visuals */
		CLEARFLAG( G(o)->Flags, SELECTED );
		rp = ObtainGIRPort( ((struct gpInput *) msg)->gpi_GInfo);
		if ( rp )
		{
#if 1		    /***  send message to self/subclasses	***/
		    SendMessage( o, GM_RENDER,
			((struct gpInput *) msg)->gpi_GInfo,
			rp,
			GREDRAW_UPDATE );
#else
		    renderButtonG( rp, ((struct gpInput *) msg)->gpi_GInfo,
			G(o), GREDRAW_UPDATE );
#endif
		}
		ReleaseGIRPort( rp );
	    }

	    D( printf("buttong making last notification\n"));

	    NotifyAttrChanges( o, ((struct gpGoInactive *)msg)->gpgi_GInfo, 0, 
		    GA_ID, overgadget? G(o)->GadgetID: -G(o)->GadgetID,
		    TAG_END );
	    D( printf("buttong done, verify if %lx\n", overgadget ) );
	    return ( (ULONG)
	    	(overgadget? (GMR_NOREUSE | GMR_VERIFY): GMR_NOREUSE ));
	}
	else if ( isTick( ((struct gpInput *)msg)->gpi_IEvent ) )
	{
	    NotifyAttrChanges( o, ((struct gpGoInactive *)msg)->gpgi_GInfo,
		OPUF_INTERIM,
		GA_ID, overgadget? G(o)->GadgetID: -G(o)->GadgetID,
		TAG_END );
	}
	/*** ZZZ: else if timer tick, strobe clickrepeat ***/
	return ( GMR_MEACTIVE );

#if 0
    case OM_GET: /* I don't have any instance data to call my own	*/
#endif
    default:	/* let the superclass handle it */
	return ( (ULONG) SSM( cl, o, msg ) );
    }
    return ( 0 );
}

/*
 * doesn't draw label
 */
renderButtonG( rp, gi, o, redraw_mode )
struct RastPort	*rp;
struct GadgetInfo	*gi;
Object		*o;
{
    struct IBox	gbox;
    int		state;

    if ( redraw_mode == GREDRAW_TOGGLE ) return;

    gadgetBox( G(o), gi, &gbox );	/* handle GREL relative position */

    state = gadgetDrawState( G(o), gi, FALSE );

	/* relativity */
    DrawImageState( rp, G(o)->GadgetRender,
	gbox.Left, gbox.Top, state, gi->gi_DrInfo);
}

setButtonAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tag;
    ULONG		tidata;

    int			refresh = 0;
    struct Image	*im;

    /* process rest */
    while ( tag = NextTagItem( &tags ) )
    {
	tidata = tag->ti_Data;
	switch ( tag->ti_Tag )
	{
	case GA_IMAGE:
	    /* reset my gadget size to fit new image */
	    if ( im = (struct Image *) G(o)->GadgetRender )
	    {
		SetSuperAttrs( cl, o,
		    GA_WIDTH, im->Width,
		    GA_HEIGHT, im->Height,
		    TAG_END );
	    }
	    else
	    {
		SetSuperAttrs( cl, o,
		    GA_WIDTH, 0,
		    GA_HEIGHT, 0,
		    TAG_END );
	    }
	    refresh = 1;
	    break;
	}
    }
    return ( refresh );
}

