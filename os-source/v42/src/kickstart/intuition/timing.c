/*** timing.c ****************************************************************
 *
 *  contains the timing routines
 *
 *  $Id: timing.c,v 40.0 94/02/15 17:53:36 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1985, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

/*****************************************************************************/

void doTiming (void)
{
    struct Screen *workscreen;
    struct Window *workwindow;
    struct IntuitionBase *IBase = fetchIBase ();

    assertLock ("timing", IBASELOCK);

    /* LOCKIBASE(); */
    /* using state machine instead of IBase lock to protect
     * screen lists and so on, but without it, IDisplayBeep
     * is vulnerable.  We correct with Forbid/Permit in
     * IDisplayBeep()
     */

    /* In 3.00 and prior, Intuition only updated its IBase->Seconds/Micros
     * fields for IECLASS_TIMER events, since those had "trusted" timestamps.
     * You may be asking why we still do we set IBase->Seconds/Micros
     * here when ism.c/dispatchToken() now sets IBase->Seconds/Micros
     * for _all_ events.  Well, the answer is that dispatchToken() does
     * a sanity check on the TimeStamp, and filters out bad time values
     * (eg. reverse time or far-future time).  This is required by
     * our very own commodities.library, which can insert InputEvents
     * with zero timestamps.
     *
     * The important bit is that we still trust IECLASS_TIMER events,
     * and therefore always obey their TimeStamp.  If we didn't, and
     * the user changed the system time (or some application choked off
     * Intuition's input handler for a time), the new TimeStamp would be
     * eternally rejected by Intuition as unreasonable.
     */

    IBase->Seconds = IE->ie_TimeStamp.tv_secs;
    IBase->Micros = IE->ie_TimeStamp.tv_micro;

    /* check all time-out cases, and effect changes as needed */
    workscreen = IBase->FirstScreen;
    while (workscreen)
    {
	if (workscreen->Flags & BEEPING)
	{
	    if (TESTFLAG (XSC (workscreen)->PrivateFlags, PSF_DELAYBEEP))
	    {
		XSC (workscreen)->PrivateFlags &= ~PSF_DELAYBEEP;
	    }
	    else
	    {
		/* ZZZ: do a test to see if it has changed,
		 * then maybe don't stuff it back
		 */
		SetRGB32 (&workscreen->ViewPort, 0,
			  XSC (workscreen)->SaveRed,
			  XSC (workscreen)->SaveGreen,
			  XSC (workscreen)->SaveBlue);

		/* jimm: move to end: keep IDisplayBeep() out of here
		 * until I'm done restoring the color
		 */
		workscreen->Flags &= ~BEEPING;
	    }
	}
	workwindow = workscreen->FirstWindow;
	while (workwindow)
	{
	    if (TESTFLAG (workwindow->MoreFlags, WMF_DEFERREDPOINTER))
	    {
		if (!(--XWINDOW (workwindow)->DeferredCounter))
		{
		    /* Time's up.  Set the new pointer.  Note that
		     * ISetWindowPointer() takes care of clearing
		     * the WMF_DEFERREDPOINTER flag.
		     */
#if 1
		    ISetWindowPointer (workwindow, XWINDOW (workwindow)->DeferredPointer);
#else
		    /* Previous dipwad did this */
		    ISetWindowPointer (workwindow, XWINDOW (workwindow)->DeferredPointer, TRUE);
#endif
		}
	    }
	    workwindow = workwindow->NextWindow;
	}

	workscreen = workscreen->NextScreen;
    }

    /* UNLOCKIBASE(); */
}
