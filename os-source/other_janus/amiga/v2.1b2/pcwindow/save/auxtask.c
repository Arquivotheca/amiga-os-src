
/* *** auxtask.c *************************************************************
 * 
 * Auxiliary Tools Task for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 29 Jan 88   -RJ         Added all the mouse and scroll stuff
 * 22 Mar 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/


#include "zaphod.h"            



extern   VOID   Cursor();
extern   VOID   InputMonitor();
extern   VOID   QueueOneKey();
LONG   TalkWithZaphod();
/*NOSCROLL extern   USHORT   *ScrollFlags;*/

SHORT   WBWindowCount = 0;

void AuxToolsManager();
void TheFatLadySings();
void OpenAuxTools();
void CloseAuxTools();
void PutNewPriority();
void KillAuxTools();
void MakeAuxTools();
void GetQueueRoutine();
void LinkDisplay();
void UnlinkDisplay();


void AuxToolsManager()
/* This task exists to manage the display list and the creation/elimination
 * of the auxiliary tools.  This is a precursor to the Zaphod library 
 * main system, which will render this extra step unnecessary since all
 * the auxiliary tools will come and go via ZaphodBase.  However,
 * knowing how engineering projects tend to go (I just barely managed
 * to fend off an unwanted trip to New York yesterday), I think I'll make
 * this kludge as robust as possible since it may become permanent.
 *
 * This guy sets up the auxiliary tasks.  It also sets up a message port
 * and then hangs around for display tasks to tell it that they are
 * coming and going.  This port is globally accessible (by nature of
 * its curious name) to all display tasks to rendevous with it as
 * desired.   Whenever a task adds its display to the fray, 
 * this task links it into the master list.  When a task signals to
 * extract its display, this task checks if all displays are gone and if 
 * so it asks everyone, including itself, to commit suicide (see the
 * TheFatLadySings() function).
 */
{
   ULONG waitBits, wakeUpBits, inputSignal;
   struct AuxToolsMessage *message;
   struct MsgPort *auxToolsPort;
   BOOL testend;

#ifdef DEBUG
   kprintf("AuxTask.c: AuxToolsManager(Enter)\n");
#endif

#ifdef AZTEC
   /* This gets A4 from a memory location.  This is needed because
    * Manx has implemented relative-addressing using A4.  Task object files
    * should be clustered around the call to SaveA4();
    */
   GetA4();
#endif

#ifdef DEBUG
   kprintf("AuxTask.c: AuxToolsManager(After A4)\n");
#endif

/* === Critical Section.  Must be completed in toto ==================== */ 
   Forbid();

   if ((auxToolsPort = CreatePort(AUX_TOOLS_PORT, 0)) == NULL)
      {
      MyAlert(ALERT_NO_INPUTPORT, NULL);
      TheFatLadySings(NULL);
      }

#ifdef DEBUG
   kprintf("AuxToolsManager: After CreatePort\n");
#endif

   inputSignal = 1 << auxToolsPort->mp_SigBit;

   MakeAuxTools();

#ifdef DEBUG
   kprintf("AuxToolsManager: After MakeAuxTools\n");
#endif

   Permit();
/* === End of Critical Section ========================================== */

   waitBits = inputSignal;

   ZaphodDisplay.FirstDisplay = NULL;

   FOREVER
      {
      wakeUpBits = Wait(waitBits);

      testend = FALSE;

      if (wakeUpBits & inputSignal)
         {
         Forbid();
         while (message = (struct AuxToolsMessage *)GetMsg(auxToolsPort))
            {
            switch(message->AuxType)
               {
               case COUNT_WBWINDOWS:
                  message->AuxArg = WBWindowCount;
                  break;
               case CHANGE_WBWINDOWS:
                  WBWindowCount += 
                     message->AuxArg;
                  break;
               case ADD_DISPLAY:
                  LinkDisplay(message->AuxArg);
                  break;
               case REMOVE_DISPLAY:
                  UnlinkDisplay(message->AuxArg);
                  testend = TRUE;
                  break;
               case GET_DISPLAYLIST:
                  message->AuxArg = (LONG)&ZaphodDisplay;
                  break;
               case GET_QUEUEROUTINE:
                  message->AuxArg = (LONG)QueueOneKey;
                  break;
               case NEW_PRIORITY:
                  DisplayPriority = (SHORT)message->AuxArg;
                  CursorPriority = DisplayPriority + CURSORPRIORITY_OFFSET;
                  if (InputTask)
                     SetTaskPri(InputTask, DisplayPriority + 1);
                  if (CursorTask) SetTaskPri(CursorTask, CursorPriority);
                  break;
               case DEADEN_BLINK:
                  DeadenBlink = TRUE;
                  break;
               case REVIVE_BLINK:
                  DeadenBlink = FALSE;
                  break;
/*???               case TOGGLE_PCMOUSE:*/
/*???                  TogglePCMouse(TRUE, message->AuxArg);*/
/*???                  break;*/
/*???               case CLEAR_PCMOUSE:*/
/*???                  ClearPCMouse(TRUE, message->AuxArg);*/
/*???                  break;*/

/*NOSCROLL                case GET_SCROLLFLAGS:*/
   /* Return the address of the scroll flags */
/*NOSCROLL                   while (ScrollFlags == NULL)*/
/*NOSCROLL                      WaitTOF();*/
/*NOSCROLL                   message->AuxArg = (LONG)ScrollFlags;*/
/*NOSCROLL                   break;*/
/*NOSCROLL case SCROLL_THAT_SUCKER:*/
/*NOSCROLL {*/
/*NOSCROLL extern ULONG ScrollSignal;*/
/*NOSCROLL if (ScrollFlags) ToggleFlag(*ScrollFlags, 1);*/
/*NOSCROLL if (InputTask) Signal(InputTask, ScrollSignal);*/
/*NOSCROLL }*/
/*NOSCROLL break;*/

               }
            ReplyMsg((struct Message *)message);
            }
         if (testend) if (ZaphodDisplay.FirstDisplay == NULL)
            TheFatLadySings(auxToolsPort);
         Permit();
         }
      }
}



LONG TalkWithZaphod(display, auxtype, auxarg)
struct Display *display;
SHORT auxtype;
LONG auxarg;
{
   struct MsgPort *port, *auxport;
   struct AuxToolsMessage *message;
   LONG returnval;

   if (port = CreatePort(0, 0))
      message = (struct AuxToolsMessage *)CreateExtIO(port, sizeof(struct AuxToolsMessage));

   if ((port == NULL) || (message == NULL))
      {
      MyAlert(ALERT_NO_MEMORY, display);
      return (NULL);
      }

   message->AuxType = auxtype;
   message->AuxArg = auxarg;

   while ((auxport = FindPort(AUX_TOOLS_PORT)) == NULL) WaitTOF();
   PutMsg(auxport, (struct Message *)message);
   WaitPort(port);
   returnval = message->AuxArg;
   DeleteExtIO((struct IORequest *)message);
   DeletePort(port);

   return (returnval);
}


void TheFatLadySings(auxToolsPort)
struct MsgPort *auxToolsPort;      
{
   struct AuxToolsMessage *message;

   Forbid();
          
   DeathLock(&ZaphodDisplay.DisplayListLock);

   KillAuxTools();

   if (auxToolsPort)
      {
      DrainPort(auxToolsPort);
      DeletePort(auxToolsPort);
      }

   if (AuxToolsFinalPort)
      {
      while (InputTask) WaitTOF();
      while (CursorTask) WaitTOF();

      if ((message = (struct AuxToolsMessage *)CreateExtIO(AuxToolsFinalPort,
            sizeof(struct AuxToolsMessage)) ) == NULL)
         MyAlert(ALERT_NO_MEMORY, NULL);
      else 
         PutMsg(AuxToolsFinalPort, (struct Message *)message);
      }

   RemTask(0);
}



void OpenAuxTools(display)
struct Display *display;
{

#ifdef DEBUG
   kprintf("OpenAuxTools(0x%lx)\n",display);
#endif

   Forbid();
   if (FindPort(AUX_TOOLS_PORT) == NULL)
      {
      if (MyCreateTask(AUX_TASK_NAME, 0, AuxToolsManager, 1000) == NULL)
         MyAlert(ALERT_NO_MEMORY, display);
      if ((AuxToolsFinalPort = CreatePort("AuxToolsFinalPort", 0)) == NULL)
         MyAlert(ALERT_NO_MEMORY, display);
      }
   Permit();

#ifdef DEBUG
   kprintf("OpenAuxTools: Before while\n");
#endif

   while (FindPort(AUX_TOOLS_PORT) == NULL) WaitTOF();

#ifdef DEBUG
   kprintf("OpenAuxTools: After while\n");
#endif
}



void CloseAuxTools(display)
struct Display *display;
{
   if (FindPort(AUX_TOOLS_PORT))
      TalkWithZaphod(display, REMOVE_DISPLAY, display);
}



void PutNewPriority(display, priority)
struct Display *display;
SHORT priority; 
{
   if (FindPort(AUX_TOOLS_PORT))
      TalkWithZaphod(display, NEW_PRIORITY, priority);
}



void KillAuxTools()
{
   Forbid();

   if (InputTask) Signal(InputTask, InputSuicideSignal);
   if (CursorTask) Signal(CursorTask, CursorSuicideSignal);

   Permit();
}


void MakeAuxTools()
{
   if ((InputTask = FindTask(INPUT_TASK_NAME)) == NULL)
      if (MyCreateTask(INPUT_TASK_NAME, DisplayPriority + 1,
            InputMonitor, 3000) == NULL)
         MyAlert(ALERT_NO_MEMORY, NULL);

   if ((CursorTask = FindTask(CURSOR_TASK_NAME)) == NULL)
      if (MyCreateTask(CURSOR_TASK_NAME, CursorPriority,
            Cursor, 2000) == NULL)
         MyAlert(ALERT_NO_MEMORY, NULL);
}



struct DisplayList *GetDisplayList(display)
struct Display *display;
{
   struct DisplayList *displayList;

   displayList = (struct DisplayList *)TalkWithZaphod(display, 
         GET_DISPLAYLIST, NULL);

   return (displayList);
}



DuplicateDisplay(display)
struct Display *display;
{
   struct DisplayList *displaylist;
   struct Display *workdisplay;
   BOOL retvalue;

#ifdef DEBUG
   kprintf("AuxTask.c: DuplicateDisplay(0x%lx)\n",display);
#endif

   retvalue = FALSE;

   if (displaylist = GetDisplayList(NULL))
      {
      MyLock(&displaylist->DisplayListLock);

      workdisplay = displaylist->FirstDisplay;

#ifdef DEBUG
   kprintf("AuxTask.c: DuplicateDisplay: workdisplay=0x%lx\n",workdisplay);
#endif

      while (workdisplay)
         {
         if ((display->Modes & COLOR_DISPLAY)
               == (workdisplay->Modes & COLOR_DISPLAY))
            retvalue = TRUE;
         workdisplay = workdisplay->NextDisplay;
         }

      Unlock(&displaylist->DisplayListLock);
      }

#ifdef DEBUG
   kprintf("AuxTask.c: DuplicateDisplay: returns 0x%lx\n",retvalue);
#endif
   return(retvalue);
}


  
void GetQueueRoutine(queuer)
LONG *queuer;
{
   *queuer = (LONG)TalkWithZaphod(NULL, GET_QUEUEROUTINE, NULL);
}



VOID   ChangeWBWindowCount(value)
{
   TalkWithZaphod(NULL, CHANGE_WBWINDOWS, value);
}



SHORT   CountWBWindows()
{
   REGIST   SHORT   count;

   count = (SHORT)TalkWithZaphod(NULL, COUNT_WBWINDOWS, 0);
   return(count);
}



/* ************************************************************************
 * A NOTE ABOUT THE ABANDONBLINKING OF Link/UnlinkDisplay();
 *
 * Whenever the display list is being mangled, hard-cancel the cursor
 * task's use of the list.  This allows the cursor task to not be required
 * to lock the display list before using it, which means that it doesn't
 * have to compete with the input monitor task for access to the display
 * list.  This is highly desirable, since the cursor task runs a *lot*
 * and it is locked out also by needing access to the Display's Lock,
 * which, since the cursor task runs at a lower priority than the
 * display rendering task, can take some considerable amount of time,
 * all during which the input monitor would be required to wait and
 * wait.  So instead, the cursor task ignores the DisplayListLock, 
 * and checks the AbandonBlink variable for a warning that the display
 * list may have changed.  This is not the most elegant systemy solution,
 * but it works very well and I for one am willing to sacrifice system
 * perfection for performance when it's safe to do so and the gain is
 * as great as this is.
 *
 * AbandonBlink = TRUE;
 */

void LinkDisplay(display)
struct Display *display;
/* See the note above */
{
   Forbid();

   /* Start out by telling the cursor task that we're about to muck 
    * about with the display list, so it should abandon any blink link
    * traversals it may be attempting.
    */
   AbandonBlink = TRUE;
   MyLock(&ZaphodDisplay.DisplayListLock);
   display->NextDisplay = ZaphodDisplay.FirstDisplay;
   ZaphodDisplay.FirstDisplay = display;
   Unlock(&ZaphodDisplay.DisplayListLock);

   Permit();
}


void UnlinkDisplay(display)
struct Display *display;
/* See the note above */
{
   struct Display *workdisplay;

   Forbid();

   /* Start out by telling the cursor task that we're about to muck 
    * about with the display list, so it should abandon any blink link
    * traversals it may be attempting.
    */
   AbandonBlink = TRUE;

   MyLock(&ZaphodDisplay.DisplayListLock);

   if (ZaphodDisplay.FirstDisplay == display)
      ZaphodDisplay.FirstDisplay = display->NextDisplay;
   else
      {
      workdisplay = ZaphodDisplay.FirstDisplay;

      while (workdisplay->NextDisplay != display)
         workdisplay = workdisplay->NextDisplay;
      workdisplay->NextDisplay = display->NextDisplay;
      }
   Unlock(&ZaphodDisplay.DisplayListLock);

   Permit();
}


