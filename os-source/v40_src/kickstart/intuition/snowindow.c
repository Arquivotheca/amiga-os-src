/*** snowindow.c ********************************************************
 *
 *  sNoWindow state processing
 *
 *  $Id: snowindow.c,v 38.8 93/01/14 14:25:33 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"

#define D(x)	;
#define DSTUB(x)	;

/* state sNoWindow
 *
 * global data used:
 *	none
 *
 * local state data:
 *	none
 */

/* transition startNoWindow()
 * called by:
 *	sIdleWindow	- itCLOSEWIN
 *	used as returnNoWindow in sIdleWindow
 * with state data:
 * description:
 */
startNoWindow()
{
    struct IntuitionBase	*IBase = fetchIBase();

    D( printf("transition function startNoWindow, cstate: %ld\n", 
	IBase->CurrentState ));

    IBase->ActiveWindow = NULL;
    IBase->CurrentState = sNoWindow;
}

/*
 * passed to:
 *	startScreenDrag
 */
returnNoWindow()
{
    D( printf("returnNoWindow\n") );
    fetchIBase()->CurrentState = sNoWindow;
}

/*
 * state dispatcher
 * state transitions called:
 *  startIdleWindow()
 *  startScreenDrag()
 */
dNoWindow()
{
    struct IntuitionBase	*IBase = fetchIBase();

    switch ( IT->it_Command )
    {
    case itACTIVATEWIN:	/* start idlewindow */
    case itSELECTDOWN:	/* start idlewindow */
	startIdleWindow();
	return;

    case itMETADRAG:	/* do it */
	startScreenDrag( returnNoWindow );
	return;

	/* default processing	*/
#if 0
    case itMENUDOWN:	/* do default */
    case itMENUUP:	/* do default */
    case itSELECTUP:	/* do default */
    case itMOUSEMOVE:	/* do default */
    case itRAWKEY:	/* do default */
    case itTIMER:	/* do default */
    case itDISKCHANGE:	/* do default */

    case itOPENSCREEN:	/* do default */
    case itACTIVATEGAD:	/* do default */
    case itSETREQ:	/* do default */
    case itCLEARREQ:	/* do default */
    case itOPENWIN:	/* do default */
    case itCLOSEWIN:	/* do default */
    case itVERIFY:	/* do default */
    case itREMOVEGAD:	/* do default */
    case itSETMENU:	/* do default */
    case itCLEARMENU:	/* do default */
    case itCHANGEWIN:	/* do default */
    case itZOOMWIN:	/* do default */
    case itDEPTHWIN:	/* do default */
    case itMOVESCREEN:	/* do default */
    case itDEPTHSCREEN:	/* do default */
    case itCLOSESCREEN:	/* do default */
    case itNEWPREFS:	/* do default */
    case itMODIFYIDCMP:	/* do default */
    case itCHANGESCBUF:	/* do default */
    case itCOPYSCBUF:	/* do default */
    case itSETPOINTER:	/* do default */
    case itGADGETMETHOD:/* do default */
    case itMODIFYPROP:	/* do default */
#endif
    default:		/* itDEFAULT: do default */
	doDefault();

    }
}

