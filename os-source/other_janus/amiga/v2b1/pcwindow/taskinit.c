
/* *** taskinit.c ************************************************************
 * 
 * Initialization Routines for the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 Mar 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"
																	

extern struct MenuItem DisplayItems[], NumberOfTextColorsSubItems[];

												

InitDisplayTask(display)
struct Display *display;
{
	display->TaskAddress = FindTask(0);

	if ((display->TaskPort = CreatePort("DisplayPort", 0)) == NULL)
		Abort(ALERT_NO_INPUTPORT, NULL);

	display->JanusStart = GetJanusStart();

	GetNewSuperWindow(display);

	GetPCDisplay(display);
	CopyCRTData(display, TRUE);
	GetCRTSetUp(display, TRUE, FALSE);

	display->CursorOldRow = display->CursorRow;
	display->CursorOldCol = display->CursorCol;
	NewCursings(display, display->CursorOldCol, display->CursorOldRow);

	if ((display->Modes & COLOR_DISPLAY) == 0)
		DisplayItems[DISPLAY_ITEMS_COUNT - DEPTH_ITEM - 1].SubItem
				= &NumberOfTextColorsSubItems[1];

	OpenDisplay(display);
	OpenDisplayWindow(display, NULL, NULL);
}

 

