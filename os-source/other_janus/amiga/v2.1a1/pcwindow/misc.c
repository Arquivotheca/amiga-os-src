
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
#include <exec/memory.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <janus/memory.h>
#include <janus/janusreg.h>

extern struct Library *DiskfontBase;

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

/****i* PCWindow/MyCreateTask ******************************************
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
   taskptr->tc_SPUpper = (APTR)(((LONG)(stackptr) + realsize-2) & 0xFFFFFE);
   taskptr->tc_SPReg   = taskptr->tc_SPUpper;

   taskptr->tc_Node.ln_Type = NT_TASK;
   taskptr->tc_Node.ln_Pri  = priority;
   taskptr->tc_Node.ln_Name = (char *)name;

   AddTask(taskptr, (char *)initialPC, 0);

#if (DEBUG2 & DBG_MY_CREATE_TASK_RETURN)
   kprintf("Misc.c:       MyCreateTask: Returns(0x%lx)\n",taskptr);
#endif

   return(taskptr);
}

/****i* PCWindow/CloseEverything ******************************************
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

VOID CloseEverything()
{
/* === Critical Section (for safety's sake) ========================== */

#if (DEBUG1 & DBG_CLOSE_EVERYTHING_ENTER)
   kprintf("Misc.c:       CloseEverything(VOID)\n");
#endif

   Forbid();

   CloseTextClip();

   FreeRemember(&GlobalKey, TRUE);

   if (GfxBase)         CloseLibrary((struct Library *)GfxBase);
   if (LayersBase)      CloseLibrary((struct Library *)LayersBase);
   if (DiskfontBase)    CloseLibrary(DiskfontBase);
   if (IntuitionBase)   CloseLibrary((struct Library *)IntuitionBase);
   if (JanusBase)       CloseLibrary((struct Library *)JanusBase);

   Permit();
/* === End of Critical Section ====================================== */
}

/****i* PCWindow/DrainPort ******************************************
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
               
VOID DrainPort(port)
struct MsgPort *port;
/* NOTE:  DrainPort() does *not* Forbid() and Permit().  There's little
 * point in doing so.  If the caller needs this quality, the caller 
 * should provide it (and probably needs to for other reasons as well).
 */
{
   struct IntuiMessage *message;

#if (DEBUG1 & DBG_DRAIN_PORT_ENTER)
   kprintf("Misc.c:       DrainPort(port=0x%lx)\n",port);
#endif

   if (port)
      while (message = (struct IntuiMessage *)GetMsg(port)) 
	     ReplyMsg((struct Message *)message);
}

/****i* PCWindow/FixPropPots ******************************************
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

VOID FixPropPots(propgadget)
struct Gadget *propgadget;
{
   SHORT start, freeplay, pot;

#if (DEBUG1 & DBG_FIX_PROP_POTS_ENTER)
   kprintf("Misc.c:       FixPropPots(propgadget=0x%lx)\n",propgadget);
#endif

   start = ((struct Image *)propgadget->GadgetRender)->LeftEdge;

   freeplay =   ((struct PropInfo *)propgadget->SpecialInfo)->CWidth;
   freeplay -= (((struct PropInfo *)propgadget->SpecialInfo)->LeftBorder*2);
   freeplay -=  ((struct Image    *)propgadget->GadgetRender)->Width;

   /* This conditional makes sure that the max value is hit.
    * It should also guard against divide-by-zero error, because if
    * freeplay is 0, start *should* be 0 too, but I put in the extra 
    * test just to be safe.
    */
   if ((start == freeplay) || (freeplay == 0)) 
      pot = MAXPOT;
   else 
      pot = (((LONG)MAXPOT * start) + (freeplay >> 1)) / freeplay;

   ((struct PropInfo *)propgadget->SpecialInfo)->HorizPot = pot;


   start = ((struct Image *)propgadget->GadgetRender)->TopEdge;

   freeplay =  ((struct PropInfo *)propgadget->SpecialInfo)->CHeight;
   freeplay -= (((struct PropInfo *)propgadget->SpecialInfo)->TopBorder * 2);
   freeplay -= ((struct Image *)propgadget->GadgetRender)->Height;

   if ((start == freeplay) || (freeplay == 0)) 
      pot = MAXPOT;
   else 
      pot = (((LONG)MAXPOT * start) + (freeplay >> 1)) / freeplay;

   ((struct PropInfo *)propgadget->SpecialInfo)->VertPot = pot;
}

/****i* PCWindow/ZAllocSignal ******************************************
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

ULONG ZAllocSignal()
{
   SHORT i;

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

/****i* PCWindow/ZFreeSignal ******************************************
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

VOID ZFreeSignal(signal)
ULONG signal;
{
   SHORT i;

#if (DEBUG1 & DBG_Z_FREE_SIGNAL_ENTER)
   kprintf("Misc.c:       ZFreeSignal(signal=0x%lx)\n",signal);
#endif

   if (signal)
   {
      i = CreateSignalNumber(signal);
      FreeSignal(i);
   }
}

/****i* PCWindow/CreateSignalNumber ***************************************
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

SHORT CreateSignalNumber(mask)
ULONG mask;
{
   SHORT i;

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

/****i* PCWindow/GetPCDisplay ******************************************
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

VOID GetPCDisplay(display)
struct Display *display;
{

#if (DEBUG1 & DBG_GET_PC_DISPLAY_ENTER)
   kprintf("Misc.c:       GetPCDisplay(display=0x%lx)\n",display);
#endif

   display->PCDisplay = display->JanusStart + display->PCDisplayOffset;
   display->PCCRTRegisterBase = display->JanusStart;

    /* If we're medium-res graphics, use the special graphics offset ... */
   if ( (display->Modes & MEDIUM_RES)&&((display->Modes & TEXT_MODE) == 0) )
      display->PCDisplay += GRAPHIC_ACCESS_OFFSET;
   else   /* ... else just use the normal byte-oriented access */
      display->PCDisplay += BYTE_ACCESS_OFFSET;
                        
   if (display->Modes & COLOR_DISPLAY)
   {
      display->PCDisplay         += COLOR_OFFSET;
      display->PCCRTRegisterBase += COLOR_CRT_OFFSET;
   } else {
      display->PCDisplay         += MONO_OFFSET;
      display->PCCRTRegisterBase += MONO_CRT_OFFSET;
   }
}

/****i* PCWindow/SetTimer ******************************************
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

VOID SetTimer(seconds, micros, timermessage)
ULONG seconds, micros;
struct IOStdReq *timermessage;
{
#if (DEBUG1 & DBG_SET_TIMER_ENTER)
   kprintf("Misc.c:       SetTimer(seconds=0x%lx,micros=0x%lx,timermessage=0x%lx)\n",seconds,micros,timermessage);
#endif

   timermessage->io_Command = TR_ADDREQUEST;/* add a new timer request */
   timermessage->io_Actual = seconds;
   timermessage->io_Length = micros;
   SendIO((struct IORequest *)timermessage);/* post a request to the timer */
}

/****i* PCWindow/DrawBox ******************************************
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

VOID DrawBox(rport, x, y, width, height, pen, hthick, vthick)
struct RastPort *rport;
SHORT x, y;
SHORT width, height, pen, hthick, vthick;
{
   SHORT right, bottom, i;
   USHORT savemode, savepen;

#if (DEBUG1 & DBG_DRAW_BOX_ENTER)
   kprintf("Misc.c:       DrawBox(rport=0x%lx,x=0x%lx,y=0x%lx,width=0x%lx,height=0x%lx,pen=0x%lx,hthick=0x%lx,vthick=0x%lx)\n",rport,x,y,width,height,hthick,vthick);
#endif

   savemode = rport->DrawMode;
   savepen  = rport->FgPen;

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

/****i* PCWindow/PALBottomBorder ******************************************
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

VOID PALBottomBorder(w, display)
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

/****i* PCWindow/ModifyDisplayProps ****************************************
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

VOID ModifyDisplayProps(display)
struct Display *display;
{
   LONG partial;
   ULONG hbody, vbody;
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
#if 0
	  kprintf("###window->GZZWidth=%ld\n",window->GZZWidth);
	  kprintf("###window->GZZHeight=%ld\n",window->GZZHeight);
	  kprintf("###window->Width=%ld\n",window->Width);
	  kprintf("###window->Height=%ld\n",window->Height);
	  kprintf("###window->MaxWidth=%ld\n",window->MaxWidth);
	  kprintf("###window->MaxHeight=%ld\n",window->MaxHeight);
#endif
         hbody = (window->GZZWidth*65535)/
		         (FlagIsSet(display->Modes,MEDIUM_RES)?320:640);
         vbody = (window->GZZHeight*65535)/ZAPHOD_HEIGHT;

         /* kprintf("###hbody=%ld\n",hbody);  */
         /* kprintf("###vbody=%ld\n",vbody);  */

         if (display->Modes & BORDER_VISIBLE)
         {
            pinfo = (struct PropInfo *)&superwindow->HorizInfo;
            ModifyProp(&superwindow->HorizGadget, window, NULL, 
                  pinfo->Flags, pinfo->HorizPot, pinfo->VertPot,
                  (USHORT)hbody, 0);
            pinfo = (struct PropInfo *)&superwindow->VertInfo;
            ModifyProp(&superwindow->VertGadget, window, NULL, 
                  pinfo->Flags, pinfo->HorizPot, pinfo->VertPot,
                  0,(USHORT) vbody);
         } else {
            superwindow->HorizInfo.HorizBody = (USHORT)hbody;
            superwindow->VertInfo.VertBody = (USHORT)vbody;
         }
      }
   }

   Unlock(&display->DisplayLock);
}

/****i* PCWindow/WaitTOFing ******************************************
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
                                  
VOID WaitTOFing(count)
SHORT count;
/* This is the Wait top of Field function.  Remember, the graphics
 * WaitTOF function is described incorrectly as being Wait top of Frame.
 * It's Field, friend.  Not Frame.
 */
{
   SHORT i;

#if (DEBUG1 & DBG_WAIT_TOFING_ENTER)
   kprintf("Misc.c:       WaitTOFing(count=0x%lx)\n",count);
#endif

   for (i = 0; i < count; i++)
   {
      WaitTOF();
   }
}

/****i* PCWindow/TextXPos ******************************************
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

/****i* PCWindow/TextYPos ******************************************
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

/****i* PCWindow/XAddGadget ******************************************
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

VOID XAddGadget(window, gadget, pos)
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

   rport    = window->RPort;
   apen     = rport->FgPen;
   drawmode = rport->DrawMode;
   SetAPen(rport, 0);
   SetDrMd(rport, JAM1);
   left     = gadget->LeftEdge;
   right    = left + gadget->Width - 1;
   top      = gadget->TopEdge;
   bottom   = top + gadget->Height - 1;
   RectFill(rport, left, top, right, bottom);
   AddGadget(window, gadget, pos);
}

/****i* PCWindow/PrintDisplayModes *****************************************
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
#if DEBUG
VOID PrintDisplayModes(struct Display *display)
{
   kprintf("Display->Modes=");

   if(FlagIsSet(display->Modes,MEDIUM_RES))
      kprintf("MEDIUM_RES | ");

   if(FlagIsSet(display->Modes,TEXT_MODE))
      kprintf("TEXT_MODE | ");

   if(FlagIsSet(display->Modes,SELECTED_AREA))
      kprintf("SELECTED_AREA | ");

   if(FlagIsSet(display->Modes,BLINK_TEXT))
      kprintf("BLINK_TEXT | ");

   if(FlagIsSet(display->Modes,PALETTE_1))
      kprintf("PALETTE_1 | ");

   if(FlagIsSet(display->Modes,OPEN_SCREEN))
      kprintf("OPEN_SCREEN | ");

   if(FlagIsSet(display->Modes,NEWSIZE_REFRESH))
      kprintf("NEWSIZE_REFRESH | ");

   if(FlagIsSet(display->Modes,SELECT_PRESSED))
      kprintf("SELECT_PRESSED | ");

   if(FlagIsSet(display->Modes,BORDER_VISIBLE))
      kprintf("BORDER_VISIBLE | ");

   if(FlagIsSet(display->Modes,GOT_NEWSIZE))
      kprintf("GOT_NEWSIZE | ");

   if(FlagIsSet(display->Modes,PAL_PRESENCE))
      kprintf("PAL_PRESENCE | ");

   if(FlagIsSet(display->Modes,COLOR_DISPLAY))
      kprintf("COLOR_DISPLAY | ");

   if(FlagIsSet(display->Modes,VERBOSE))
      kprintf("VERBOSE | ");

   if(FlagIsSet(display->Modes,SQUELCH_NEWSIZE))
      kprintf("SQUELCH_NEWSIZE | ");

   if(FlagIsSet(display->Modes,PROP_ACTIVE))
      kprintf("PROP_ACTIVE | ");

   if(FlagIsSet(display->Modes,COUNT_DISPLAY))
      kprintf("COUNT_DISPLAY | ");

   kprintf("\n");

   kprintf("Display->Modes2=");

   if(FlagIsSet(display->Modes2,WBCOLORS_GRABBED))
      kprintf("WBCOLORS_GRABBED | ");

   if(FlagIsSet(display->Modes2,WINDOW_PRESETS))
      kprintf("WINDOW_PRESETS | ");

   if(FlagIsSet(display->Modes2,FULLSIZE))
      kprintf("FULLSIZE | ");

   kprintf("\n");
}
#endif
