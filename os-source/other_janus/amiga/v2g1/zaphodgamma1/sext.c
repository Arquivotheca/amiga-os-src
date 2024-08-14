
/* *** sext.c ****************************************************************
 * 
 * ScreenExt Routines or the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1987, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 19 Jan 87   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"




struct Screen *GetExtraScreen(display, superwindow)
struct Display *display;
struct SuperWindow *superwindow;
{
	struct Screen *screen;
	struct ScreenExt *sext;
	struct NewScreen *newscreen;

	newscreen = &superwindow->DisplayNewScreen;

	if (FlagIsClear(superwindow->Flags, WANTS_PRIVACY))
		screen = FindSharedScreen(display, newscreen);
	else screen = NULL;
	if (screen == NULL)
		{
		if (FlagIsClear(superwindow->Flags, WANTS_PRIVACY))
			newscreen->DefaultTitle = &ScreenTitle[0];
		else
			newscreen->DefaultTitle = &ScreenUnsharedTitle[0];

		screen = OpenScreen(newscreen);
		if (screen)
			{
			sext = (struct ScreenExt *)AllocMem(
					sizeof(struct ScreenExt), MEMF_CLEAR);
			if (sext == NULL)
				{
				CloseScreen(screen);
				Alert(ALERT_NO_MEMORY, NULL);
				return(NULL);
				}

			screen->UserData = (UBYTE *)sext;
			sext->Screen = screen;

			if (FlagIsSet(superwindow->Flags, WANTS_PRIVACY))
				{
				SetFlag(sext->Flags, PRIVATE_SCREENING);
				}

			LinkExtraScreen(display, screen);
			}
		}
	return(screen);
}



ReleaseExtraScreen(display, screen)
struct Display *display;
struct Screen *screen;
{
	struct NewScreen *newscreen;
			
	Lock(&display->DisplayLock);
	if (display->FirstWindow)
		{
		screen = display->FirstWindow->DisplayScreen;
		display->FirstWindow->DisplayScreen = NULL;
		if (screen)
			{
			newscreen = &display->FirstWindow->DisplayNewScreen;
			newscreen->TopEdge = screen->TopEdge;
			if ( ((struct ScreenExt *)screen->UserData)->UserCount <= 0 )
				{
				UnlinkExtraScreen(display, screen);
				FreeMem(screen->UserData, sizeof(struct ScreenExt));
				CloseScreen(screen);
				}
			}
		}
	Unlock(&display->DisplayLock);
}



LinkExtraScreen(display, screen)
struct Display *display;
struct Screen *screen;
{
	struct DisplayList *displaylist;
	struct ScreenExt *sext;

	displaylist = GetDisplayList(display);
	if (NOT Lock(&displaylist->ScreenListLock)) return;

	sext = (struct ScreenExt *)screen->UserData;

	sext->NextScreenExt = displaylist->FirstScreenExt;
	displaylist->FirstScreenExt = sext;

	Unlock(&displaylist->ScreenListLock);
}



UnlinkExtraScreen(display, screen)
struct Display *display;
struct Screen *screen;
{
	struct DisplayList *displaylist;
	struct ScreenExt *sext, *nextsext;

	displaylist = GetDisplayList(display);
	if (NOT Lock(&displaylist->ScreenListLock)) return;

	sext = (struct ScreenExt *)screen->UserData;

	if (displaylist->FirstScreenExt == sext)
		displaylist->FirstScreenExt = sext->NextScreenExt;
	else
		{
		nextsext = displaylist->FirstScreenExt;
		while (nextsext->NextScreenExt != sext)
			nextsext = nextsext->NextScreenExt;
		nextsext->NextScreenExt = sext->NextScreenExt;	  
		}

	Unlock(&displaylist->ScreenListLock);
}



