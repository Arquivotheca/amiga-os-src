
/* ***************************************************************************
 * 
 * Auxiliary Tools Task for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 22 Mar 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"			    

extern Cursor();
extern InputMonitor();


 
AuxToolsManager()
/* This task exists to manage the display list and the creation/elimination
 * of the auxiliary tools.  This is a precursor to the Zaphod library 
 * main system, which will render this extra step unnecessary since all
 * the auxiliary tools will come and go via ZaphodBase.  However,
 * knowing how engineering projects tend to go (I just barely managed
 * to fend off an unwanted trip to New York yesterday), I think I'll make
 * this kludge as robust as possible since it may become permanent.
 *
 * This guy sets up the auxiliary tasks.  It also sets up a message port
 * and then hangs around for display tasks to tell it that they are
 * coming and going.  This port is globally accessible (by nature of
 * its curious name) to all display tasks to rendevous with it as
 * desired.   Whenever a task adds its display to the fray, 
 * this task links it into the master list.  When a task signals to
 * extract its display, this task checks if all displays are gone and if 
 * so it asks everyone, including itself, to commit suicide (see the
 * TheFatLadySings() function).
 */
{
    ULONG waitBits, wakeUpBits, inputSignal;
    struct AuxToolsMessage *message;
    struct MsgPort *auxToolsPort;


/* === Critical Section.  Must be completed in toto ==================== */ 
    Forbid();

    if ((auxToolsPort = CreatePort(AUX_TOOLS_PORT, 0)) == NULL)
	{
	Alert(ALERT_NO_INPUTPORT, NULL);
	TheFatLadySings(NULL);
	}

    inputSignal = 1 << auxToolsPort->mp_SigBit;

    MakeAuxTools();

    Permit();
/* === End of Critical Section ========================================== */

    waitBits = inputSignal;

    ZaphodDisplay.FirstDisplay = NULL;

    FOREVER
	{
	wakeUpBits = Wait(waitBits);

	if (wakeUpBits & inputSignal)
	    {
	    Forbid();
	    while (message = (struct AuxToolsMessage *)GetMsg(auxToolsPort))
		{
		switch(message->AuxType)
		    {
		    case ADD_DISPLAY:
			LinkDisplay(message->AuxArg);
			break;
		    case REMOVE_DISPLAY:
			UnlinkDisplay(message->AuxArg);
			break;
		    case GET_DISPLAYLIST:
			message->AuxArg = (LONG)&ZaphodDisplay;
			break;
		    case NEW_PRIORITY:
			DisplayPriority = (SHORT)message->AuxArg;
			CursorPriority = DisplayPriority - 1;
			if (InputTask)
			    SetTaskPri(InputTask, DisplayPriority + 1);
			if (CursorTask) SetTaskPri(CursorTask, CursorPriority);
			if (SL_Task) SetTaskPri(SL_Task, DisplayPriority);
			break;
		    }
		ReplyMsg(message);
		}
	    if (ZaphodDisplay.FirstDisplay == NULL)
		TheFatLadySings(auxToolsPort);
	    Permit();
	    }
	}
}



TheFatLadySings(auxToolsPort)
struct MsgPort *auxToolsPort;	
{
    struct AuxToolsMessage *message;

    Forbid();
	     
    DeathLock(&ZaphodDisplay.DisplayListLock);

    KillAuxTools();

    if (auxToolsPort)
	{
	DrainPort(auxToolsPort);
	DeletePort(auxToolsPort);
	}

    if (AuxToolsFinalPort)
	{
	while (InputTask) WaitTOF();
	while (CursorTask) WaitTOF();
	while (SL_Task) WaitTOF();

	if ((message = CreateExtIO(AuxToolsFinalPort,
		sizeof(struct AuxToolsMessage)) ) == NULL)
	    Alert(ALERT_NO_MEMORY, NULL);
	else 
	    PutMsg(AuxToolsFinalPort, message);
	}

    RemTask(0);
}


OpenAuxTools(display)
struct Display *display;
{
    Forbid();
    if (FindPort(AUX_TOOLS_PORT) == NULL)
	{
	if (CreateTask(AUX_TASK_NAME, 0, AuxToolsManager, 1000) == NULL)
	    Alert(ALERT_NO_MEMORY, display);
	if ((AuxToolsFinalPort = CreatePort("AuxToolsFinalPort", 0)) == NULL)
	    Alert(ALERT_NO_MEMORY, NULL);
	}
    Permit();

    TalkWithZaphod(display, ADD_DISPLAY, display);
}



CloseAuxTools(display)
struct Display *display;
{
    if (FindPort(AUX_TOOLS_PORT))
	TalkWithZaphod(display, REMOVE_DISPLAY, display);
}



PutNewPriority(display, priority)
struct Display *display;
SHORT priority; 
{
    if (FindPort(AUX_TOOLS_PORT))
	TalkWithZaphod(display, NEW_PRIORITY, priority);
}



KillAuxTools()
{
    Forbid();

    if (InputTask) Signal(InputTask, InputSuicideSignal);
    if (CursorTask) Signal(CursorTask, CursorSuicideSignal);
    if (SL_Task) Signal(SL_Task, SL_SuicideSignal);

    Permit();
}



MakeAuxTools()
{
    if ((InputTask = FindTask(INPUT_TASK_NAME)) == NULL)
	if (CreateTask(INPUT_TASK_NAME, DisplayPriority + 1, 
		InputMonitor, 2000) == NULL)
	    Alert(ALERT_NO_MEMORY, NULL);

    if ((CursorTask = FindTask(CURSOR_TASK_NAME)) == NULL)
	if (CreateTask(CURSOR_TASK_NAME, CursorPriority,
		Cursor, 2000) == NULL)
	    Alert(ALERT_NO_MEMORY, NULL);
}



struct DisplayList *GetDisplayList(display)
struct Display *display;
{
    struct DisplayList *displayList;

    displayList = (struct DisplayList *)TalkWithZaphod(display, 
	   GET_DISPLAYLIST, NULL);

    return (displayList);
}



LONG TalkWithZaphod(display, auxtype, auxarg)
struct Display *display;
SHORT auxtype;
LONG auxarg;
{
    struct MsgPort *port, *auxport;
    struct AuxToolsMessage *message;
    LONG returnval;

    if ((port = CreatePort(0, 0)) == NULL)
	{
	Alert(ALERT_NO_MEMORY, display);
	return(NULL);
	}

    if ((message = CreateExtIO(port, sizeof(struct AuxToolsMessage)) ) == NULL)
	{
	Alert(ALERT_NO_MEMORY, display);
	return (NULL);
	}

    message->AuxType = auxtype;
    message->AuxArg = auxarg;

    while ((auxport = FindPort(AUX_TOOLS_PORT)) == NULL) WaitTOF();
    PutMsg(auxport, message);
    WaitPort(port);
    returnval = message->AuxArg;
    DeleteExtIO(message, sizeof(struct AuxToolsMessage));
    DeletePort(port);

    return (returnval);
}


/* ************************************************************************
 * A NOTE ABOUT THE ABANDONBLINKING OF Link/UnlinkDisplay();
 *
 * Whenever the display list is being mangled, hard-cancel the cursor
 * task's use of the list.  This allows the cursor task to not be required
 * to lock the display list before using it, which means that it doesn't
 * have to compete with the input monitor task for access to the display
 * list.  This is highly desirable, since the cursor task runs a *lot*
 * and it is locked out also by needing access to the Display's Lock,
 * which, since the cursor task runs at a lower priority than the
 * display rendering task, can take some considerable amount of time,
 * all during which the input monitor would be required to wait and
 * wait.  So instead, the cursor task ignores the DisplayListLock, 
 * and checks the AbandonBlink variable for a warning that the display
 * list may have changed.  This is not the most elegant systemy solution,
 * but it works very well and I for one am willing to sacrifice system
 * perfection for performance when its safe to do so and the gain is
 * as great as this is.
 *
 * AbandonBlink = TRUE;
 */

LinkDisplay(display)
struct Display *display;
/* See the note above */
{
    Forbid();

    /* Start out by telling the cursor task that we're about to muck 
     * about with the display list, so it should abandon any blink link
     * traversals it may be attempting.
     */
    AbandonBlink = TRUE;
    Lock(&ZaphodDisplay.DisplayListLock);
    display->NextDisplay = ZaphodDisplay.FirstDisplay;
    ZaphodDisplay.FirstDisplay = display;
    Unlock(&ZaphodDisplay.DisplayListLock);

    Permit();
}


UnlinkDisplay(display)
struct Display *display;
/* See the note above */
{
    struct Display *workdisplay;

    Forbid();

    /* Start out by telling the cursor task that we're about to muck 
     * about with the display list, so it should abandon any blink link
     * traversals it may be attempting.
     */
    AbandonBlink = TRUE;

    Lock(&ZaphodDisplay.DisplayListLock);

    if (ZaphodDisplay.FirstDisplay == display)
	ZaphodDisplay.FirstDisplay = display->NextDisplay;
    else
	{
	workdisplay = ZaphodDisplay.FirstDisplay;

	while (workdisplay->NextDisplay != display)
	    workdisplay = workdisplay->NextDisplay;
	workdisplay->NextDisplay = display->NextDisplay;
	}
    Unlock(&ZaphodDisplay.DisplayListLock);

    Permit();
}


