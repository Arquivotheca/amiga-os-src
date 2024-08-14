
/**** auxtask.c *************************************************************
 * 
 * Auxiliary Tools Task for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 29 Jan 88   -RJ         Added all the mouse and scroll stuff
 * 22 Mar 86   =RJ Mical=  Created this file
 *
 ***************************************************************************/

#include "zaphod.h"            
#include <proto/exec.h>
#include <proto/graphics.h>

#define  DBG_AUX_TOOLS_MANAGER_ENTER         1
#define  DBG_AUX_TOOLS_MANAGER_RETURN        1
#define  DBG_TALK_WITH_ZAPHOD_ENTER          1
#define  DBG_TALK_WITH_ZAPHOD_RETURN         1
#define  DBG_THE_FAT_LADY_SINGS_ENTER        1
#define  DBG_THE_FAT_LADY_SINGS_RETURN       1
#define  DBG_OPEN_AUX_TOOLS_ENTER            1
#define  DBG_OPEN_AUX_TOOLS_RETURN           1
#define  DBG_CLOSE_AUX_TOOLS_ENTER           1
#define  DBG_CLOSE_AUX_TOOLS_RETURN          1
#define  DBG_PUT_NEW_PRIORITY_ENTER          1
#define  DBG_PUT_NEW_PRIORITY_RETURN         1
#define  DBG_KILL_AUX_TOOLS_ENTER            1
#define  DBG_KILL_AUX_TOOLS_RETURN           1
#define  DBG_MAKE_AUX_TOOLS_ENTER            1
#define  DBG_MAKE_AUX_TOOLS_RETURN           1
#define  DBG_GET_DISPLAY_LIST_ENTER          1
#define  DBG_GET_DISPLAY_LIST_RETURN         1
#define  DBG_DUPLICATE_DISPLAY_ENTER         1
#define  DBG_DUPLICATE_DISPLAY_RETURN        1
#define  DBG_GET_QUEUE_ROUTINE_ENTER         1
#define  DBG_GET_QUEUE_ROUTINE_RETURN        1
#define  DBG_CHANGE_WB_WINDOW_COUNT_ENTER    1
#define  DBG_CHANGE_WB_WINDOW_COUNT_RETURN   1
#define  DBG_COUNT_WB_WINDOWS_ENTER          1
#define  DBG_COUNT_WB_WINDOWS_RETURN         1
#define  DBG_LINK_DISPLAY_ENTER              1
#define  DBG_LINK_DISPLAY_RETURN             1
#define  DBG_UNLINK_DISPLAY_ENTER            1
#define  DBG_UNLINK_DISPLAY_RETURN           1

extern   VOID   Cursor();
extern   VOID   InputMonitor();
extern   VOID   QueueOneKey();

SHORT   WBWindowCount = 0;

/****i* PCWindow/AuxToolsManager ******************************************
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
* This task exists to manage the display list and the creation/elimination
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
*
*/

VOID AuxToolsManager()
{
   ULONG waitBits, wakeUpBits, inputSignal;
   struct AuxToolsMessage *message;
   struct MsgPort *auxToolsPort;
   BOOL testend;

#if (DEBUG1 & DBG_AUX_TOOLS_MANAGER_ENTER)
   kprintf("AuxTask.c:    AuxToolsManager(VOID)\n");
#endif

/* === Critical Section.  Must be completed in toto ==================== */ 
   Forbid();

   /*********************/
   /* Open AuxToolsPort */
   /*********************/
   if ((auxToolsPort = CreatePort(AUX_TOOLS_PORT, 0)) == NULL)
   {
      MyAlert(ALERT_NO_INPUTPORT, NULL);
      TheFatLadySings((struct MsgPort *)NULL);
   }

   inputSignal = 1 << auxToolsPort->mp_SigBit;

   MakeAuxTools();

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
                  LinkDisplay((struct Display *)message->AuxArg);
                  break;
               case REMOVE_DISPLAY:
                  UnlinkDisplay((struct Display *)message->AuxArg);
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
            }
            ReplyMsg((struct Message *)message);
         }
         if (testend) if (ZaphodDisplay.FirstDisplay == NULL)
            TheFatLadySings((struct MsgPort *)auxToolsPort);
         Permit();
      }
   }

#if (DEBUG2 & DBG_AUX_TOOLS_MANAGER_RETURN)
   kprintf("AuxTask.c:    AuxToolsManager: Returns(VOID)\n");
#endif
}

/****i* PCWindow/TalkWithZaphod ******************************************
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

LONG TalkWithZaphod(display, auxtype, auxarg)
struct Display *display;
SHORT auxtype;
LONG auxarg;
{
   struct MsgPort *port, *auxport;
   struct AuxToolsMessage *message;
   LONG returnval;

#if (DEBUG1 & DBG_TALK_WITH_ZAPHOD_ENTER)
   kprintf("AuxTask.c:    TalkWithZaphod(display=0x%lx,auxtype=0x%lx,auxarg=0x%lx)\n",display,auxtype,auxarg);
#endif

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

#if (DEBUG2 & DBG_TALK_WITH_ZAPHOD_RETURN)
   kprintf("AuxTask.c:    TalkWithZaphod: Returns(0x%lx)\n",returnval);
#endif

   return (returnval);
}

/****i* PCWindow/TheFatLadySings ******************************************
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

VOID TheFatLadySings(auxToolsPort)
struct MsgPort *auxToolsPort;      
{
   struct AuxToolsMessage *message;

#if (DEBUG1 & DBG_THE_FAT_LADY_SINGS_ENTER)
   kprintf("AuxTask.c:    TheFatLadySings(auxtoolsport=0x%lx)\n",auxToolsPort);
#endif

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

#if (DEBUG2 & DBG_THE_FAT_LADY_SINGS_RETURN)
   kprintf("AuxTask.c:    TheFatLadySings: Returns(VOID)\n");
#endif

   RemTask(0);
}

/****i* PCWindow/OpenAuxTools ******************************************
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

VOID OpenAuxTools(struct Display *display)
{
#if (DEBUG1 & DBG_OPEN_AUX_TOOLS_ENTER)
   kprintf("AuxTask.c:    OpenAuxTools(display=0x%lx)\n",display);
#endif

   Forbid();
   if (FindPort(AUX_TOOLS_PORT) == NULL)
   {
      if(MyCreateTask(AUX_TASK_NAME, 0, (APTR)AuxToolsManager, 1000) == NULL)
         MyAlert(ALERT_NO_MEMORY, display);

      if ((AuxToolsFinalPort = CreatePort("AuxToolsFinalPort", 0)) == NULL)
         MyAlert(ALERT_NO_MEMORY, display);
   }
   Permit();

   while (FindPort(AUX_TOOLS_PORT) == NULL) WaitTOF();

#if (DEBUG2 & DBG_OPEN_AUX_TOOLS_RETURN)
   kprintf("AuxTask.c:    OpenAuxTools: Returns(VOID)\n");
#endif
}

/****i* PCWindow/CloseAuxTools ******************************************
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

VOID CloseAuxTools(struct Display *display)
{
#if (DEBUG1 & DBG_CLOSE_AUX_TOOLS_ENTER)
   kprintf("AuxTask.c:    CloseAuxTools(display=0x%lx)\n",display);
#endif

   if (FindPort(AUX_TOOLS_PORT))
      TalkWithZaphod(display, REMOVE_DISPLAY, (LONG)display);

#if (DEBUG2 & DBG_CLOSE_AUX_TOOLS_RETURN)
   kprintf("AuxTask.c:    CloseAuxTools: Returns(VOID)\n");
#endif
}

/****i* PCWindow/PutNewPriority ******************************************
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

VOID PutNewPriority(struct Display *display,SHORT priority) 
{
#if (DEBUG1 & DBG_PUT_NEW_PRIORITY_ENTER)
   kprintf("AuxTask.c:    PutNewPriority(display=0x%lx,priority=0x%lx)\n",display,priority);
#endif

   if (FindPort(AUX_TOOLS_PORT))
      TalkWithZaphod(display, NEW_PRIORITY, priority);

#if (DEBUG2 & DBG_PUT_NEW_PRIORITY_RETURN)
   kprintf("AuxTask.c:    PutNewPriority: Returns(VOID)\n");
#endif
}

/****i* PCWindow/KillAuxTools ******************************************
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

VOID KillAuxTools()
{
#if (DEBUG1 & DBG_KILL_AUX_TOOLS_ENTER)
   kprintf("AuxTask.c:    KillAuxTools(VOID)\n");
#endif

   Forbid();

   if (InputTask) Signal(InputTask, InputSuicideSignal);
   if (CursorTask) Signal(CursorTask, CursorSuicideSignal);

   Permit();

#if (DEBUG2 & DBG_KILL_AUX_TOOLS_RETURN)
   kprintf("AuxTask.c:    KillAuxTools: Returns(VOID)\n");
#endif
}

/****i* PCWindow/MakeAuxTools ******************************************
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

VOID MakeAuxTools()
{
#if (DEBUG1 & DBG_MAKE_AUX_TOOLS_ENTER)
   kprintf("AuxTask.c:    MakeAuxTools(VOID)\n");
#endif

   if ((InputTask = FindTask(INPUT_TASK_NAME)) == NULL)
      if (MyCreateTask(INPUT_TASK_NAME, (UBYTE)(DisplayPriority + 1),
            (APTR)InputMonitor, 3000) == NULL)
         MyAlert(ALERT_NO_MEMORY, NULL);

   if ((CursorTask = FindTask(CURSOR_TASK_NAME)) == NULL)
      if (MyCreateTask(CURSOR_TASK_NAME, (UBYTE)CursorPriority,
            (APTR)Cursor, 2000) == NULL)
         MyAlert(ALERT_NO_MEMORY, NULL);

#if (DEBUG2 & DBG_MAKE_AUX_TOOLS_RETURN)
   kprintf("AuxTask.c:    MakeAuxTools: Returns(VOID)\n");
#endif
}

/****i* PCWindow/GetDisplayList ******************************************
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

struct DisplayList *GetDisplayList(struct Display *display)
{
   struct DisplayList *displayList;

#if (DEBUG1 & DBG_GET_DISPLAY_LIST_ENTER)
   kprintf("AuxTask.c:    GetDisplayList(display=0x%lx)\n",display);
#endif

   displayList = (struct DisplayList *)TalkWithZaphod(display, 
         GET_DISPLAYLIST, NULL);

#if (DEBUG2 & DBG_GET_DISPLAY_LIST_RETURN)
   kprintf("AuxTask.c:    GetDisplayList: Returns(0x%lx)\n",displayList);
#endif

   return (displayList);
}

/****i* PCWindow/DuplicateDisplay ******************************************
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

BOOL DuplicateDisplay(struct Display *display)
{
   struct DisplayList *displaylist;
   struct Display *workdisplay;
   BOOL retvalue;

#if (DEBUG1 & DBG_DUPLICATE_DISPLAY_ENTER)
   kprintf("AuxTask.c:    DuplicateDisplay(display=0x%lx)\n",display);
#endif

   retvalue = FALSE;

   if (displaylist = GetDisplayList(NULL))
      {
      MyLock(&displaylist->DisplayListLock);

      workdisplay = displaylist->FirstDisplay;

      while (workdisplay)
         {
         if ((display->Modes & COLOR_DISPLAY)
               == (workdisplay->Modes & COLOR_DISPLAY))
            retvalue = TRUE;
         workdisplay = workdisplay->NextDisplay;
         }

      Unlock(&displaylist->DisplayListLock);
      }

#if (DEBUG2 & DBG_DUPLICATE_DISPLAY_RETURN)
   kprintf("AuxTask.c:    DuplicateDisplay: returns 0x%lx\n",retvalue);
#endif
   return(retvalue);
}
  
/****i* PCWindow/GetQueueRoutine ******************************************
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

VOID GetQueueRoutine(LONG *queuer)
{
#if (DEBUG1 & DBG_GET_QUEUE_ROUTINE_ENTER)
   kprintf("AuxTask.c:    GetQueueRoutine(queuer=0x%lx)\n",queuer);
#endif

   *queuer = (LONG)TalkWithZaphod(NULL, GET_QUEUEROUTINE, NULL);

#if (DEBUG2 & DBG_GET_QUEUE_ROUTINE_RETURN)
   kprintf("AuxTask.c:    GetQueueRoutine: Returns(VOID)\n");
#endif
}

/****i* PCWindow/ChangeWBWindowCount ***************************************
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

VOID ChangeWBWindowCount(LONG value)
{
#if (DEBUG1 & DBG_CHANGE_WB_WINDOW_COUNT_ENTER)
   kprintf("AuxTask.c:    ChangeWBWindowCount(value=0x%lx)\n",value);
#endif

   TalkWithZaphod(NULL, CHANGE_WBWINDOWS, value);

#if (DEBUG2 & DBG_CHANGE_WB_WINDOW_COUNT_RETURN)
   kprintf("AuxTask.c:    ChangeWBWindowCount: Returns(VOID)\n");
#endif
}

/****i* PCWindow/CountWBWindows ******************************************
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

SHORT CountWBWindows()
{
   SHORT count;

#if (DEBUG1 & DBG_COUNT_WB_WINDOWS_ENTER)
   kprintf("AuxTask.c:    CountWBWindows(VOID)\n");
#endif

   count = (SHORT)TalkWithZaphod(NULL, COUNT_WBWINDOWS, 0);

#if (DEBUG2 & DBG_COUNT_WB_WINDOWS_RETURN)
   kprintf("AuxTask.c:    CountWBWindows: Returns(0x%lx)\n",count);
#endif

   return(count);
}

/****i* PCWindow/LinkDisplay ******************************************
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

VOID LinkDisplay(struct Display *display)
{
#if (DEBUG1 & DBG_LINK_DISPLAY_ENTER)
   kprintf("AuxTask.c:    LinkDisplay(display=0x%lx)\n",display);
#endif

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

#if (DEBUG2 & DBG_LINK_DISPLAY_RETURN)
   kprintf("AuxTask.c:    LinkDisplay: Returns(VOID)\n");
#endif
}

/****i* PCWindow/UnlinkDisplay ******************************************
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

VOID UnlinkDisplay(struct Display *display)
{
   struct Display *workdisplay;

#if (DEBUG1 & DBG_UNLINK_DISPLAY_ENTER)
   kprintf("AuxTask.c:    UnlinkDisplay(display=0x%lx)\n",display);
#endif

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

#if (DEBUG2 & DBG_UNLINK_DISPLAY_RETURN)
   kprintf("AuxTask.c:    UnlinkDisplay: Returns(VOID)\n");
#endif
}
