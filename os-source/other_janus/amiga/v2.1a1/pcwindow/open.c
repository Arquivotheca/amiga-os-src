
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
#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

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
   SHORT    width, depth, mode, displaymodes;
   REGIST   struct SuperWindow *superwindow;
   struct   Screen *screen;
   BOOL     retvalue;

#if (DEBUG1 & DBG_OPEN_DISPLAY_ENTER)
   kprintf("Open.c:       OpenDisplay(0x%lx)\n",display);
#endif

   retvalue = FALSE;

   MyLock(&display->DisplayLock);

   superwindow  = display->FirstWindow;
   displaymodes = display->Modes;

   if (displaymodes & MEDIUM_RES)
   {
      width = 320;
      mode = NULL;
   } else {
      width = 640;
      mode = HIRES;
   }

   superwindow->DisplayNewScreen.Width = width;
   superwindow->DisplayNewScreen.ViewModes = mode;

   /* Now, use width to calculate the gettin'-small variables. */
   superwindow->LastSmallX = superwindow->LastSmallWidth = width / 2;
   superwindow->LastSmallY = 50;
   superwindow->LastSmallHeight = 100;

   if (displaymodes & TEXT_MODE)
   {
      depth = superwindow->TextDepth;
   } else {
      /* graphics-mode can be either 4 colors (2 planes) or 2 (1 plane) */
      if (displaymodes & MEDIUM_RES) 
	     depth = 2;
      else 
	     depth = 1;
   }

   superwindow->DisplayNewScreen.Depth = superwindow->DisplayDepth = depth;

   if ((displaymodes & TEXT_MODE) && (width == 640) && (depth == 2)
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

   SetColorColors(display, -1);

#if 0
   if (FlagIsClear(display->Modes, OPEN_SCREEN))
      SetThosePrefs();
#endif

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
   SHORT displaywidth, heightadjust, i;
   struct NewWindow *newwindow;
   struct Window *window;
   struct WindowExtension *windowext;
   struct SuperWindow *superwindow;
   BOOL success;
   struct Screen TScreen;
   struct Screen *pubscreen=NULL;

#if (DEBUG1 & DBG_OPEN_DISPLAY_WINDOW_ENTER)
   kprintf("Open.c:       OpenDisplayWindow(display=0x%lx,port=0x%lx,idcmp=0x%lx)\n",display,port,idcmp);
#endif

   window    = NULL;
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
   if (display->Modes & OPEN_SCREEN)
   {
      ((struct ScreenExt *)
	               superwindow->DisplayScreen->UserData)->UserCount += 1;

      newwindow->Screen = superwindow->DisplayScreen;
      newwindow->Type   = CUSTOMSCREEN;
      ScreenToFront(superwindow->DisplayScreen);
   } else {
      newwindow->Type = WBENCHSCREEN;
      WBenchToFront();
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
      if(IntuitionBase->LibNode.lib_Version >= 36)
      {
         pubscreen=LockPubScreen("Workbench");
         if(pubscreen)
         {
            TScreen.Height=pubscreen->Height;
            TScreen.Width =pubscreen->Width;
         }
      }
   }

#if (DEBUG2 & DBG_OPEN_DISPLAY_WINDOW_TOP_EDGE_CALC) 
   kprintf("TScreen.Width =%ld\n",TScreen.Width); 
   kprintf("TScreen.Height=%ld\n",TScreen.Height); 
#endif

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
      displaywidth = 320;

      superwindow->HorizGadget.LeftEdge = 4;
      superwindow->HorizGadget.Width    = -16;
      superwindow->HorizGadget.TopEdge  = -9;
      superwindow->HorizGadget.Height   = 9;
      superwindow->VertGadget.LeftEdge  = -11;
      superwindow->VertGadget.Width     = 11;
      superwindow->VertGadget.TopEdge   = 11;
      superwindow->VertGadget.Height    = -21;
   } else {
      displaywidth = 640;

      superwindow->HorizGadget.LeftEdge = 4;
      superwindow->HorizGadget.Width    = -22;
      superwindow->HorizGadget.TopEdge  = -8;
      superwindow->HorizGadget.Height   = 8;
      superwindow->VertGadget.LeftEdge  = -16;
      superwindow->VertGadget.Width     = 16;
      superwindow->VertGadget.TopEdge   = 11;
      superwindow->VertGadget.Height    = -20;
   }

   if(FlagIsSet(display->Modes,MEDIUM_RES))
   {
      newwindow->MaxWidth=displaywidth+4+13;
	  newwindow->MaxHeight=200+11+11;
   } else {
      newwindow->MaxWidth=displaywidth+4+18;
	  newwindow->MaxHeight=200+11+10;
   }

   /***********************/
   /* First open defaults */
   /***********************/
   if(FlagIsClear(display->Modes2,OPENED_ONCE))
   {
      newwindow->LeftEdge = 0;
      newwindow->TopEdge  = 0;

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
   if(newwindow->MaxWidth>TScreen.Width)
      newwindow->MaxWidth=TScreen.Width;

   if(newwindow->MaxHeight>TScreen.Height)
      newwindow->MaxHeight=TScreen.Height;

   if((newwindow->Width+newwindow->LeftEdge)>TScreen.Width)
   {
      SHORT xx;

      xx=((SHORT)TScreen.Width)-((SHORT)newwindow->Width);

	  if(xx<0)						/* Too wide */
	  {
	     newwindow->LeftEdge=0;
		 newwindow->Width=TScreen.Width;
	  } else {
         newwindow->LeftEdge=xx;
      }
   }

   if((newwindow->Height+newwindow->TopEdge)>TScreen.Height)
   {
      SHORT xx;

      xx=((SHORT)TScreen.Height)-((SHORT)newwindow->Height);

	  if(xx<0)						/* Too wide */
	  {
	     newwindow->TopEdge=0;
		 newwindow->Height=TScreen.Height;
	  } else {
         newwindow->TopEdge=xx;
      }
   }


#if (DEBUG2 & DBG_OPEN_DISPLAY_WINDOW_TOP_EDGE_CALC) 
   kprintf("Open.c:       OpenDisplayWindow: LeftEdge = %ld\n",newwindow->LeftEdge);
   kprintf("Open.c:       OpenDisplayWindow: TopEdge  = %ld\n",newwindow->TopEdge);
   kprintf("Open.c:       OpenDisplayWindow: Width    = %ld\n",newwindow->Width);
   kprintf("Open.c:       OpenDisplayWindow: Height   = %ld\n",newwindow->Height);
   kprintf("Open.c:       OpenDisplayWindow: MinWidth = %ld\n",newwindow->MinWidth);
   kprintf("Open.c:       OpenDisplayWindow: MinHeight= %ld\n",newwindow->MinHeight);
   kprintf("Open.c:       OpenDisplayWindow: MaxWidth = %ld\n",newwindow->MaxWidth);
   kprintf("Open.c:       OpenDisplayWindow: MaxHeight= %ld\n",newwindow->MaxHeight);
#endif

   /*******************/
   /* Open the window */
   /*******************/
   if ( (window = OpenWindow(newwindow)) == NULL)
   {
      MyAlert(ALERT_NO_WINDOW_MEMORY, NULL);
      goto UNLOCK_RETURN;
   }
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

   SetMenuStrip(window, &MenuHeaders[MENU_HEADERS_COUNT - 1]);

   InitDisplayBitMap(display, superwindow, displaywidth);

   ClearFlag(display->CursorFlags, CURSOR_ON);
   SetFlag(display->CursorFlags, CURSOR_MOVED);

   ModifyDisplayProps(display);

   if (display->Modes & BORDER_VISIBLE)
   {
      RenderWindow(display, TRUE, TRUE, TRUE, TRUE);
   } else {
      if(FlagIsSet(display->Modes,OPEN_SCREEN))
	  {
         SetFlag(display->Modes, BORDER_VISIBLE);
         BorderPatrol(display, BORDER_OFF, TRUE);
	  } else {
	     /* Be smart! */
         SetFlag(display->Modes, BORDER_VISIBLE);
         BorderPatrol(display, BORDER_OFF, TRUE);
         BorderPatrol(display, BORDER_ON, TRUE);
	  }
   }

   SetColorColors(display, CountWBWindows());

   if (FlagIsClear(display->Modes, OPEN_SCREEN))
   {
      ChangeWBWindowCount(1);
   }

   success = TRUE;

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

/****i* PCWindow/SetLeftOfCloseGadget **************************************
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

#if 0
void SetLeftOfCloseGadget(window)
struct Window *window;
{
   struct Gadget *gadget;

#if (DEBUG1 & DBG_SET_LEFT_OF_CLOSE_GADGET_ENTER)
   kprintf("Open.c:       SetLeftOfCloseGadget(window=0x%lx)\n",window);
#endif

   gadget = window->FirstGadget;

   while (gadget)
   {
      if ((gadget->GadgetType & ~GADGETTYPE) == CLOSE)
      {
         LeftOfCloseGadget = gadget->LeftEdge + gadget->Width;
         gadget = NULL;
      }
      else 
	     gadget = gadget->NextGadget;
   }
}
#endif

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
		    i = 1;
         else 
		    i = 0;
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

#if (DEBUG1 & DBG_SET_COLOR_COLORS_ENTER)
   kprintf("Open.c:       SetColorColors(display=0x%lx,wbwindows=0x%lx)\n",display,wbwindows);
#endif

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
         if ( FlagIsClear(display->Modes, OPEN_SCREEN)
               &&  (wbwindows == 0) )
         {
            /* This is Workbench viewport, so grab 
             * copy of colors until later
             */
            for (i = 0; i < 4; i++)
               display->WBColors[i] = GetRGB4( vport->ColorMap, i);
            SetFlag(display->Modes2, WBCOLORS_GRABBED);
         }

         colortable = GetColorAddress(display->Modes,
               superwindow->DisplayDepth);
   
         for (i = 0; i < (1 << superwindow->DisplayDepth); i++)
         {
            red   = *(colortable + i) >> 8;
            green = *(colortable + i) >> 4;
            blue  = *(colortable + i);
            SetRGB4(vport, i, red, green, blue);
         }
      }
   }
}
