
/* *** privacy.c ************************************************************
 * 
 * The Window Privacy Routines
 *
 * Copyright (C) 1987, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 22 Jan 87   =RJ Mical=  Created this file
 *
 * *************************************************************************/

#include "zaphod.h"


WantsPrivacy(display)
struct Display *display;
{
	struct SuperWindow *superwindow;
	BOOL closerup;
	struct ScreenExt *sext;

	superwindow = display->FirstWindow;
	if (FlagIsSet(superwindow->Flags, WANTS_PRIVACY)) return;
	SetFlag(superwindow->Flags, WANTS_PRIVACY);

	closerup = FALSE;
	if (FlagIsClear(display->Modes, OPEN_SCREEN))
		closerup = TRUE;
	else 
		{
		sext = ((struct ScreenExt *)superwindow->DisplayScreen->UserData);
		if (sext->UserCount > 1) closerup = TRUE;
		}
	if (closerup)
		RevampDisplay(display, display->Modes, TRUE, FALSE);

	sext = ((struct ScreenExt *)superwindow->DisplayScreen->UserData);
	SetFlag(sext->Flags, PRIVATE_SCREENING);
	superwindow->DisplayScreen->DefaultTitle = &ScreenUnsharedTitle[0];
	SetWindowTitles(display->FirstWindow->DisplayWindow, -1,
			&ScreenUnsharedTitle[0]);
}



WantsCommunity(display)
struct Display *display;
{
	struct SuperWindow *superwindow;
	struct ScreenExt *sext;
	BOOL revamp;

	superwindow = display->FirstWindow;
	if (FlagIsClear(superwindow->Flags, WANTS_PRIVACY)) return;
	ClearFlag(superwindow->Flags, WANTS_PRIVACY);

	revamp = FALSE;
	if (FindSharedScreen(display, &superwindow->DisplayNewScreen))
		revamp = TRUE;
	else if ( FlagIsSet(superwindow->DisplayModes, TEXT_MODE) 
			&& FlagIsClear(superwindow->Flags, MEDIUM_RES)
			&& (superwindow->DisplayDepth == 2) )
		revamp = TRUE;

	if (revamp)
		RevampDisplay(display, display->Modes, TRUE, FALSE);
	else
		{
		sext = ((struct ScreenExt *)superwindow->DisplayScreen->UserData);
		ClearFlag(sext->Flags, PRIVATE_SCREENING);
		superwindow->DisplayScreen->DefaultTitle = &ScreenTitle[0];
		SetWindowTitles(display->FirstWindow->DisplayWindow, -1,
				&ScreenTitle[0]);
		}
}





