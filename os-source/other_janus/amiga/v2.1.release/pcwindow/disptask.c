
/**** disptask.c ************************************************************
 * 
 * Display Task  --  for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 13 Jan 87   =RJ=        Added call to FullSize() on select double-click
 * 13 Mar 86   =RJ=        Changed this from cdtask.c to disptask.c
 * 20 Feb 86   =RJ Mical=  Created this file
 *   
 ***************************************************************************/
#define LINT_ARGS
#define PRAGMAS

#include "zaphod.h"
#include <janus/jfuncs.h>
#include <stdlib.h>

#define D(x) ;

static LONG (*CountRoutine)(void);

static ULONG DisplayWriteSignal = 0L;
static ULONG CRTRegisterWriteSignal = 0L;

/****i* PCWindow/DisplayTask ******************************************
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

VOID DisplayTask(display)
struct Display *display;
/* The Display structure must be pre-initialized in this way:
 *   - Modes must be initialized to reflect the type of display desired.
 *   - The rest of the structure must be initialized to NULL.
 */
{
   ULONG wakeUpSignals;
   ULONG waitSignals;
   ULONG windowSignal;
   ULONG janusDisplayNum, janusCRTNum;
   struct SuperWindow *superwindow;
   struct Window *window;
   BOOL retvalue;

#if (DEBUG1 & DBG_DISPLAY_TASK_ENTER)
   kprintf("Disptask.c:   DisplayTask(display=0x%lx)\n",display);
#endif

/* === Critical Section.  Must be completed in toto ====================== */
   Forbid();
/*###############################################################*/



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

   if (FlagIsClear(display->Modes, COLOR_DISPLAY))
      DisplayItems[DISPLAY_ITEMS_COUNT - DEPTH_ITEM - 1].SubItem
            = &NumberOfTextColorsSubItems[1];

   if (NOT OpenDisplay(display)) goto DONE;
   if (NOT OpenDisplayWindow(display, NULL, NULL,TRUE)) goto DONE;

   retvalue = TRUE;

DONE:

 

/*###############################################################*/
   if (NOT retvalue) 
   {
      Permit();
      return;
   }

   TalkWithZaphod(display, ADD_DISPLAY,(LONG) display);

   Permit();
/* === End of critical section ========================================== */

   DisplayWriteSignal = ZAllocSignal();
   windowSignal = 1L << display->TaskPort->mp_SigBit;
   CRTRegisterWriteSignal = ZAllocSignal();

   waitSignals = DisplayWriteSignal | windowSignal | CRTRegisterWriteSignal;


   /* Tell the interrupt server that I'm alive and looking for
    * the various interrupts.
    * Tell about these interrupts:
    *     - displayWrite
    *     - CRTRegisterWrite
    * (Some great names follow.  They're not my names.)
    */
   if (display->Modes & COLOR_DISPLAY)
   {
      janusDisplayNum = JSERV_GINT;
      janusCRTNum = JSERV_CRT2INT;
   } else {
      janusDisplayNum = JSERV_MINT;
      janusCRTNum = JSERV_CRT1INT;
   }

   display->CRTWriteSig=(struct SetupSig *)SetupJanusSig((USHORT)janusCRTNum,
                           CreateSignalNumber(CRTRegisterWriteSignal), 0, 0);

   display->DisplayWriteSig=(struct SetupSig *)SetupJanusSig((USHORT)
               janusDisplayNum,CreateSignalNumber(DisplayWriteSignal), 0, 0);

   if ((display->CRTWriteSig == NULL) || (display->DisplayWriteSig == NULL))
   {
      MyAlert(ALERT_NO_JANUS_SIG, display);
      CloseDisplayTask(display);
   }

   Forbid();
   superwindow = display->FirstWindow;
   window = superwindow->DisplayWindow;
   while ((window->UserPort = FindPort(INPUT_PORT_NAME)) == NULL)
      WaitTOF();
   ModifyIDCMP(window, DISPLAY_IDCMP_FLAGS);
   Permit();

   GetQueueRoutine((LONG *)&KeyQueuer);

   FOREVER
   {
      /* Wait for the interrupt */
      wakeUpSignals = Wait(waitSignals);

      /* NOTE:  Window Events should be handled before display signals! */
      /* (Don't ask me why, just do it!) */
      if (wakeUpSignals & windowSignal)
      {
         /* A window event has occurred.  Go handle it. */
         WindowEvent(display);
      }

      if (wakeUpSignals & CRTRegisterWriteSignal)
         GetCRTSetUp(display, FALSE, FALSE);

      /* Give the cursor a chance to flash */
      WaitTOF();

      if (wakeUpSignals & DisplayWriteSignal)
         /* What?  Someone bothering the Blue display memory?  Well! */
         if (FlagIsClear(display->Modes, SELECTED_AREA))
            RenderWindow(display, FALSE, FALSE, FALSE, TRUE);
   }

}           

/****i* PCWindow/WindowEvent ******************************************
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

VOID WindowEvent(display)
struct Display *display;
/* Some sort of event (not RAWKEY, you know) has occurred with our
 * Zaphod Window.  Handle it here.  
 * This routine is a beast.  
 * Main plan:  get messages from display's TaskPort.  
 * (alternate plan:  panic, run around with hair on fire.)
 */
{
   struct IntuiMessage *message;
   ULONG class;
   USHORT code;
   ULONG seconds, micros;
   BOOL recalc, locked, moved;
   BOOL render;
   BOOL stuffText;
   BOOL stuffLines;
   BOOL redrawAll;
   APTR IAddress;
   struct SuperWindow *superwindow;
   struct Window *IDCMPWindow;
   struct Window *window;
   SHORT oldcol, oldrow, x, y, i;
   /*struct Preferences *prefs;*/

#if (DEBUG1 & DBG_WINDOW_EVENT_ENTER)
   kprintf("Disptask.c:   WindowEvent(display=0x%lx)\n",display);
#endif

   render = recalc = stuffText = stuffLines = redrawAll = locked = FALSE;
   moved = FALSE;

   while (message = (struct IntuiMessage *)GetMsg(display->TaskPort))
   {
      class = message->Class;
      code = message->Code;
      seconds = message->Seconds;
      micros = message->Micros;
      IDCMPWindow = message->IDCMPWindow;
      IAddress = message->IAddress;
      x = message->MouseX;
      y = message->MouseY;
      ReplyMsg((struct Message *)message);

      /* Find the window and superwindow associated with this event. */
      window = NULL;
      superwindow = display->FirstWindow;
      while (superwindow && (window == NULL))
      {
         if (superwindow->DisplayWindow == IDCMPWindow)
            window = IDCMPWindow;
         else 
          superwindow = superwindow->NextWindow;
      }
      if (superwindow != display->FirstWindow)
         if ((class != ACTIVEWINDOW) && (class != INACTIVEWINDOW))
         {
            if (superwindow)
            {
               UnlinkSuperWindow(display, superwindow);
               LinkSuperWindow(display, superwindow);
            }
         }

      if ((superwindow == NULL) || (window == NULL))
         goto DONE_WITH_MESSAGE;

      /* Now, if Intuition had provided a NEW_LOCATION
       * IDCMP event to complement the NEW_SIZE event,
       * I wouldn't have to waste everyone's time by
       * doing this here.
       */
      /*BBB   RecordLastSmall(display, superwindow, window);*/

      switch (class)
      {
         case RAWKEY:
               /* Well, a RAWKEY.  This is a new one on me, as of Feb 88.
                * Never used to get rawkeys down here in the slums.
                * Now if we get one it means that the help key was pressed, 
                * so go give the user some help.
                */
               Help(-1, display);
               break;
         case NEWSIZE:
               ModifyDisplayProps(display);
#if 0
               if ( (window->Width == window->MaxWidth)
                    && (window->Height == window->MaxHeight) )
               {
                  if (FlagIsSet(display->Modes, PAL_PRESENCE))
                     PALBottomBorder(window, display);
               }
#endif
               if ( (window->Width == window->MaxWidth)
                    && (window->Height == window->MaxHeight) 
                    && FlagIsSet(display->Modes, BORDER_VISIBLE)
                    && FlagIsClear(superwindow->Flags, FROZEN_HOSEHEAD)
                    && FlagIsClear(display->Modes, SQUELCH_NEWSIZE) ) 
			   {
                  /* if pcwindow is on its own screen, turn off border */
                  if(FlagIsSet(display->Modes,OPEN_SCREEN))
                     BorderPatrol(display, BORDER_OFF, TRUE);

                  SetFlag(display->Modes2, FULLSIZE);
               }
                  /* Since we know that NEWSIZE *will* be followed by
                   * REFRESHWINDOW (poof), we can get away with deferring
                   * the redraw until later.
                   */
               SetFlag(display->Modes, GOT_NEWSIZE);
               ClearFlag(display->Modes, SQUELCH_NEWSIZE);
			   if(display->FirstWindow->DisplayWindow->Flags&WINDOWACTIVE)
                  SetMenuState(display);
               break;
         case REFRESHWINDOW:
               if (display->Modes & GOT_NEWSIZE) {
                  recalc = stuffText = stuffLines = redrawAll = TRUE;
                  ClearFlag(display->Modes, GOT_NEWSIZE);
               }
               BeginRefresh(window);
               EndRefresh(window, TRUE);
               render = TRUE;

			   if(display->FirstWindow->DisplayWindow->Flags&WINDOWACTIVE)
                  SetMenuState(display);
               break;

         case GADGETDOWN:
               if ((((struct Gadget *)IAddress)->GadgetID == VERT_GADGET)
                  || (((struct Gadget *)IAddress)->GadgetID == HORIZ_GADGET))
                  SetFlag(display->Modes, PROP_ACTIVE);
               break;
         case GADGETUP:
               ClearFlag(display->Modes, PROP_ACTIVE);
               /* recalc remains FALSE */
               stuffText = stuffLines = render = redrawAll = TRUE;

               if (NOT locked)
               {
                  locked = TRUE;
                  MyLock(&display->DisplayLock);
               }

               oldrow = display->DisplayStartRow;
               oldcol = display->DisplayStartCol;
               RecalcDisplayParms(display);
               MoveAndSetRegion(display,
                          (SHORT)(display->DisplayStartCol - oldcol),
                             (SHORT)(display->DisplayStartRow - oldrow),
                             (SHORT)window->BorderLeft,
                             (SHORT)window->BorderTop,
                             (SHORT)(window->Width-window->BorderRight-1),
                             (SHORT)(window->Height-window->BorderBottom-1));
               break;
         case MOUSEBUTTONS:
               switch (code)
               {
                  case SELECTDOWN:
                     SetFlag(display->Modes, SELECT_PRESSED);
                     ClearFlag(display->Modes, SELECTED_AREA);
                     ClipStartX = x;
                     ClipStartY = y;
                     ClipCurrentX = x;
                     ClipCurrentY = y;
                     break;
                  case SELECTUP:
                        if (FlagIsSet(display->Modes, SELECTED_AREA))
                        {
                           render = stuffText = stuffLines = TRUE;
                           redrawAll = TRUE;
                           WriteSelectedArea(display);
                        }
                        ClearFlag(display->Modes,
                            (SELECT_PRESSED | SELECTED_AREA));
                     break;
               }
               break;
         case ACTIVEWINDOW:
               ClearFlag(display->Modes, PROP_ACTIVE);
               if (NOT locked)
               {
                  locked = TRUE;
                  MyLock(&display->DisplayLock);
               }
               SetTaskPri(FindTask(0), DisplayPriority);

               if (superwindow != display->FirstWindow)
               {
                  UnlinkSuperWindow(display, superwindow);
                  LinkSuperWindow(display, superwindow);
                  recalc = stuffText = stuffLines = TRUE;
                  redrawAll = render = TRUE;
                  GetCRTSetUp(display, TRUE, FALSE);
                  if (FlagIsSet(display->Modes, MEDIUM_RES)) 
                 i = 320;
                  else 
                 i = 640;
                  InitDisplayBitMap(display, superwindow, i);
               }

               /*display->OldSeconds = display->OldMicros = 0;*/
               /*kprintf("ACTIVEWINDOW\n");*/
			   if(display->FirstWindow->DisplayWindow->Flags&WINDOWACTIVE)
                  SetMenuState(display);
			   /*PrintSuperWindows(display);*/
               break;
         case INACTIVEWINDOW:
               ClearFlag(display->Modes, PROP_ACTIVE);
               if (NOT locked)
               {
                  locked = TRUE;
                  MyLock(&display->DisplayLock);
               }

               /* If the border is hidden as we go inactive ... */
               if (display->FirstWindow == superwindow)
               {
                  if (display->CursorFlags & CURSOR_ON)
                  {
                     Curse(display);
                     ClearFlag(display->CursorFlags, CURSOR_ON);
                  }
               }
               D(kprintf("INACTIVEWINDOW\n");)
			   /*PrintSuperWindows(display);*/
               /*SetMenuState(display);*/
			   if(display->FirstWindow->DisplayWindow->Flags&WINDOWACTIVE)
                  SetMenuState(display);
               break;
         case MENUPICK:
               if (FlagIsSet(display->Modes, SELECTED_AREA))
               {
                  render = stuffText = stuffLines = redrawAll = TRUE;
               }
               ClearFlag(display->Modes, (SELECT_PRESSED | SELECTED_AREA));
               MenuEvent(display, code, seconds, micros);
			   if(display->FirstWindow->DisplayWindow->Flags&WINDOWACTIVE)
                  SetMenuState(display);
               break;
#if 0
         case NEWPREFS:
               prefs = (struct Preferences *)AllocMem(
               sizeof(struct Preferences), NULL);
               if (prefs)
               {
                  GetPrefs(prefs, sizeof(struct Preferences));
                  TextTwoPlaneColors[0] = prefs->color0;
                  TextTwoPlaneColors[1] = prefs->color1;
                  TextTwoPlaneColors[2] = prefs->color2;
                  TextTwoPlaneColors[3] = prefs->color3;
                  if (FlagIsSet(display->Modes2, WBCOLORS_GRABBED))
                  {
                     display->WBColors[0] = prefs->color0;
                     display->WBColors[1] = prefs->color1;
                     display->WBColors[2] = prefs->color2;
                     display->WBColors[3] = prefs->color3;
                  }
               }
               break;
#endif
         case MOUSEMOVE:
               moved = TRUE;
               break;
         case CLOSEWINDOW:
               CloseDisplayTask(display);
               break;
      }
DONE_WITH_MESSAGE: ;

   }

   if (moved && FlagIsSet(display->Modes, SELECT_PRESSED)
       && FlagIsSet(display->Modes, TEXT_MODE))
   {
      if (render && FlagIsClear(display->Modes, SELECTED_AREA))
      {
         RenderWindow(display, TRUE, TRUE, TRUE, TRUE);
       /*
         if (display->Modes & BORDER_VISIBLE)
            TopRightLines(display->FirstWindow->DisplayWindow);
       */
         SetFlag(display->CursorFlags, CURSOR_MOVED);
         recalc = stuffText = stuffLines = redrawAll = FALSE;
      }

      if (FlagIsSet(display->Modes, SELECTED_AREA)) 
        InvertClipArea(display);
      SetFlag(display->Modes, SELECTED_AREA);
      ClipCurrentX = x;
      ClipCurrentY = y;
      InvertClipArea(display);
      RenderWindow(display, FALSE, FALSE, TRUE, TRUE);
      render = FALSE;
   }

   if (FlagIsSet(display->Modes, COUNT_DISPLAY))
      (*CountRoutine)();

   if (moved && FlagIsSet(display->Modes, PROP_ACTIVE)
       && FlagIsClear(display->Modes, SELECTED_AREA))
   {
      render = recalc = stuffText = stuffLines = redrawAll = TRUE;
   }

   if (render) {
      RenderWindow(display, recalc, stuffText, stuffLines, redrawAll);
      SetFlag(display->CursorFlags, CURSOR_MOVED);
     /*
      if (display->Modes & BORDER_VISIBLE)
         TopRightLines(display->FirstWindow->DisplayWindow);
     */
   }

   if (locked)
   {
   /*
      ClearMenuStrip(display->FirstWindow->DisplayWindow);
     kprintf("WindowEvent ");
     SetMenuState(display);
      SetMenuStrip(display->FirstWindow->DisplayWindow,
                  &MenuHeaders[MENU_HEADERS_COUNT - 1]);*/
      Unlock(&display->DisplayLock);
   }
}

/****i* PCWindow/CloseDisplayTask ******************************************
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

VOID CloseDisplayTask(display)
 struct Display *display;
{
/*   struct Task *task; */
   struct AuxToolsMessage *message;

#if (DEBUG1 & DBG_CLOSE_DISPLAY_TASK_ENTER)
   kprintf("Disptask.c:   CloseDisplayTask(display=0x%lx)\n",display);
#endif
 
   Forbid();

   MyLock(&display->DisplayLock);

   DestroySuperWindow(display);

   if (display->FirstWindow == NULL) {
      DeathLock(&display->DisplayLock);

      ZFreeSignal(DisplayWriteSignal);
      ZFreeSignal(CRTRegisterWriteSignal);

      if (display->TaskPort)
      {
         DrainPort(display->TaskPort);
         DeletePort(display->TaskPort);
         display->TaskPort = NULL;
      }

      CloseAuxTools(display);
      display->TaskAddress = NULL;

      if (display->DisplayWriteSig)
         CleanupJanusSig(display->DisplayWriteSig);
      if (display->CRTWriteSig) 
        CleanupJanusSig(display->CRTWriteSig);

      /* Sadly, until this becomes a library, I either do this or 
       * learn the CreateProc() kludge stuff which no one has documentation
       * on and there's too few days left so here's the even kludgier
       * temporary solution, like it or not ...
       */
      if (AuxToolsFinalPort)
      {
         WaitPort(AuxToolsFinalPort);

         if (message = (struct AuxToolsMessage *)GetMsg(AuxToolsFinalPort))
            DeleteExtIO((struct IORequest *)message);
         DeletePort(AuxToolsFinalPort);
      }

      FreeRemember(&display->BufferKey, TRUE);
      CloseEverything();
      FreeMem(display, sizeof(struct Display));

      /*??? Taken out while DisplayTask() is still a procedure call
            RemTask(task);
      ???*/
      exit(0);
   }

   Unlock(&display->DisplayLock);
}
VOID SetMenuState(struct Display *display)
{
   struct Screen TScreen;
#if 0
   kprintf("Display->NextDisplay=0x%lx\n",display->NextDisplay);
   kprintf("Display->FirstWindow=0x%lx\n",display->FirstWindow);
   kprintf("Display->TaskAddress=0x%lx\n",display->TaskAddress);
   kprintf("Display->TaskPort   =0x%lx\n",display->TaskPort);
   kprintf("Display->Buffer     =0x%lx\n",display->Buffer);
   kprintf("Display->BufferKey  =0x%lx\n",display->BufferKey);

   kprintf("Display->Modes = ");
   if(display->Modes&MEDIUM_RES)
      kprintf("MEDIUM_RES ");
   if(display->Modes&TEXT_MODE)
      kprintf("TEXT_MODE ");
   if(display->Modes&SELECTED_AREA)
      kprintf("SELECTED_AREA ");
   if(display->Modes&BLINK_TEXT)
      kprintf("BLINK_TEXT ");
   if(display->Modes&PALETTE_1)
      kprintf("PALETTE_1 ");
   if(display->Modes&OPEN_SCREEN)
      kprintf("OPEN_SCREEN ");
   if(display->Modes&NEWSIZE_REFRESH)
      kprintf("NEWSIZE_REFRESH ");
   if(display->Modes&SELECT_PRESSED)
      kprintf("SELECT_PRESSED ");
   if(display->Modes&BORDER_VISIBLE)
      kprintf("BORDER_VISIBLE ");
   if(display->Modes&GOT_NEWSIZE)
      kprintf("GOT_NEWSIZE ");
   if(display->Modes&PAL_PRESENCE)
      kprintf("PAL_PRESENCE ");
   if(display->Modes&COLOR_DISPLAY)
      kprintf("COLOR_DISPLAY ");
   if(display->Modes&VERBOSE)
      kprintf("VERBOSE ");
   if(display->Modes&SQUELCH_NEWSIZE)
      kprintf("SQUELCH_NEWSIZE ");
   if(display->Modes&PROP_ACTIVE)
      kprintf("PROP_ACTIVE ");
   if(display->Modes&COUNT_DISPLAY)
      kprintf("COUNT_DISPLAY ");
   kprintf("\n");

   kprintf("Display->Modes2 = ");
   if(display->Modes2&WBCOLORS_GRABBED)
      kprintf("WBCOLORS_GRABBED ");
   if(display->Modes2&WINDOW_PRESETS)
      kprintf("WINDOW_PRESETS ");
   if(display->Modes2&FULLSIZE)
      kprintf("FULLSIZE ");
   if(display->Modes2&OPENED_ONCE)
      kprintf("OPENED_ONCE ");
   kprintf("\n");

   kprintf("Display->DisplayWidth      =0x%lx\n",display->DisplayWidth);
   kprintf("Display->DisplayHeight     =0x%lx\n",display->DisplayHeight);
   kprintf("Display->DisplayStartCol   =0x%lx\n",display->DisplayStartCol);
   kprintf("Display->DisplayStartRow   =0x%lx\n",display->DisplayStartRow);
   /*kprintf("Display->OldSeconds        =0x%lx\n",display->OldSeconds);*/
   /*kprintf("Display->OldMicrosi        =0x%lx\n",display->OldMicros);*/
   kprintf("Display->FirstWindow->Flags=0x%lx\n",display->FirstWindow->Flags);
#endif

   D(kprintf("SetMenuState(display=0x%lx ",display);)
   D(if(display->FirstWindow->DisplayWindow->Flags&WINDOWACTIVE)
   {
      kprintf("WINDOWACTIVE ");
   })

   /******************************************************************/
   /* If WBS Borders enabled? Workbench Checked? Depth, Color Enabled*/
   /******************************************************************/
   if(display->FirstWindow->DisplayScreen==NULL)
   {
      D(kprintf("WSCREEN ");)

      DisplayItems[DISPLAY_ITEMS_COUNT-1-SHOWBORDER_ITEM].Flags&=
        ~ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-DEPTH_ITEM].Flags&=
        ~ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-COLOR_ITEM].Flags&=
        ~ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-INTERLACE_ITEM].Flags&=
        ~ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-LOCALE_ITEM].Flags|=
        CHECKED;

   } else {
      D(kprintf("CSCREEN ");)

      if(
      (display->FirstWindow->DisplayWindow->Width==display->FirstWindow->DisplayScreen->Width)&&
      (display->FirstWindow->DisplayWindow->Height==/*200L*/display->FirstWindow->DisplayScreen->Height))
      DisplayItems[DISPLAY_ITEMS_COUNT-1-SHOWBORDER_ITEM].Flags|=
        ITEMENABLED;
      else
      DisplayItems[DISPLAY_ITEMS_COUNT-1-SHOWBORDER_ITEM].Flags&=
        ~ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-DEPTH_ITEM].Flags|=
        ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-COLOR_ITEM].Flags|=
        ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-INTERLACE_ITEM].Flags|=
        ITEMENABLED;

      DisplayItems[DISPLAY_ITEMS_COUNT-1-LOCALE_ITEM].Flags&=
        ~CHECKED;
   }

   /********************/
   /* Borders Checked? */
   /********************/
   if(display->Modes&BORDER_VISIBLE)
   {
      D(kprintf("BORDER--ON ");)
      DisplayItems[DISPLAY_ITEMS_COUNT-1-SHOWBORDER_ITEM].Flags|=
        CHECKED;
   } else {
      D(kprintf("BORDER-OFF ");)
      DisplayItems[DISPLAY_ITEMS_COUNT-1-SHOWBORDER_ITEM].Flags&=
        ~CHECKED;
   }

   /*******************/
   /* Frozen Checked? */
   /*******************/
   if(display->FirstWindow->Flags&FROZEN_HOSEHEAD)
   {
      D(kprintf("FREEZE--ON ");)
      DisplayItems[DISPLAY_ITEMS_COUNT-1-FREEZE_ITEM].Flags|=
        CHECKED;
   } else {
      D(kprintf("FREEZE-OFF ");)
      DisplayItems[DISPLAY_ITEMS_COUNT-1-FREEZE_ITEM].Flags&=
        ~CHECKED;
   }

   /*********************/
   /* Workbench Enabled */
   /*********************/
   GetScreenData(&TScreen,sizeof(struct Screen),WBENCHSCREEN,NULL);

   /*CurrentSettings.Depth=display->FirstWindow->DisplayDepth;

   if(CurrentSettings.Depth==TScreen.BitMap.Depth)*/

   if(display->FirstWindow->DisplayDepth==TScreen.BitMap.Depth)
   {
      D(kprintf("WB--ENABLED ");)
      DisplayItems[DISPLAY_ITEMS_COUNT-1-LOCALE_ITEM].Flags|=ITEMENABLED;
   } else {
      D(kprintf("WB-DISABLED ");)
      DisplayItems[DISPLAY_ITEMS_COUNT-1-LOCALE_ITEM].Flags&=~ITEMENABLED;
   }

   /************************/
   /* Uncheck all subitems */
   /************************/
   { 
      UBYTE ll;

      for(ll=0;ll<5;ll++)
      {
         if(ll<4)
         {
            SetCursorBlinkRateSubItems[ll].Flags&=~CHECKED;
            NumberOfTextColorsSubItems[ll].Flags&=~CHECKED;
         }
         DisplayPrioritySubItems[ll].Flags&=~CHECKED;
      }
   }

   /***********************/
   /* DisplayTaskPriority */
   /***********************/
   D(kprintf("DP=%ld ",DisplayPriority);)
   switch((LONG)DisplayPriority)
   {
      case  10L:
         DisplayPrioritySubItems[0].Flags|=CHECKED;
        break;
     case   5L:
         DisplayPrioritySubItems[1].Flags|=CHECKED;
        break;
     case   0L:
         DisplayPrioritySubItems[2].Flags|=CHECKED;
        break;
     case  -5L:
         DisplayPrioritySubItems[3].Flags|=CHECKED;
        break;
     case -10L:
         DisplayPrioritySubItems[4].Flags|=CHECKED;
        break;
   }

   /********************/
   /* Number of Colors */
   /********************/
   switch(display->FirstWindow->DisplayScreen?display->FirstWindow->DisplayScreen->RastPort.BitMap->Depth:TScreen.BitMap.Depth)
   {
      case 1:
         NumberOfTextColorsSubItems[0].Flags|=CHECKED;
        break;
     case 2:
         NumberOfTextColorsSubItems[1].Flags|=CHECKED;
        break;
     case 3:
         NumberOfTextColorsSubItems[2].Flags|=CHECKED;
        break;
     case 4:
         NumberOfTextColorsSubItems[3].Flags|=CHECKED;
        break;
   }

   /*********************/
   /* Cursor Blink Rate */
   /*********************/
   switch(display->FirstWindow->CursorMicros)
   {
      case 0L:
         SetCursorBlinkRateSubItems[0].Flags|=CHECKED;
        break;
     case 1000000L /2L:
         SetCursorBlinkRateSubItems[1].Flags|=CHECKED;
        break;
     case 1000000L /4L:
         SetCursorBlinkRateSubItems[2].Flags|=CHECKED;
        break;
     case 1000000L /8L:
         SetCursorBlinkRateSubItems[3].Flags|=CHECKED;
        break;
   }
   D(kprintf("\n");)
}
#if 0
VOID PrintSuperWindows(struct Display *display)
{
   struct SuperWindow *sw;

   kprintf("Display->Modes = ");
   if(display->Modes&MEDIUM_RES)
      kprintf("MEDIUM_RES ");
   if(display->Modes&TEXT_MODE)
      kprintf("TEXT_MODE ");
   if(display->Modes&SELECTED_AREA)
      kprintf("SELECTED_AREA ");
   if(display->Modes&BLINK_TEXT)
      kprintf("BLINK_TEXT ");
   if(display->Modes&PALETTE_1)
      kprintf("PALETTE_1 ");
   if(display->Modes&OPEN_SCREEN)
      kprintf("OPEN_SCREEN ");
   if(display->Modes&NEWSIZE_REFRESH)
      kprintf("NEWSIZE_REFRESH ");
   if(display->Modes&SELECT_PRESSED)
      kprintf("SELECT_PRESSED ");
   if(display->Modes&BORDER_VISIBLE)
      kprintf("BORDER_VISIBLE ");
   if(display->Modes&GOT_NEWSIZE)
      kprintf("GOT_NEWSIZE ");
   if(display->Modes&PAL_PRESENCE)
      kprintf("PAL_PRESENCE ");
   if(display->Modes&COLOR_DISPLAY)
      kprintf("COLOR_DISPLAY ");
   if(display->Modes&VERBOSE)
      kprintf("VERBOSE ");
   if(display->Modes&SQUELCH_NEWSIZE)
      kprintf("SQUELCH_NEWSIZE ");
   if(display->Modes&PROP_ACTIVE)
      kprintf("PROP_ACTIVE ");
   if(display->Modes&COUNT_DISPLAY)
      kprintf("COUNT_DISPLAY ");
   kprintf("\n");

   kprintf("Display->Modes2 = ");
   if(display->Modes2&WBCOLORS_GRABBED)
      kprintf("WBCOLORS_GRABBED ");
   if(display->Modes2&WINDOW_PRESETS)
      kprintf("WINDOW_PRESETS ");
   if(display->Modes2&FULLSIZE)
      kprintf("FULLSIZE ");
   if(display->Modes2&OPENED_ONCE)
      kprintf("OPENED_ONCE ");
   kprintf("\n");

   sw=display->FirstWindow;

   while(sw!=NULL)
   {
	  kprintf("sw=0x%lx ",sw);
	  kprintf("dw=0x%lx ",sw->DisplayWindow);
	  if(sw->DisplayWindow->Flags&WINDOWACTIVE)
	  {
	     kprintf("WINDOWACTIVE ");
	  } 
	  kprintf("\n");
	  sw=sw->NextWindow;
   }
}
#endif
