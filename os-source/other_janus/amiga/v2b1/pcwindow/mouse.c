
/* **************************************************************************
 * 
 * MOUSE.COM Support routines
 *
 * Copyright (C) 1988, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  -------------------------------------------------
 * 25 Jan 88  =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <devices/inputevent.h>
#include <devices/input.h>



/* MOUSE: a label for this whole file */



#define LEFT_BUTTON_DOWN	0x0001
#define RIGHT_BUTTON_DOWN	0x0002
#define MIDDLE_BUTTON_DOWN	0x0004

#define MOUSE_INTERRUPT_KEYCODE	0xEE


USHORT MouseFlags = 0;



TogglePCMouse(fromauxtask, displaylocked)
BOOL fromauxtask, displaylocked;
{
	ToggleFlag(MouseFlags, PC_HAS_MOUSE);
	if (FlagIsSet(MouseFlags, PC_HAS_MOUSE))
		SetPCMouse(fromauxtask, displaylocked);
	else
		ClearPCMouse(fromauxtask, displaylocked);
}



SetPCMouse(fromauxtask, displaylocked)
BOOL fromauxtask, displaylocked;
{
	SetFlag(MouseFlags, PC_HAS_MOUSE);
	PCMouseGrunt(TRUE, fromauxtask, displaylocked);
}



ClearPCMouse(fromauxtask, displaylocked)
BOOL fromauxtask, displaylocked;
{
	ClearFlag(MouseFlags, PC_HAS_MOUSE);
	PCMouseGrunt(FALSE, fromauxtask, displaylocked);
}



PCMouseGrunt(setit, fromauxtask, displaylocked)
BOOL setit;
BOOL fromauxtask, displaylocked;
{
	struct Display *display;
	struct DisplayList *displaylist;
	struct SuperWindow *superwindow;
	BOOL displaylistsafe, displaysafe;

	if (fromauxtask) displaylist = &ZaphodDisplay;
	else displaylist = GetDisplayList(NULL);
	if (displaylist)
		{
		if (fromauxtask) displaylistsafe = TRUE;
		else displaylistsafe = Lock(&displaylist->DisplayListLock);
		if (displaylistsafe)
			{
			display = displaylist->FirstDisplay;
			while (display)
				{
				if (displaylocked) displaysafe = TRUE;
				else displaysafe = Lock(&display->DisplayLock);
				if (displaysafe)
					{
					superwindow = display->FirstWindow;
					while (superwindow)
						{
if (setit)
	{
SetFlag(display->Modes2, PCMOUSE_TEST);
	SetFlag(superwindow->DisplayWindow->Flags, DELTAMOVE);
	SetPointer(superwindow->DisplayWindow, &NullPointer,
			1, 1, 0, 0);
	}
else
	{
ClearFlag(display->Modes2, PCMOUSE_TEST);
	ClearFlag(superwindow->DisplayWindow->Flags, DELTAMOVE);
	ClearPointer(superwindow->DisplayWindow);
	}
						superwindow = superwindow->NextWindow;
						}
					if (NOT displaylocked) 
						Unlock(&display->DisplayLock);
					}
				display = display->NextDisplay;
				}

			if (NOT fromauxtask) 
				Unlock(&displaylist->DisplayListLock);
			}
		}
}



struct InputEvent *MouseEventToPC(event)
struct InputEvent *event;
{
	struct InputEvent *firstevent, *recentevent;

	firstevent = recentevent = NULL;

	while (event)
		{
		if (event->ie_Class == IECLASS_RAWMOUSE)
			SendMouseEvent(event);
		else
			{
			if (firstevent == NULL) firstevent = event;
			if (recentevent) recentevent->ie_NextEvent = event;
			recentevent = event;
			}
		event = event->ie_NextEvent;
		}

	if (recentevent) recentevent->ie_NextEvent = NULL;
	return(firstevent);
}



SendMouseEvent(event)
struct InputEvent *event;
{
	SHORT qualifier;

	qualifier = event->ie_Qualifier;

	if (FlagIsSet(qualifier, IEQUALIFIER_LBUTTON))
		SetFlag(*PCMouseFlags, LEFT_BUTTON_DOWN);
	else
		ClearFlag(*PCMouseFlags, LEFT_BUTTON_DOWN);

	if (FlagIsSet(qualifier, IEQUALIFIER_RBUTTON))
		SetFlag(*PCMouseFlags, RIGHT_BUTTON_DOWN);
	else
		ClearFlag(*PCMouseFlags, RIGHT_BUTTON_DOWN);

	if (FlagIsSet(qualifier, IEQUALIFIER_MBUTTON))
		SetFlag(*PCMouseFlags, MIDDLE_BUTTON_DOWN);
	else
		ClearFlag(*PCMouseFlags, MIDDLE_BUTTON_DOWN);

	*PCMouseX += event->ie_X;
	*PCMouseY += event->ie_Y;

	if (KeyQueuer)
		{
/*???		(*KeyQueuer)(MOUSE_INTERRUPT_KEYCODE);*/
		}
}



SHORT pcmouseflags = 0;
SHORT pcmousex = 0;
SHORT pcmousey = 0;



MakePCMouseConnection()
{
	if (NOT MakeHandler())
		Alert(ALERT_INCOMPLETESYSTEM, NULL);

	PCMouseFlags = &pcmouseflags;
	PCMouseX = &pcmousex;
	PCMouseY = &pcmousey;
}



UnmakePCMouseConnection()
{
	UnmakeHandler();
}



