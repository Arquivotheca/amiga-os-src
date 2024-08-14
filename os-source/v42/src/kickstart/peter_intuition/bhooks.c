
/*** bhooks.c ****************************************************************
 *
 *  Boolean Gadget Hooks
 *
 *  $Id: bhooks.c,v 38.3 92/08/02 12:40:55 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1988, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "gadgetclass.h"

#ifndef HARDWARE_BLIT_H
#include <hardware/blit.h>
#endif
#include "bhooks_protos.h"

/*---------------------------------------------------------------------------*/

static int boolHitTest(struct Gadget * g,
                       struct gpHitTest * cgp);

static int boolRender(struct Gadget * g,
                      struct gpRender * cgp);

static int boolGoActive(struct Gadget * g,
                        struct gpInput * cgp);

static int boolGoInactive(struct Gadget * g,
                          struct gpGoInactive * cgp);

static int boolHandleInput(struct Gadget * g,
                           struct gpInput * cgp);

static int boolVisuals(struct RastPort * rp,
                       struct Gadget * g,
                       struct GadgetInfo * gi,
                       int redraw_mode,
                       struct IBox * gbox);

static int toggleBoolGadget(struct Gadget * g,
                            struct GadgetInfo * gi,
                            struct IBox * gbox);

static int toggleBoolHighlight(struct RastPort * rp,
                               struct Gadget * g,
                               struct GadgetInfo * gi,
                               struct IBox * gbox);

static int xormask(struct RastPort * rp,
                   struct IBox * box,
                   UWORD * mask);

/*---------------------------------------------------------------------------*/


void	stubReturn();

#define D(x)	;
#define DEND(x)	;


#if 1
ULONG
boolHook( h, g, cgp )
struct Hook		*h;
struct Gadget		*g;
ULONG			*cgp;	/* only interested in HookID	*/
{
    ULONG retval = 0;

    switch ( (WORD) *cgp )
    {
#if 0
    case GM_SETUPINFO:
	retval = 0;
	break;
#endif
    case GM_HITTEST:
        retval =   boolHitTest( g, cgp );
	break;
    case GM_HELPTEST:
        retval = GMR_HELPHIT;
	break;
    case GM_RENDER:
	retval = boolRender( g, cgp );
	break;
    case GM_GOACTIVE:
        retval = boolGoActive( g, cgp );
	break;
    case GM_GOINACTIVE:
        retval = boolGoInactive( g, cgp );
	break;
    case GM_HANDLEINPUT:
        retval = boolHandleInput( g, cgp );
	break;
    }
    return ( retval );
}
#endif

static
boolHitTest( g, cgp )
struct Gadget		*g;
struct gpHitTest	*cgp;
{
    ULONG	retval = GADGETON;
    struct BoolInfo	*bi = BI( g );
    struct IBox		gadgetbox;

    /* masked gadgets must pass MASKTEST, too	*/
    if ( TESTFLAG( g->Activation, BOOLEXTEND)
	&& TESTFLAG( bi->Flags, BOOLMASK ) )
    {
	gadgetBox( g, cgp->gpht_GInfo, &gadgetbox );

	if ( ! MASKTEST( cgp->gpht_Mouse, gadgetbox.Width,  bi->Mask) )
	{
	    retval = 0;	/* must be 0 or GADGETON	*/
	}
    }

    return ( retval );
}

static
boolRender ( g, cgp )
struct Gadget		*g;
struct gpRender	*cgp;
{
    struct IBox		gadgetbox;
    /* don't redraw image if a simple toggle is called for */
    gadgetBox( g, cgp->gpr_GInfo, &gadgetbox );
    boolVisuals( cgp->gpr_RPort, g, cgp->gpr_GInfo,
	cgp->gpr_Redraw, &gadgetbox);
}

static
boolGoActive( g, cgp )
struct Gadget		*g;
struct gpInput	*cgp;
{
    struct IntuitionBase *IBase = fetchIBase();

    gadgetBox( g, cgp->gpi_GInfo, &IBase->ActiveGBox );

    /* set SELECTED and draw	*/
    toggleBoolGadget( g, cgp->gpi_GInfo, &IBase->ActiveGBox );

    /* NOTE: GADGETON is used to detect transitions as the pointer
     * rolls on and off the gadget, and is used to know
     * that this gadget should be turned off.
     */
    SETFLAG( IBase->Flags, GADGETON ); 	/* am over gadget */

#if 1	/* tighten this up */
    /* tell him GMR_VERIFY because I have happily completed
     * my GADGIMMEDIATE action, rather than "refused to go
     * active.  No GADGETUP message will be sent unless
     * the RELVERIFY Activation flag is set (see ghReturn in
     * sgadget.c).
     */
    return (  TESTFLAG( g->Activation, RELVERIFY )?
	GMR_MEACTIVE: (GMR_NOREUSE | GMR_VERIFY) );
#else
    /* handle non-relverify gadgets quicklike */
    if ( ! TESTFLAG( g->Activation, RELVERIFY ))
    {
	return ( GMR_NOREUSE );
    }
    return ( GMR_MEACTIVE );
#endif
}

static
boolGoInactive( g, cgp )
struct Gadget		*g;
struct gpGoInactive	*cgp;
{
    struct IntuitionBase *IBase = fetchIBase();

    D( printf("goinactive\n") );

    /* ZZZ: this doesn't support abort operations very well */

    if ( TESTFLAG( IBase->Flags, GADGETON ) )
    {
	/* turn him off, since he is not to be left SELECTED */
	if ( ! TESTFLAG( g->Activation, TOGGLESELECT ) )
	{
	    toggleBoolGadget( g, cgp->gpgi_GInfo, &IBase->ActiveGBox );
	}
    }
}

static
boolHandleInput( g, cgp )
struct Gadget		*g;
struct gpInput	*cgp;
{
    struct IntuitionBase *IBase = fetchIBase();
    struct GadgetInfo	*gi = cgp->gpi_GInfo;

    /* transition: gadgeton has changed	*/

    /** ZZZZ call my own hit routine **** with what mouse? **/

    if ((IBase->Flags ^ hitGGrunt( g, gi, &IBase->ActiveGBox, GM_HITTEST )) & GADGETON )
    {
	IBase->Flags ^= GADGETON;	/* toggle gadgeton */
	toggleBoolGadget( g, gi, &IBase->ActiveGBox );
    }

    if ( isSelectup( cgp->gpi_IEvent ) )
    {
	if ( ! TESTFLAG( IBase->Flags, GADGETON ) )
	{
#if 1		/* this should do the trick */
	    DEND( printf("bool done, no verify, saying reuse\n") );
	    return ( GMR_REUSE );
#else
	    /* ZZZ: I don't like this here very much	*/
	    activeEvent( IECLASS_RAWMOUSE, SELECTUP );
	    return ( GMR_NOREUSE );
#endif
	}
	/* else */
	    DEND( printf("bool done, saying noreuse\n") );
	    return ( GMR_NOREUSE | GMR_VERIFY );
    }
    /* else */
	return ( GMR_MEACTIVE );
}

/* toggleBoolGadget requires that this is a no-op
 * for GREDRAW_TOGGLE and GADGHNONE
 */
static
boolVisuals( rp, g, gi, redraw_mode, gbox )
struct RastPort		*rp;
struct Gadget		*g;
struct GadgetInfo	*gi;
struct IBox		*gbox;
{
    if ( redraw_mode == GREDRAW_TOGGLE )
    {
	/* XOR stuff	*/
	D( printf("toggling xor gadget\n" ) );
	toggleBoolHighlight( rp, g, gi, gbox );
    }
    else if ( redraw_mode == GREDRAW_REDRAW )
    {
	D( printf("toggling redraw gadget\n") );
	/* redraw in highlighted state	*/
	commonGadgetRender( rp, g, gi, gbox, FALSE );
	commonGadgetTextAndGhost( rp, g, gi, gbox );
    }
    /* nothing for GREDRAW_UPDATE	*/
}

static
toggleBoolGadget( g, gi, gbox )
struct Gadget		*g;
struct GadgetInfo	*gi;
struct IBox		*gbox;
{
    struct RastPort	*rp;
    int			redraw_mode;

    rp = ObtainGIRPort( gi );

    g->Flags ^= SELECTED;

    /*
     * redraw if gadghimage,
     * toggle if comp/box 
     * nothing, if  gadghnone
     */
    if ( (g->Flags & GADGHIGHBITS) == GADGHIMAGE )
    {
	redraw_mode = GREDRAW_REDRAW;
    }
    else
    {
	redraw_mode = GREDRAW_TOGGLE;	/* no-op for GADGHNONE */
    }

    boolVisuals( rp, g, gi, redraw_mode, gbox );

    FreeGIRPort( rp );
}

/* does XOR highlighting to a boolean gadget.
 * toggleBoolGadget requires that this is a no-op
 * for GREDRAW_TOGGLE and GADGHNONE
 */
static
toggleBoolHighlight( rp, g, gi, gbox )
struct RastPort		*rp;
struct Gadget		*g;
struct GadgetInfo	*gi;
struct IBox		*gbox;
{
    int	(*boxfunc)();
    int xorbox();
    int outbox();
    struct BoolInfo	*bi;

    boxfunc = outbox;

    switch (g->Flags & GADGHIGHBITS)
    {
    case GADGHCOMP:
	bi = (struct BoolInfo *) g->SpecialInfo;

	/* call xorbox */
	if ((g->Activation & BOOLEXTEND) && (bi->Flags & BOOLMASK))
	{
	    /* complement through user-supplied bitmask */
	    xormask(rp, gbox, bi->Mask );
		break;
	}
	/* else * fall through */
	boxfunc = xorbox;

    case GADGHBOX:
	/* call outbox */
	boxModernize( boxfunc, rp, gbox );
    }
}

/* complement area in rastport within box, 
 * subject to mask
 */
static
xormask(rp, box, mask )
struct RastPort *rp;
struct IBox	*box;
UWORD		*mask;		/* has to be in CHIP ram	*/
{
    struct BitMap bmap;
    short depth;

    /* set up bitmap with mask as all planes */
    InitBitMap( &bmap, depth = rp->BitMap->Depth, box->Width, box->Height );
    while (depth--)
    {
	bmap.Planes[depth] = (PLANEPTR)mask;
    }

    /* complement blit */
    /* rp->Mask = 0xFF; already */
    BltBitMapRastPort(&bmap, 0, 0, rp, box->Left, box->Top,
	    box->Width, box->Height, ANBC | ABNC);
}

#if 0
PFU	boolHookTable[] = {
	boolHitTest,	/* GM_HITTEST	*/
	boolRender,	/* GM_RENDER	*/
	boolGoActive,	/* GM_GOACTIVE	*/
	boolGoInactive,	/* GM_GOINACTIVE	*/
	boolHandleInput,	/* GM_HANDLEINPUT	*/
	stubReturn,	/* GM_SETUPINFO	*/
};

#endif
