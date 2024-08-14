
/**** fileio.c **************************************************************
 * 
 * File Input/Output Routines  --  Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY   Name      Description
 * -------   ------      --------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 14 Apr 86   =RJ Mical=   Created this file
 *
 ***************************************************************************/

#include "zaphod.h"
#include <stdio.h>

#if 0
#define FILE_BUFFER_SIZE    82

static BYTE FileBuffer[FILE_BUFFER_SIZE];
static SHORT FileIndex;
#endif

static LONG FileNumber;

extern struct Library *DOSBase;
   


static struct Settings *TSettings;
static UBYTE sbuf[sizeof(struct Settings)+4L];

 
/****i* PCWindow/WriteSettingsFile ****************************************
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

VOID   WriteSettingsFile(display)
struct   Display *display;
{
   SHORT i, i2;
   struct Process *process;
   struct   Window *window;
   struct   Window *procwindow;
   struct   SuperWindow *swindow;

   if (display == NULL) return;
   if ((swindow = display->FirstWindow) == NULL) return;
   if ((window = swindow->DisplayWindow) == NULL) return;

   if (NOT MyLock(&display->DisplayLock)) return;

   process = (struct Process *)FindTask(0);
   procwindow = (struct Window *)process->pr_WindowPtr;
   process->pr_WindowPtr = (APTR)window;


   i = PRESETS_GOT;

   if (FlagIsSet(display->Modes, BORDER_VISIBLE)) 
      i |= PRESET_BORDER_ON;

   if (FlagIsClear(display->Modes, MEDIUM_RES)) 
      i |= PRESET_HIRES;

   if (FlagIsSet(swindow->Flags, WANTS_PRIVACY))
      i |= PRESET_PRIVACY;

   /* need PAL check here */

   /* Set new settings for this display type */
   if (FlagIsClear(display->Modes, COLOR_DISPLAY)) {
      /* Reset the Mono Settings */
      CurrentSettings.PresetMonoWidth       = window->Width;
      CurrentSettings.PresetMonoHeight      = window->Height;
      CurrentSettings.PresetMonoTopEdge     = window->TopEdge;
      CurrentSettings.PresetMonoLeftEdge    = window->LeftEdge;
      CurrentSettings.PresetMonoDepth       = swindow->TextDepth;

      CurrentSettings.PresetMonoFlags       = i;
   } 
   else {
      /* Reset the Color Settings */
      CurrentSettings.PresetColorWidth      = window->Width;
      CurrentSettings.PresetColorHeight     = window->Height;
      CurrentSettings.PresetColorTopEdge    = window->TopEdge;
      CurrentSettings.PresetColorLeftEdge   = window->LeftEdge;

      CurrentSettings.PresetColorFlags      = i;

      CurrentSettings.DefaultColorTextDepth = DefaultColorTextDepth;
   }


   /* If there's already a settings file out there, zap it! */
   if(!DeleteFile(&SidecarSettingsFile[0]))
   {
      /* No Old file so write color depth */
   }

   if ( (FileNumber = Open(&SidecarSettingsFile[0], MODE_NEWFILE)) == 0)
   {
      MyAlert(ALERT_BAD_SETTINGS_FILE, display);
      goto UNLOCK_RETURN;
   }


   /*******************/
   /* Setup structure */
   /*******************/
   CurrentSettings.CursorSeconds=swindow->CursorSeconds;
   CurrentSettings.CursorMicros=swindow->CursorMicros;
   CurrentSettings.DisplayPriority=DisplayPriority;

   for (i = 0; i < 2; i++)
   {
      CurrentSettings.TextOnePlaneColors[i]=TextOnePlaneColors[i];
   }

   for (i = 0; i < 4; i++)
   {
      CurrentSettings.TextTwoPlaneColors[i]=TextTwoPlaneColors[i];
   }

   for (i = 0; i < 8; i++)
   {
      CurrentSettings.TextThreePlaneColors[i]=TextThreePlaneColors[i];
   }

   for (i = 0; i < 8; i++)
   {
      CurrentSettings.TextFourPlaneColors[i]=TextFourPlaneColors[i];
   }

   for (i = 8; i < 16; i++)
   {
      CurrentSettings.TextFourPlaneColors[i]=TextFourPlaneColors[i];
   }

   for (i = 0; i < 2; i++)
   {
      CurrentSettings.HighGraphicsColors[i]=HighGraphicsColors[i];
   }

   for (i = 0; i < 2; i++)
      for (i2 = 0; i2 < 4; i2++)
      {
         CurrentSettings.LowGraphicsColors[0][i][i2]=LowGraphicsColors[0][i][i2];
      }

   for (i = 0; i < 2; i++)
      for (i2 = 0; i2 < 4; i2++)
      {
         CurrentSettings.LowGraphicsColors[1][i][i2]=LowGraphicsColors[1][i][i2];
      }

   /*******************/
   /* Write structure */
   /*******************/
   if(Write(FileNumber,&CurrentSettings,sizeof(struct Settings))==-1L)
       goto CLOSE_RETURN;

   LastSavedSettings=CurrentSettings;

CLOSE_RETURN:
   Close(FileNumber);
                          
UNLOCK_RETURN:
   process->pr_WindowPtr = (APTR)procwindow;

   Unlock(&display->DisplayLock);
}
 
/****i* PCWindow/ReadSettingsFile ******************************************
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
* The display argument can be NULL if there's no display for this fetch 
*/

BOOL ReadSettingsFile(struct Display *display, BOOL reporterrors)
{
   BOOL   retvalue;
   SHORT i , i2;
   struct   Process *process;
   struct   Window *procwindow, *w;
   struct   SuperWindow *swindow=NULL;

   retvalue = FALSE;

   if (display)
      if(NOT MyLock(&display->DisplayLock)) 
        goto RETURN;

   process = (struct Process *)FindTask(0);
   procwindow = (struct Window *)process->pr_WindowPtr;
   if (display)
   {
      if (display->FirstWindow)
      {
        swindow=display->FirstWindow;
         if (w = display->FirstWindow->DisplayWindow)
            process->pr_WindowPtr = (APTR)w;
      }
   }

   if ( (FileNumber = Open(&SidecarSettingsFile[0], MODE_OLDFILE)) == 0)
   {
      if (reporterrors) MyAlert(ALERT_BAD_SETTINGS_FILE, display);
         goto UNLOCK_RETURN;
   }

   if (display == NULL) goto CLOSE_RETURN;

   TSettings=(struct Settings *)sbuf;

   if(Read(FileNumber,(char *)TSettings,sizeof(struct Settings)+4L)==sizeof(struct Settings))
   {
      LastSavedSettings = CurrentSettings=(*TSettings);
      retvalue = TRUE;
   } else {
      if (reporterrors) MyAlert(ALERT_BAD_SETTINGS_FILE, display);
   }

CLOSE_RETURN:
   Close(FileNumber);

UNLOCK_RETURN:
   if (display)
      Unlock(&display->DisplayLock);

   process->pr_WindowPtr = (APTR)procwindow;

RETURN:

   if (NOT retvalue) goto DONE;

   /****************/
   /* Use Settings */
   /****************/
   DisplayPriority=CurrentSettings.DisplayPriority;
   if(swindow)
   {
      swindow->CursorSeconds=CurrentSettings.CursorSeconds;
      swindow->CursorMicros=CurrentSettings.CursorMicros;
   }

   DefaultColorTextDepth = CurrentSettings.DefaultColorTextDepth;

   /* Text One Plane: */
   for (i = 0; i < 2; i++)
      TextOnePlaneColors[i] = CurrentSettings.TextOnePlaneColors[i];

   /* Text Two Plane */
   for (i = 0; i < 4; i++)
      TextTwoPlaneColors[i] = CurrentSettings.TextTwoPlaneColors[i];

   /* Text Three Plane */
   for (i = 0; i < 8; i++)
      TextThreePlaneColors[i] = CurrentSettings.TextThreePlaneColors[i];

   /* Text Four Plane */
   for (i = 0; i < 16; i++)
      TextFourPlaneColors[i] = CurrentSettings.TextFourPlaneColors[i];

   /* High Graphics */
   for (i = 0; i < 2; i++)
      HighGraphicsColors[i] = CurrentSettings.HighGraphicsColors[i];

   /* Low Graphics Normal */
   for (i = 0; i < 2; i++)
      for (i2 = 0; i2 < 4; i2++)
         LowGraphicsColors[0][i][i2] = CurrentSettings.LowGraphicsColors[0][i][i2];

   /* Low Graphics Intensified */
   for (i = 0; i < 2; i++)
      for (i2 = 0; i2 < 4; i2++)
         LowGraphicsColors[1][i][i2] = CurrentSettings.LowGraphicsColors[1][i][i2];

   display->PresetMonoWidth     = CurrentSettings.PresetMonoWidth;
   display->PresetMonoHeight    = CurrentSettings.PresetMonoHeight;
   display->PresetMonoTopEdge   = CurrentSettings.PresetMonoTopEdge;
   display->PresetMonoLeftEdge  = CurrentSettings.PresetMonoLeftEdge;
   display->PresetMonoDepth     = CurrentSettings.PresetMonoDepth;
   display->PresetColorWidth    = CurrentSettings.PresetColorWidth;
   display->PresetColorHeight   = CurrentSettings.PresetColorHeight;
   display->PresetColorTopEdge  = CurrentSettings.PresetColorTopEdge;
   display->PresetColorLeftEdge = CurrentSettings.PresetColorLeftEdge;
   display->PresetMonoFlags     = CurrentSettings.PresetMonoFlags;
   display->PresetColorDepth    = CurrentSettings.DefaultColorTextDepth;
   display->PresetColorFlags    = CurrentSettings.PresetColorFlags;
   SetFlag(display->Modes2, WINDOW_PRESETS);
   /*if(FlagIsSet(display->PresetColorFlags,PRESET_PRIVACY))
   {
      SetFlag(display->Modes2, WANTS_PRIVACY);
   } else {
      ClearFlag(display->Modes2, WANTS_PRIVACY);
   }*/

DONE:

   return(retvalue);
}
