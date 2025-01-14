/*** phooks.c ****************************************************************
 *
 *  Proportional Gadget Hooks
 *
 *  $Id: phooks.c,v 38.4 92/07/07 15:22:14 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1988, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "gadgetclass.h"

#define D(x)	;

#if 1
ULONG
propHook( h, g, cgp )
struct Hook		*h;
struct Gadget		*g;
ULONG			*cgp;	/* only interested in HookID	*/
{
    ULONG retval = 0;

    D( printf("propHook dispatcher h %lx, g %lx, msg %lx\n",
	h, g, cgp ) );

    switch ( (WORD) *cgp )
    {
#if 0
    case GM_SETUPINFO:
	retval = propSetupInfo( g, cgp );
	break;
#endif
    case GM_HITTEST:
        retval = GMR_GADGETHIT;	/* I have no special hit conditions	*/
	break;
    case GM_HELPTEST:
        retval = GMR_HELPHIT;
	break;
    case GM_RENDER:
	retval = propRender( g, cgp );
	break;
    case GM_GOACTIVE:
        retval = propGoActive( g, cgp );
	break;
    case GM_GOINACTIVE:
        retval = propGoInactive( g, cgp );
	break;
    case GM_HANDLEINPUT:
        retval = propHandleInput( g, cgp );
	break;
    }
    return ( retval );
}
#endif

#if 0
/*
 * this type of thing really should be done at on a one-time
 * basis in a hook called by gadgetInfo() 
 * ZZZ: per class data
 */
propSetupInfo( g, cgp )
struct Gadget		*g;
struct CGPSetupInfo	*cgp;
{
	return ( 0 );
}
#endif


propGoActive( g, cgp )
struct Gadget		*g;
struct gpInput	*cgp;
{
    struct IntuitionBase *IBase = fetchIBase();
    struct GadgetInfo	*gi = cgp->gpi_GInfo;
    struct PropInfo	*pi = (struct PropInfo *) g->SpecialInfo;

    struct Point	mouse;
    struct IBox		knobbox;
    int			fudge;

    /***** Initialize ActivePGX	****/
    setupPGX( g, gi, &IBase->ActivePGX );

    /***** KNOB HIT DETECTION ****/

    /* I assume that the select box has enjoyed a hit	*/
    mouse =  *( (struct Point *) &cgp->gpi_Mouse );

    /* test now for knob hit */
    getImageBox( g->GadgetRender, &knobbox );

    /* stretch that baby a little, for the good of User */
    knobbox.Top -= (fudge = pi->TopBorder);
    knobbox.Height += fudge << 1;
    knobbox.Left -= (fudge = pi->LeftBorder);
    knobbox.Width += fudge << 1;

    /* knob is in container coordinates, convert mouse to same */
    transfPoint( &mouse, *((struct Point * ) &pi->LeftBorder) );

    if ( ptInBox( mouse, &knobbox ) )
    {
	pi->Flags |= KNOBHIT;
    }
    else pi->Flags &= ~KNOBHIT;

    /*** SAVE INFORMATION NEEDED FOR DRAGGING ***/

    /* save mouse position...
     * ...get it relative to container...
     * ... then get mouse position relative to image
     */
    /* ZZZ: why window mouse, not gpi_Mouse?  I dunno.  Works. */
    currMouse( gi->gi_Window, &IBase->OldPt );

    /* ZZZ: NEW: cache a copy of the relativity box for
     * use in handleinput.
     */
    gadgetBox( g, gi, &IBase->ActiveGBox );

    /* get knob offset in container coords	*/
    gadgetPoint( g,gi,IBase->OldPt, &IBase->KnobOffset, &IBase->ActiveGBox);
    transfPoint( &IBase->KnobOffset, *((struct Point * ) &pi->LeftBorder) );

    /* use IFrame as a temp box data structure: think of "knob select box" */
    getImageBox( g->GadgetRender, &IBase->IFrame );
    transfPoint( &IBase->KnobOffset,  upperLeft( &IBase->IFrame ) );

    /* if this wasn't a KNOBHIT then we want to advance by one Body fraction
     * in response to a Container hit.
     */
    if ( !TESTFLAG( pi->Flags, KNOBHIT ))
    {
	/* increment pot variables depending on sign of KnobOffset */
	containerBumpPots( g, gi, IBase->KnobOffset, &IBase->IFrame );

	positionNewKnobForPots( g, gi, &IBase->ActivePGX );

	renderNewKnob( g, gi, &IBase->ActivePGX, NULL );
	return ( GMR_NOREUSE | GMR_VERIFY );
    }

    /** else KNOB HIT **/
	SETFLAG( g->Flags, SELECTED );
	safeDisplayProp( g, gi, &IBase->ActivePGX );
	return ( GMR_MEACTIVE );
}

propGoInactive( g, cgp )
struct Gadget		*g;
struct gpGoInactive	*cgp;
{
    D( printf("goinactive\n") );

    /* if he's not selected, I don't need to turn him off */
    if ( TESTFLAG( g->Flags, SELECTED ) )
    {
	CLEARFLAG( g->Flags, SELECTED );
	safeDisplayProp( g, cgp->gpgi_GInfo, &fetchIBase()->ActivePGX );
    }
}

propHandleInput( g, cgp )
struct Gadget		*g;
struct gpInput	*cgp;
{
    struct IntuitionBase *IBase = fetchIBase();
    struct GadgetInfo	*gi = cgp->gpi_GInfo;
    struct Window	*window = gi->gi_Window;

    if ((IBase->OldPt.X != window->MouseX)||(IBase->OldPt.Y!=window->MouseY))
    {
	/* save current mouse vals */
	currMouse( window, &IBase->OldPt );

	/* update pots for new mouse position, get new knob box */
	dragNewKnob( g, gi, IBase->OldPt, &IBase->ActiveGBox );
	    
	/* update visuals for new pots */
	renderNewKnob( g, gi, &IBase->ActivePGX, NULL );
    }

    if ( isSelectup( cgp->gpi_IEvent ) ) return ( GMR_NOREUSE | GMR_VERIFY );

    return ( GMR_MEACTIVE );
}

/*
 * calls display routine with lock on rastport
 */
safeDisplayProp( g, gi, pgx )
struct Gadget		*g;
struct GadgetIngo	*gi;
struct PGX		*pgx;
{
    struct RastPort	*rp;

    D( printf("safeDisplayProp\n") );

    rp = ObtainGIRPort( gi );
    D( printf("rastport: %lx\n", rp) );

    /* display routine	*/
    drawKnob( g, gi, rp, pgx );	/* if selected, will highlight */

    FreeGIRPort( rp );
}

#if 0
PFU	propHookTable[] = {
	propHitTest,	/* GM_HITTEST	*/
	propRender,	/* GM_RENDER	*/
	propGoActive,	/* GM_GOACTIVE	*/
	propGoInactive,	/* GM_GOINACTIVE	*/
	propHandleInput,	/* GM_HANDLEINPUT	*/
	propSetupInfo,	/* GM_SETUPINFO	*/
};

#endif
