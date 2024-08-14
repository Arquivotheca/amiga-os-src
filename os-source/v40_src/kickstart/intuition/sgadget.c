/*** sgadget.c *****************************************************************
 *
 *  sgadget.c -- state processing
 *
 *  $Id: sgadget.c,v 38.21 93/03/29 10:32:16 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuition.h"
#include "intuall.h"
#include "gadgetclass.h"

#define D(x)	;
#define DAG(x)	;	/* activegadget	*/
#define DZ(x)	;
#define D2(x)	;

/* state sGadget
 *
 * global data used:
 *	ActiveGadget
 *
 * local state data:
 *	GadgetReturn
 */


/* transition startGadget()
 * called by:
 *	sIdleWindow	- itSELECTDOWN
 *	sIdleWindow	- itACTIVATEGAD
 *	sRequester	- itSELECTDOWN
 *	sRequester	- itACTIVATEGAD
 *  NOTE/WARNING: proper SIZEVERIFY processing of
 *  the Zoom gadget requires special code in each of
 *  these states.
 * with state data:
 *	ActiveGadget already set
 * description:
 *	parameter is return function, which will be passed the
 *	requester ENDGADGET parameter.
 *	1 - no longer active
 *	2 - ENDGADGET for requesters
 */
startGadget( gadgetreturn )
int	(*gadgetreturn)();
{
    struct IntuitionBase	*IBase = fetchIBase();
    USHORT			ghreturn;
    LONG			gadgetupcode;
    int				endgadget;
    struct Gadget		*g;
    struct InputEvent		*initial_event;
    struct Point		gadgetmouse;
    int				returnSysGadget();
    struct Hook			*gadgetHook();

    struct IBox			gadgetbox;
    /* I feel funny about not stashing this somewhere, but
     * I only need it for precalcing gadget mouse, which
     * could be optimized, and I already am letting
     * prop/bool/string cache their leaf-node boxes
     */

    D( printf("transition startGadget, cstate: %ld command %lx, agadget %lx\n", 
	IBase->CurrentState, IT->it_Command,  g ));

    /* itSELECTDOWN: 
     *	hitGadgets() has already set up ActiveGadget,
     *	GadgetInfo, etc.
     *
     * itACTIVATE:
     *  same stuff set up by IActivateGadget()
     */

    if ( IT->it_Command == itACTIVATEGAD )
    {
	/* initialize IBase ActiveGadget fields	*/
	/* Peter 23-Aug-90: ZZZ if a custom gadget declines to
	 * go active from ActivateGadget(), the caller of AG()
	 * still will get a "success" return code
	 */
	if ( IActivateGadget( IT ) != 0 )	/* error */
	{
	    D( printf("sG: ActivateGadget returned error\n") );
	    (*gadgetreturn)( 1 );	/* don't say ENDGADGET */
	    return;
	}
	D( printf("IActivateGadget returned, with AG %lx\n",
	    IBase->ActiveGadget ) );
    }
    else /* ( IT->it_Command == itSELECTDOWN ) */
    {
	/* Peter 21-Jan-91: We track the token-synchronous state
	 * of the select button, to know if we can abort this gadget
	 * or not.  (We defer tokens if the select button is held,
	 * else we abort the gadget).
	 */
	IBase->Flags |= GAD_SELECTDOWN;
    }

    /* ActiveGadget (and it's ginfo) is all setup by either
     * hitGadgets (hitGGrunt) or IActivateGadget.
     * We still need to cache it's hook right here.
     */
    g = IBase->ActiveGadget;
    IBase->ActiveHook = gadgetHook( g );

    /* application hears something: either gadget down or
     * mouse move (not both)
     */
    if ( TESTFLAG( g->Activation, GADGIMMEDIATE ) )
    {
	activeEvent( IECLASS_GADGETDOWN, 0L );
    }
    else if ( TESTFLAG( g->Activation, FOLLOWMOUSE ) )
    {
	sendWindowMouse();
    }

    /* the difference to the gadget is that the initial input
     * is NULL for the ActivateGadget() function
     */
    initial_event = ( IT->it_Command == itACTIVATEGAD )? NULL: IE;

    /* this operation respects relativity */
    gadgetBox( g, &IBase->GadgetEnv.ge_GInfo, &gadgetbox );
    gadgetMouse( g, &IBase->GadgetEnv.ge_GInfo,
		&gadgetmouse, &gadgetbox );

    ghreturn = callGadgetHook( IBase->ActiveHook, g,
			 GM_GOACTIVE,
			    &IBase->GadgetEnv.ge_GInfo,
			    initial_event,
			    &gadgetupcode,
			    gadgetmouse, IT->it_Tablet );

    /* get comfortable in this state */
    IBase->GadgetReturn = gadgetreturn;
    IBase->CurrentState = sGadget;

    /* dispatch to system gadget heavyweights.
     * these return through returnSysGadget(), and
     * skip gadget state almost completely.  They
     * _do_ use GadgetReturn to get back to the
     * previous state.
     */
    switch ( g->GadgetType & GTYP_SYSTYPEMASK )
    {
    case SIZING:
	startWSizeDrag( returnSysGadget, WSD_SIZE );
	return;
    case WDRAGGING:
	startWSizeDrag( returnSysGadget, WSD_DRAG );
	return;
    case SDRAGGING:
	startScreenDrag( returnSysGadget );
	return;
    D( default: printf("ok, go ahead with sgadget state\n"));
    /* default: go on ahead with sGadget state */
    }

    /* handle return from first processing */
    if ((endgadget = ghResult( ghreturn, gadgetupcode ) ) != GMR_MEACTIVE)
    {
	/* state change (return)	*/
	D( printf("startGadget gadget won't go active\n") );
	(*gadgetreturn)( endgadget );
	return;
    }
    /* else */
    DAG( printf("setting ACTIVEGADGET for %lx\n", g ) );
    SETFLAG( g->Activation, ACTIVEGADGET );

    /* state change complete, remain in gadget state */
    return;
}

/*
 * process value returned from GM_GOACTIVE, and GM_HANDLEINPUT
 *
 * returns:
 *	0 - gadget is still active
 *	1 - no longer active
 *	2 - ENDGADGET for requesters
 */
ghResult( ghreturn, upcode )
USHORT			ghreturn;
LONG			upcode;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Gadget		*g;
    int retval = 1;	/* done, nothing special */
    UWORD			sysgadtype;
    struct Window		*AWindow = IBase->ActiveWindow;
    struct Screen		*screenFamily();

    if ( ghreturn == GMR_MEACTIVE ) return ( 0 );

    g = IBase->ActiveGadget;

    /*** from here on, gadget is done ***/

    if ( TESTFLAG( ghreturn, (GMR_REUSE | GMR_NEXTACTIVE | GMR_PREVACTIVE) ) )
    {
	D2( printf("ghResult saying: REUSE THAT TOKEN\n") );
	reuseToken();
	if ( TESTFLAG( ghreturn, (GMR_NEXTACTIVE | GMR_PREVACTIVE) ))
	{
	    D2( printf("ghResult saying: find a new gadget to activate\n") );
	    newActiveGadget(g, ghreturn);
	}
    }
    D2( else printf("swallow ghResult token\n") );

    if ( TESTFLAG( ghreturn, GMR_VERIFY ) )
    {
	D2( printf("ghReturn: happy gadget camper is done\n") );

	if ( sysgadtype = (g->GadgetType & GTYP_SYSTYPEMASK ) )
	{
	    /* system gadget processing */
	    D( printf("inputResult: system gadgets\n") );

	    if ( sysgadtype == CLOSE )
	    {
		activeEvent( IECLASS_CLOSEWINDOW, NULL );
	    }
	    else if ( sysgadtype == SUPFRONT )
	    {
		/* Starting with V39, we go through the ScreenDepth()
		 * LVO for screen-depth-arrangement.  Formerly, we just
		 * morphed the token.
		 */

		/* By default, a screen comes forward */
		ULONG flags = SDEPTH_TOFRONT;

		/* But if it's already frontmost, or if its in the
		 * same family as the frontmost screen, then send
		 * it to the back.  Also, if you had a SHIFT
		 * key pressed when you release the gadget,
		 * then we do "to back" absolutely
		 * ZZZ: note that the case where IE is not
		 * used is for the initial ActivateGadget(),
		 * which doesn't happen for depth verify release.
		 */
		if ( ( screenFamily( IBase->HitScreen ) ==  
		    screenFamily( IBase->FirstScreen ) ) ||
		    ( IE && TESTFLAG( IE->ie_Qualifier, SHIFTY ) ) )
		{
		    flags = SDEPTH_TOBACK;
		}
		ScreenDepth( IBase->HitScreen, flags, NULL );
	    }
	    else /*** Change token into command, and reuse ***/
	    {
		D2( printf("reuse token for system gadget\n") );
		reuseToken();

		switch ( sysgadtype )
		{
		/* UPFRONT is the new front/back combined */
		case WUPFRONT:
		    IT->it_Command = itDEPTHWIN;
		    IT->it_Object1 = (CPTR) AWindow;

		    /* ZZZ: probably need to do more for backdrop
		     * windows: check to see if it is the
		     * frontmost backdrop
		     */

		    IT->it_SubCommand = 
			windowObscured( AWindow )?
			WDEPTH_TOFRONT : WDEPTH_TOBACK;

		    /* special processing: if you had a SHIFT
		     * key pressed when you release the gadget,
		     * then we do "to back" absolutely
		     * ZZZ: note that the case where IE is not
		     * used is for the initial ActivateGadget(),
		     * which doesn't happen for depth verify release.
		     */
		    if ( IE && TESTFLAG( IE->ie_Qualifier, SHIFTY ))
		    {
			IT->it_SubCommand = WDEPTH_TOBACK;
		    }

		    DZ( printf("front/back gadget doing %ld\n", 
			IT->it_SubCommand ) );

		    break;

		/* zoom gadget: jimm: 6/5/90:
		 * this gets a little weird: we need a SIZEVERIFY
		 * sequence for this.  It's explicitly done by
		 * the states processing this morphed token.
		 * It's simple, but it has to be in EACH state
		 * which might see this token.
		 * Today, that's definitely just sIdleWindow and
		 * sRequester.
		 *
		 * The price of failure is not high: a normal IZoomWindow
		 * will be done by doDefault(), and the only problem
		 * would be a skipped SIZEVERIFY.
		 */
		case WZOOM:
		    IT->it_Command = itZOOMWIN;
		    IT->it_SubCommand = TRUE;
		    IT->it_Object1 = (CPTR) AWindow;
		    break;
		}
	    }
	}
	else	/* application gadget */
	{
	    /* should I send him GADGETUP? */
	    if ( TESTFLAG( g->Activation, RELVERIFY ) )
	    {
		D2( printf("sending gadgetup\n") );
		activeEvent( IECLASS_GADGETUP, (ULONG) upcode );
	    }

	    /* test for requester ENDGADGET */
	    if ( TESTFLAG( g->Activation, ENDGADGET )
		&& TESTFLAG( g->GadgetType, REQGADGET ) )
	    {
		retval = 2;	/* ENDGADGET */
	    }
	}
    }
    else
    {
	/* ZZZ: send him SOMETHING? */
	D( printf( "ghReturn: non VERIFY gadget done\n") );
    }

    /* turn him off (not an abort) */
    turnOffGadget( FALSE );

    return ( retval );
}


/* newActiveGadget() finds the next or previous gadget in sequence
 * that has the gadget flag GFLG_TABCYCLE set, and morphs the
 * current token into an itACTIVATEGAD token.
 */

newActiveGadget( current, ghreturn )
struct Gadget	*current;
USHORT		ghreturn;

{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window		*win = IBase->ActiveWindow;
    struct Requester		*req = win->FirstRequest;
    struct InputToken		*it = IT;
    struct Gadget		*g;
    struct Gadget		*last;

    /* If either shift key is down, then we'll search backwards
     * through the list (actually, we'll look for the last one
     * forwards, which is the same).  Otherwise we look for the
     * next one forwards.
     */
    ghreturn &= GMR_PREVACTIVE;

    DAG( printf("newActiveGadget for gadget %lx\n", current ) );
    g = last = current;

    /* Here is one of those while loops with the test in the middle */
    for (;;)
    {
	/* Try next gadget, looping around to the beginning */
	if (! ( g = g->NextGadget ) )
	{
	    g = (req ? req->ReqGadget : win->FirstGadget);
	    DAG( printf("nAG: Starting at the beginning with " ) );
	}
	DAG( printf("next gadget %lx\n", g ) );

	/* Stop when we find ourselves again */
	if (g == current)
	{
	    break;
	}

	DAG( printf("nAG: candidate for activation: %lx\n", g ) );
	/* Non-disabled TABCYCLE gadgets are what we want: */
	if ( ( g->Flags & ( GFLG_DISABLED | GFLG_TABCYCLE ) ) == GFLG_TABCYCLE )
	{
	    DAG( printf("nAG: This one wants to be activated!\n" ) );
	    last = g;
	    /* If we're going forward, we're done.  If we're going
	     * backward, we want to keep going, and remember the
	     * last eligible gadget we see.
	     */
	    if (!ghreturn)
	    {
		break;
	    }
	}
    }


    /* When going backwards, the one to activate is the last candidate
     * we saw.
     */
    if ( ghreturn )
    {
	g = last;
    }

    /* ghResult() called reuseToken() earlier */
    it->it_Command = itACTIVATEGAD;
    it->it_Object1 = (CPTR) win;
    it->it_Object2 = (CPTR) g;
    it->it_SubCommand = (ULONG) req;

    DAG( printf( "Morphing to itACTIVATEGAD for win %lx, req %lx, gad %lx\n",
	win, req, g ) );
}


/*
 * return function from size/drag gadget state changes
 * turn off active gadget
 */
returnSysGadget( )
{
    turnOffGadget( FALSE );

    /* end gadget state */
    (*fetchIBase()->GadgetReturn)( 0 );  /* ZZZ: so, it can't be an ENDGADGET */
}

turnOffGadget( abort )
LONG abort;		/* is this an abort of the gadget action? */
{
    struct IntuitionBase	*IBase = fetchIBase();

    if ( IBase->ActiveGadget )
    {
	callGadgetHook( IBase->ActiveHook, IBase->ActiveGadget,
		GM_GOINACTIVE, &IBase->GadgetEnv.ge_GInfo,
		abort );
	DAG( printf("clearing ACTIVEGADGET for %lx\n", IBase->ActiveGadget ) );
	CLEARFLAG( IBase->ActiveGadget->Activation, ACTIVEGADGET );
	IBase->ActiveGadget = NULL;		/* just to be sure */

	/* Peter 5-Feb-91: We track the token-synchronous state
	 * of the select button, to know if we can abort this gadget
	 * or not.  To be sure, we clear this bit whenever the gadget
	 * is turned off.  (We defer tokens if the select button is held,
	 * else we abort the gadget).
	 */
	IBase->Flags &= ~GAD_SELECTDOWN;
    }
}

/*
 * passed to:
 *	startScreenDrag - itMETADRAG
 * back to sGadget state, no action
 */
returnGadget()
{
    fetchIBase()->CurrentState = sGadget;
}


/*
 * state dispatcher
 * state transitions called:
 * We're here to service the ActiveGadget
 */
dGadget()
{
    struct IntuitionBase	*IBase = fetchIBase();
    CPTR			object1 = IT->it_Object1;
    USHORT			ghreturn;
    LONG			gadgetupcode;
    int				endgadget;
    struct Point		gadgetmouse;
    struct IBox			gadgetbox;
    struct Window		*AWindow = IBase->ActiveWindow;

    switch ( IT->it_Command )
    {
    /* pass input along to the gadget */
    case itTIMER:	/* test GadgetHelp, do default, let gadget hear it */
	if ( !TESTFLAG( IBase->Flags, GAD_SELECTDOWN ) )
	{
	    gadgetHelpTimer();
	}
	/*** fall through ***/
    case itMOUSEMOVE:	/* do default, let gadget hear it */
	D( printf("sgadget: mouse/timer\n"));
    	doDefault();

	if ( IT->it_Command == itMOUSEMOVE &&
	    (TESTFLAG( IBase->ActiveGadget->Activation, FOLLOWMOUSE ) 
	     || ( AWindow && TESTFLAG( AWindow->Flags, REPORTMOUSE ) ) ) )
	{
	    sendWindowMouse();
	}
	/*** fall through ***/
    case itMENUDOWN:	/* let gadget hear it */
    case itMENUUP:	/* let gadget hear it */
    case itSELECTDOWN:	/* let gadget hear it */
    case itSELECTUP:	/* let gadget hear it */
    case itRAWKEY:	/* let gadget hear it */
	/* Peter 21-Jan-91: We track the token-synchronous state
	 * of the select button, to know if we can abort this gadget
	 * or not.  (We defer tokens if the select button is held,
	 * else we abort the gadget).
	 */
	if ( IT->it_Command == itSELECTUP )
	{
	    CLEARFLAG( IBase->Flags, GAD_SELECTDOWN );
	}
	else if ( IT->it_Command == itSELECTDOWN )
	{
	    SETFLAG( IBase->Flags, GAD_SELECTDOWN );
	}

	gadgetBox( IBase->ActiveGadget,
		&IBase->GadgetEnv.ge_GInfo, &gadgetbox );
	gadgetMouse( IBase->ActiveGadget,
		&IBase->GadgetEnv.ge_GInfo,
		&gadgetmouse, &gadgetbox );

	if ( IT->it_Command == itSELECTDOWN )
	{
	    /* Peter 25-Feb-91: Check to see if user clicked in
	     * a different screen.  If so, abort this sucker.
	     */
	    struct Layer *layer;

	    hitUpfront(&layer);
	    if ( layer != IBase->GadgetEnv.ge_GInfo.gi_Layer )
	    {
		/* Abort the gadget, and let someone else hear the click */
		turnOffGadget( TRUE );
		reuseToken();
		(*IBase->GadgetReturn)( 0 );
		return;
	    }
	}

    	ghreturn = callGadgetHook( IBase->ActiveHook, 
			IBase->ActiveGadget, GM_HANDLEINPUT,
			&IBase->GadgetEnv.ge_GInfo, IE, &gadgetupcode,
			gadgetmouse, IT->it_Tablet );

	if ( endgadget = ghResult( ghreturn, gadgetupcode ) )
	{
	    /* state change (return)	*/
	    D2( printf("gadget state done\n") );
	    (*IBase->GadgetReturn)( endgadget );
	    D2( printf("dGadget returning\n") );
	}
	return;

    case itACTIVATEWIN:	/* if (!button-down) or already active, do it, else fix window borders & defer */
	/* jimm: 3/14/90: don't block */
	/* Peter 26-Mar-93: If this is the initial window activation,
	 * then deferIf() actually lets the token return, and inserts
	 * an async itACTIVATEWIN token to complete the job.  This
	 * is because OpenWindow() mustn't wait.
	 */
	if ( ! deferIf( ((struct Window *)object1) != AWindow ) )
	{
	    /* We get here if the window activation was for the already
	     * active window.  deferIf() will have done doDefault()
	     * on this token, We want to pretend to succeed, since the window
	     * is already active.
	     */

	    /* Mirror the action from sIdleWindow:
	     * First, the PPage kludge for transparent menus,
	     * then send an IDCMP_ACTIVEWINDOW event.
	     */
	    setScreen( IBase->ActiveScreen );
	    activeEvent(IECLASS_ACTIVEWINDOW, NULL );
	}
	return;

    case itREMOVEGAD:	/* if (!button-down) or activegad not among them, then do it */
    	deferIf( inGList( IBase->ActiveGadget,
		((struct Gadget *)object1), IT->it_SubCommand) );
	return;

    case itCLOSESCREEN:	/* if (!button-down) or not this screen, then do it (==screen: bad!) */
	deferIf( ((struct Screen *)object1) ==
		IBase->GadgetEnv.ge_GInfo.gi_Screen );
	return;

    /* some other state must close the active window */
    case itCHANGEWIN:	/* if (!button-down) or not this window, then do it */
    case itZOOMWIN:	/* if (!button-down) or not this window, then do it */
    case itCLOSEWIN:	/* if (!button-down) or not this window, then do it */
    case itSETREQ:	/* if (!button-down) or not this window, then do it */
    case itCLEARREQ:	/* if (!button-down) or not this window, then do it */
	/* jimm: 3/14/90: don't block */
	deferIf( ((struct Window *)object1) == AWindow );
	return;

    case itMETADRAG:	/* do it */
    	startScreenDrag( returnGadget );
	return;

    /* want to return true if gadget is already active */
    case itACTIVATEGAD:	/* Succeeds only if already active */
	/* success is it_Error == 0 */
	IT->it_Error =
	    ((struct Gadget *)IT->it_Object2==IBase->ActiveGadget)? 0: 1;
	return;


    case itCHANGESCBUF:	/* if this screen, then error, else do it */
	if (((struct Screen *)object1) == IBase->GadgetEnv.ge_GInfo.gi_Screen )
	{
	    IT->it_Error = 1;
	}
	else
	{
	    doDefault();
	}
	return;

    case itCOPYSCBUF:	/* defer */
	/* We defer until the screen imagery has stabilized,
	 * i.e. gadget activity is stopped
	 */
	deferToken();
	return;

    /*** default processing	***/
#if 0
    case itDISKCHANGE:	/* do default */

    case itOPENWIN:	/* do default */
    case itVERIFY:	/* do default */
    case itSETMENU:	/* do default */
    case itCLEARMENU:	/* do default */

    case itDEPTHWIN:	/* do default (*** OOPS: I don't think so ***) */

    case itMOVESCREEN:	/* do default */
    case itDEPTHSCREEN:	/* do default */

    case itOPENSCREEN:	/* do default */

    case itNEWPREFS:	/* do default */
    case itMODIFYIDCMP:	/* do default */
    case itSETPOINTER:	/* do default */
    case itGADGETMETHOD:/* do default */
    case itMODIFYPROP:	/* do default */
#endif
    default:		/* itDEFAULT: do default */
	doDefault();
    }
}

/* If the supplied condition is TRUE, then defer the token if
 * the user has the mouse select-button down, or abort the
 * gadget if the mouse button is not down.
 *
 * If the condition is FALSE, the token is allowed to proceed
 * without interfering.
 *
 * Peter 21-Jan-91: Gadgets that require active user-work to
 * remain active (bools, props, and most customs) won't be
 * aborted.  We detect this by looking for a down mouse-button.
 * For them, the token will be deferred.  String gadgets
 * and related custom gadgets don't have the mouse down to stay
 * active.  Them we turn off.
 *
 * We return value of expression to support further processing.
 */
deferIf( expression )
{
    struct IntuitionBase	*IBase = fetchIBase();

    if ( expression  )
    {
	if ( IBase->Flags & GAD_SELECTDOWN )
	{
	    /* Since the user is holding the select button, we'll
	     * defer the token.
	     * Peter 26-Mar-93: For Initial window activation,
	     * we patch up the window borders and let the token
	     * return, so as to not block OpenWindow().  We
	     * emit an asynchronous ActivateWindow() request
	     * to handle the actual activation.
	     */
	    if ( ( IT->it_Command == itACTIVATEWIN ) &&
		( IT->it_Object2 == AWIN_INITIAL ) )
	    {
		    fixWindowBorders();
		    ActivateWindow( IT->it_Object1 );
	    }
	    else
	    {
		deferToken();
	    }
	}
	else
	{
	    /* abort the active gadget */
	    turnOffGadget( TRUE );

	    /* we need to let some other state process this token,
	     * since these things happening to the active window
	     * require state changes and stuff.
	     */
	    reuseToken();
	    (*IBase->GadgetReturn)( 0 );
	}
    }
    else
    {	
	/* some irrelevant window, screen, gadget  or  whatever */
	doDefault();
    }
    return (expression);
}
