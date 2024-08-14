
/* *** misc.c ****************************************************************
 * 
 * Miscellaneous Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 23 Feb 86   =RJ Mical=  Created this file

 * **************************************************************************/

#include "zaphod.h"
#include <janus/memory.h>
#include <janus/janusreg.h>
#include <exec/memory.h>

#define  DBG_MY_CREATE_TASK_ENTER            1
#define  DBG_MY_CREATE_TASK_RETURN           1
#define  DBG_CLOSE_EVERYTHING_ENTER          1
#define  DBG_DRAIN_PORT_ENTER                1
#define  DBG_FIX_PROP_POTS_ENTER             1
#define  DBG_Z_ALLOC_SIGNAL_ENTER            1
#define  DBG_Z_FREE_SIGNAL_ENTER             1
#define  DBG_SET_THOSE_PREFS_ENTER           1
#define  DBG_CREATE_SIGNAL_NUMBER_ENTER      1
#define  DBG_TOP_RIGHT_LINES_ENTER           1
#define  DBG_GET_PC_DISPLAY_ENTER            1
#define  DBG_SET_TIMER_ENTER                 0
#define  DBG_DRAW_BOX_ENTER                  1
#define  DBG_PAL_BOTTOM_BORDER_ENTER         1
#define  DBG_MY_REFRESH_WINDOW_FRAME_ENTER   1
#define  DBG_ULONG_DIVIDE_ENTER              1
#define  DBG_MODIFY_DISPLAY_PROPS_ENTER      1
#define  DBG_WAIT_TOFING_ENTER               1
#define  DBG_TEXT_X_POS_ENTER                1
#define  DBG_TEXT_Y_POS_ENTER                1
#define  DBG_X_ADD_GADGET_ENTER              1

void ModifyProp();

extern UBYTE FakeColorCRT[CRT_IOBLOCKSIZE];

#ifdef RELEASE
printf() { }
kprintf() { }
#endif

struct Task *MyCreateTask(name, priority, initialPC, stacksize)
UBYTE *name;
UBYTE priority;
APTR initialPC;
ULONG stacksize;
{
   struct Task *taskptr;
   ULONG realsize;
   UBYTE *stackptr;

#if (DEBUG1 & DBG_MY_CREATE_TASK_ENTER)
   kprintf("Misc.c:       MyCreateTask(name=%s,\n",name);
   kprintf("Misc.c:                    priority=0x%lx,initialPC=0x%lx,stacksize=0x%lx)\n",priority,initialPC,stacksize);
#endif

   taskptr = (struct Task *)AllocRemember(&GlobalKey, sizeof(struct Task), 
         MEMF_CLEAR | MEMF_CHIP);

   realsize = (stacksize + 3) & 0xFFFFFC;
   stackptr = AllocRemember(&GlobalKey, realsize,
         MEMF_CLEAR | MEMF_PUBLIC | MEMF_CHIP);

   if ((taskptr == NULL) || (stackptr == NULL))
      {
      MyAlert(ALERT_NO_TASK_STACK_MEMORY,NULL);
      return(NULL);
      }

   taskptr->tc_SPLower = (APTR)stackptr;
   taskptr->tc_SPUpper = (APTR)(((LONG)(stackptr) + realsize - 2) & 0xFFFFFE);
   taskptr->tc_SPReg = taskptr->tc_SPUpper;

   taskptr->tc_Node.ln_Type = NT_TASK;
   taskptr->tc_Node.ln_Pri = priority;
   taskptr->tc_Node.ln_Name = (char *)name;

   AddTask(taskptr, (char *)initialPC, 0);

#if (DEBUG2 & DBG_MY_CREATE_TASK_RETURN)
   kprintf("Misc.c:       MyCreateTask: Returns(0x%lx)\n",taskptr);
#endif

   return(taskptr);
}


void CloseEverything()
{
/* === Critical Section (for safety's sake) ========================== */

#if (DEBUG1 & DBG_CLOSE_EVERYTHING_ENTER)
   kprintf("Misc.c:       CloseEverything(VOID)\n");
#endif

   Forbid();

   CloseTextClip();

   FreeRemember(&GlobalKey, TRUE);

   if (GfxBase)         CloseLibrary(GfxBase);
   if (LayersBase)      CloseLibrary(LayersBase);
   if (DiskfontBase)    CloseLibrary(DiskfontBase);
   if (IntuitionBase)   CloseLibrary(IntuitionBase);
   if (JanusBase)       CloseLibrary(JanusBase);

   Permit();
/* === End of Critical Section ====================================== */
}


               
void DrainPort(port)
REGISTER struct MsgPort *port;
/* NOTE:  DrainPort() does *not* Forbid() and Permit().  There's little
 * point in doing so.  If the caller needs this quality, the caller 
 * should provide it (and probably needs to for other reasons as well).
 */
{
   REGISTER struct IntuiMessage *message;

#if (DEBUG1 & DBG_DRAIN_PORT_ENTER)
   kprintf("Misc.c:       DrainPort(port=0x%lx)\n",port);
#endif

   if (port)
      while (message = (struct IntuiMessage *)GetMsg(port)) ReplyMsg((struct Message *)message);
}



void FixPropPots(propgadget)
REGISTER struct Gadget *propgadget;
{
   REGISTER SHORT start, freeplay, pot;

#if (DEBUG1 & DBG_FIX_PROP_POTS_ENTER)
   kprintf("Misc.c:       FixPropPots(propgadget=0x%lx)\n",propgadget);
#endif

   start = ((struct Image *)propgadget->GadgetRender)->LeftEdge;

   freeplay = ((struct PropInfo *)propgadget->SpecialInfo)->CWidth;
   freeplay -= (((struct PropInfo *)propgadget->SpecialInfo)->LeftBorder * 2);
   freeplay -= ((struct Image *)propgadget->GadgetRender)->Width;

   /* This conditional makes sure that the max value is hit.
    * It should also guard against divide-by-zero error, because if
    * freeplay is 0, start *should* be 0 too, but I put in the extra 
    * test just to be safe.
    */
   if ((start == freeplay) || (freeplay == 0)) pot = MAXPOT;
   else pot = (((LONG)MAXPOT * start) + (freeplay >> 1)) / freeplay;

   ((struct PropInfo *)propgadget->SpecialInfo)->HorizPot = pot;


   start = ((struct Image *)propgadget->GadgetRender)->TopEdge;

   freeplay = ((struct PropInfo *)propgadget->SpecialInfo)->CHeight;
   freeplay -= (((struct PropInfo *)propgadget->SpecialInfo)->TopBorder * 2);
   freeplay -= ((struct Image *)propgadget->GadgetRender)->Height;

   if ((start == freeplay) || (freeplay == 0)) pot = MAXPOT;
   else pot = (((LONG)MAXPOT * start) + (freeplay >> 1)) / freeplay;

   ((struct PropInfo *)propgadget->SpecialInfo)->VertPot = pot;
}



ULONG ZAllocSignal()
{
   REGISTER SHORT i;

#if (DEBUG1 & DBG_Z_ALLOC_SIGNAL_ENTER)
   kprintf("Misc.c:       ZAllocSignal(VOID)\n");
#endif

   if ((i = AllocSignal(-1)) == -1)
      {
      MyAlert(ALERT_NO_SIGNALS, NULL);
      return(-1);
      }
   return((ULONG)1 << i);
}



void ZFreeSignal(signal)
REGISTER ULONG signal;
{
   REGISTER SHORT i;

#if (DEBUG1 & DBG_Z_FREE_SIGNAL_ENTER)
   kprintf("Misc.c:       ZFreeSignal(signal=0x%lx)\n",signal);
#endif

   if (signal)
      {
      i = CreateSignalNumber(signal);
      FreeSignal(i);
      }
}



void SetThosePrefs()
{
/*
   struct Preferences preferences;
*/
#if (DEBUG1 & DBG_SET_THOSE_PREFS_ENTER)
   kprintf("Misc.c:       SetThosePrefs(VOID)\n");
#endif

/*???   Forbid();*/
/*???   GetPrefs(&preferences, sizeof(struct Preferences));*/
/*???   preferences.color0 = TextTwoPlaneColors[0];*/
/*???   preferences.color1 = TextTwoPlaneColors[1];*/
/*???   preferences.color2 = TextTwoPlaneColors[2];*/
/*???   preferences.color3 = TextTwoPlaneColors[3];*/
/*???   SetPrefs(&preferences, sizeof(struct Preferences), TRUE);*/
/*???   Permit();*/
}



SHORT CreateSignalNumber(mask)
REGISTER ULONG mask;
{
   REGISTER SHORT i;

#if (DEBUG1 & DBG_CREATE_SIGNAL_NUMBER_ENTER)
   kprintf("Misc.c:       CreateSignalNumber(mask=0x%lx)\n",mask);
#endif

   i = 0;
   while ((mask & 1) == 0)
      {
      i++;
      mask >>= 1;
      }

   return(i); 
}


          
void TopRightLines(window)
REGISTER struct Window *window;
{
   REGISTER SHORT xright, ytop, ybottom;

#if (DEBUG1 & DBG_TOP_RIGHT_LINES_ENTER)
   kprintf("Misc.c:       TopRightLines(window=0x%lx)\n",window);
#endif

/*
   kprintf("Misc.c:       Width =%ld\n",window->Width);
   kprintf("Misc.c:       Height=%ld\n",window->Height);
*/

   xright = window->Width - window->BorderRight - 1;
   ytop = window->BorderTop - 1;
   ybottom = window->Height - window->BorderBottom - 1;

   SetDrMd(window->RPort, JAM2);

   SetAPen(window->RPort, 1);
   Move(window->RPort, 0, ytop - 1);
   Draw(window->RPort, window->Width - 1, ytop - 1);
   Move(window->RPort, LeftOfCloseGadget, 0);
   Draw(window->RPort, LeftOfCloseGadget, ytop - 1);
   Move(window->RPort, LeftOfCloseGadget + 1, 0);
   Draw(window->RPort, LeftOfCloseGadget + 1, ytop - 1);

   SetAPen(window->RPort, 0);
   Move(window->RPort, window->BorderLeft, ytop);
   Draw(window->RPort, xright, ytop);
}



void GetPCDisplay(display)
REGISTER struct Display *display;
{

#if (DEBUG1 & DBG_GET_PC_DISPLAY_ENTER)
   kprintf("Misc.c:       GetPCDisplay(display=0x%lx)\n",display);
#endif

#ifdef JANUS
   display->PCDisplay = display->JanusStart + display->PCDisplayOffset;
   display->PCCRTRegisterBase = display->JanusStart;

    /* If we're medium-res graphics, use the special graphics offset ... */
   if ( (display->Modes & MEDIUM_RES) 
         && ((display->Modes & TEXT_MODE) == 0) )
      display->PCDisplay += GRAPHIC_ACCESS_OFFSET;
   else   /* ... else just use the normal byte-oriented access */
      display->PCDisplay += BYTE_ACCESS_OFFSET;
                        
   if (display->Modes & COLOR_DISPLAY)
      {
      display->PCDisplay += COLOR_OFFSET;
      display->PCCRTRegisterBase += COLOR_CRT_OFFSET;
      }
   else
      {
      display->PCDisplay += MONO_OFFSET;
      display->PCCRTRegisterBase += MONO_CRT_OFFSET;
      }

#else
   if (display->PCDisplay == NULL)
      if ((display->PCDisplay
            = AllocRemember(&GlobalKey, 0x4000, MEMF_CLEAR | MEMF_CHIP) ) 
            == NULL)
         MyAlert(ALERT_NO_PC_DISPLAY_MEMORY, NULL);

   display->PCCRTRegisterBase = &FakeColorCRT[0];
#endif
}



void SetTimer(seconds, micros, timermessage)
REGISTER ULONG seconds, micros;
REGISTER struct IOStdReq *timermessage;
{
#if (DEBUG1 & DBG_SET_TIMER_ENTER)
   kprintf("Misc.c:       SetTimer(seconds=0x%lx,micros=0x%lx,timermessage=0x%lx)\n",seconds,micros,timermessage);
#endif

   timermessage->io_Command = TR_ADDREQUEST; /* add a new timer request */
   timermessage->io_Actual = seconds;
   timermessage->io_Length = micros;
   SendIO(timermessage);      /* post a request to the timer */
}



void DrawBox(rport, x, y, width, height, pen, hthick, vthick)
REGISTER struct RastPort *rport;
REGISTER SHORT x, y;
SHORT width, height, pen, hthick, vthick;
{
   REGISTER SHORT right, bottom, i;
   USHORT savemode, savepen;

#if (DEBUG1 & DBG_DRAW_BOX_ENTER)
   kprintf("Misc.c:       DrawBox(rport=0x%lx,x=0x%lx,y=0x%lx,width=0x%lx,height=0x%lx,pen=0x%lx,hthick=0x%lx,vthick=0x%lx)\n",rport,x,y,width,height,hthick,vthick);
#endif

   savemode = rport->DrawMode;
   savepen = rport->FgPen;

   SetAPen(rport, pen);
   SetDrMd(rport, JAM1);

   right = x + width - 1;
   bottom = y + height - 1;

   for (i = 0; i < hthick; i++)
      {
      Move(rport, x + i, y);
      Draw(rport, x + i, bottom);
      Move(rport, right - i, y);
      Draw(rport, right - i, bottom);
      }

   x += hthick;
   right -= hthick;

   for (i = 0; i < vthick; i++)
      {
      Move(rport, x, y + i);
      Draw(rport, right, y + i);
      Move(rport, x, bottom - i);
      Draw(rport, right, bottom - i);
      }

   SetDrMd(rport, savemode);
   SetAPen(rport, savepen);
}



void PALBottomBorder(w, display)
struct Window *w;
struct Display *display;
{
   struct RastPort *rport;
   SHORT borderbottom;
   SHORT windowbottom, savemode, savepen;

#if (DEBUG1 & DBG_PAL_BOTTOM_BORDER_ENTER)
   kprintf("Misc.c:       PALBottomBorder(w=0x%lx,display=0x%lx)\n",w,display);
#endif

   if (FlagIsSet(display->Modes, BORDER_VISIBLE)) return;

   borderbottom = w->BorderBottom;
   if (borderbottom - 1 > 0)
      {
      rport = w->RPort;
      windowbottom = w->Height - 1;
      savemode = rport->DrawMode;
      savepen = rport->FgPen;

      SetAPen(rport, 1);
      SetDrMd(rport, JAM1);
      borderbottom--;
      RectFill(rport, 0, windowbottom - (borderbottom - 1),
            w->Width - 1, windowbottom);
      }
}



void MyRefreshWindowFrame(w, display)
struct Window *w;
struct Display *display;
{
   struct RastPort *rport;
   SHORT borderleft, bordertop, borderright, borderbottom;
   SHORT windowbottom, windowright, savemode, savepen;

#if (DEBUG1 & DBG_MY_REFRESH_WINDOW_FRAME_ENTER)
   kprintf("Misc.c:       MyRefreshWindowFrame(w=0x%lx,display=0x%lx)\n",w,display);
#endif

/*
   kprintf("Misc.c:       Width =%ld\n",w->Width);
   kprintf("Misc.c:       Height=%ld\n",w->Height);
*/

   rport = w->RPort;
   windowbottom = w->Height - 1;
   windowright = w->Width - 1;
   borderleft = w->BorderLeft;
   borderright = w->BorderRight;
   bordertop = w->BorderTop;
   borderbottom = w->BorderBottom;

   savemode = rport->DrawMode;
   savepen = rport->FgPen;

   SetAPen(rport, 0);
   SetDrMd(rport, JAM1);

   if (borderleft)
      RectFill(rport, 0, 0, borderleft - 1, windowbottom);

   if (borderright)
      RectFill(rport, windowright - (borderright - 1), 0, 
            windowright, windowbottom);

   if (bordertop)
      RectFill(rport, 0, 0, windowright, bordertop - 1);

   if (borderbottom)
      {
      RectFill(rport, 0, windowbottom - (borderbottom - 1),
            windowright, windowbottom);

      if (FlagIsSet(display->Modes, PAL_PRESENCE))
         PALBottomBorder(w, display);
      }


   if ((w->Flags & BORDERLESS) == 0)
      {
      SetAPen(rport, 1);
      if ((bordertop - 2) <= 0)
         RectFill(rport, 0, 0, windowright, bordertop - 2);

      DrawBox(rport, 0, 0, w->Width, w->Height, (SHORT)w->BlockPen, 
            (SHORT)(borderleft >> 1), 1);
      }

   SetWindowTitles(w, w->Title, (UBYTE *)-1);

   RefreshGadgets(w->FirstGadget, w, NULL);

   SetAPen(rport, savepen);
   SetDrMd(rport, savemode);
}



/*???*/
ULONG ULongDivide(longa, longb)
LONG longa, longb;
{
   ULONG result;
   ULONG partial;
   LONG tempb;

#if (DEBUG1 & DBG_ULONG_DIVIDE_ENTER)
   kprintf("Misc.c:       ULongDivide(longa=0x%lx,longb=0x%lx)\n",longa,longb);
#endif
        
   longa >>= 1;
   result = 0;

   while (longa >= longb)
      {
      partial = 2;
      tempb = longb << 1;
      while (longa > tempb)
         {
         tempb <<= 1;
         partial <<= 1;
         }
      partial >>= 1;
      tempb >>= 1;
      result |= partial;
      longa -= tempb;
      }

   return(result << 1);
}




void ModifyDisplayProps(display)
struct Display *display;
{
   LONG partial;
   USHORT hbody, vbody;
   struct SuperWindow *superwindow;
   struct Window *window;
   struct PropInfo *pinfo;

#if (DEBUG1 & DBG_MODIFY_DISPLAY_PROPS_ENTER)
   kprintf("Misc.c:       ModifyDisplayProps(display=0x%lx)\n",display);
#endif

   MyLock(&display->DisplayLock);

   if (superwindow = display->FirstWindow)
      {
      if (window = superwindow->DisplayWindow)
         {
         partial = window->GZZWidth << 16;
         hbody = ULongDivide((ULONG)partial, (ULONG)window->MaxWidth);
         if (window->GZZHeight >= ZAPHOD_HEIGHT)
            vbody = 0xFFFF;
         else
            {
            partial = window->GZZHeight << 16;
            vbody = ULongDivide((ULONG)partial,
/*BBB                  (ULONG)window->MaxHeight);*/
                  ZAPHOD_HEIGHT);
            }

         if (display->Modes & BORDER_VISIBLE)
            {
            pinfo = (struct PropInfo *)&superwindow->HorizInfo;
            ModifyProp(&superwindow->HorizGadget, window, NULL, 
                  pinfo->Flags, pinfo->HorizPot, pinfo->VertPot,
                  hbody, 0);
            pinfo = (struct PropInfo *)&superwindow->VertInfo;
            ModifyProp(&superwindow->VertGadget, window, NULL, 
                  pinfo->Flags, pinfo->HorizPot, pinfo->VertPot,
                  0, vbody);
            }
         else
            {
            superwindow->HorizInfo.HorizBody = hbody;
            superwindow->VertInfo.VertBody = vbody;
            }
         }
      }

   Unlock(&display->DisplayLock);
}

                                  

void WaitTOFing(count)
SHORT count;
/* This is the Wait top of Field function.  Remember, the graphics
 * WaitTOF function is described incorrectly as being Wait top of Frame.
 * It's Field, friend.  Not Frame.
 */
{
      SHORT i, countdown;

#if (DEBUG1 & DBG_WAIT_TOFING_ENTER)
   kprintf("Misc.c:       WaitTOFing(count=0x%lx)\n",count);
#endif

   countdown = 0;
      for (i = 0; i < count; i++)
            {
            if (countdown == 0)
                  {
/*???                  printf(".");*/
                  countdown = 9;
                  }
            else countdown--;
            WaitTOF();
            }
}



SHORT TextXPos(display, x)
struct Display *display;
SHORT x;
{
      SHORT i;
      struct Window *window;

#if (DEBUG1 & DBG_TEXT_X_POS_ENTER)
   kprintf("Misc.c:       TextXPos(display=0x%lx,x=0x%lx)\n",display,x);
#endif

      window = display->FirstWindow->DisplayWindow;

      /* Build up the equation slowly here.  Start with the offset into
       * the non-border area of the window.
       */
      i = x - window->BorderLeft;
      /* Add this to the display column that's at the left edge of the window,
       * resulting in the display column position of x.
       */
      i += display->DisplayStartCol;
      /* Divide this by the width per character (rounding down) to find
       * the text column of x.
       */
      i = i / CHAR_WIDTH;

      return(i);
}


SHORT TextYPos(display, y)
struct Display *display;
SHORT y;
{
      SHORT i;
      struct Window *window;

#if (DEBUG1 & DBG_TEXT_Y_POS_ENTER)
   kprintf("Misc.c:       TextYPos(display=0x%lx,y=0x%lx)\n",display,y);
#endif

      window = display->FirstWindow->DisplayWindow;

      /* Build up the equation slowly here.  Start with the offset into
       * the non-border area of the window.
       */
      i = y - window->BorderTop;
      /* Add this to the display row that's at the top edge of the window,
       * resulting in the display row position of y.
       */
      i += display->DisplayStartRow;
      /* Divide this by the width per character (rounding down) to find
       * the text row of y.
       */
      i = i / CHAR_WIDTH;

      return(i);
}

void XAddGadget(window, gadget, pos)
struct Window *window;
struct Gadget *gadget;
SHORT pos;
/* Simple version of the real thing.  No relativity.  */
{
   SHORT apen, drawmode, left, right, top, bottom;
   struct RastPort *rport;

#if (DEBUG1 & DBG_X_ADD_GADGET_ENTER)
   kprintf("Misc.c:       XAddGadget(window=0x%lx,gadget=0x%lx,pos=0x%lx)\n",window,gadget,pos);
#endif

   rport = window->RPort;
   apen = rport->FgPen;
   drawmode = rport->DrawMode;
   SetAPen(rport, 0);
   SetDrMd(rport, JAM1);
   left = gadget->LeftEdge;
   right = left + gadget->Width - 1;
   top = gadget->TopEdge;
   bottom = top + gadget->Height - 1;
   RectFill(rport, left, top, right, bottom);
   AddGadget(window, gadget, pos);
}





