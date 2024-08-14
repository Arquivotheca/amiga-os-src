
/**** open.c ***************************************************************
 * 
 * Window & Screen Open Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 28 Feb 86   =RJ Mical=  Created this file
 *
 ***************************************************************************/

#include "zaphod.h"
#include <intuition/intuitionbase.h>
#include <exec/memory.h>

#define  DBG_OPEN_DISPLAY_ENTER                 1
#define  DBG_OPEN_DISPLAY_WINDOW_ENTER          1
#define  DBG_OPEN_DISPLAY_WINDOW_TOP_EDGE_CALC  1
#define  DBG_SET_LEFT_OF_CLOSE_GADGET_ENTER     1
#define  DBG_GET_COLOR_ADDRESS_ENTER            1
#define  DBG_SET_COLOR_COLORS_ENTER             1

/****i* PCWindow/OpenDisplay ******************************************
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


BOOL OpenDisplay(display)
struct Display *display;
{
   SHORT              width, 
                      depth, 
			          mode, 
			          displaymodes;
   struct SuperWindow *superwindow;
   struct Screen      *screen;
   BOOL               retvalue;
   struct Screen      TScreen;

   retvalue = FALSE;

   MyLock(&display->DisplayLock);

   superwindow  = display->FirstWindow;
   displaymodes = display->Modes;

   if (displaymodes & MEDIUM_RES) {
      width = 320L;
      mode = NULL;
   }
   else {
      width = 640L;
      mode = HIRES;
   }

   GetScreenData((char *)&TScreen,sizeof(struct Screen),
                         WBENCHSCREEN,NULL);


   superwindow->DisplayNewScreen.Width = width;
   superwindow->DisplayNewScreen.ViewModes = mode;

#if 0
   if(FlagIsSet(display->Modes,PAL_PRESENCE) || specialpal) 
   {
      superwindow->DisplayNewScreen.Height = 256L;
   } else {
      superwindow->DisplayNewScreen.Height = 200L;
   }
#endif
   superwindow->DisplayNewScreen.Height = 200L;

   /* Now, use width to calculate the getting small variables. */
   superwindow->LastSmallX = superwindow->LastSmallWidth = width / 2L;
   superwindow->LastSmallY = 0L; /* was 50 */
   superwindow->LastSmallHeight = 100L;

   if(displaymodes & TEXT_MODE) 
   {
      depth = superwindow->TextDepth;
   } else {
      /* graphics-mode can be either 4 colors (2 planes) or 2 (1 plane) */
      if (displaymodes & MEDIUM_RES)
	  depth = 2L;
         else 
	  depth = 1L;
   }

   superwindow->DisplayNewScreen.Depth = superwindow->DisplayDepth = depth;

   if ((displaymodes & TEXT_MODE) && (width == 640L)
     && ((depth==TScreen.BitMap.Depth)||
	    (FlagIsClear(display->Modes,COLOR_DISPLAY)))
     && FlagIsClear(superwindow->Flags, WANTS_PRIVACY)) 
   {
      /* Oh goodie, we can use the Workbench */
      ClearFlag(display->Modes, OPEN_SCREEN);
      superwindow->DisplayScreen = NULL;
   } else {
      screen = (struct Screen *)GetExtraScreen(display, superwindow);
      if (superwindow->DisplayScreen = screen) 
	  {
         SetFlag(display->Modes, OPEN_SCREEN);
      } else {
         MyAlert(ALERT_NO_EXTRA_SCREEN_MEMORY, NULL);
         ClearFlag(display->Modes, OPEN_SCREEN);
         goto DONE;
      }
   }

   SetColorColors(display, -1L);

   retvalue = TRUE;

DONE:
   Unlock(&display->DisplayLock);
   return(retvalue);
}

/****i* PCWindow/OpenDisplayWindow *****************************************
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
* If this window is due for a CUSTOMSCREEN, before calling this routine, 
* the Screen must be initialized
*
*/

BOOL OpenDisplayWindow(display, port, idcmp,usepresets)
struct Display *display;
struct MsgPort *port;
ULONG idcmp;
BOOL usepresets;
{
   SHORT displaywidth;
/*   SHORT heightadjust; */
   struct NewWindow *newwindow;
   struct Window *window;
   struct WindowExtension *windowext;
   struct SuperWindow *superwindow;
   BOOL success;
   struct Screen TScreen;
   struct Screen *pubscreen=NULL;
   SHORT MODES;


/*    window    = NULL; */
   windowext = NULL;
   success   = FALSE;

   /****************/
   /* Lock display */
   /****************/
   MyLock(&display->DisplayLock);

   GetPCDisplay(display);

   superwindow = display->FirstWindow;
   newwindow   = &superwindow->DisplayNewWindow;

   /**************************/
   /* CUSTOM or WBENCHSCREEN */
   /**************************/
   if (display->Modes & OPEN_SCREEN) {
      ((struct ScreenExt *)
	               superwindow->DisplayScreen->UserData)->UserCount += 1L;

      newwindow->Screen = superwindow->DisplayScreen;
      newwindow->Type   = CUSTOMSCREEN;
	  /*CurrentSettings.WorkbenchScreen=FALSE;*/
/*      ScreenToFront(superwindow->DisplayScreen); */
   }
   else {
     newwindow->Type = WBENCHSCREEN;
	 /*CurrentSettings.WorkbenchScreen=TRUE;*/
/*     WBenchToFront(); */
   }

   /*************************/
   /* Get Screen dimensions */
   /*************************/
   /* kprintf("newwindow->Type=%lx\n",newwindow->Type); */

   GetScreenData((char *)&TScreen,sizeof(struct Screen),
                         newwindow->Type,newwindow->Screen);


   /***************************************************/
   /* If V36 then GetScreen lies so try LockPubScreen */
   /***************************************************/
   if(FlagIsClear(display->Modes,OPEN_SCREEN))
   {
      if(IntuitionBase->LibNode.lib_Version >= 36L)
      {
         pubscreen=LockPubScreen(NULL);
         if(pubscreen)
         {
            TScreen.Height=pubscreen->Height;
            TScreen.Width =pubscreen->Width;
         }
      }
   }
   /*kprintf("TScreen.Height = %ld w = %ld\n",(ULONG)TScreen.Height,(ULONG)TScreen.Width);*/

   /*************************************/
   /* Mono or Color Active window title */
   /*************************************/
   if (FlagIsSet(display->Modes, COLOR_DISPLAY))
   {
      newwindow->Title = &ColorActiveTitle[0];
   } else {
      newwindow->Title = &MonoActiveTitle[0];
   }

   /*************************/
   /* Set Width and Gadgets */
   /*************************/
   if (FlagIsSet(display->Modes, MEDIUM_RES)) 
   {
      displaywidth = 320L;

      superwindow->HorizGadget.LeftEdge = 4L;
      superwindow->HorizGadget.Width    = -16L;
      superwindow->HorizGadget.TopEdge  = -9L;
      superwindow->HorizGadget.Height   = 9L;
      superwindow->VertGadget.LeftEdge  = -11L;
      superwindow->VertGadget.Width     = 11L;
      superwindow->VertGadget.TopEdge   = 11L;
      superwindow->VertGadget.Height    = -21L;
   } else {
      displaywidth = 640L;

      superwindow->HorizGadget.LeftEdge = 4L;
      superwindow->HorizGadget.Width    = -22L;
      superwindow->HorizGadget.TopEdge  = -8L;
      superwindow->HorizGadget.Height   = 8L;
      superwindow->VertGadget.LeftEdge  = -16L;
      superwindow->VertGadget.Width     = 16L;
      superwindow->VertGadget.TopEdge   = 11L;
      superwindow->VertGadget.Height    = -20L;
   }

   /***********************/
   /* First open defaults */
   /***********************/
   if(FlagIsClear(display->Modes2,OPENED_ONCE)) 
   {
      newwindow->LeftEdge = 0L;
      newwindow->TopEdge  = 0L;

      newwindow->Width  = newwindow->MaxWidth;
      newwindow->Height = newwindow->MaxHeight;
   }

   /***********************************/
   /* newwindow settings from presets */
   /***********************************/
   if(FlagIsSet(display->Modes2, WINDOW_PRESETS)&&usepresets) 
   {
      if(FlagIsSet(display->Modes,COLOR_DISPLAY)) 
	  {
	     newwindow->LeftEdge = display->PresetColorLeftEdge;
	     newwindow->TopEdge  = display->PresetColorTopEdge;
	     newwindow->Width    = display->PresetColorWidth;
	     newwindow->Height   = display->PresetColorHeight;
      } else {
	     newwindow->LeftEdge = display->PresetMonoLeftEdge;
	     newwindow->TopEdge  = display->PresetMonoTopEdge;
	     newwindow->Width    = display->PresetMonoWidth;
	     newwindow->Height   = display->PresetMonoHeight;
      }
   }

   /*****************/
   /* Sanity checks */
   /*****************/
   if(newwindow->MaxWidth>TScreen.Width)newwindow->MaxWidth=TScreen.Width;

   if(newwindow->MaxHeight>TScreen.Height)newwindow->MaxHeight=TScreen.Height;

   if((newwindow->Width+newwindow->LeftEdge)>TScreen.Width) {
      SHORT xx;

      xx=((SHORT)TScreen.Width)-((SHORT)newwindow->Width);

	  if(xx<0L)						/* Too wide */
	  {
	     newwindow->LeftEdge=0L;
		 newwindow->Width=TScreen.Width;
	  } else {
         newwindow->LeftEdge=xx;
      }
   }

   if((newwindow->Height+newwindow->TopEdge)>TScreen.Height) {
      SHORT xx;

      xx=((SHORT)TScreen.Height)-((SHORT)newwindow->Height);

	  if(xx<0L)						/* Too wide */
	  {
	     newwindow->TopEdge=0;
		 newwindow->Height=TScreen.Height;
	  } else {
         newwindow->TopEdge=xx;
      }
   }

   /*******************/
   /* Open the window */
   /*******************/

   if ( (window = OpenWindow(newwindow)) == NULL) 
   {
      MyAlert(ALERT_NO_WINDOW_MEMORY, NULL);
      goto UNLOCK_RETURN;
   }

   window->MaxHeight=200L+window->BorderTop+window->BorderBottom;
   window->MaxWidth=displaywidth+window->BorderLeft+window->BorderRight;
   superwindow->VertGadget.TopEdge   = window->BorderTop;
   superwindow->VertGadget.Height    = -window->BorderTop-window->BorderBottom;

   JustOpened=TRUE;
   SetFlag(display->Modes2,OPENED_ONCE);

   /* Debug(); */

   /************************/
   /* Get window extension */
   /************************/
   if ( (windowext = (struct WindowExtension *)AllocMem(
         sizeof(struct WindowExtension), 
         MEMF_CLEAR)) == NULL)
   {
      MyAlert(ALERT_NO_EXTENSION_WINDOW_MEMORY, NULL);
      goto UNLOCK_RETURN;
   }

   window->UserData = (BYTE *)windowext;
   windowext->Window = window;
   windowext->Display = display;

   if (window->UserPort = port) ModifyIDCMP(window, idcmp);

   superwindow->DisplayWindow = window;


   InitDisplayBitMap(display, superwindow, displaywidth);

   ClearFlag(display->CursorFlags, CURSOR_ON);
   SetFlag(display->CursorFlags, CURSOR_MOVED);

   ModifyDisplayProps(display);

   /*kprintf("Before b\n");*/
   if (display->Modes & BORDER_VISIBLE)
   {
      /*CurrentSettings.BordersOn=TRUE;*/
      RenderWindow(display, TRUE, TRUE, TRUE, TRUE);
	  /*kprintf("render\n");*/
   } else {
      if(FlagIsSet(display->Modes,OPEN_SCREEN))
	  {
	     /*kprintf("os\n");*/
         SetFlag(display->Modes, BORDER_VISIBLE);
         BorderPatrol(display, BORDER_OFF, TRUE);
         /*CurrentSettings.BordersOn=FALSE;*/
	  } else {
	     /* Be smart! */
	     /*kprintf("smart\n");*/
         SetFlag(display->Modes, BORDER_VISIBLE);
         BorderPatrol(display, BORDER_OFF, TRUE);
         BorderPatrol(display, BORDER_ON, TRUE);
         /*CurrentSettings.BordersOn=TRUE;*/
	  }
   }
   /*kprintf("After b\n");*/

#if 0
   kprintf("Before b\n");

   if(display->Modes & COLOR_DISPLAY)
   {
      MODES=display->PresetColorFlags;
   } else {
      MODES=display->PresetMonoFlags;
   }
         /*MyLock(&display->DisplayLock);*/


   if(MODES & PRESET_BORDER_ON)
   {
      BorderPatrol(display, BORDER_ON, TRUE);
      if (FlagIsSet(display->Modes, PAL_PRESENCE))
         SetFlag(display->Modes, SQUELCH_NEWSIZE);
      SetFlag(display->CursorFlags, CURSOR_MOVED);
      /*CurrentSettings.BordersOn=TRUE;*/
   } else {
      if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
      {
         MyAlert(ALERT_FROZEN_BORDER, display);
      } else {
         if(FlagIsSet(display->Modes,OPEN_SCREEN))
         {
            BorderPatrol(display, BORDER_OFF, TRUE);
            SetFlag(display->CursorFlags, CURSOR_MOVED);
         }
      }
   }

   kprintf("After b\n");
#endif

       /*Unlock(&display->DisplayLock);*/

   SetColorColors(display, CountWBWindows());

   if (FlagIsClear(display->Modes, OPEN_SCREEN))
   {
      ChangeWBWindowCount(1);
   }

   success = TRUE;

   /* bring proper display to the front */

   if (display->Modes & OPEN_SCREEN) {
	ScreenToFront(superwindow->DisplayScreen);
   }
   else WBenchToFront();

   /*ClearMenuStrip(window);*/
   SetMenuState(display);

   MenuHeaders[2].Height=window->WScreen->Font->ta_YSize+4L;
   MenuHeaders[2].LeftEdge=0L;
   MenuHeaders[2].Width=TextLength(&window->WScreen->RastPort,"Project  ",9L);

   MenuHeaders[1].Height=window->WScreen->Font->ta_YSize+4L;
   MenuHeaders[1].LeftEdge=MenuHeaders[2].LeftEdge+MenuHeaders[2].Width;
   MenuHeaders[1].Width=TextLength(&window->WScreen->RastPort,"Display  ",9L);

   MenuHeaders[0].Height=window->WScreen->Font->ta_YSize+4L;
   MenuHeaders[0].LeftEdge=MenuHeaders[1].LeftEdge+MenuHeaders[1].Width;
   MenuHeaders[0].Width=TextLength(&window->WScreen->RastPort,"Edit  ",6L);

   SetMenuStrip(window, &MenuHeaders[MENU_HEADERS_COUNT - 1L]);

UNLOCK_RETURN:
   if(pubscreen) UnlockPubScreen(NULL,pubscreen);
   Unlock(&display->DisplayLock);
   if (NOT success)
   {
      if (window) 
      {
         Forbid();
         if (window->UserPort) 
		    DrainPort(window->UserPort);
         window->UserPort = NULL;
         ModifyIDCMP(window, NULL);
         Permit();
         CloseWindow(window);
      }
      if (windowext) 
         FreeMem(windowext, sizeof(struct WindowExtension));
   }
   return(success);
}

/****i* PCWindow/GetColorAddress ******************************************
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

USHORT *GetColorAddress(modes, depth)
SHORT modes, depth;
{
   USHORT *colortable;
   SHORT i;

#if (DEBUG1 & DBG_GET_COLOR_ADDRESS_ENTER)
   kprintf("Open.c:       GetColorAddress(modes=0x%lx,depth=0x%lx)\n",modes,depth);
#endif

   /* If we're in graphics and high-res, use the special 
    * high-res table 
    */
   if (modes & TEXT_MODE)
   {
      /* OK, text mode is easy */ 
      switch (depth)
      {
         case 1:
            colortable = &TextOnePlaneColors[0];
            break;
         case 2:
            colortable = &TextTwoPlaneColors[0];
            break;
         case 3:
            colortable = &TextThreePlaneColors[0];
            break;
         case 4:
         default:
            colortable = &TextFourPlaneColors[0];
            break;
      }
   } else {
      /* This is graphics.  Now, low or high resolution? */
      if (modes & MEDIUM_RES)
      {
         if (modes & PALETTE_1)  
		    i = 1L;
         else 
		    i = 0L;
         colortable = &LowGraphicsColors[ColorIntenseIndex][i][0];
      }
      else
         /* this is high-res graphics */
         colortable = &HighGraphicsColors[0];
   }

   return(colortable);
}

/****i* PCWindow/SetColorColors ******************************************
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

VOID SetColorColors(display, wbwindows)
struct Display *display;
SHORT   wbwindows;
/* The wbwindows count specifies the number of workbench windows out there.  
 * This is used when opening/closing displays.  If you're calling this 
 * routine just to set colors (for instance, from GetCRTSetup()) then 
 * use the default value of -1.  
 */
{
   SHORT red, green, blue, i;
   USHORT *colortable;
   struct ViewPort *vport;
   struct SuperWindow *superwindow;

   /*kprintf("Open.c: SetColorColors(display=0x%lx,wbwindows=0x%lx)\n",display,wbwindows);*/

   if (superwindow = display->FirstWindow)
   {
      vport = NULL;
      if (superwindow->DisplayWindow)
         vport = &superwindow->DisplayWindow->WScreen->ViewPort;
      else 
	     if (superwindow->DisplayScreen)
            vport = &superwindow->DisplayScreen->ViewPort;

      if (vport) 
	  {
         if ( FlagIsClear(display->Modes, OPEN_SCREEN)&&(wbwindows == 0)) 
		 {
            /* This is Workbench viewport, so grab 
             * copy of colors until later
             */
            for (i = 0L; i < 4L; i++)
               display->WBColors[i] = GetRGB4( vport->ColorMap, i);
            SetFlag(display->Modes2, WBCOLORS_GRABBED);
	  }

	 /* if we're opening on the workbench screen, don't play with colors */
	 if ( FlagIsClear(display->Modes, OPEN_SCREEN))return;
         colortable = GetColorAddress(display->Modes,
		superwindow->DisplayDepth);
   
         for (i = 0L; i < (1L << superwindow->DisplayDepth); i++) {
            red   = *(colortable + i) >> 8L;
            green = *(colortable + i) >> 4L;
            blue  = *(colortable + i);
            SetRGB4(vport, i, red, green, blue);
         }
      }
   }
}
