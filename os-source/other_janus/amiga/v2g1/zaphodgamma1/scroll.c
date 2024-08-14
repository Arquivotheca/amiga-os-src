
/* *** scroll.c **************************************************************
 * 
 * Scroll routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 27 Jan 88   -RJ         Finally put the first stuff into this file
 * 27 Feb 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"
#include <janus/services.h>



#define PC_SCROLL_FLAG	0x01


extern	USHORT	*GetScrollAddress();

ULONG	ScrollSignal = 0;
struct	SetupSig *ScrollSig = NULL;
USHORT	*ScrollFlags = NULL;
USHORT	flagscroll;



VOID	SetupScroll()
{
	/* Others have been promised that ScrollFlags will point to some 
	 * real memory location sooner or later.
	 */

	ScrollSignal = ZAllocSignal();

	if (ScrollSig = SetupJanusSig(JSERV_SCROLL,
			CreateSignalNumber(ScrollSignal), 0, NULL))
		{
		ScrollFlags = GetScrollAddress();
/*???kprintf("ScrollFlags=%lx *ScrollFlags=%04lx JanusStart=%lx\n", */
/*???ScrollFlags, *ScrollFlags, GetJanusStart());*/
		}
	else

		ScrollFlags = &flagscroll;
}



VOID	CleanupScroll()
{
	if (ScrollSig) CleanupJanusSig(ScrollSig);
	if (ScrollSignal) ZFreeSignal(ScrollSignal);
}



Scroll()
{

	if (*ScrollFlags) StartScroll();
	else EndScroll();
}



StartScroll()
{
	struct SuperWindow *superwindow;
	struct Window *window;
	struct Display *display;

/*???kprintf("s");*/
return;

	Forbid();
	if (window = GetActiveWindow(NULL))
		{
		display = ((struct WindowExtension *)window->UserData)->Display;
		TalkWithZaphod(display, DEADEN_BLINK, NULL);
		if (display->CursorFlags & CURSOR_ON)
			{
			Curse(display);
			ClearFlag(display->CursorFlags, CURSOR_ON);
			}

		if (FlagIsSet(display->Modes, SELECTED_AREA)) goto BREAK;
		if (superwindow = display->FirstWindow)
			if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) 
				goto BREAK;
		SetFlag(display->Modes2, SCROLLING);

		ScrollRaster(window->RPort, 
				0, CHAR_HEIGHT, 
				window->BorderLeft, window->BorderTop,
				window->Width - window->BorderRight, 
				window->Height - window->BorderBottom);

		ScrollTextBuffer(display);

BREAK:		;

		}
	Permit();
}



EndScroll()
{
	struct DisplayList *displaylist;
	struct Display *display;

/*???kprintf("e");*/
return;

	Forbid();
	TalkWithZaphod(NULL, REVIVE_BLINK, NULL);
	if (displaylist = GetDisplayList(NULL))
		{
		Lock(&displaylist->DisplayListLock);
		display = displaylist->FirstDisplay;
		while (display)
			{
			ClearFlag(display->Modes2, SCROLLING);
			display = display->NextDisplay;
			}
		Unlock(&displaylist->DisplayListLock);
		}
	Permit();
}



struct Window *GetActiveWindow(display)
struct Display *display;
/* This routine Forbid()s and Permit()s, but you probably want to do the same 
 * around a call to this routine to help make sure that the window is still 
 * valid when you get your hands on the pointer.
 */
{
	struct DisplayList *displaylist;
	struct Display *nextdisplay;
	struct SuperWindow *superwindow;
	struct Window *returnwindow;

	returnwindow = NULL;

	Forbid();
	if (displaylist = GetDisplayList(display))
		{
		Lock(&displaylist->DisplayListLock);

		nextdisplay = displaylist->FirstDisplay;
		while (nextdisplay)
			{
			Lock(&nextdisplay->DisplayLock);
			superwindow = nextdisplay->FirstWindow;
			while (superwindow)
				{
				if (FlagIsSet(superwindow->DisplayWindow->Flags,
						WINDOWACTIVE))
					{
					returnwindow = superwindow->DisplayWindow;
					Unlock(&nextdisplay->DisplayLock);
					Unlock(&displaylist->DisplayListLock);
					goto DISPLAY_DONE;
					}
				superwindow = superwindow->NextWindow;
				}
			Unlock(&nextdisplay->DisplayLock);
			nextdisplay = nextdisplay->NextDisplay;
			}

		Unlock(&displaylist->DisplayListLock);
		}

DISPLAY_DONE:
	Permit();

	return(returnwindow);
}



