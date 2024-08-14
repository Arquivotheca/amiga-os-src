
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
   {
      superwindow->LastSmallWidth = window->MaxWidth-superwindow->LastSmallX;
   }

   y = superwindow->LastSmallY + superwindow->LastSmallHeight;

   if (y > screenheight)
   {
      superwindow->LastSmallHeight = screenheight - superwindow->LastSmallY;
   }
 
   x = superwindow->LastSmallX - window->LeftEdge;
   y = superwindow->LastSmallY - window->TopEdge;

   width  = superwindow->LastSmallWidth  - window->Width;
   height = superwindow->LastSmallHeight - window->Height;

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

   if (sizexfirst && sizeyfirst) 
   {
      if (width || height) 
	     SizeWindow(window, width, height);
   } else {
      if (sizexfirst)
         if (width) 
		    SizeWindow(window, width, 0L);

      if (sizeyfirst) 
	  {
         if (height)
		    SizeWindow(window, 0L, height); 
      }
   }

   if ((x) || (y)) 
   {
	  MoveWindow(window, x, y);
   }

   if ((NOT sizexfirst) && (NOT sizeyfirst)) 
   {
      if (width || height) 
	     SizeWindow(window, width, height);
   } else {
      if (NOT sizexfirst) 
	  {
         if (width) 
		    SizeWindow(window, width, 0L);
      }

      if (NOT sizeyfirst) 
	  {
         if (height)
		    SizeWindow(window, 0L, height);
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
   if ((width < 0L) && (height < 0L))
   {
      if (FlagIsSet(display->Modes, BORDER_VISIBLE | PAL_PRESENCE))
      {
         /* MyRefreshWindowFrame(window, display); */
         RefreshWindowFrame(window);
      }
   }

UNLOCK_RETURN:
   ClearFlag(display->Modes2,FULLSIZE);
   SetFlag(display->CursorFlags, CURSOR_MOVED);
   Unlock(&display->DisplayLock); 
   if (needpermit) Permit();
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
   LONG x, y, width, height;
   LONG screenheight,screenwidth,le,te;
   struct SuperWindow *superwindow;
   struct Window *window;


   Forbid();
   MyLock(&display->DisplayLock);

   if ((superwindow = display->FirstWindow) == NULL) goto UNLOCK_RETURN;
   if ((window = superwindow->DisplayWindow) == NULL) goto UNLOCK_RETURN;

   LockLayerInfo(&window->WScreen->LayerInfo);
   LockLayer((long)&window->WScreen->LayerInfo, window->RPort->Layer);


   screenheight = window->WScreen->Height;
   screenwidth  = window->WScreen->Width;

   /*kprintf("sh=%ld sw=%ld le=%ld te=%ld w=%ld h=%ld mw=%ld mh=%ld\n",
      (LONG)screenheight,
	  (LONG)screenwidth,
	  (LONG)window->LeftEdge,
	  (LONG)window->TopEdge,
	  (LONG)window->Width,
	  (LONG)window->Height,
	  (LONG)window->MaxWidth,
	  (LONG)window->MaxHeight);*/

   /***********************************************/
   /* Move up and left if necessary before sizing */
   /***********************************************/

   if(window->LeftEdge+window->MaxWidth<=screenwidth)
      x=0L;
   else
   {
      le=screenwidth-window->MaxWidth;

      if(le<=0L)
	     x=(window->LeftEdge*-1L);
	  else
	     x=(window->LeftEdge-le)*-1L;
   }

   if(window->TopEdge+window->MaxHeight<=screenheight)
      y=0L;
   else
   {
      te=screenheight-window->MaxHeight;

      if(te<=0L)
	     y=window->TopEdge*-1L;
	  else
	     y=(window->TopEdge-te)*-1L;
   }

#if 0
   x = window->LeftEdge;
   y = window->TopEdge;

   if (y + window->MaxHeight <= screenheight) 
      y = 0L;
   else 
      y = y - (screenheight - window->MaxHeight);

   width  = window->MaxWidth  - window->Width;
   height = window->MaxHeight - window->Height;
#endif

   if(window->Width !=
      (window->MaxWidth<=screenwidth?window->MaxWidth:screenwidth))
   {
      width=(window->MaxWidth<=screenwidth?window->MaxWidth:screenwidth)-
	        window->Width;
   } else
      width=0L;

   if(window->Height !=
      (window->MaxHeight<=screenheight?window->MaxHeight:screenheight))
   {
      height=(window->MaxHeight<=screenheight?window->MaxHeight:screenheight)-
	        window->Height;
   } else
      height=0L;

   /*kprintf("x=%ld y=%ld w=%ld h=%ld\n",x,y,width,height);*/

   if ((x!=0L) || (y!=0L) || (width > 0L) || (height > 0L))
      RecordLastSmall(display, superwindow, window);

   if ((x!=0L) || (y!=0L)) 
   { 
      MoveWindow(window,x,y);
   }

   if ((width > 0L) || (height > 0L))
   {
      SizeWindow(window,width,height);
   }

   UnlockLayer(window->RPort->Layer);
   UnlockLayerInfo(&window->WScreen->LayerInfo);



UNLOCK_RETURN:
   SetFlag(display->Modes2,FULLSIZE);
   SetFlag(display->CursorFlags, CURSOR_MOVED); /* any effect ? */
   Unlock(&display->DisplayLock);   
   Permit();
   MoveSizeWait();
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

   if ((display == NULL) || (superwindow == NULL) || (window == NULL))
      return;

   if((window->Width==window->MaxWidth)&&(window->Height==window->MaxHeight))
      return;

   superwindow->LastSmallX      = window->LeftEdge;
   superwindow->LastSmallY      = window->TopEdge;
   superwindow->LastSmallWidth  = window->Width;
   superwindow->LastSmallHeight = window->Height;
#if 0
   if (FlagIsClear(display->Modes, BORDER_VISIBLE)) 
   {
      if (FlagIsSet(display->Modes, MEDIUM_RES)) 
	     i = PAL_MEDIUM_OFFSET;
      else 
	     i = PAL_HIGH_OFFSET;

      superwindow->LastSmallHeight += i;
   }
#endif
}
