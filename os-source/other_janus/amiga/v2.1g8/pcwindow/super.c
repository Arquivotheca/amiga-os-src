
/* *** super.c **************************************************************
 * 
 * Initialization Routines for the Display Tasks of the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 20 jul 89   -BK         Fixed SetPresets to use Current Screen Height
 *                         instead of newwindow->MaxHeight so windows can
 *                         open at the bottom of interlaced screens
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 18 Mar 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#define  DBG_INSTALL_FIRST_WINDOW_ENTER   1
#define  DBG_LINK_SUPER_WINDOW_ENTER      1
#define  DBG_SET_PRESETS_ENTER            1
#define  DBG_SET_PRESETS_TOP_EDGE_CALC    1
#define  DBG_SET_PRESETS_RETURN           1
#define  DBG_GET_NEW_SUPER_WINDOW_ENTER   1
#define  DBG_UNLINK_SUPER_WINDOW_ENTER    1
#define  DBG_SUPER_CLOSE_WINDOW_ENTER     1
#define  DBG_DESTROY_SUPER_WINDOW_ENTER   1

/****i* PCWindow/InstallFirstWindow ****************************************
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

VOID InstallFirstWindow(struct Display *display)
{
   struct SuperWindow *superwindow;

#if (DEBUG1 & DBG_INSTALL_FIRST_WINDOW_ENTER)
   kprintf("super.c:      InstallFirstWindow(display=0x%lx)\n",display);
#endif
      
   MyLock(&display->DisplayLock);
   if (superwindow = display->FirstWindow)
   {
      display->Modes = superwindow->DisplayModes;
      display->Modes2 = superwindow->DisplayModes2;
      RecalcDisplayParms(display);
   }
   Unlock(&display->DisplayLock);
}
  
/****i* PCWindow/LinkSuperWindow ******************************************
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

VOID LinkSuperWindow(struct Display *display,struct SuperWindow *superwindow)
{

#if (DEBUG1 & DBG_LINK_SUPER_WINDOW_ENTER)
   kprintf("super.c:      LinkSuperWindow(display=0x%lx,superwindow=0x%lx)\n",display,superwindow);
#endif

   MyLock(&display->DisplayLock);

   /* If we were scrolling, forget about it! 
    * This allows any new window to start refreshing its display
    * immediately.
    */
   if (display->FirstWindow)
   {
      display->FirstWindow->DisplayModes = display->Modes;
      display->FirstWindow->DisplayModes2 = display->Modes2;
   }

   superwindow->NextWindow = display->FirstWindow;
   display->FirstWindow = superwindow;
   InstallFirstWindow(display);

   Unlock(&display->DisplayLock);
}

/****i* PCWindow/SetPresets ******************************************
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

VOID SetPresets(display, newwindow, superwindow, presetWidth, presetHeight, 
                presetTopEdge, presetLeftEdge, presetDepth, presetFlags)
struct Display     *display;
struct NewWindow   *newwindow;
struct SuperWindow *superwindow;
SHORT              presetWidth;
SHORT              presetHeight;
SHORT              presetTopEdge;
SHORT              presetLeftEdge;
SHORT              presetDepth;
SHORT              presetFlags;
{
#if (DEBUG1 & DBG_SET_PRESETS_ENTER)
   kprintf("super.c:      SetPresets(display=0x%lx,newwindow=0x%lx,\n",display,newwindow);
   kprintf("                         superwindow=0x%lx,presetWidth=%ld,\n",superwindow,presetWidth);
   kprintf("                         presetHeight=%ld,presetTopEdge=%ld,\n",presetHeight,presetTopEdge);
   kprintf("                         presetLeftEdge=%ld,presetDepth=%ld,presetFlags=0x%lx)\n",presetLeftEdge,presetDepth,presetFlags);
#endif

   if (presetWidth <= newwindow->MaxWidth)
      newwindow->Width = presetWidth;

   if (presetHeight <= newwindow->MaxHeight)
      newwindow->Height = presetHeight;

#if (DEBUG3 & DBG_SET_PRESETS_TOP_EDGE_CALC)
   kprintf("Super.c:      presetTopEdge = %ld\n",presetTopEdge);
   kprintf("Super.c:      Height        = %ld\n",newwindow->Height);
   kprintf("Super.c:      MaxHeight     = %ld\n",newwindow->MaxHeight);
#endif

#if 0
   if (presetTopEdge + newwindow->Height   <= newwindow->MaxHeight)
   {
      newwindow->TopEdge = presetTopEdge;
#if (DEBUG3 & DBG_SET_PRESETS_TOP_EDGE_CALC)
   kprintf("Super.c:      new TopEdge = presetTopEdge\n");
#endif
   }
#endif
   {
      struct Screen TScreen;

      GetScreenData((char *)&TScreen,sizeof(struct Screen),
	                        newwindow->Type,newwindow->Screen);
      /* kprintf("TScreen.Height=%ld\n",TScreen.Height); */
      if (presetTopEdge + newwindow->Height   <= TScreen.Height)
      {
         newwindow->TopEdge = presetTopEdge;
      }
   }

   if (presetLeftEdge + newwindow->Width  <= newwindow->MaxWidth)
      newwindow->LeftEdge = presetLeftEdge;

   superwindow->TextDepth = presetDepth;

   if (FlagIsSet(presetFlags, PRESET_HIRES))
      SetFlag(superwindow->Flags, WINDOW_WAS_HIRES);
   else 
      ClearFlag(superwindow->Flags, WINDOW_WAS_HIRES);

   if (FlagIsSet(presetFlags, PRESET_BORDER_ON))
      SetFlag(display->Modes, BORDER_VISIBLE);
   else 
      ClearFlag(display->Modes, BORDER_VISIBLE);

   if (FlagIsSet(presetFlags, PRESET_PRIVACY))
      SetFlag(superwindow->Flags, WANTS_PRIVACY);
   else 
      ClearFlag(superwindow->Flags, WANTS_PRIVACY);

#if (DEBUG2 & DBG_SET_PRESETS_RETURN)
   kprintf("Super.c:      SetPresets: returns TopEdge = %ld\n",newwindow->TopEdge);
   kprintf("Super.c:      SetPresets: returns Height  = %ld\n",newwindow->Height);
#endif
}

/****i* PCWindow/GetNewSuperWindow ****************************************
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

BOOL GetNewSuperWindow(display, usePresets)
struct Display *display;
BOOL           usePresets;
/* When usePresets is TRUE, presets (if any) will be used with the 
 * super window 
 */
{ 
   struct SuperWindow *superwindow, *firstwindow;
   struct NewWindow   *newwindow;
   BOOL               retvalue;

#if (DEBUG1 & DBG_GET_NEW_SUPER_WINDOW_ENTER)
   kprintf("super.c:      GetNewSuperWindow(display=0x%lx,usePresets=0x%lx)\n",display,usePresets);
#endif

   retvalue = FALSE;
      
   MyLock(&display->DisplayLock);

   if ((superwindow = (struct SuperWindow *)AllocMem(
         sizeof(struct SuperWindow), MEMF_CLEAR)) == NULL)
   {
      MyAlert(ALERT_NO_SUPER_WINDOW_MEMORY,NULL);
      goto UNLOCK_RETURN;
   }

   superwindow->DisplayModes = display->Modes;
   superwindow->DisplayModes2 = display->Modes2;

   if (firstwindow = display->FirstWindow)
   {
      superwindow->TextDepth = firstwindow->TextDepth;
      superwindow->CursorSeconds = firstwindow->CursorSeconds;
      superwindow->CursorMicros = firstwindow->CursorMicros;
   } else {
      if (display->Modes & COLOR_DISPLAY)
         superwindow->TextDepth = DefaultColorTextDepth;
      else 
	     superwindow->TextDepth = 2;
      superwindow->CursorSeconds = DEFAULT_CURSOR_SECONDS;
      superwindow->CursorMicros = DEFAULT_CURSOR_MICROS;
   }

   superwindow->DisplayNewWindow = NewWindow;
   superwindow->DisplayNewScreen = NewScreen;

   newwindow = &superwindow->DisplayNewWindow;

   SetFlag(superwindow->Flags, WINDOW_WAS_HIRES);
   newwindow->Width = 640;

   superwindow->HorizInfo = HorizInfo;
   superwindow->VertInfo = VertInfo;
   superwindow->HorizGadget = HorizGadget;
   superwindow->VertGadget = VertGadget;

   superwindow->HorizGadget.NextGadget = NULL;
   superwindow->HorizGadget.GadgetRender = (APTR)&superwindow->HorizImage;
   superwindow->HorizGadget.SpecialInfo = (APTR)&superwindow->HorizInfo;
   superwindow->VertGadget.NextGadget = &superwindow->HorizGadget;
   superwindow->VertGadget.GadgetRender = (APTR)&superwindow->VertImage;
   superwindow->VertGadget.SpecialInfo = (APTR)&superwindow->VertInfo;
   newwindow->FirstGadget = &superwindow->VertGadget;

   LinkSuperWindow(display, superwindow);

   SetFlag(display->Modes, BORDER_VISIBLE);

   if ( usePresets && FlagIsSet(display->Modes2, WINDOW_PRESETS) )
   {
      if (FlagIsClear(display->Modes, COLOR_DISPLAY)
           && FlagIsSet(display->PresetMonoFlags, PRESETS_GOT))
         SetPresets(display, newwindow, superwindow, 
               display->PresetMonoWidth, 
               display->PresetMonoHeight, 
               display->PresetMonoTopEdge, 
               display->PresetMonoLeftEdge, 
               display->PresetMonoDepth, 
               display->PresetMonoFlags);
      else 
	     if (FlagIsSet(display->Modes, COLOR_DISPLAY)
           && FlagIsSet(display->PresetColorFlags, PRESETS_GOT))
            SetPresets(display, newwindow, superwindow, 
                  display->PresetColorWidth, 
                  display->PresetColorHeight, 
                  display->PresetColorTopEdge, 
                  display->PresetColorLeftEdge, 
                  display->PresetColorDepth, 
                  display->PresetColorFlags);

      RecalcDisplayParms(display);
   }

   retvalue = TRUE;

UNLOCK_RETURN:
   Unlock(&display->DisplayLock);
   return(retvalue);
}

/****i* PCWindow/UnlinkSuperWindow *****************************************
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

VOID UnlinkSuperWindow(display, superwindow)
struct Display *display;
struct SuperWindow *superwindow;
{
   struct   SuperWindow *nextwindow;

#if (DEBUG1 & DBG_UNLINK_SUPER_WINDOW_ENTER)
   kprintf("super.c:      UnlinkSuperWindow(display=0x%lx,superwindow=0x%lx)\n",display,superwindow);
#endif

   MyLock(&display->DisplayLock);

   /* If we were scrolling, forget about it! 
    * This allows any new window to start refreshing its display
    * immediately.
    */
   if (display->FirstWindow == superwindow)
   {
      superwindow->DisplayModes = display->Modes;
      superwindow->DisplayModes2 = display->Modes2;

      display->FirstWindow = superwindow->NextWindow;
      InstallFirstWindow(display);
   } else {
      nextwindow = display->FirstWindow;
      while (nextwindow->NextWindow != superwindow)
         nextwindow = nextwindow->NextWindow;
      nextwindow->NextWindow = superwindow->NextWindow;     
   }

   Unlock(&display->DisplayLock);
}

/****i* PCWindow/SuperCloseWindow ******************************************
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

VOID SuperCloseWindow(display)
struct   Display *display;
/* NOTE:  this routine decouples the IDCMP Port from the window before
 * closing the window.
 * NOTE:  this routine drains any input from the display task's port
 * before returning.
 */
{
   struct   SuperWindow *superwindow;
   struct   Window *window;
#if 0
   SHORT   i, i2;
   struct   ViewPort *vport;
#endif

#if (DEBUG1 & DBG_SUPER_CLOSE_WINDOW_ENTER)
   kprintf("super.c:      SuperCloseWindow(display=0x%lx)\n",display);
#endif

   MyLock(&display->DisplayLock);

   if (superwindow = display->FirstWindow)
   {
      if (window = superwindow->DisplayWindow)
      {
         superwindow->DisplayNewWindow.LeftEdge = window->LeftEdge;
         superwindow->DisplayNewWindow.TopEdge = window->TopEdge;
         superwindow->DisplayNewWindow.Width = window->Width;
         superwindow->DisplayNewWindow.Height = window->Height;
         if (FlagIsSet(display->Modes, MEDIUM_RES))
            ClearFlag(superwindow->Flags, WINDOW_WAS_HIRES);
         else
            SetFlag(superwindow->Flags, WINDOW_WAS_HIRES);

         if (display->Modes & OPEN_SCREEN)
            ((struct ScreenExt *)window->WScreen->UserData)->UserCount--;
         else
         {
            /* This is Workbench viewport, so if we're 
             * the last window then restore the colors 
             */
            ChangeWBWindowCount(-1);
            if ( (CountWBWindows() == 0) && FlagIsSet(display->Modes2, 
                   WBCOLORS_GRABBED) )
            {
#if 0
               if (vport = &window->WScreen->ViewPort)
                  for (i = 0; i < 4; i++) {
                     i2 =  display->WBColors[i];
                     SetRGB4(vport, i,  i2 >> 8, i2 >> 4, i2);
                  }
#endif
               ClearFlag(display->Modes2,  WBCOLORS_GRABBED);
            }

         }


         if (display->TaskPort) 
		    DrainPort(display->TaskPort);

         superwindow->DisplayWindow = NULL;

         ClearMenuStrip(window);

         Forbid();
         if (window->UserPort) 
		    DrainPort(window->UserPort);
         window->UserPort = NULL;
         ModifyIDCMP(window, NULL);
         Permit();

         if (window->UserData) 
            FreeMem(window->UserData,sizeof(struct WindowExtension));

         CloseWindow(window);
      }
   }

   Unlock(&display->DisplayLock);
}

/****i* PCWindow/DestroySuperWindow ****************************************
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

VOID DestroySuperWindow(struct Display *display)
{
   struct SuperWindow *superwindow;

#if (DEBUG1 & DBG_DESTROY_SUPER_WINDOW_ENTER)
   kprintf("super.c:      DestroySuperWindow(display=0x%lx)\n",display);
#endif
     
   MyLock(&display->DisplayLock);

   if (superwindow = display->FirstWindow)
   {
      if (superwindow->DisplayWindow)
      {
         if ((superwindow->DisplayModes & BORDER_VISIBLE) == 0)
            BorderPatrol(display, BORDER_ON, FALSE);
         SuperCloseWindow(display);
         if (superwindow->DisplayScreen)
            ReleaseExtraScreen(display,superwindow->DisplayScreen);
      }

      UnlinkSuperWindow(display, superwindow);

      FreeMem(superwindow, sizeof(struct SuperWindow));
   }

   Unlock(&display->DisplayLock);
}
