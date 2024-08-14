/*** sidlewindow.c ********************************************************
 *
 *  sidlewindow state processing
 *
 *  $Id: sidlewindow.c,v 40.0 94/02/15 17:57:18 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

/*****************************************************************************/

#define D(x)		;
#define DZOOM(x)	;
#define D2(x)		;

/*****************************************************************************/

/* little safe utility for use in startIdleWindow() when there
 * might not be an active window. */
static LONG haveActiveRequest (void)
{
    struct Window *w;
    LONG retval = FALSE;

    if (w = fetchIBase ()->ActiveWindow)
	retval = (w->FirstRequest != NULL);
    return (retval);
}

/*****************************************************************************/

/* passed to:
 *	startGadget
 *	startScreenDrag */
static void returnIdleWindow (void)
{
    D2 (printf ("returnIdleWindow\n"));
    fetchIBase ()->CurrentState = sIdleWindow;
}

/*****************************************************************************/

/* about the same as returnRequesterZoom().
 *
 * Making the (safe) assumption that the window which had a zoom gadget selection
 * is still the active window, sVerify doesn't allow change of activation, and as
 * sGadget morphs the zoom gadget selectup token, it gets processed immediately
 * afterward */
static void returnIdleZoom (okcode)
{
    if (okcode != OKOK)
    {
	/* if aborted (or timeout), send "verify clear"
	 * ZZZ: would need to change for "MOVEVERIFY" */
	activeEvent (IECLASS_SIZEWINDOW, 0);
    }
    else
    {
	/* Peter 4-Dec-90:  TRUE parameter indicates that this zoom
	 * resulted from user-interaction.  In particular, a NEWSIZE
	 * is required if the window has SIZEVERIFY set. */
	IZoomWindow (fetchIBase ()->ActiveWindow, TRUE);
    }
    returnIdleWindow ();
}

/*****************************************************************************/

/* state dispatcher
 * state transitions called:
 *	startIdleWindow()
 *	startGadget()
 *	startMenu()
 *	startRequest()
 */
void dIdleWindow (void)
{
    struct IntuitionBase *IBase = fetchIBase ();
    UWORD menunum;

    struct Window *itwindow = (struct Window *) IT->it_Object1;

    switch (IT->it_Command)
    {
	case itMENUDOWN:	/* lend menus, send event or start menus */
	    {
		if (TESTFLAG (IBase->ActiveWindow->Flags, RMBTRAP))
		{
		    activeEvent (IECLASS_RAWMOUSE, MENUDOWN);
		}
		else
		{
		    /* If this window has menu-lending, then send the
		 * appropriate itACTIVATEWIN token, otherwise
		 * start menu-state.
		 */
		    if (XWINDOW (IBase->ActiveWindow)->MenuLend)
		    {
			/* Menu-lending is in effect.  Convert
		     * this token to an itACTIVATEWIN token.
		     */
			reuseToken ();
			IT->it_Command = itACTIVATEWIN;
			IT->it_Object1 = (CPTR) XWINDOW (IBase->ActiveWindow)->MenuLend;
			IT->it_Object2 = AWIN_LENDMENU;
			return;
		    }
		    else
		    {
			startMenu ();
		    }
		}
	    }
	    return;

	case itSELECTDOWN:	/* start idlewindow */
	    startIdleWindow ();
	    return;

	case itMOUSEMOVE:	/* do default, then maybe send MOUSEMOVE */
	    doDefault ();	/* updateMousePointer()	*/

	    /* send mouse message to active window */
	    if (TESTFLAG (IBase->ActiveWindow->Flags, REPORTMOUSE))
	    {
		sendWindowMouse ();
	    }
	    return;

	case itRAWKEY:		/* Do menu-key or send key-event */

	    /* Is this a non-RMBTRAP window, and does it look like a menu-key?
	 * (right Amiga-key modified downstroke)
	 */
	    if (!TESTFLAG (IBase->ActiveWindow->Flags, RMBTRAP) &&
		(TESTFLAG (IE->ie_Qualifier, AMIGARIGHT)) &&
		(!TESTFLAG (IE->ie_Code, IECODE_UP_PREFIX)))
	    {
		/* Now figure out if this window has menu lending: */
		struct Window *menuwindow = IBase->ActiveWindow;

		if (XWINDOW (menuwindow)->MenuLend)
		{
		    menuwindow = XWINDOW (menuwindow)->MenuLend;
		}
		if ((menunum = findMenuKey (menuwindow->MenuStrip,
					    IT->it_Code)) != MENUNULL)
		{
		    if (menuwindow == IBase->ActiveWindow)
		    {
			/* Go do regular menu operation */
			D (printf ("sIdleWindow: found  key equiv of %lx\n", menunum));
			startMenuKey (menunum);
		    }
		    else
		    {
			/* Menu-lending is in effect, and the this keystroke
		     * is a valid menu-key in the target window.  Convert
		     * this token to an itACTIVATEWIN token.
		     */
			reuseToken ();
			IT->it_Command = itACTIVATEWIN;
			IT->it_Object1 = (CPTR) menuwindow;
			IT->it_Object2 = (CPTR) AWIN_LENDMENUKEY;
			IT->it_SubCommand = menunum;
		    }
		    return;
		}

		/* else not a menu-key-equiv for this strip,
	     * so fall through...
	     */
	    }
	    /* else RMBTRAP, or not a menu-key-equiv type keystroke,
	 * so fall through...
	 */

	    /* note that all command equiv. and mouse button
	 * voodoo translation have occured already.
	 */
	    activeEvent (IECLASS_RAWKEY, IE->ie_Code);
	    return;

	case itACTIVATEWIN:	/* start idlewindow */
	    startIdleWindow ();
	    return;

	case itACTIVATEGAD:	/* do it only if gad is in active window */
	    D (printf ("siw hearing activategad\n"));
	    /* only activate gadgets in window proper, and if no requester */
	    if ((itwindow == IBase->ActiveWindow) && (IT->it_SubCommand == 0))
		startGadget (returnIdleWindow);
	    else
		doDefault ();	/* return error */
	    return;

	case itSETREQ:		/* do default, maybe go to requester state */
	    /* put up requester and send REQSET message */
	    doDefault ();

	    /* change to requester state if the requester is
	 * for the active window
	 */
	    if ((IT->it_Error == 0) && itwindow == IBase->ActiveWindow)
	    {
		startRequester ();
		return;
	    }
	    return;

	case itMETADRAG:	/* go to screen-drag state */
	    startScreenDrag (returnIdleWindow);
	    return;

	case itCLOSEWIN:	/* do it, change activation if needed */
	    doDefault ();	/* let it be delinked	*/
	    if (itwindow == IBase->ActiveWindow)
	    {
		startIdleWindow ();	/* make parent window active */
	    }
	    return;

	    /* normal input sent to application */
	case itMENUUP:		/* send event */
	    activeEvent (IECLASS_RAWMOUSE, IECODE_RBUTTON | IECODE_UP_PREFIX);
	    return;
	case itSELECTUP:	/* send event */
	    D2 (printf ("sidlewindow, normal selectup\n"));
	    activeEvent (IECLASS_RAWMOUSE, IECODE_LBUTTON | IECODE_UP_PREFIX);
	    return;

	case itZOOMWIN:	/* do default, maybe with SIZEVERIFY */
	    /* it_SubCommand specifies a user-initiated zoom, meaning
	 * that we need a SIZEVERIFY session.
	 */
	    if (IT->it_SubCommand)
	    {
		DZOOM (printf ("sidlewindow starting sizev for itZOOM\n"));
		startVerify (returnIdleZoom, itwindow, SIZEVERIFY, 0);
	    }
	    else
	    {
		DZOOM (printf ("sidlewindow got non-verify zoom\n"));
		doDefault ();
	    }
	    return;

	case itTIMER:		/* check for gadget help, then do default */
	    gadgetHelpTimer ();
	    /* FALL THROUGH!!! */

	/* default processing */
#if 0
	case itDISKCHANGE:	/* do default */
	case itOPENSCREEN:	/* do default */
	case itCLEARREQ:	/* do default */
	case itOPENWIN:		/* do default */
	case itVERIFY:		/* do default */
	case itREMOVEGAD:	/* do default */
	case itSETMENU:		/* do default */
	case itCLEARMENU:	/* do default */
	case itCHANGEWIN:	/* do default */
	case itDEPTHWIN:	/* do default */
	case itMOVESCREEN:	/* do default */
	case itDEPTHSCREEN:	/* do default */
	case itCLOSESCREEN:	/* do default */
	case itNEWPREFS:	/* do default */
	case itMODIFYIDCMP:	/* do default */
	case itCHANGESCBUF:	/* do default */
	case itCOPYSCBUF:	/* do default */
	case itUNKNOWNIE:	/* do default */
	case itSETPOINTER:	/* do default */
	case itGADGETMETHOD:	/* do default */
	case itMODIFYPROP:	/* do default */
#endif
	default:		/* itDEFAULT: do default */
	    doDefault ();
    }
}

/*****************************************************************************/

/* transition startIdleWindow()
 * called by:
 *	sNoWindow	- itACTIVATEWIN
 *	sNoWindow	- itSELECTDOWN
 *
 *	sIdleWindow	- itSELECTDOWN
 *	sIdleWindow	- itACTIVATEWIN
 *	sIdleWindow	- itCLOSEWIN
 *
 *	sMenu		- return from DMR timeout or no menu
 *
 *	sRequester	- itSELECTDOWN
 *	sRequester	- itACTIVATEWIN
 *	sRequester	- itCLOSEWIN
 *	sRequester	- itCLEARREQ
 *	returnRequester	-  (ZZZ: which state?) endgadget?
 *
 * description:
 *	activates a window
 * enhancements:
 *	pick up with last activated gadget
 *	fold together some of these cases ...
 */
void startIdleWindow (void)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct Window *newawindow;
    struct Window *itwindow;
    struct Layer *hitlayer;	/* ZZZ: in IBase? */
    struct Window *prevactivewin = IBase->ActiveWindow;

    D (printf ("transition startIdleWindow, cstate: %ld, command %ld\n",
	       IBase->CurrentState, IT->it_Command));

    IBase->MenuLendingReturn = NULL;

    /* itSELECTDOWN: may be new active, or change in active.
     * may also be already active, and
     * test for gadget hits go here, and
     * ZZZ: future: may be screenbar hit: ignore
     */

    itwindow = (struct Window *) IT->it_Object1;

    switch (IT->it_Command)
    {
	case itSELECTDOWN:	/* start: check if new window hit */
	    newawindow = hitUpfront (&hitlayer);
	    if ((newawindow != IBase->ActiveWindow) ||
		((!newawindow) && (IBase->ActiveScreen != IBase->HitScreen)))
	    {
		/* will change ActiveWindow, etc. */
		setWindow (newawindow, FALSE);
	    }

	    /* if this window has a requester in it, change state
	 * and let sRequester handle the click, including checking
	 * for legal requester gadgets.
	 */
	    if (haveActiveRequest ())	/* returns NULL if NoWindow */
	    {
		startRequester ();
		return;
	    }

	    /* jimm: 3/5/90: handle click in unwindowed area of screen */

	    D (printf ("sIW: about to call hitGadgets \n"));
	    if (hitGadgets (hitlayer))
	    {
		D (printf ("sIW: startGadget\n"));

		/* startNoWindow is the same as returnNoWindow would be */
		startGadget (IBase->ActiveWindow ?
			     returnIdleWindow : startNoWindow);
		return;
	    }
	    else if (IBase->ActiveWindow == NULL)
	    {
		/* If the user clicked in the "no-window" area of
	     * the screen, we want to still change the
	     * Active Screen.  This fixes the bug where you
	     * couldn't select a screen for autoscrolling
	     * by clicking in the no-window area.
	     */
		setScreen (IBase->HitScreen);
		startNoWindow ();	/* just jump to sNoWindow	*/
		return;
	    }
	    else
	    {
		D (printf ("sIW: rawmouse down\n"));
		D (Debug ());
		activeEvent (IECLASS_RAWMOUSE, SELECTDOWN);
		D (Debug ());
		D (printf ("sIW: activeE returned\n"));
	    }
	    break;

	case itACTIVATEWIN:	/* start: do new active window */
	    newawindow = itwindow;

	    D (printf ("sIW ACTIVATE window %lx\n"));

	    /* better check that it's still around */
	    if (!knownWindow (newawindow))
	    {
		/* no state change */
		return;
	    }

	    if (newawindow != IBase->ActiveWindow)
	    {
		/* will change ActiveWindow, etc. */
		setWindow (newawindow, (IT->it_Object2 == AWIN_INITIAL));
	    }
	    else
	    {
		/* Intuition relies on itACTIVATEWIN with an Object2 of AWIN_INITIAL
	     * to render all the gadgets of the window border.  This is
	     * used to avoid drawing the border and border gadgets
	     * inactive then immediately active.  In the event that
	     * itACTIVATEWIN goes through straight away, there is no
	     * problem.  However, it's possible that someone sneaked in
	     * and activated the window through ActivateWindow() or
	     * through clicking on it.  Thus, if we get to here with
	     * it_Object2 of AWIN_INITIAL, we have to call fixWindowBorders()
	     * (which knows only to act if it_Object2 is AWIN_INITIAL).
	     */
		fixWindowBorders ();

		/* This code is executed when itACTIVATEWIN is
	     * received for the already-active window.  For various
	     * reasons, this code needs to be replicated in
	     * sgadget.c, under itACTIVATEWIN.  Some day we
	     * should clean things up so this hand-dependency is
	     * unnecessary.
	     */

		/* ZZZ: PPage compatibility hack, to eliminate their
	     * transparent menus.  It would be better to put
	     * all this stuff into setWindow(), so that if you
	     * setWindow() the already-active window, it just
	     * does the setScreen() and the activeEvent(ACTIVEWINDOW)

	     * Peter 26-Jun-91:  We know newawindow can't be NULL
	     * because we performed knownWindow() on it higher up.
	     */
		setScreen (newawindow->WScreen);

		/* send him a message anyway, so he doesn't wait for
	     * it forever
	     */
		activeEvent (IECLASS_ACTIVEWINDOW, NULL);
	    }

	    if (IT->it_Object2 == AWIN_LENDMENU)
	    {
		IBase->MenuLendingReturn = prevactivewin;

		startMenu ();
		return;
	    }

	    else if (IT->it_Object2 == AWIN_LENDMENUKEY)
	    {
		IBase->MenuLendingReturn = prevactivewin;

		startMenuKey (IT->it_SubCommand);
		return;
	    }

	    /* if this window has a requester in it, change state
	 * and let sRequester handle the click, including checking
	 * for legal requester gadgets.
	 */
	    if (haveActiveRequest ())
	    {
		startRequester ();
		return;
	    }

	    /** ZZZ: enhance: if WGadget, startGadget **/
	    break;

	case itCLOSEWIN:	/* start: figure new active window */

	    /* RESTART: this is called if dIdleWindow closes
	 * the Active Window, to set up the new
	 * active window.
	 */

	    /* NOTE WELL: window has been delinked by now,
	 * but since this token is still being processed,
	 * CloseWindow() hasn't been signalled to free
	 * the window yet.  So the pointer 'itwindow' is still
	 * valid.
	 *
	 * We know that the ActiveWindow is the one that
	 * is being closed.  Huh?  You don't know that yet.
	 * Yes you do, since this is startIdleWindow(), not
	 * dIdleWindow(), chump.
	 */

	    /* ZZZ: am I making a bad assumption then? */
	    /* no, you're just a fool.	*/

	    /* window to activate is either Parent, or if none,
	 * the FirstWindow in the same screen.
	 */
	    /* WARNING: relying on expression evaluation rules */
	    (newawindow = itwindow->Parent)
		|| (newawindow = itwindow->WScreen->FirstWindow);

	    /* Peter 2-Dec-90: We used to have to restore this window's
	 * screen title to default if newawindow was NULL.  But now
	 * setWindow() does that even though we NULL out IBase->ActiveWindow.
	 */

	    IBase->ActiveWindow = NULL;	/* don't deactivate closer */
	    setWindow (newawindow, FALSE);	/* may be NULL	*/

	    /* Peter 8-Jan-91:  setWindow() sets IBase->ActiveWindow to
	 * ( knownWindow( newawindow ) ? newawindow : NULL ).
	 * We want to pick up the case where the window isn't known.
	 * This kludge helps FantaVision, which unlinks the windows
	 * it doesn't want to deal with at the moment.  The result
	 * is that the closing window's parent is not known.  This
	 * keeps them from crashing.
	 */
	    newawindow = IBase->ActiveWindow;

	    if (newawindow)
	    {
		if (haveActiveRequest ())
		{
		    startRequester ();
		    return;
		}
	    }
	    else
		/* both newawindow candidates are NULL */
	    {
		startNoWindow ();
		return;
	    }
	    break;

	default:
	    /* all  sorts of DMR timeouts, other non-changes to
	 * sIdleWindow and sRequester, dropped requester, etc.
	 */
	    if (haveActiveRequest ())
	    {
		startRequester ();
		return;
	    }
    }

    /** IBase->ActiveWindow = newwindow; ** done in setWindow() **/
    IBase->CurrentState = sIdleWindow;
}
