
/* ***************************************************************************
 * 
 * Display Task  --  for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name	    Description
 * -----------	----------  -------------------------------------------------
 * 20 Feb 86	=RJ Mical=  Created this file
 * 13 Mar 86	=RJ=	    Changed this from cdtask.c to disptask.c
 *   
 * **************************************************************************/

#include "zaphod.h"


DisplayTask (display)
struct Display *display;
/* The Display structure must be pre-initialized in this way:
 *	- Modes must be initialized to reflect the type of display desired.
 *	- The rest of the structure must be initialized to NULL.
 */
{
    ULONG waitSignals, wakeUpSignals;
    ULONG displayWriteSignal;
    ULONG windowSignal, CRTRegisterWriteSignal;
    ULONG janusDisplayNum, janusCRTNum;
    struct SuperWindow *superwindow;
    struct Window  *window;

/* === Critical Section.  Must be completed in toto ====================== */
    Forbid ();

    OpenAuxTools (display);
    InitDisplayTask (display);

    Permit ();
/* === End of critical section ========================================== */


    displayWriteSignal = ZAllocSignal ();
    windowSignal = 1 << display -> TaskPort -> mp_SigBit;
    CRTRegisterWriteSignal = ZAllocSignal ();

    waitSignals = displayWriteSignal | windowSignal | CRTRegisterWriteSignal;


/* Tell the interrupt server that I'm alive and looking for
 * the various interrupts.
 * Tell about these interrupts:
 *        - displayWrite
 *	  - CRTRegisterWrite
 * (Some great names follow.  They're not my names.) */


    if (display -> Modes & COLOR_DISPLAY) {
	janusDisplayNum = JSERV_GINT;
	janusCRTNum = JSERV_CRT2INT;
    }
    else {
	janusDisplayNum = JSERV_MINT;
	janusCRTNum = JSERV_CRT1INT;
    }


#ifdef JANUS

    display -> CRTWriteSig = SetupJanusSig (janusCRTNum,
	    StupidCreateSignalNumber (CRTRegisterWriteSignal), 0, 0);
    display -> DisplayWriteSig = SetupJanusSig (janusDisplayNum,
	    StupidCreateSignalNumber (displayWriteSignal), 0, 0);

#endif

 if ((display -> CRTWriteSig == NULL) || (display -> DisplayWriteSig) == NULL) {
	Alert (ALERT_NO_JANUS_SIG, display);
	CloseDisplayTask (display);
    }

    Forbid ();
    superwindow = display -> FirstWindow;
    window = superwindow -> DisplayWindow;
    while ((window -> UserPort = FindPort (INPUT_PORT_NAME)) == NULL)
	WaitTOF ();
    ModifyIDCMP (window, DISPLAY_IDCMP_FLAGS);
    Permit ();


    FOREVER
    {
    /* Wait for the interrupt */
	wakeUpSignals = Wait (waitSignals);
    /* NOTE:  Window Events should be handled before display signals! */
    /* (Don't ask me why, just do it!) */
	if (wakeUpSignals & windowSignal) {
	/* A window event has occurred.  Go handle it. */
	    WindowEvent (display);
	}

	if (wakeUpSignals & CRTRegisterWriteSignal)
	    GetCRTSetUp (display);

	if (wakeUpSignals & displayWriteSignal)
	    /* What?  Someone bothering the Blue display memory?  Well! */
	    RenderWindow (display, FALSE, FALSE, TRUE);

    }

}


WindowEvent (display)
struct Display *display;
/* Some sort of event (not RAWKEY, you know) has occurred with our
 * Color Window.  Handle it here.
 */
{
    struct IntuiMessage *message;
    ULONG class;
    USHORT code;
    ULONG seconds, micros;
    BOOL render, recalc, stuffText, redrawAll, locked;
    APTR IAddress;
    struct SuperWindow *superwindow;
    struct Window  *IDCMPWindow;
    struct Window  *window;
    SHORT oldcol, oldrow, modes;

    render = recalc = stuffText = redrawAll = locked = FALSE;

    while (message = GetMsg (display -> TaskPort)) {
	class = message -> Class;
	code = message -> Code;
	seconds = message -> Seconds;
	micros = message -> Micros;
	IDCMPWindow = message -> IDCMPWindow;
	IAddress = message -> IAddress;
	ReplyMsg (message);

    /* Find the window and superwindow associated with this event. */
	window = NULL;
	superwindow = display -> FirstWindow;
	while (superwindow && (window == NULL)) {
	    if (superwindow -> DisplayWindow == IDCMPWindow)
		window = IDCMPWindow;
	    else
		superwindow = superwindow -> NextWindow;
	}

#ifdef DIAG
	if (superwindow != display -> FirstWindow)
	    if ((class != ACTIVEWINDOW) && (class != INACTIVEWINDOW))
		kprintf ("\nMAYDAY MAYDAY WindowEvent Class=%lx", class);
#endif

	if (window == NULL)
	    goto DONE_WITH_MESSAGE;

	switch (class) {
	    case NEWSIZE: 
		ModifyDisplayProps (display);

		if ((window -> Width == window -> MaxWidth)
			&& (window -> Height == window -> MaxHeight)
			&& (display -> Modes & BORDER_VISIBLE)) {
		    BorderPatrol (display, BORDER_OFF, TRUE);
		    SetFlag (display -> CursorFlags, CURSOR_MOVED);
		}
		else {

		/* Since we know that NEWSIZE *will* be followed by
		 * REFRESHWINDOW (poof), we can get away with deferring
		 * the redraw until later. */

		    SetFlag (display -> Modes, GOT_NEWSIZE);
		}
		break;
	    case REFRESHWINDOW: 
		if (display -> Modes & GOT_NEWSIZE) {
		    recalc = stuffText = redrawAll = TRUE;
		    ClearFlag (display -> Modes, GOT_NEWSIZE);
		}
		render = TRUE;
		break;
	    case GADGETUP: 
	    /* recalc remains FALSE */
		stuffText = render = redrawAll = TRUE;

		if (NOT locked) {
		    locked = TRUE;
		    Lock (&display -> DisplayLock);
		}

		oldrow = display -> DisplayStartRow;
		oldcol = display -> DisplayStartCol;
		RecalcDisplayParms (display);
		MoveAndSetRegion (display,
			display -> DisplayStartCol - oldcol,
			display -> DisplayStartRow - oldrow,
			window -> BorderLeft,
			window -> BorderTop,
			window -> Width - window -> BorderRight - 1,
			window -> Height - window -> BorderBottom - 1);
		break;
	    case MOUSEBUTTONS: 
		switch (code) {
		    case SELECTDOWN: 
			if (DoubleClick (display -> OldSeconds,
				    display -> OldMicros,
				    seconds, micros)) {
			    BorderPatrol (display, BORDER_TOGGLE, TRUE);
			    SetFlag (display -> CursorFlags, CURSOR_MOVED);
			    display -> OldSeconds = display -> OldMicros = 0;
			}
			else {
			    display -> OldSeconds = seconds;
			    display -> OldMicros = micros;
			}
			break;
		}
		break;
	    case ACTIVEWINDOW: 
		if (NOT locked) {
		    locked = TRUE;
		    Lock (&display -> DisplayLock);
		}

		SetTaskPri (FindTask (0), DisplayPriority);

		if (superwindow != display -> FirstWindow) {
		    UnlinkSuperWindow (display, superwindow);
		    LinkSuperWindow (display, superwindow);
		    recalc = stuffText = redrawAll = render = TRUE;
		}

		display -> OldSeconds = display -> OldMicros = 0;
		break;
	    case INACTIVEWINDOW: 
		if (NOT locked) {
		    locked = TRUE;
		    Lock (&display -> DisplayLock);
		}

	    /* If the border is hidden as we go inactive ... */
		if (display -> FirstWindow == superwindow) {
		    modes = display -> Modes;
		    if (display -> CursorFlags & CURSOR_ON) {
			Curse (display);
			ClearFlag (display -> CursorFlags, CURSOR_ON);
		    }
		}
		else
		    modes = superwindow -> DisplayModes;

		break;
	    case MENUPICK: 
		MenuEvent (display, code);
		break;
	    case CLOSEWINDOW: 
		CloseDisplayTask (display);
		break;
	}
DONE_WITH_MESSAGE: 
    }


    if (render) {
	RenderWindow (display, recalc, stuffText, redrawAll);
	if (display -> Modes & BORDER_VISIBLE)
	    TopRightLines (display -> FirstWindow -> DisplayWindow);
	SetFlag (display -> CursorFlags, CURSOR_MOVED);
    }

    if (locked)
	Unlock (&display -> DisplayLock);
}



CloseDisplayTask (display)
struct Display *display;
{
    struct Task *task;
    struct AuxToolsMessage *message;

    Forbid ();

    Lock (&display -> DisplayLock);

    DestroySuperWindow (display);

    if (display -> FirstWindow == NULL) {
	DeathLock (&display -> DisplayLock);

	if (display -> TaskPort) {
	    DrainPort (display -> TaskPort);
	    DeletePort (display -> TaskPort);
	    display -> TaskPort = NULL;
	}

	CloseAuxTools (display);

	task = display -> TaskAddress;
	display -> TaskAddress = NULL;

#ifdef JANUS
	if (display -> DisplayWriteSig)
	    CleanupJanusSig (display -> DisplayWriteSig);
	if (display -> CRTWriteSig)
	    CleanupJanusSig (display -> CRTWriteSig);
#endif

/*??? Taken out while DisplayTask() is still a procedure call
	RemTask(task);
???*/

    /* Sadly, until this becomes a library, I either do this or
     * learn the CreateProc() kludge stuff which no one has documentation
     * on and there's too few days left so here's the even kludgier
     * temporary solution, like it or
     * not ... */

	if (AuxToolsFinalPort) {
	    WaitPort (AuxToolsFinalPort);

	    if (message = (struct AuxToolsMessage  *)
		 GetMsg (AuxToolsFinalPort))
	    DeleteExtIO (message, sizeof (struct AuxToolsMessage));
	    DeletePort (AuxToolsFinalPort);
	}

	FreeRemember (&display -> BufferKey, TRUE);
	CloseEverything ();

	exit (0);
    }

    Unlock (&display -> DisplayLock);
}



