/*** srequester.c *********************************************************
 *
 * sRequester state processing
 *
 *  $Id: srequester.c,v 38.11 93/01/14 14:26:13 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"

#define D(x)		;
#define DZOOM(x)	;

/* state sRequester
 *
 * global data used:
 *	ActiveWindow
 *
 * local state data:
 *	ActiveRequester
 */


/* transition startRequester()
 * called by:
 *	sIdleWindow	- itSELECTDOWN
 *
 *     find window with requester in it
 *	sIdleWindow	- itACTIVATE
 *	sIdleWindow	- itCLEARREQ
 *	sIdleWindow	- DMR abort (timer, cleardmr, mousemove)
 *	sIdleWindow	- itCLOSEWIN
 *	sIdleWindow	- itSETREQ
 *	sRequester	- itSETREQ (new req)
 * DMR:
 *	sDMRTimeout	- itMENUDOWN (in time)
 *
 * with state data:
 *	ActiveWindow->FirstRequest != NULL
 * WARNING: and of course ActiveWindow != NULL
 * description:
 */
startRequester()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Layer		*hitlayer;
    int				returnRequesterGadget();

    D( printf("transition function startRequester, cstate: %ld\n", 
	IBase->CurrentState ));

    /* set ActiveRequester, if needed for anything */
    IBase->ActiveRequester = IBase->ActiveWindow->FirstRequest;

    /* process my own gadget hits.  hitGadgets() will only
     * allow hits to gadgets in the requester itself, or system
     * gadgets except CLOSE
     */
    if ( IT->it_Command == itSELECTDOWN )
    {
	hitUpfront( &hitlayer );	/* calling a second time */

	if ( hitGadgets( hitlayer ) )
	{
	    startGadget( returnRequesterGadget );
	    return;
	}
	else if ( TESTFLAG( IBase->ActiveWindow->FirstRequest->Flags, NOISYREQ ) )
	{
	    /* Peter 8-Aug-90: send SELECTDOWN to NOISYREQ people */
	    activeEvent( IE->ie_Class, IE->ie_Code );
	}
    }
    IBase->CurrentState = sRequester;
}

/*
 * passed to:
 *	startGadget	- itENDGADGET?
 *	handle ENDGADGET deal: see sgadget.c ghResult()
 *	2 means ENDGADGET
 * rely on:
 *	ActiveGadget
 *	relverify parameter passed by sGadget if the
 *	 ENDGADGET should be processed.
 */
returnRequesterGadget( endrequest )
{
    struct IntuitionBase	*IBase = fetchIBase();

    /* ENDGADGET */
    if ( endrequest == 2 )
    {
	IEndRequest( IBase->ActiveWindow, IBase->ActiveWindow->FirstRequest );
	startIdleWindow();
	return;
    }

    IBase->CurrentState = sRequester;
}

/*
 * passed to:
 *	startScreenDrag	- itMETADRAG
 */
returnRequesterSDrag()
{
    /* assuming that ActiveRequester still valid */
    fetchIBase()->CurrentState = sRequester;
}

/*
 * state dispatcher
 * state transitions called:
 *	startIdleWindow() - itSELECTDOWN
 *	startIdleWindow() - itACTIVATEWIN
 *	startIdleWindow() - itCLOSEWIN
 *	startIdleWindow() - itCLEARREQ
 *
 *	
 */
dRequester()
{
    UWORD			returnRequesterZoom();
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window		*itwindow = ( struct Window *) IT->it_Object1;

    /* ZZZ: maybe group all noisyreq conditionals
     * in another switch in the default: case for this one
     */
    switch ( IT->it_Command )
    {
    case itSELECTDOWN:	/* start idlewindow */
    	startIdleWindow();	/* not passed along in V33 */
	return;

    /* NOISYREQ input passed along */
    case itMOUSEMOVE:	/* pass input if WFLG_REPORTMOUSE */
	doDefault();

	/* NOISYREQs may send mouse message to active window */
	/* Peter 15-Nov-90:  non-NOISYREQs got MOUSEMOVEs in 1.3
	 * Amazing.
	 */
	if ( TESTFLAG( IBase->ActiveWindow->Flags, REPORTMOUSE ) )
	{
	    sendWindowMouse();
	}
	return;


    case itMENUDOWN:	/* pass input if NOISYREQ */
    case itMENUUP:	/* pass input if NOISYREQ */
    case itSELECTUP:	/* pass input if NOISYREQ */
    case itRAWKEY:	/* pass input if NOISYREQ */
	if ( TESTFLAG( IBase->ActiveRequester->Flags, NOISYREQ ) )
	{
	    activeEvent( IE->ie_Class, IE->ie_Code );
	}
    	return;

    case itACTIVATEWIN:	/* start idlewindow */
    	startIdleWindow();
	return;

    case itACTIVATEGAD:	/* allow only if gadget's in this requester */
	/* if it's this requester, sure, go ahead. */
	if ( IBase->ActiveRequester ==
		(struct Requester *) IT->it_SubCommand )
	{
	    startGadget( returnRequesterGadget );
	}
	return;

    case itSETREQ:	/* do it, maybe startRequester */
	doDefault();

	if ( itwindow == IBase->ActiveWindow ) {
	    startRequester();
	}
	return;

    case itCLOSEWIN:	/* delink window and make parent active */
    case itCLEARREQ:	/* return to requester state if not last requester */
	doDefault();
	if ( itwindow == IBase->ActiveWindow ) {
	    startIdleWindow();
	}
    	return;

    case itMETADRAG:	/* do it */
	startScreenDrag( returnRequesterSDrag );
	return;

    case itZOOMWIN:	/* do default, maybe with SIZEVERIFY */
	/* it_SubCommand specifies a user-initiated zoom, meaning
	 * that we need a SIZEVERIFY session.
	 */
	if ( IT->it_SubCommand )
	{
	    DZOOM( printf("sidlewindow starting sizev for itZOOM\n"));
	    startVerify( returnRequesterZoom, itwindow, SIZEVERIFY, 0 );
	}
	else
	{
	    DZOOM( printf("sidlewindow got non-verify zoom\n"));
	    doDefault();
	}
	return;

    case itCHANGESCBUF:	/* fail: app should never double-buffer with requesters */
	IT->it_Error = 1;
	return;

    case itTIMER:	/* check for gadget help, then do default */
	gadgetHelpTimer();
	/* FALL THROUGH!!! */

    /*** default processing	***/
#if 0
    case itDISKCHANGE:	/* do default */

    case itOPENWIN:	/* do default */
    case itVERIFY:	/* do default */
    case itREMOVEGAD:	/* do default */
    case itSETMENU:	/* do default */
    case itCLEARMENU:	/* do default */
    case itCHANGEWIN:	/* do default */
    case itDEPTHWIN:	/* do default */
    case itMOVESCREEN:	/* do default */
    case itDEPTHSCREEN:	/* do default */
    case itOPENSCREEN:	/* do default */
    case itCLOSESCREEN:	/* do default */
    case itNEWPREFS:	/* do default */
    case itMODIFYIDCMP:	/* do default */
    case itCOPYSCBUF:	/* do default (would like to defer, but fear deadlock) */
    case itSETPOINTER:	/* do default */
    case itGADGETMETHOD:/* do default */
    case itMODIFYPROP:	/* do default */
#endif
    default:		/* itDEFAULT: do default */
	doDefault();
    }
}


/*
 * almost the same as returnIdleZoom()
 *
 * Making the (safe) assumption that the
 * window which had a zoom gadget selection
 * is still the active window, sVerify doesn't
 * allow change of activation, and as sGadget morphs
 * the zoom gadget selectup token, it gets processed immediately
 * afterward
 */
UWORD
returnRequesterZoom( okcode )
{
    struct IntuitionBase	*IBase = fetchIBase();

    if ( okcode != OKOK )
    {
	/*
	 * if aborted (or timeout), send "verify clear"
	 * ZZZ: would need to change for "MOVEVERIFY"
	 */
	activeEvent( IECLASS_SIZEWINDOW, 0 );
    }
    else
    {
	/* Peter 4-Dec-90:  TRUE parameter indicates that this zoom
	 * resulted from user-interaction.  In particular, a NEWSIZE
	 * is required if the window has SIZEVERIFY set.
	 */
	IZoomWindow( IBase->ActiveWindow, TRUE );
    }

    IBase->CurrentState = sRequester;
}
