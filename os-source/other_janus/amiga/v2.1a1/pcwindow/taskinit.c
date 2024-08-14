
/**** taskinit.c ************************************************************
 * 
 * Initialization Routines for the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 18 Mar 86   =RJ Mical=  Created this file
 *
 ***************************************************************************/

#define LINT_ARGS
#define PRAGMAS

#include "zaphod.h"
#include <proto/exec.h>
#include <janus/jfuncs.h>

#define  DBG_INIT_DISPLAY_TASK_ENTER      1


extern struct MenuItem DisplayItems[], NumberOfTextColorsSubItems[];



BOOL   InitDisplayTask(display)
REGIST   struct   Display *display;
{
   REGIST   BOOL retvalue;

#if (DEBUG1 & DBG_INIT_DISPLAY_TASK_ENTER)
   kprintf("taskinit.c:   InitDisplayTask(display=0x%lx)\n",display);
#endif

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
#if 0
   kprintf("Display=%lx\n",display);
   kprintf("Display->FirstWindow=%lx\n",display->FirstWindow);
   kprintf("Display->FirstWindow->DisplayWindow=%lx\n",display->FirstWindow->DisplayWindow);
#endif
   /* NewCursings(display, display->CursorOldCol, display->CursorOldRow); */

   if (FlagIsClear(display->Modes, COLOR_DISPLAY))
      DisplayItems[DISPLAY_ITEMS_COUNT - DEPTH_ITEM - 1].SubItem
            = &NumberOfTextColorsSubItems[1];

   if (NOT OpenDisplay(display)) goto DONE;
   if (NOT OpenDisplayWindow(display, NULL, NULL,TRUE)) goto DONE;

   retvalue = TRUE;

DONE:
   return(retvalue);
}

 

