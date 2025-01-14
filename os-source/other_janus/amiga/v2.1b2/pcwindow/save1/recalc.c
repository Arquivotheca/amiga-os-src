
/* *** recalc.c **************************************************************
 * 
 * Recalculate Display Parameter Routines for Zaphod (Amiga-Blue Project)
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 27 Feb 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

#define  DBG_RECALC_DISPLAY_PARMS_ENTER   1
#define  DBG_RECALC_DISPLAY_PARMS_HEIGHT  1
#define  DBG_RECALC_DISPLAY_PARMS_RETURN  1

void RecalcDisplayParms(display)
struct Display *display;
/* Whenever the display size has changed, this routine recalculates
 * the Display position variables based on the current size and
 * proportional gadget settings.  If we're in text mode, the text size
 * and position variables are recalculated too.
 */
{
   ULONG temp;
   SHORT i, endrow, endcol;
   struct SuperWindow *superwindow;
   struct Window *window;

#if (DEBUG1 & DBG_RECALC_DISPLAY_PARMS_ENTER)
   kprintf("Recalc.c:     RecalcDisplayParms(display=0x%lx)\n",display);
#endif

   if ((superwindow = display->FirstWindow) == NULL) goto RECALC_DONE;
   if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) goto RECALC_DONE;
   if ((window = superwindow->DisplayWindow) == NULL) goto RECALC_DONE;  

   if (display->Modes & BORDER_VISIBLE)
      {
      display->DisplayWidth = window->GZZWidth;
      display->DisplayHeight = window->GZZHeight;
      }
   else
      {
      display->DisplayWidth = window->Width;
/*BBB      display->DisplayHeight = window->Height;*/
           display->DisplayHeight = window->Height
            - window->BorderTop - window->BorderBottom;
      }

   /* Start by getting the amount of free play in the size along this axis */
   if (display->Modes & MEDIUM_RES) i = 320 - display->DisplayWidth;
   else i = 640 - display->DisplayWidth;

   FixPropPots(&superwindow->HorizGadget);
   FixPropPots(&superwindow->VertGadget);

   temp = superwindow->HorizInfo.HorizPot * i;      /* Get first partial */
   display->DisplayStartCol = (temp + 0x8000) >> 16;
   endcol = display->DisplayStartCol + display->DisplayWidth - 1;
   
   /* Start by getting the amount of free play in the size along this axis */
/*BBB   i = 200 - display->DisplayHeight;*/
   i = ZAPHOD_HEIGHT - display->DisplayHeight;

#if (DEBUG3 & DBG_RECALC_DISPLAY_PARMS_HEIGHT)
   kprintf("recalc.c:     ZAPHOD_HEIGHT=%ld DisplayHeight=%ld i=%ld\n",ZAPHOD_HEIGHT,display->DisplayHeight,i);
#endif

   temp = superwindow->VertInfo.VertPot * i;           /* Get first partial */
   display->DisplayStartRow = (temp + 0x8000) >> 16;
   endrow = display->DisplayStartRow + display->DisplayHeight - 1;

   if (display->Modes & TEXT_MODE)
      {
      /* We're in text mode and the size or position of the window has
       * changed, so we need to recalculate the parameters describing
       * the block of text that is visible in the window
       */       
      display->TextStartCol = (display->DisplayStartCol / CHAR_WIDTH) 
            & -2;  /* Char pairs */
      display->TextStartRow = display->DisplayStartRow / CHAR_HEIGHT;
      display->TextWidth = (( (endcol + (CHAR_WIDTH * 2 - 1))
            / CHAR_WIDTH ) 
            & -2) - display->TextStartCol;      
      display->TextHeight = ((endrow + CHAR_HEIGHT)
            / CHAR_HEIGHT) - display->TextStartRow;
      display->TextBaseRow = (display->TextStartRow * CHAR_HEIGHT) 
            - display->DisplayStartRow;
      display->TextBaseRow += window->BorderTop;
      display->TextBaseCol = (display->TextStartCol * CHAR_WIDTH) 
            - display->DisplayStartCol;
      display->TextBaseCol += window->BorderLeft;
      }

RECALC_DONE: ;

#if (DEBUG2 & DBG_RECALC_DISPLAY_PARMS_RETURN)
   kprintf("Recalc.c:     RecalcDisplayParms() Returns!\n");
#endif
}
