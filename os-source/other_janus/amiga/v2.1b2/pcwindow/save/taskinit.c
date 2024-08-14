
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



BOOL   InitDisplayTask(display)
REGIST   struct   Display *display;
{
   REGIST   BOOL retvalue;

   retvalue = FALSE;

   display->TaskAddress = FindTask(0);

   if ((display->TaskPort = CreatePort("DisplayPort", 0)) == NULL)
      {
      MyAlert(ALERT_NO_INPUTPORT, NULL);
      goto DONE;
      }

   display->JanusStart = (UBYTE *)GetJanusStart();

   if (NOT GetNewSuperWindow(display, TRUE)) goto DONE;

   GetPCDisplay(display);
   CopyCRTData(display, TRUE);
   GetCRTSetUp(display, TRUE, FALSE);

   display->CursorOldRow = display->CursorRow;
   display->CursorOldCol = display->CursorCol;
   NewCursings(display, display->CursorOldCol, display->CursorOldRow);

   if (FlagIsClear(display->Modes, COLOR_DISPLAY))
      DisplayItems[DISPLAY_ITEMS_COUNT - DEPTH_ITEM - 1].SubItem
            = &NumberOfTextColorsSubItems[1];

   if (NOT OpenDisplay(display)) goto DONE;
   if (NOT OpenDisplayWindow(display, NULL, NULL)) goto DONE;

   retvalue = TRUE;

DONE:
   return(retvalue);
}

 

