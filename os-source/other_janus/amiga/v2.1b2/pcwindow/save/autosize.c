
/* *** autosize.c ************************************************************
 * 
 * The Auto-sizing window routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 16 Apr 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

void SmallSize();
void FullSize();
void RecordLastSmall();

void SmallSize(display)
REGISTER struct Display *display;
{
   REGISTER SHORT x, y, width, height;
   SHORT screenheight, i;
   REGISTER struct SuperWindow *superwindow;
   REGISTER struct Window *window;
   BOOL sizexfirst, sizeyfirst, needpermit;

   Forbid();
   needpermit = TRUE;
   MyLock(&display->DisplayLock);

   if ((superwindow = display->FirstWindow) == NULL) goto UNLOCK_RETURN;
   if ((window = superwindow->DisplayWindow) == NULL) goto UNLOCK_RETURN;

   screenheight = window->WScreen->Height;

   LockLayerInfo(&window->WScreen->LayerInfo);
   LockLayer(&window->WScreen->LayerInfo, window->RPort->Layer);

   x = superwindow->LastSmallX + superwindow->LastSmallWidth;
   if (x > window->MaxWidth)
      superwindow->LastSmallWidth = window->MaxWidth
            - superwindow->LastSmallX;

   y = superwindow->LastSmallY + superwindow->LastSmallHeight;
   if (y > screenheight)
      superwindow->LastSmallHeight = screenheight - superwindow->LastSmallY;
 
   x = superwindow->LastSmallX - window->LeftEdge;
   y = superwindow->LastSmallY - window->TopEdge;
   width = superwindow->LastSmallWidth - window->Width;
   height = superwindow->LastSmallHeight - window->Height;

   if (FlagIsClear(display->Modes, BORDER_VISIBLE))
      {
      if (FlagIsSet(display->Modes, MEDIUM_RES)) i = PAL_MEDIUM_OFFSET;
      else i = PAL_HIGH_OFFSET;
      height -= i;
      }

   /* Now, if the destination position and the current size will go
    * beyond the screen boundaries, then size first, else move first
    */
   if (superwindow->LastSmallX  + window->Width > window->MaxWidth)
      sizexfirst = TRUE;
   else sizexfirst = FALSE;

   if (superwindow->LastSmallY  + window->Height > window->MaxHeight)
      sizeyfirst = TRUE;
   else sizeyfirst = FALSE;

   if (sizexfirst && sizeyfirst)
      {
      if (width || height) SizeWindow(window, width, height);
      }
   else
      {
      if (sizexfirst)
         if (width) SizeWindow(window, width, 0);
      if (sizeyfirst)
         if (height) SizeWindow(window, 0, height);
      }

   if ((x) || (y)) MoveWindow(window, x, y);

   if ((NOT sizexfirst) && (NOT sizeyfirst))
      {
      if (width || height) SizeWindow(window, width, height);
      }
   else
      {
      if (NOT sizexfirst)
         if (width) SizeWindow(window, width, 0);
      if (NOT sizeyfirst)
         if (height) SizeWindow(window, 0, height);
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
         MyRefreshWindowFrame(window, display);
         }
      }

UNLOCK_RETURN:
   Unlock(&display->DisplayLock); 
   if (needpermit)
      {
      Permit();
      needpermit = FALSE;
      }
}



void FullSize(display)
REGISTER struct Display *display;
{
   REGISTER SHORT x, y, width, height;
   SHORT screenheight;
/*
   SHORT i;
*/
   REGISTER struct SuperWindow *superwindow;
   REGISTER struct Window *window;

   Forbid();
   MyLock(&display->DisplayLock);

   if ((superwindow = display->FirstWindow) == NULL) goto UNLOCK_RETURN;
   if ((window = superwindow->DisplayWindow) == NULL) goto UNLOCK_RETURN;

   LockLayerInfo(&window->WScreen->LayerInfo);
   LockLayer(&window->WScreen->LayerInfo, window->RPort->Layer);

   RecordLastSmall(display, superwindow, window);

   screenheight = window->WScreen->Height;

   x = window->LeftEdge;
   y = window->TopEdge;
   if (y + window->MaxHeight <= screenheight) y = 0;
   else y = y - (screenheight - window->MaxHeight);

   width = window->MaxWidth - window->Width;
   height = window->MaxHeight - window->Height;

   if ((x) || (y)) MoveWindow(window, -x, -y);

   if ((width > 0) || (height > 0))
      SizeWindow(window, width, height);

   UnlockLayer(window->RPort->Layer);
   UnlockLayerInfo(&window->WScreen->LayerInfo);

   MoveSizeWait();

UNLOCK_RETURN:
   Unlock(&display->DisplayLock);   
   Permit();
}


void RecordLastSmall(display, superwindow, window)
REGISTER struct Display *display;
REGISTER struct SuperWindow *superwindow;
REGISTER struct Window *window;
{
   REGISTER SHORT i;

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
}


