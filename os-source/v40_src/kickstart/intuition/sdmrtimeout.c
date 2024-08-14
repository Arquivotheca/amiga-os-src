/*** sdmrtimeout.c ************************************************************
 *
 *  state processing
 *
 *  $Id: sdmrtimeout.c,v 38.9 93/01/14 14:25:52 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"

#define D(x)	;

/* state sDMRTimeout
 *
 * global data used:
 *
 * local state data:
 *	timeout counter/original
 */


/* transition startDMRTimeout()
 * called by:
 *	sMenu	- startMenu
 * with state data:
 * description:
 *	proceeds with first part of menu processing:
 *	-shows menu bar
 *	-if a double-menu requester exists, will do timeout
 *	 processing for that.
 */
startDMRTimeout( dmrreturn, okcode )
UWORD	(*dmrreturn)();
{
    struct IntuitionBase	*IBase = fetchIBase();

    D( printf("transition function startDMRTimeout, cstate: %ld\n", 
	IBase->CurrentState ));

    /* always start out by showing him the menu strip,
     * even if there is no DMR.  We promise to have the
     * menu strip ship-shape before we return
     * to menu state: visible if we want menus (OKOK),
     * invisible if we want DMR or cancel menus
     *
     * if the MENUVERIFY has aborted, don't show the strip:
     * we're either bailing out of menus or going straight
     * for the DMR double click (second click).
     */
    if ( okcode == OKOK )
    {
	showMStrip( IBase->ActiveWindow );
	CLEARFLAG( IBase->Flags, RELEASED );
    }
    else
    {
	SETFLAG( IBase->Flags, RELEASED );
    }
    
    if ( ! IBase->ActiveWindow->DMRequest )
    {
	D( printf("dmrtimeout no dmr\n") );
	(*dmrreturn)( OKOK );	/* go ahead with menus (or abort) */
	return;
    }

    /* init timeoutDMR tooFarDMR values */
    IBase->StartSecs = IBase->Seconds;
    IBase->StartMicros = IBase->Micros;
    currMouse( IBase->ActiveWindow, &IBase->FirstPt );


    IBase->DMRReturn = dmrreturn;
    IBase->CurrentState = sDMRTimeout;
}


/*
 * state dispatcher
 *   not reached unless there is double-menu requester around
 * state transitions called:
 *	startRequester
 *	startIdleWindow
 */
dDMRTimeout()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window		*itwindow = ( struct Window *) IT->it_Object1;

    switch ( IT->it_Command )
    {
    /* second click heard, in time, go for DMR	*/
    case itMENUDOWN:	/* Do DMR */
	/* menu strip is not painted now */
	D( printf("dmrtimeout two clicks\n") );
    	(*IBase->DMRReturn)( OKCANCEL );	/* no menus, do dmr */
	return;

    case itMENUUP:	/* Not much */
	SETFLAG( IBase->Flags, RELEASED );
	removeMStrip( IBase->ActiveWindow->WScreen );
	return;		/* ... and keep waiting */

    /* abort conditions: timeout and "move too far" */
    case itTIMER:	/* do default, then maybe abort */
    case itMOUSEMOVE:	/* do default, then maybe abort */

    	doDefault();

	/* DMR timeout
	 * if menu button still down: go into menu state.
	 * if it's released when the timeout occurs, forget
	 * it, abort (back to sIdleWindow)
	 */
	if ( tooFarDMR() )
	{
	    D( printf("dmrtimeout too far\n") );
	    (*IBase->DMRReturn)(
		TESTFLAG( IBase->Flags, RELEASED )? OKABORT: OKOK );
	    return;
	}
	/* else, keep waiting */
	return;

    case itSETREQ:	/* defer */
    case itCLEARREQ:	/* defer */
    case itREMOVEGAD:	/* defer (ZZZ: not just window?) */
    case itSETMENU:	/* defer */
    case itCLEARMENU:	/* defer */
    case itCHANGEWIN:	/* defer */
    case itZOOMWIN:	/* defer */
    case itDEPTHWIN:	/* defer */
    case itMOVESCREEN:	/* defer */
    case itDEPTHSCREEN:	/* defer */
    case itCLOSEWIN:	/* defer (ZZZ: perhaps want to abort rather than defer) */
    case itCLOSESCREEN:	/* defer */
    case itSHOWTITLE:	/* defer */
    case itONOFFMENU:	/* defer */
    case itCOPYSCBUF:	/* defer */
#if 0
    	if ( itwindow->WScreen = IBase->ActiveWindow->WScreen )
	{
	    deferToken();
	}
	else
	{
	    doDefault();
	}
	return;
#else
	deferToken();
#endif

    case itCHANGESCBUF:	/* fail: app should never double-buffer with requesters */
	IT->it_Error = 1;
	return;

    /*** default processing	***/
#if 0
    case itSELECTDOWN:	/* do default */
    case itSELECTUP:	/* do default */
    case itRAWKEY:	/* do default */
    case itDISKCHANGE:	/* do default */

    case itACTIVATEWIN:	/* do default */
    case itACTIVATEGAD:	/* do default */
    case itMETADRAG:	/* do default (should I abort, do metadrag?) */
    case itOPENWIN:	/* do default */
    case itVERIFY:	/* do default */
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

/*
 * returns TRUE if mouse has moved "significantly", or double-click
 * interval has passed.
 * ZZZ: could use a better definition of "significant"
 */
tooFarDMR()
{
    struct IntuitionBase	*IBase  = fetchIBase();

    return (
	 ( iabs( IBase->ActiveWindow->MouseX - IBase->FirstPt.X ) > 10 )
      || ( iabs( IBase->ActiveWindow->MouseY - IBase->FirstPt.Y ) > 5 )
      || ( ! DoubleClick( IBase->StartSecs, IBase->StartMicros,
		IBase->Seconds, IBase->Micros ) )
	    );
}
