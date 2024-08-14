
/**** autosize.c ************************************************************
 * 
 * The Auto-sizing window routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 16 Apr 86   =RJ Mical=  Created this file
 *
 ***************************************************************************/

#include "zaphod.h"
#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>

#define  DBG_SMALL_SIZE_ENTER          1
#define  DBG_SMALL_SIZE_RETURN         1
#define  DBG_FULL_SIZE_ENTER           1
#define  DBG_FULL_SIZE_RETURN          1
#define  DBG_RECORD_LAST_SMALL_ENTER   1
#define  DBG_RECORD_LAST_SMALL_RETURN  1

/****i* PCWindow/SmallSize ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
VOID SmallSize(struct Display *display)
{
   SHORT x, y, width, height;
   SHORT screenheight;
   struct SuperWindow *superwindow;
   struct Window *window;
   BOOL sizexfirst, sizeyfirst, needpermit;

#if (DEBUG1 & DBG_SMALL_SIZE_ENTER)
   kprintf("Autosize.c:   SmallSize(display=0x%lx)\n",display);
#endif

   Forbid();
   needpermit = TRUE;
   MyLock(&display->DisplayLock);

   if ((superwindow = display->FirstWindow) == NULL) goto UNLOCK_RETURN;
   if ((window = superwindow->DisplayWindow) == NULL) goto UNLOCK_RETURN;

   screenheight = window->WScreen->Height;

   LockLayerInfo(&window->WScreen->LayerInfo);
   LockLayer((long)&window->WScreen->LayerInfo, window->RPort->Layer);

   x = superwindow->LastSmallX + superwindow->LastSmallWidth;
   if (x > window->MaxWidth)
      superwindow->LastSmallWidth = window->MaxWidth-superwindow->LastSmallX;

   y = superwindow->LastSmallY + superwindow->LastSmallHeight;
   if (y > screenheight)
      superwindow->LastSmallHeight = screenheight - superwindow->LastSmallY;
 
   x = superwindow->LastSmallX - window->LeftEdge;
   y = superwindow->LastSmallY - window->TopEdge;
   width = superwindow->LastSmallWidth - window->Width;
   height = superwindow->LastSmallHeight - window->Height;

#if 0
   if (FlagIsClear(display->Modes, BORDER_VISIBLE))
   {
      if (FlagIsSet(display->Modes, MEDIUM_RES)) 
	     i = PAL_MEDIUM_OFFSET;
      else 
	     i = PAL_HIGH_OFFSET;
      height -= i;
   }
#endif

   /* Now, if the destination position and the current size will go
    * beyond the screen boundaries, then size first, else move first
    */
   if (superwindow->LastSmallX  + window->Width > window->MaxWidth)
      sizexfirst = TRUE;
   else 
      sizexfirst = FALSE;

   if (superwindow->LastSmallY  + window->Height > window->MaxHeight)
      sizeyfirst = TRUE;
   else 
      sizeyfirst = FALSE;

   if (sizexfirst && sizeyfirst) {
      if (width || height) SizeWindow(window, width, height);
   } 
   else {
      if (sizexfirst)
         if (width) SizeWindow(window, width, 0);
      if (sizeyfirst) {
         if (height)SizeWindow(window, 0, height); 
      }
   }

   if ((x) || (y))MoveWindow(window, x, y);

   if ((NOT sizexfirst) && (NOT sizeyfirst)) {
      if (width || height) SizeWindow(window, width, height);
   } 
   else {
      if (NOT sizexfirst) {
         if (width) SizeWindow(window, width, 0);
      }
      if (NOT sizeyfirst) {
         if (height)SizeWindow(window, 0, height);
      }
   }

   UnlockLayer(window->RPort->Layer);
   UnlockLayerInfo(&window->WScreen->LayerInfo);

   needpermit = FALSE;
   Permit();


   /* Now, try to get around a bug in V1.1 Intuition:  when the SizeWindow()
    * function is called with an argument less than zero, the gadgets aren't
    * redrawn.  No problem, you say, just redraw the gadgets after 
    * calling SizeWindow().  Uh uh, bozo.  Calling SizeWindow() doesn't 
    * mean that your window has been sized, it only means that your window
    * will be sized next time Intuition gets control.  Now don't you wish
    * you had had time to make Intuition a device?  Anyway, the following
    * is totally from Kludge-City.  If all sizing was to smaller sizes,
    * the gadget imagery is sure to be totalled under 1.1, so call
    * WaitTOF() a few times to give Intuition a chance to get in there,
    * and then refresh the gadgets.  This isn't going to work 100% of the
    * time, but it should work 99.99% anyway.  Excuse me ...
    */
   MoveSizeWait();
   if ((width < 0) && (height < 0))
   {
      if (FlagIsSet(display->Modes, BORDER_VISIBLE | PAL_PRESENCE))
      {
         /* MyRefreshWindowFrame(window, display); */
         RefreshWindowFrame(window);
      }
   }

UNLOCK_RETURN:
   ClearFlag(display->Modes2,FULLSIZE);
   Unlock(&display->DisplayLock); 
   if (needpermit) Permit();

#if (DEBUG2 & DBG_SMALL_SIZE_RETURN)
   kprintf("Autosize.c:   SmallSize: Returns(VOID)\n");
#endif
}

/****i* PCWindow/FullSize ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID FullSize(struct Display *display)
{
   SHORT x, y, width, height;
   SHORT screenheight;
/*
   SHORT i;
*/
   struct SuperWindow *superwindow;
   struct Window *window;

#if (DEBUG1 & DBG_FULL_SIZE_ENTER)
   kprintf("Autosize.c:   FullSize(display=0x%lx)\n",display);
#endif

   Forbid();
   MyLock(&display->DisplayLock);

   if ((superwindow = display->FirstWindow) == NULL) goto UNLOCK_RETURN;
   if ((window = superwindow->DisplayWindow) == NULL) goto UNLOCK_RETURN;

   LockLayerInfo(&window->WScreen->LayerInfo);
   LockLayer((long)&window->WScreen->LayerInfo, window->RPort->Layer);

   RecordLastSmall(display, superwindow, window);

   screenheight = window->WScreen->Height;

   x = window->LeftEdge;
   y = window->TopEdge;
   if (y + window->MaxHeight <= screenheight) y = 0;
   else y = y - (screenheight - window->MaxHeight);

   width = window->MaxWidth - window->Width;
   height = window->MaxHeight - window->Height;

   if ((x) || (y)) 
      MoveWindow(window, -x, -y);

   if ((width > 0) || (height > 0))
      SizeWindow(window, width, height);

   UnlockLayer(window->RPort->Layer);
   UnlockLayerInfo(&window->WScreen->LayerInfo);

   MoveSizeWait();

UNLOCK_RETURN:
   SetFlag(display->Modes2,FULLSIZE);
   SetFlag(display->CursorFlags, CURSOR_MOVED); /* any effect ? */
   Unlock(&display->DisplayLock);   
   Permit();

#if (DEBUG2 & DBG_FULL_SIZE_RETURN)
   kprintf("Autosize.c:   FullSize: Returns(VOID)\n");
#endif
}

/****i* PCWindow/RecordLastSmall ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID RecordLastSmall(display, superwindow, window)
struct Display *display;
struct SuperWindow *superwindow;
struct Window *window;
{
   SHORT i;

#if (DEBUG1 & DBG_RECORD_LAST_SMALL_ENTER)
   kprintf("Autosize.c:   RecordLastSmall(display=0x%lx,superwindow=0x%lx,window=0x%lx)\n",display,superwindow,window);
#endif

   if ((display == NULL) || (superwindow == NULL) || (window == NULL))
      return;

   if ((window->Width == window->MaxWidth) 
         && (window->Height == window->MaxHeight))
      return;

   superwindow->LastSmallX = window->LeftEdge;
   superwindow->LastSmallY = window->TopEdge;
   superwindow->LastSmallWidth = window->Width;
   superwindow->LastSmallHeight = window->Height;

   if (FlagIsClear(display->Modes, BORDER_VISIBLE))
      {
      if (FlagIsSet(display->Modes, MEDIUM_RES)) i = PAL_MEDIUM_OFFSET;
      else i = PAL_HIGH_OFFSET;
      superwindow->LastSmallHeight += i;
      }

#if (DEBUG2 & DBG_RECORD_LAST_SMALL_RETURN)
   kprintf("Autosize.c:   RecordLastSmall: Returns(VOID)\n");
#endif
}
