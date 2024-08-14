/*** sverify.c *****************************************************************
 *
 * sVerify state processing
 *
 *  $Id: sverify.c,v 40.0 94/02/15 17:54:41 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

/*****************************************************************************/

#define D(x)	;

/*****************************************************************************/

static void verifyAbort (LONG okcode)
{
    struct IntuitionBase *IBase = fetchIBase ();

    IBase->VerifyWindow = NULL;	/* must be clean */
    IBase->VerifyMessage = NULL;/* must be clean */
    D (printf ("verifyAbort, okcode %lx\n", okcode));
    (*IBase->VerifyReturn) (okcode);
}

/*****************************************************************************/

/* state sVerify
 *
 * global data used:
 *
 * local state data:
 */

/* transition startVerify()
 * called by:
 * with state data:
 * description:
 */
void startVerify (void (*vreturn)(LONG), struct Window *window, ULONG class, UWORD code)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct IntuiMessage *imsg;

    D (printf ("startVerify, cstate: %ld, window %lx class %lx\n", IBase->CurrentState, window, class));

    /* if the window doesn't have the appropriate VERIFY
     * IDCMPFlag set, it's considered the same as an OK */
    if (!TESTFLAG (window->IDCMPFlags, class))
    {
	D (printf ("sV: window doesn't want it\n"));
	(*vreturn) (OKOK);	/* done */
	return;
    }

    /* try to get a message to send, init it and send */
    /* ZZZ: Could pass NULL for tablet stuff */
    imsg = initIMsg (class, code, IT->it_IE, window, IT->it_Tablet);

    if (imsg == NULL)
    {
	D (printf ("sV: no message\n"));
	(*vreturn) (OKABORT);	/* bail out: mem too low anyway */
	return;
    }
    D (printf ("sV: put him the message %lx\n", imsg));

    /* Peter 16-Oct-90:
     * As a result of CloseWindowSafely(), there is a possibility
     * of encountering a window with IDCMPFlags != NULL which
     * nevertheless has a UserPort of NULL.  This closes that problem.
     */

    if (!PutMsgSafely (window->UserPort, imsg))
    {
	D (printf ("sV: no UserPort\n"));
	(*vreturn) (OKABORT);	/* bail out: window going away anyway */
	return;
    }

    /* set up state data */
    IBase->VerifyReturn = vreturn;
    IBase->VerifyWindow = window;
    IBase->VerifyClass = class;
    IBase->VerifyCode = code;	/* MENUWAITING for inactive windows */
#define VERIFYTIMEOUT	(10 * 3)
    IBase->VerifyTimeout = VERIFYTIMEOUT;

    /* reclaimIMsg() will check to see if this message came
     * back, and will issue an itVERIFY when it does.
     * At that time, it will clear the field itself.
     */
    IBase->VerifyMessage = imsg;

    /*
     * reclaimMessages() will put an itVERIFY token
     * right on the TokenQueue if the VerifyMessage has
     * been replied by now.
     */
    WaitTOF ();			/* give the other app a little time to run */
    reclaimMessages (window);

    IBase->CurrentState = sVerify;
}

/*****************************************************************************/

/* state dispatcher state transitions called: */
void dVerify (void)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct Window *itwindow = (struct Window *) IT->it_Object1;
    UWORD code;
    struct Task *FindTask ();

    switch (IT->it_Command)
    {
	case itTIMER:		/* do default, then count down to abort */
	    doDefault ();	/* jimm: 5/23/90	*/
	    if (--IBase->VerifyTimeout < 0)
	    {
		/* timeout is normally cancel, except for
	     * inactive windows and MENUVERIFY
	     */
		D (printf ("verify timeout abort\n"));
		verifyAbort ((IBase->VerifyCode == MENUWAITING) ? OKOK : OKABORT);
		/* returns to VerifyReturn */
	    }
	    break;

	case itMENUUP:		/* abort MENUVERIFY */
	    /* user abort: terminates menu verify */
	    /* NOTE: ZZZ: terminates even menu key equiv. sessions */
	    if (TESTFLAG (IBase->VerifyClass, MENUVERIFY))
	    {
		/* jimm: 5/14/90: make sure they see this	*/
		activeEvent (IECLASS_RAWMOUSE, MENUUP);
		verifyAbort (OKABORT);
		/* returns to VerifyReturn */
	    }
	    return;

	case itSELECTUP:	/* abort SIZEVERIFY */
	    /* user abort: terminates size verify */
	    /* ZZZ: we don't have programmatic sizeverify, but if
	 * we do, we might want to be careful here.  DMouse/zoom
	 * or somebody might want to zoom when it hears a click.
	 * If so, the upclick could easily beat the verify reply.
	 */
	    if (TESTFLAG (IBase->VerifyClass, SIZEVERIFY))
	    {
		D (printf ("sizeverify select up abort\n"));
		verifyAbort (OKABORT);
		/* returns to VerifyReturn */
	    }
	    return;

	    /* yippie!!! we got the message back from the sucker */
	case itVERIFY:		/* We got the reply from the application */
#if 0
	    if (itwindow != IBase->VerifyWindow)
	    {
		/* skip: from earlier window which we gave up on */
		D (printf ("dV: itVERIFY, but for window %lx not %lx\n",
			   itwindow, IBase->VerifyWindow));
		return;
	    }
#endif
	    /* We are assured that this itVERIFY event corresponds
	 * to the verify message that we are waiting for.
	 * Extraneous verify replies must be handled,
	 * by sending a "clear" signal such as menuup
	 */

	    D (printf ("dVerify got itVERIFY,subcommand: %lx\n", IT->it_SubCommand));
	    if (TESTFLAG (IBase->VerifyClass, SIZEVERIFY | REQVERIFY)
		|| IT->it_SubCommand == OKOK)
	    {
		/* can't cancel size or request verify */
		code = OKOK;
	    }
	    else
		code = OKCANCEL;

	    D (printf ("dV: itVERIFY ok, calculated code: %lx\n", code));
	    verifyAbort (code);
	    /* returns to VerifyReturn */
	    return;

	case itMODIFYIDCMP:	/* do default, but catch him turning off the verify */
	    /* IDCMPFlags are stuffed in it_Object2	*/
	    doDefault ();	/* let him get his flags changed	*/

	    /* Is he clearing the VERIFY IDCMP flag?
	 * If so, it's equivalent to saying OK, IF the message
	 * is in his queue (or he's hanging on to it).  If
	 * he's replied to it already, do nothing special.
	 */
	    if (itwindow == IBase->VerifyWindow &&
		!TESTFLAG ((ULONG) IT->it_Object2, IBase->VerifyClass))
	    {
		/* If he's already replied the verify message, I can
	     * just wait for normal processing of that.
	     *
	     * if not, I want to abort this transaction, but the V1.2
	     * scam of snatching the message away won't float.  So,
	     * I've got other logic to deal with extraneous abort
	     * processing, and letting the guy know the menu or
	     * sizing session is done.
	     */
		if (!snoopVerifyReply ())
		{
		    D (printf ("sV: modifyidcmp abort OK\n"));

		    /* DANGER: Intuition will start menus during
		 * this call.  You *must* be the input.device
		 * to start menus, or the wrong task will get
		 * the locks.
		 */
		    if (FindTask (0) != IBase->InputDeviceTask)
		    {
			D (printf ("defer itMODIFYIDCMP token\n"));
			deferToken ();
		    }
		    else
		    {
			D (printf ("ahh, that's more like it\n"));
			verifyAbort (OKOK);
			/* returns to VerifyReturn */
		    }
		}
		D (
		      else
		      printf ("sVerify reply is pending, don't abort.\n"));
	    }
	    return;

	case itMENUDOWN:	/* do default */
	case itSELECTDOWN:	/* do default */
	case itMOUSEMOVE:	/* do default */
	case itRAWKEY:		/* do default */
	case itDISKCHANGE:	/* do default */
	case itNEWPREFS:	/* do default */
	case itREMOVEGAD:	/* do default */
	case itSETMENU:	/* do default */
	case itCLEARMENU:	/* do default */

	case itONOFFMENU:	/* do default (do not defer: jimm: 4/18/90) */
	case itSHOWTITLE:	/* do default */
	case itDEPTHSCREEN:	/* do default (and some more: jimm: 4/30/90) */
	case itCHANGEWIN:	/* do default */
	case itZOOMWIN:	/* do default */
	case itMOVESCREEN:	/* do default */
	case itDEPTHWIN:	/* do default */
	case itOPENSCREEN:	/* do default */
	case itCLOSESCREEN:	/* do default (can't close screen with (active) window) */
	case itSETPOINTER:	/* do default */
	case itGADGETMETHOD:	/* do default */
	case itMODIFYPROP:	/* do default */
	    doDefault ();
	    return;

	case itCHANGESCBUF:	/* fail (menu bar can be up) */
	    IT->it_Error = 1;
	    return;

	case itACTIVATEWIN:	/* fix window borders, then defer (warning, might change return state) */
	    fixWindowBorders ();

	    /* FALL THROUGH !!! */

#if 0
	    /* can't let a window be closed because menu verify
     * might be walking the linked list of windows sending
     * that stupid MENUWAITING message
     */
	case itCLOSEWIN:	/* defer */
	case itOPENWIN:	/* defer */
	    /*
     * if the guy I'm waiting for is waiting for one
     * of these tokens to complete, we'll have a mini-deadlock.
     * this means that the user-abort will be required
     *
     * This is not good, so it's nice to let as many synchronous
     * operations happen as we can manage.
     */
	case itACTIVATEGAD:	/* defer (warning, might change return state) */
	case itSETREQ:		/* defer (warning, might change return state) */
	case itCLEARREQ:	/* defer (warning, might change return state) */
	case itMETADRAG:	/* defer (I'm busy now, and this is a mistake) */
	case itCOPYSCBUF:	/* defer (menu bar can be up) */
#endif
	default:		/* itDEFAULT: defer */
	    deferToken ();
    }
}
