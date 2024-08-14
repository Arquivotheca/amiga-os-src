
/* ***************************************************************************
 * 
 * Initialization Routines for the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 18 Mar 86	=RJ Mical=	Created this file from init.c
 *
 * **************************************************************************/

#include "zaphod.h"
								    

extern struct MenuItem DisplayItems[], DepthofTextDisplaySubItems[];

						

InitDisplayTask(display)
struct Display *display;
{
    display->TaskAddress = FindTask(0);

    if ((display->TaskPort = CreatePort("DisplayPort", 0)) == NULL)
	Abort(ALERT_NO_INPUTPORT, NULL);

#ifdef JANUS
    display->JanusStart = GetJanusStart();
#endif

    GetNewSuperWindow(display);

    GetPCDisplay(display);
    CopyCRTData(display, TRUE);
    GetCRTSetUp(display);

    display->CursorOldRow = display->CursorRow;
    display->CursorOldCol = display->CursorCol;
    NewCursings(display, display->CursorOldCol, display->CursorOldRow);

    if ((display->Modes & COLOR_DISPLAY) == 0)
	DisplayItems[DISPLAY_ITEMS_COUNT - DEPTH_ITEM - 1].SubItem
		= &DepthofTextDisplaySubItems[1];

    OpenDisplay(display);
    OpenDisplayWindow(display);
}

 


