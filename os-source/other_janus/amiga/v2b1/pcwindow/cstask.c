
/* *** cstask.c **************************************************************
 * 
 * Cursor Task for both displays of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 27 Feb 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"					  
									

Cursor()
/* This task checks the existence of the windows, and verifies that
 * the color display is in text mode, and renders the cursors in the
 * windows as appropriate.  Wake up 6 times a second and toggle the
 * state of the cursor.
 *											 
 * This task is created by the others.		When another task wants to find
 * this one but finds that this one doesn't exist, it creates this one.
 *
 * If the SuicideSignal is sent, this task quietly suicides.
 */
{
	ULONG waitBits, timerSignal;
	REGISTER ULONG wakeUpBits;
	struct MsgPort *timerPort;
	struct IOStdReq *timerMsg;
	REGISTER struct Display *display, *olddisplay;
	REGISTER struct SuperWindow *superwindow;
	REGISTER struct Window *window;
	REGISTER LONG seconds, micros;
	LONG counter;

														   
#ifdef AZTEC
	/* This gets A4 from a memory location.  This is needed because
	 * Manx has implemented relative-addressing using A4.  Task object files
	 * should be clustered around the call to SaveA4();
	 */
	GetA4();
#endif


/* === Critical Section.  Must be completed in toto ==================== */ 
	Forbid();

	CursorTask = FindTask(0);
	CursorSuicideSignal = ZAllocSignal();

	Permit();
/* === End of Critical Section ========================================== */

	/* CreatePort(name, priority); */			   
	if (timerPort = CreatePort("TimerPort", 0))
		timerMsg = CreateStdIO(timerPort);

	if ((timerPort == NULL) || (timerMsg == NULL))
		{
		Alert(ALERT_NO_TIMERPORT, NULL);
		CloseCursorTask(timerPort, timerMsg);
		}

	if (OpenDevice(TIMERNAME, UNIT_VBLANK, timerMsg, 0) NOT= 0)
		{
		Alert(ALERT_INCOMPLETESYSTEM, NULL);
		CloseCursorTask(timerPort, timerMsg);
		}

	counter = 0;

	timerSignal = 1 << timerPort->mp_SigBit;

	waitBits = timerSignal | CursorSuicideSignal;

	SetTimer(DEFAULT_CURSOR_SECONDS, DEFAULT_CURSOR_MICROS, timerMsg);

	FOREVER
		{
		wakeUpBits = Wait(waitBits);

		/* *Must* be called before the timer signal check */
		if (wakeUpBits & CursorSuicideSignal)
			CloseCursorTask(timerPort, timerMsg);

		if (wakeUpBits & timerSignal)
			{
			seconds = DEFAULT_CURSOR_SECONDS;
			micros = DEFAULT_CURSOR_MICROS;
			counter++;

			/* The following is complicated, but it allows me to avoid
			 * locking the DisplayListLock while traversing the display
			 * list.  The theory here is:  In one uninterrupted stroke
			 * (using Forbid()/Permit()) test if I should abandon the
			 * the list traversal *and* test if the display I'm thinking of
			 * examining is actually a valid (non-NULL) display pointer *and*
			 * see if I can get a lock on the display.		If I make it through
			 * all three steps, then the AuxToolsManager hasn't corrupted
			 * the display list yet so the non-NULL display pointer I have is
			 * valid, and I could lock it so it will continue
			 * to be valid until I unlock it.  So far so good.		Then I
			 * blink the current display.  Then I Forbid() and then start
			 * the process over again by again loading the next display 
			 * pointer from the display structure before unlocking the
			 * display structure, which could invalid at this
			 * point, but would be *only* if the AuxToolsManager has also
			 * set the AbandonBlink flag TRUE.		So, while remaining Forbid(),
			 * I loop back up to the three tests above.
			 */
			AbandonBlink = FALSE;   /* Changed later by me or AuxToolManager */

			Forbid();
			display = ZaphodDisplay.FirstDisplay;
			while (display && (NOT AbandonBlink) 
					&& (NOT DeadenBlink))
				{
				/* I can afford to not bother locking the display list, but 
				 * I *must* lock the display down whether you like it or not.
				 */
				if (Lock(&display->DisplayLock))
					{
					Permit();

/*???					if (counter > 300) SetFlag(display->Modes, COUNT_DISPLAY);*/
					if (superwindow = display->FirstWindow)
						{
						window = superwindow->DisplayWindow;
						if (window->Flags & WINDOWACTIVE)
							{
							BlinkCursor(display);
							seconds = superwindow->CursorSeconds;
							micros = superwindow->CursorMicros;
							}
						}

					Forbid();
					olddisplay = display;
					display = display->NextDisplay;
					Unlock(&olddisplay->DisplayLock);
					}
				else 
					/* We couldn't get a lock on one of the displays,
					 * so abandon ship for now (who knows what's going
					 * on, who cares, just run away).
					 */
					AbandonBlink = TRUE;
				}
			Permit();		/* Balance them out at last */

			GetMsg(timerPort);
			SetTimer(seconds, micros, timerMsg);
			}
		}
}


											  

BlinkCursor(display)
REGISTER struct Display *display;
{
	REGISTER struct Layer *layer;
	REGISTER struct SuperWindow *superwindow;
	REGISTER struct Window *window;

	if ((superwindow = display->FirstWindow) == NULL) return;
	if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) return;
	if ((window = superwindow->DisplayWindow) == NULL) return;
	if (FlagIsClear(display->Modes, TEXT_MODE)) return;

/* === Critical Area because of the several shared data areas =========== */
	Forbid();

	layer = window->RPort->Layer;

	LockLayerRom(layer);

	display->CursorOldCol = display->CursorCol;
	display->CursorOldRow = display->CursorRow;

	if (display->CursorFlags & CURSOR_MOVED)
		{
		if (display->CursorFlags & CURSOR_ON) Curse(display);
		ClearFlag(display->CursorFlags, CURSOR_MOVED | CURSOR_ON);
		NewCursings(display, display->CursorOldCol, display->CursorOldRow);
		}

	ToggleFlag(display->CursorFlags, CURSOR_ON);
	Curse(display);
	UnlockLayerRom(layer);

	Permit();
/* === End of Critical Area ============================================ */
}




CloseCursorTask(timerPort, timerMsg)
REGISTER struct MsgPort *timerPort;
REGISTER struct IOStdReq *timerMsg;
/* This routine presumes that it is called while there is still a timer
 * message outstanding, which is to say that either a time-out has not
 * yet occurred or it has already occurred but the message hasn't been
 * retrieved yet.
 */
{
	REGISTER struct Task *task;

	Forbid();

	if (timerMsg)
		{
		if (GetMsg(timerPort) == NULL) Wait(1 << timerPort->mp_SigBit);
		CloseDevice(timerMsg);
		DeleteStdIO(timerMsg);
		}
	if (timerPort) DeletePort(timerPort);

	task = CursorTask;
	CursorTask = NULL;

	RemTask(task);
}


