/*** propgadclass.c *******************************************************
 *
 * propgadclass.c -- proportional gadget class
 *
 *  $Id: propgadclass.c,v 38.2 92/08/06 16:59:06 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "gadgetclass.h"

#include "propgadclass_protos.h"

/*---------------------------------------------------------------------------*/

static ULONG propgadgetDispatch(Class * cl,
                                Object * o,
                                Msg msg);

static int getPropGadgetAttrs(Class * cl,
                              Object * o,
                              struct opGet * msg);

static int setPropGadgetAttrs(Class * cl,
                              Object * o,
                              struct opSet * msg);

static int updatePropInfo(struct PGIData * pgi);

static UWORD propTop(struct PGIData * pgi);

static UWORD propBody(struct ScrollSet * ss);

static UWORD propPot(struct ScrollSet * ss);

static UWORD propHidden(struct ScrollSet * ss);

/*---------------------------------------------------------------------------*/


#define D(x)	;
#define DN(x)	;
#define DM(x)	;

struct ScrollSet {
    UWORD	ss_Top;
    UWORD	ss_Visible;
    UWORD	ss_Total;
};
UWORD propTop();
UWORD propBody();
UWORD propPot();
UWORD propHidden();

/* new instance data for objects of propgadget class */
struct PGIData {
    struct PropInfo	pgi_PInfo;
    struct Image	pgi_AutoKnob;	/* ZZZ: better if dynamic alloc */
    struct ScrollSet	pgi_ScrollSet;
};

#define G(o)		((struct Gadget *) (o))		/* base class */
#define PGI( pgid ) 	(&pgid->pgi_PInfo)

/*
 * should be replaced by magic somewhere, but this
 * is a ROM class, so I don't worry about it today.
 */
Class	*
initPropGadgetClass()
{
    ULONG propgadgetDispatch();
    Class	*makePublicClass();
    extern UBYTE	*PropGClassName;
    extern UBYTE	*GadgetClassName;

    return ( makePublicClass( PropGClassName, GadgetClassName ,
		sizeof(struct PGIData), propgadgetDispatch ) );
}

/*
 * The main dispatcher (or hook) for this class of custom gadgets
 */
static ULONG
propgadgetDispatch( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    struct IntuitionBase	*IBase = fetchIBase();
    Object  		*newobj;
    struct PGIData	*pgi;

    ULONG		retval = 0;
    UWORD		oldtop;

    pgi = INST_DATA( cl, o );

    switch ( msg->MethodID )
    {
    case OM_NEW:
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    pgi = INST_DATA( cl, newobj );

	    /* set defaults for some fields, and
	     * also set required values which I will enforce
	     * in OM_SET.
	     */
	    mySetSuperAttrs( cl, newobj,
			/* defaults */
		GA_IMAGE,	GetUserTagData( GA_IMAGE, &pgi->pgi_AutoKnob,
				    ((struct opSet *)msg)->ops_AttrList ),
			/* required */
		GA_SPECIALINFO,	&pgi->pgi_PInfo,
		TAG_END );

	    /* init the instance data of this class
	     */
	    PGI(pgi)->Flags = AUTOKNOB;
	    pgi->pgi_ScrollSet.ss_Visible = 25;
	    pgi->pgi_ScrollSet.ss_Total   = 100;
	    /* propinfo internals will be set by OM_SET */

	    /* set initial attributes, for THIS class, not 'true' class */
#if 1
	    setPropGadgetAttrs( cl, newobj, msg );
#else
	    CoerceMessage( cl, newobj, OM_SET, 
		((struct opSet *)msg)->ops_AttrList, NULL );
#endif
	}
	return ( (ULONG) newobj );

    case OM_SET:
    case OM_UPDATE:
	{
	    ULONG		refresh;
	    struct RastPort	*rp;

	    refresh =  SSM( cl, o, msg );
	    refresh +=  setPropGadgetAttrs( cl, o, msg );

	    /* if I'm the true class, then handle refreshing of visuals */
	    if ( refresh && ( OCLASS(o) == cl ) )
	    {
		rp = ObtainGIRPort( ((struct opSet *)msg)->ops_GInfo );
		if ( rp ) 
		{
#if 0
have put GREDRAW_UPDATE logic in phooks

		    struct PGX	mypgx;

		    /* This is from NewModifyProp.
		     * Want to update knob only, and this is
		     * a good example of where inheritance is lame.
		     */
		    setupPGX( o, ((struct opSet *)msg)->ops_GInfo, &mypgx );
		    renderNewKnob( o,((struct opSet *)msg)->ops_GInfo, &mypgx, rp );
#else
		    CoerceMessage( cl, o, GM_RENDER, 
			((struct opSet *)msg)->ops_GInfo,
			rp,
			GREDRAW_UPDATE );	/* redraw */
#endif
		}
		ReleaseGIRPort( rp );
		return ( 0 );		/* don't need refresh any more */
	    }
	    /* else */
	    return ( (ULONG) refresh );	/* return to subclass perhaps	*/
	}

    case OM_GET:
	return ( getPropGadgetAttrs( cl, o, msg ) );

    case GM_HITTEST:
    case GM_RENDER:
    case GM_GOINACTIVE:
	/* This is what it's all about: here you "demote" one of
	 * these fancy class objects into a plain old Intuition
	 * boolean gadget by using the stripped down boolean hooks.
	 *
	 * This is why objects have their secret header instance
	 * data before the gadget/object address.
	 *
	 * This is why Hooks have a MinNode so that a Hook
	 * pointer and a class pointer are the same.
	 */
	DM( printf("propgadclass pass to phooks\n") );
	return ( callHookPkt( &IBase->IIHooks[ IHOOK_PROPG ], o, msg ) );

	/* ZZZ: separate GADGETUP notification of change? */

    /* I need to update my ScrollSet if things move */
    case GM_GOACTIVE:	/* stash original top value */
    case GM_HANDLEINPUT:
	oldtop = pgi->pgi_ScrollSet.ss_Top;
	retval = callHookPkt( &IBase->IIHooks[ IHOOK_PROPG ], o, msg );
	if ( oldtop != (pgi->pgi_ScrollSet.ss_Top = propTop( pgi ) )
		|| (retval != GMR_MEACTIVE) )
	{
	    DN( printf("propg handlei notify top change: %lx\n", 
		pgi->pgi_ScrollSet.ss_Top ) );
		
	    /* NOTIFY PGA_TOP */
	    NotifyAttrChanges( o, ((struct gpInput *)msg)->gpi_GInfo,
		(retval == GMR_MEACTIVE)? OPUF_INTERIM: 0,
		PGA_TOP,	pgi->pgi_ScrollSet.ss_Top,
		GA_ID,		G(o)->GadgetID,
		TAG_END );
	}
	return ( retval );

    default:
	DM( printf("pgD: default case\n") );
	return ( SSM( cl, o, msg ) );	/* let the superclass handle it */
    }
}

#define FREEDOM	(FREEHORIZ | FREEVERT )

static
getPropGadgetAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opGet	*msg;
{
    struct PGIData 	*pgi;
    pgi = INST_DATA( cl, o );

    switch ( msg->opg_AttrID )
    {
    case PGA_FREEDOM:
	*msg->opg_Storage = PGI(pgi)->Flags & FREEDOM;
	break;
    case PGA_TOP:
	D( printf("prop om_get returning top value: %ld\n",
	    pgi->pgi_ScrollSet.ss_Top ) );
	*msg->opg_Storage = pgi->pgi_ScrollSet.ss_Top;
	break;

    /* let the superclass try	*/
    default:
	return ( SSM( cl, o, msg ) );
    }
    return ( 1 );	/* found it */
}

static
setPropGadgetAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct PGIData 	*pgi = INST_DATA( cl, o );

    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tstate = tags;
    struct TagItem	*tag;
    ULONG		tidata;

    struct ScrollSet	oldss;

    /* ZZZ: weird scheme: perhaps would be more convenient
     * to clone and filter the att list
     */

    /* will make this true if anything makes us want to
     * override some of the values the ancestors have seen.
     */
    BOOL		need_override = FALSE;

    int		refresh = 0;

    DN( printf("setPropGA \n"));

    /* remember values for which changing means refresh */
    oldss = pgi->pgi_ScrollSet;

    /* process rest */
    while ( tag = NextTagItem( &tstate ) )
    {
	tidata = tag->ti_Data;

	DN( printf("proptag %lx, data %lx\n", tag->ti_Tag, tag->ti_Data ));
	switch ( tag->ti_Tag )
	{

	/* dimension minimums enforced on box attributes? */

	/* special handling: set autoknob if null image or
	 * someone tries to put a border past us.
	 */
	/* init time only, please */
	case GA_IMAGE:
	    if ( tidata != NULL )
	    {
		CLEARFLAG( PGI( pgi )->Flags, AUTOKNOB );
		break;
	    }
	    /* else fall through and set default
	     */
	/* init time only, please */
	case GA_BORDER:	/* illegal */
	    mySetSuperAttrs( cl, o,
		GA_IMAGE, &pgi->pgi_AutoKnob,
		TAG_END );
	    SETFLAG( PGI( pgi )->Flags, AUTOKNOB );
	    break;

	/* illegal attribute or values: override below */
	case GA_HIGHLIGHT:
	    if ( tidata != GADGHBOX ) break;
	    /* else, will convert GADGHBOX to GADGHCOMP */
	    /* ZZZ: oops, looks like it will convert GADGHNONE to
	     * GADGHCOMP!
	     */
	case GA_SPECIALINFO:
	    need_override = TRUE;
	    break;

	/* propg specific attributes.
	 * these should have been unrecognized
	 * by ancestors.
	 */
	/* init time only, please */
	case PGA_FREEDOM:
	    PGI( pgi )->Flags = (PGI( pgi )->Flags & ~FREEDOM ) | tidata;
	    break;

	/* init time only, please */
	case PGA_BORDERLESS:
	    CLEARFLAG( PGI( pgi )->Flags, PROPBORDERLESS );
	    if ( tidata )
	    {
		SETFLAG( PGI( pgi )->Flags, PROPBORDERLESS );
	    }
	    break;

	/* init time only, please */
	case PGA_NewLook:
	    CLEARFLAG( PGI( pgi )->Flags, PROPNEWLOOK );
	    if ( tidata )
	    {
		SETFLAG( PGI( pgi )->Flags, PROPNEWLOOK );
	    }
	    break;

	case PGA_TOTAL:
	    pgi->pgi_ScrollSet.ss_Total = tidata;
	    DN( printf("propgad setAtts ss_Total: %ld\n", tidata ) );
	    break;

	case PGA_VISIBLE:
	    pgi->pgi_ScrollSet.ss_Visible = tidata;
	    break;

	case PGA_TOP:
	    pgi->pgi_ScrollSet.ss_Top = tidata;
	    DN( printf("propgad setAtts ss_Top: %ld\n", tidata ) );
	    break;
	}
    }

    /* if anything important changed, do refresh */
    /* WARNING: structure comparison */
    if ( oldss != pgi->pgi_ScrollSet )
    {
	updatePropInfo( pgi );		/* set pots/bodies */
	refresh = 1;
    }

    /* check that there's something sensible in some fields
     */
    if ( ! TESTFLAG( PGI( pgi )->Flags, FREEDOM ) )
	SETFLAG( PGI( pgi )->Flags, FREEVERT );

    /*
     * if anything smelled funny above, patch them all
     * up at once.   ZZZ: maybe they should be filtered
     * from the superclass.
     */
    if ( need_override )
    {
	mySetSuperAttrs( cl, o,
	    GA_HIGHLIGHT, GADGHCOMP,
	    GA_SPECIALINFO, &pgi->pgi_PInfo,
	    TAG_END );
    }

    return ( refresh );
}

/* update pot, body from ScrollSet */
static
updatePropInfo( pgi )
struct PGIData 	*pgi;
{
    if ( TESTFLAG( PGI(pgi)->Flags, FREEHORIZ ) )
    {
	PGI(pgi)->HorizBody = propBody( &pgi->pgi_ScrollSet );
	PGI(pgi)->HorizPot = propPot( &pgi->pgi_ScrollSet );
    }
    else
    {
	PGI(pgi)->VertBody = propBody( &pgi->pgi_ScrollSet );
	PGI(pgi)->VertPot = propPot( &pgi->pgi_ScrollSet );
    }
}

/* calculate ss_Top from other ss_* and propinfo variables */
static UWORD
propTop( pgi )
struct PGIData 	*pgi;
{
    UWORD	hidden = propHidden( &pgi->pgi_ScrollSet );
    UWORD	pot;

    pot = TESTFLAG( PGI(pgi)->Flags, FREEHORIZ )? PGI(pgi)->HorizPot:
						PGI(pgi)->VertPot;

    return ( ((ULONG) hidden * pot + MAXPOT/2 )/MAXPOT );
}

/* return Vert/HorizBody for given ScrollSet */
static UWORD
propBody( ss )
struct ScrollSet	*ss;
{
    UWORD	hidden = propHidden( ss );
    UWORD	overlap;
    UWORD	body;
    ULONG	tmp;

    if ( ss->ss_Top > hidden ) ss->ss_Top = hidden;

    overlap = (ss->ss_Visible > 1)? 1: 0;

    if ( hidden == 0 )
	return( MAXBODY );

    tmp = (((ULONG) ss->ss_Visible-overlap)*MAXBODY);

    body =  tmp / (ss->ss_Total-overlap);

    D( printf("propBody. hidden %ld, overlap %ld, bodycalc %ld (0x%lx)\n",
	hidden, overlap, body, body ) );

    D( printf("calc from visible %ld, total %ld tmp 0x%lx\n", 
	ss->ss_Visible, ss->ss_Total, tmp ) );

    return ( body );
}

static UWORD
propPot( ss )
struct ScrollSet	*ss;
{
    UWORD	hidden = propHidden( ss );

    if ( hidden == 0 )
	return ( 0 );
    
    return ( (UWORD) (((ULONG) ss->ss_Top * MAXPOT) / hidden ) );
}

static UWORD
propHidden( ss )
struct ScrollSet	*ss;
{
    return ( imax( ss->ss_Total - ss->ss_Visible, 0 ) );
}
