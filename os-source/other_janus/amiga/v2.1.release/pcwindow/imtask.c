
/**** imtask.c **************************************************************
 * 
 * Input Monitor Task for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 24 Jul 88   -RJ         Added the cursor key support for Henri
 * 26 Jan 88   -RJ         Started adding the mouse and scroll stuff to the 
 *                         logic of the input monitor
 * 15 Oct 87   - RJ        Added call to timer (variable duration) between 
 *                         sending keys to PC.  Fixes a problem for Rudolf 
 *                         and hopefully can be used by Clipboard stuff too.
 *                         All changes should be marked TIMER
 * 1 Mar 86    =RJ Mical=  Created this file
 *
 ***************************************************************************/
#define LINT_ARGS
#define PRAGMAS

#include "zaphod.h"
#include <janus/jfuncs.h>
#include <janus/janusvar.h>
#include <janus/janusreg.h>
#include <stdio.h>


#define D(x) ;

#define  DBG_INPUT_MONITOR_ENTER                         0
#define  DBG_INPUT_MONITOR_OFFSETS                       0
#define  DBG_QUEUE_PC_RAW_KEY_ENTER                      0
#define  DBG_QUEUE_ONE_KEY_ENTER                         0
#define  DBG_SEND_ONE_KEY_ENTER                          0
#define  DBG_SEND_ONE_KEY_ADDRESSES                      0
#define  DBG_CLOSE_INPUT_MONITOR_TASK_ENTER              0
#define  DBG_IRVING_FORCES_FF_DOWN_ZAPHODS_THROAT_ENTER  0

ULONG ChangeState(ULONG NewState,ULONG CurrentState);

VOID PrintState(ULONG State,UBYTE *s);

#define NOSHIFT  0x0000
#define CONTROL  0x0001
#define CAPSLOCK 0x0002
#define LSHIFT   0x0004
#define RSHIFT   0x0008
#define ALT      0x0010
#define ALTK     0x0020
#define LAMIGA   0x0040
#define RAMIGA   0x0080
#define NUMLOCK  0x0100
/*
#define SCRLOCK  0x0200
#define PAUSE    0x0400
#define PRTSC    0x0800
*/
#define  INPUT_BUFFERSIZE    6
static struct   IOStdReq IOStdReq = {0};
static struct   InputEvent InputEvent = {0};
static UBYTE    InputBuffer[INPUT_BUFFERSIZE];

/* === Keyboard Events Variables ======================================== */
static SHORT    KeyBufferNextSend = 0;
static SHORT    KeyBufferNextSlot = 0;
static UBYTE    KeyBuffer[KEYBUFFER_SIZE];
static BOOL     PCWantsKey = FALSE;
static SHORT    KeyFlags = NULL;
static SHORT    InputFlags = NULL;
static SHORT    MoveStartX, MoveStartY;
static UBYTE    *KeyboardAddress;
static ULONG    PCWantsKeySignal = 0;

/* === InputFlags definitions =========================================== */
#define IN_SELECT_MODE  0x0001
#define IN_MOVE_MODE    0x0002

static ULONG PCCurrentState=0;

ULONG KeyPadCode(UBYTE code);
/****i* PCWindow/InputMonitor ******************************************
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
* The input monitor task spends its energy monitoring the main input
* port for the Zaphod tasks.  When input events arrive, the following
* evaluation is made:
*
*    - if the input is not RAWKEY, it is transmitted to the appropriate
*      display task
*    - if the input is RAWKEY, this input is translated into a PC-style
*      keycode and then sent via the Janus interrupt manager to the PC
*
*
* The InputMonitor creates the main input port, which all Zaphod windows
* will use for their IDCMP ports.  All display tasks must create 
* alternate input ports, through which they will receive input events
* from the InputMonitor after the InputMonitor has tested for RAWKEY.
*
* The motivation for all of this is to allow input to flow quickly to
* the PC while other tasks manage the load of dealing with the display.
* 
* This task is created by the others.  When another task wants to find
* this one but finds that this one doesn't exist, it creates this one.
*
* If the SuicideSignal is received, this task quietly suicides.
*
*/

VOID InputMonitor()
{
   ULONG    wakeUpBits;
   ULONG    waitBits, inputSignal;
   ULONG    timerSignal;
   struct IntuiMessage *message;
   struct SuperWindow *superwindow;
   struct   Window *inputWindow;
   struct Display *display;
   struct   MsgPort *inputPort;
   struct   MsgPort *timerPort;
   struct   IOStdReq *timerMsg;
   BOOL     timerSet, pcrawkey;
   SHORT    i;
   BOOL   TimeToDie=FALSE;
   /*static struct   Library *ConsoleDevice = NULL; */

#if (DEBUG1 & DBG_INPUT_MONITOR_ENTER)
   kprintf("imtask.c:     InputMonitor(VOID)\n");
#endif

   timerPort = NULL;
   timerMsg  = NULL;
   timerSet  = NULL;

   /*=== Critical Section. Must be completed in toto ===*/ 
   Forbid();

   InputTask = FindTask(0);
   InputSuicideSignal = ZAllocSignal();

   Permit();
   /*=== End of Critical Section ===*/

   if(OpenDevice("console.device",-1,(struct IORequest *)&IOStdReq,0)==0)
   {
      ConsoleDevice = (struct Library *)IOStdReq.io_Device;
      InputEvent.ie_Class = IECLASS_RAWKEY;
   }

   if ((inputPort = CreatePort(INPUT_PORT_NAME, 0)) == NULL)
   {
      MyAlert(ALERT_NO_INPUTPORT, NULL);
      CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet);
   }

   inputSignal = 1L << inputPort->mp_SigBit;


   /* CreatePort(name, priority); */
   if (timerPort = CreatePort("KeyTimerPort", 0))
      timerMsg = CreateStdIO(timerPort);

   if ((timerPort == NULL) || (timerMsg == NULL))
   {
      MyAlert(ALERT_NO_TIMERPORT, NULL);
      CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet);
   }

   if(OpenDevice(TIMERNAME,UNIT_MICROHZ,(struct IORequest *)timerMsg,0)!=0)
   {
      MyAlert(ALERT_INCOMPLETESYSTEM, NULL);
      CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet);
   }

   timerSignal = 1L << timerPort->mp_SigBit;

   /* Establish the PC interrupt stuff */
   KeyboardAddress= (UBYTE *)((ULONG)GetJanusStart() + (ULONG)IOREG_OFFSET);
   KeyboardAddress+=((ULONG)JanusBase->jb_KeyboardRegisterOffset);

#if (DEBUG3 & DBG_INPUT_MONITOR_OFFSETS)
   kprintf("Imtask.c:     Board offset = 0x%lx\n",GetJanusStart());
   kprintf("Imtask.c:     Register offset = 0x%lx\n",JanusBase->jb_KeyboardRegisterOffset);
   kprintf("Imtask.c:     Keyboard offset = 0x%lx\n",KeyboardAddress);
#endif

   /* When the PC is ready to receive a rawkey event, this task will
    * receive this signal.  Until we receive the first PCWantsKeySignal
    * (which comes to us when the PC establishes the link back to us)
    * we presume that PCWantsKey is FALSE.
    */
   PCWantsKeySignal = ZAllocSignal();
   PCWantsKey = FALSE;

   if ( (PCKeySig = (struct SetupSig *)SetupJanusSig(JSERV_ENBKB,
         CreateSignalNumber(PCWantsKeySignal), 0, 0)) == NULL)
   {
      MyAlert(ALERT_NO_JANUS_SIG, NULL);
      CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet);
   }

   KeyBufferNextSend = KeyBufferNextSlot = 0;
   PCWantsKey = TRUE;
   /*???    QueueOneKey(0xFF);*/

   /* Queue up some events that will "clear" the PC keyboard state */
   /*???    QueueOneKey(PCCtrlCode | 0x80);  /* CTRL UP */
   /*???    QueueOneKey(PCAltCode | 0x80);  /* ALT UP */
   /*???    QueueOneKey(PCLeftShiftCode | 0x80);  /* Left SHIFT UP */
   /*???    QueueOneKey(PCRightShiftCode | 0x80);  /* Right SHIFT UP */

   ClearFlag(KeyFlags, KEY_ALTKEY);
   ClearFlag(KeyFlags, KEY_AMIGAPENDING);
   InputFlags = 0;

   if (FlagIsSet(JanusAmigaWA->ja_AmigaState, AMIGA_NUMLOCK_RESET))
   {
      /*?kprintf("Imtask.c:     On entry, RESET flag was set\n");*/
      ClearFlag(JanusAmigaWA->ja_AmigaState, AMIGA_NUMLOCK_RESET);
      /*???   QueueOneKey(PCNumLockCode);*/
      /*???   QueueOneKey(PCNumLockCode | 0x80);*/
      SetFlag(KeyFlags, KEY_NUMLOCK_SET);
   } else
      if (FlagIsSet(JanusAmigaWA->ja_AmigaState, AMIGA_NUMLOCK_SET))
      {
         /*?kprintf("Imtask.c:     On entry SET flag was set\n");*/
         SetFlag(KeyFlags, KEY_NUMLOCK_SET);
      }

   waitBits = inputSignal | PCWantsKeySignal | InputSuicideSignal
             | timerSignal;

   FOREVER
   {
      wakeUpBits = Wait(waitBits);

      if (wakeUpBits & InputSuicideSignal) 
     {
        ChangeState(NOSHIFT,PCCurrentState);
        TimeToDie=TRUE;
     }
      if((TimeToDie)&&(KeyBufferNextSend == KeyBufferNextSlot))
         CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet);

      if (wakeUpBits & PCWantsKeySignal)
      {
         if (NOT timerSet)
         {
            SetTimer(KeyDelaySeconds, KeyDelayMicros, timerMsg);
            timerSet = TRUE;
         }
      }

      if (wakeUpBits & timerSignal)
      {
         GetMsg(timerPort);
         timerSet = FALSE;
         /* TIMER:  This used to be part of 
          * "if (wakeUpBits & PCWantsKeySignal)" above
          * but now happens when the timer tells 
          * us that it's time to consider sending 
          * another key
          */
         if (KeyBufferNextSend == KeyBufferNextSlot)
         /* There's no key events queued to be sent, so just
          * indicate that the PC is hungry for keyboard input
          */
            PCWantsKey = TRUE;
         else
            SendOneKey();
      }

      if (wakeUpBits & inputSignal)
      {
         while (message = (struct IntuiMessage *)GetMsg(inputPort))
         {
            /* I've put the Forbid() inside the loop intentionally.
             * This will allow other urgent tasks to get a time slice.
             * This is program politesse.
             */
            Forbid();
            inputWindow = message->IDCMPWindow;

            /* By the way, I don't reply immediately with the
             * message here because I don't always want to reply, 
             * for code below may send the message to another port
             */

      if (message->Class == RAWKEY)
      {
         /*
          * If the class of the message is RAWKEY, then it's a
          * raw keycode that we're going to send to the PC
          * except if it's the HELP keycode, in which case
          * we send it to the application.
          */
         pcrawkey = TRUE;
         if (ConsoleDevice)
         {
            InputEvent.ie_Code = message->Code;
            InputEvent.ie_Qualifier = message->Qualifier;
            i = RawKeyConvert(&InputEvent, &InputBuffer[0], 
                           INPUT_BUFFERSIZE, NULL);

         /***********************/
            /* RawKeyConvert DEBUG */
         /***********************/
#if 0
            kprintf("i=%ld ",i);
         { 
            UBYTE gg;

               if(i>0)
            {
               kprintf("HEX: ");
               for(gg=0;gg<i;gg++)
               {
                  kprintf("%02lx ",InputBuffer[gg]);
               }
               kprintf("ASCII: ");
               for(gg=0;gg<i;gg++)
               {
                  if(
                       ((InputBuffer[gg]>=0x20)&&(InputBuffer[gg]<=0x7e))||
                       ((InputBuffer[gg]>=0xa1)&&(InputBuffer[gg]<=0xff))
                    )
                     kprintf("%2lc ",(ULONG)InputBuffer[gg]);
                 else
                     kprintf("NP ");
               }
            } else {
#if 0
               kprintf("HEX: ");
               kprintf("%02lx ",InputEvent.ie_Code);
               kprintf("ASCII: ");
               if(
                    ((InputEvent.ie_Code>=0x20)&&(InputEvent.ie_Code<=0x7e))||
                    ((InputEvent.ie_Code>=0xa1)&&(InputEvent.ie_Code<=0xff))
                 )
                  kprintf(" %c ",InputEvent.ie_Code);
              else
                  kprintf("NP ");
#endif
            }
         }
         kprintf("\n");
#endif

            if ( (i == 3)
                && (InputBuffer[0] == 0x9B)
                && (InputBuffer[1] == 0x3F)
                && (InputBuffer[2] == 0x7E) )
               pcrawkey = FALSE;
         }
      }
      else
         pcrawkey = FALSE;

      if (pcrawkey)
      {
         /* So, here I could do one of two things.  I could
          * check to make sure that the window of this message
          * is the same as the window of the display that the
          * PC regards as active for output (monochrome or
          * color, doncha know), and not transmit the key
          * if the two don't agree.  On the other hand, why
          * bother?  Why not just send all key events of PC
          * windows to the PC, and let the user sort it out.
          * I'm not even sure that I *can* tell which
          * display the PC regards as active.
          */
       if(!TimeToDie)
            QueuePCRawKey(message);
         ReplyMsg((struct Message *)message);
      } else {
         /* The default is to transmit the event
          * to the display port
          */
         if (message->Class == MOUSEBUTTONS)
         {
            if (message->Code == SELECTDOWN)
            {
               SetFlag(InputFlags, IN_SELECT_MODE);
               MoveStartX = message->MouseX;
               MoveStartY = message->MouseY;
            } else
               if (message->Code == SELECTUP)
               {
                  ClearFlag(InputFlags, IN_SELECT_MODE);
                  ClearFlag(InputFlags, IN_MOVE_MODE);
               }
         } else
            if (message->Class == GADGETDOWN)
            {
               SetFlag(InputFlags, IN_SELECT_MODE);
               MoveStartX = message->MouseX;
               MoveStartY = message->MouseY;
            } else
               if (message->Class == GADGETUP)
               {
                  ClearFlag(InputFlags, IN_SELECT_MODE);
                  ClearFlag(InputFlags, IN_MOVE_MODE);
               }

            if (message->Class == MOUSEMOVE)
            {
               if (FlagIsSet(InputFlags, IN_SELECT_MODE))
                  if (FlagIsClear(InputFlags, IN_MOVE_MODE))
                     if((ABS(message->MouseX - MoveStartX) > CHAR_WIDTH)
                         ||(ABS(message->MouseY - MoveStartY)>CHAR_HEIGHT) )
                        SetFlag(InputFlags, IN_MOVE_MODE);

               if (FlagIsClear(InputFlags, IN_MOVE_MODE))
               {
                  ReplyMsg((struct Message *)message);
                  goto END_OF_MESSAGE;
               }
            }

            if (MyLock(&ZaphodDisplay.DisplayListLock))
            {
               Forbid();
               display = ZaphodDisplay.FirstDisplay;
               while (display)
               {
                  if (MyLock(&display->DisplayLock))
                  {
                     superwindow = display->FirstWindow;
                     while (superwindow)
                     {
                        if (inputWindow == superwindow->DisplayWindow)
                        {
                           if (display->TaskPort)
                                 PutMsg(display->TaskPort,
                                    (struct Message *) message);
                              Unlock(&display->DisplayLock);
                              goto WHEW;
                           }   /* end of finding our window */
                           superwindow = superwindow->NextWindow;
                        }   /* end of superwindow while-loop */
                        Unlock(&display->DisplayLock);
                     }   /* end of examining one display */
                     display = display->NextDisplay;
                  }   /* end of display while-loop */
WHEW: /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
                  Permit();
                  Unlock(&ZaphodDisplay.DisplayListLock);
               }   /* end of feeling through display list */
            }   /* end of wanting to send this message to a window */
END_OF_MESSAGE:
            Permit();
         }   /* end of processing one inputSignal message */
      }   /* end of having been awakened by the inputSignal */
   }   /* end of FOREVER */
}   /* end of the world */


/****i* PCWindow/QueueOneKey ******************************************
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

VOID QueueOneKey(UBYTE code)
{
   SHORT i;

   /*D(kprintf("QueueOneKey(%lx)\n", code);)*/

   i = (KeyBufferNextSlot + 1) & KEYBUFFER_MASK;
   if (i == KeyBufferNextSend)
   {
      /* Oops, buffer overflow.  If this is the down transition, beep */
      if ((code & 0x80) == 0)
         DisplayBeep(NULL);
   } else {
      KeyBuffer[KeyBufferNextSlot] = code;
      KeyBufferNextSlot = i;
   } 

   if (PCWantsKey) SendOneKey();
}

/****i* PCWindow/SendOneKey ******************************************
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

VOID SendOneKey()
/* The caller should correctly ascertain that there's a key to send
 * before calling this routine.  This allows this routine to avoid
 * checking if there's a key to send, which optimizes the case where
 * somebody already knows there's one to send (for instance, when one
 * was just stuffed into the buffer while the PC is waiting for a key).
 */  
{
   UBYTE *janus;

#if (DEBUG1 & DBG_SEND_ONE_KEY_ENTER)
   kprintf("imtask.c:     SendOneKey(VOID)\n");
#endif

   janus = (UBYTE *)GetJanusStart();

   /* Write the data to the PC keycode-input register */
   *(KeyboardAddress) = KeyBuffer[KeyBufferNextSend];

   /* kprintf("imtask.c:     SendOneKey: Sending 0x%lx\n",
              (int)KeyBuffer[KeyBufferNextSend]); */

#if (DEBUG3 & DBG_SEND_ONE_KEY_ADDRESSES)
   kprintf("imtask.c:     SendOneKey Janus = 0x%lx\n",janus);
   kprintf("imtask.c:     SendOneKey KeyboardAddress = 0x%lx\n",KeyboardAddress);
#endif


   /* Tell the PC that we've got a key ready in the register.  Writing
    * this value to this register causes a keyboard interrupt on the PC.
    */
   *(janus + PCINTGEN_OFFSET) = JPCKEYINT;
   /* kprintf("Key sent\n"); */

   PCWantsKey = FALSE;

   KeyBufferNextSend = (KeyBufferNextSend + 1) & KEYBUFFER_MASK;
}

/****i* PCWindow/CloseInputMonitorTask *************************************
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

VOID CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet)
struct MsgPort *inputPort;
struct MsgPort *timerPort;
struct IOStdReq *timerMsg;
BOOL timerSet;
{
   struct Task *task;

#if (DEBUG1 & DBG_CLOSE_INPUT_MONITOR_TASK_ENTER)
   kprintf("imtask.c:     CloseInputMonitorTask(inputPort=0x%lx,timerPort=0x%lx,timerMsg=0x%lx,timerSet=0x%lx)\n",inputPort,timerPort,timerMsg,timerSet);
#endif

   Forbid();

   if (JanusAmigaWA)
   {
      if (FlagIsSet(KeyFlags, KEY_NUMLOCK_SET))
         SetFlag(JanusAmigaWA->ja_AmigaState, AMIGA_NUMLOCK_SET);
      else
         ClearFlag(JanusAmigaWA->ja_AmigaState, AMIGA_NUMLOCK_SET);
   }

   if (inputPort)
   {
      DrainPort(inputPort);
      DeletePort(inputPort);
   }

   if (timerMsg)
   {
      if (timerSet) 
         if (GetMsg(timerPort) == NULL) 
          Wait(1L << timerPort->mp_SigBit);
      CloseDevice((struct IORequest *)timerMsg);
      DeleteStdIO(timerMsg);
   }
   if (timerPort) 
      DeletePort(timerPort);

   if (PCKeySig) 
      CleanupJanusSig(PCKeySig);
   ZFreeSignal(PCWantsKeySignal);

   task = InputTask;
   InputTask = NULL;

   RemTask(task);
}
#if 0

/****i* PCWindow/QueuePCRawKey ******************************************
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

VOID QueuePCRawKey(message)
struct IntuiMessage *message;
{   
   UBYTE  code;
   UBYTE  outcode,
          outkey;
   USHORT qualifier;

#if (DEBUG1 & DBG_QUEUE_PC_RAW_KEY_ENTER)
   kprintf("imtask.c:     QueuePCRawKey(message=0x%lx)\n",message);
   kprintf("imtask.c:     QueuePCRawKey: code = 0x%lx)\n",message->Code);
#endif
           
   code      = message->Code & 0xFF;
   qualifier = message->Qualifier;

   D(kprintf("QueuePCRawKey(%lx, %lx)\n", code, qualifier);)

   /***************/
   /* AmigaRight? */
   /***************/
   if (qualifier & AMIGARIGHT)
      SetFlag(KeyFlags, KEY_AMIGA);
   else
      ClearFlag(KeyFlags, KEY_AMIGA);


   if((code==AmigaCapsLockCode) || (code==(AmigaCapsLockCode | 0x80)))
   {
      /* Thus sprach Dieter.   Both CAPSLOCK DOWN
       * and CAPSLOCK UP should generate both CAPSLOCK DOWN and CAPSLOCK UP.
       */
      QueueOneKey(PCCapsLockCode);
      QueueOneKey((UBYTE)(PCCapsLockCode | 0x80));
   } else
      if((code==AmigaLeftAltCode) || (code==AmigaRightAltCode))
      {
         if ((KeyFlags & KEY_ALTKEY) == 0)
         {
            /* We don't have ALT down yet, so transmit one now */
            QueueOneKey(PCAltCode);
            SetFlag(KeyFlags, KEY_ALTKEY);
         }
      } else
         if ((code == (AmigaLeftAltCode | 0x80))
              || (code == (AmigaRightAltCode | 0x80)))
         {
            if (KeyFlags & KEY_ALTKEY) /* If we've seen an alt-down */
         {
               if ((qualifier & ALTKEYS) == 0) /* If both alts are up */
               {
                  QueueOneKey((UBYTE)(PCAltCode | 0x80));
                  ClearFlag(KeyFlags, KEY_ALTKEY); 
               }
         }
         } else
            if ( (code != 0xFF)
                 && ((code == AmigaTildeBackDash)
                 || (code == AmigaBarBackSlash)) )
            {
               QueueOneKey(PCCtrlCode);
               QueueOneKey(PCAltCode);
               if (FlagIsSet(qualifier, SHIFTY))
               {
                  if (code == AmigaTildeBackDash)
              {
                     outkey = PCTildeCode;
                  } else {
                     outkey = PCBarCode;
              }
               } else {
                  if (code == AmigaTildeBackDash)
              {
                     outkey = PCBackDashCode;
              } else {
                     outkey = PCBackSlashCode;
              }
               }
               QueueOneKey(outkey);
               QueueOneKey((UBYTE)(outkey | 0x80));
               QueueOneKey((UBYTE)(PCAltCode | 0x80));
               QueueOneKey((UBYTE)(PCCtrlCode | 0x80));
            } else
               if ( (code != 0xFF)
                     && ( (code == AmigaCursorUp) 
                     || (code == AmigaCursorLeft) 
                     || (code == AmigaCursorRight) 
                     || (code == AmigaDelCode) 
                     || (code == AmigaCursorDown) ) )
               {
                  /* Cursor Up == PCNumLock + PCKeypad8Code */
                  /* Cursor Left == PCNumLock + PCKeypad4Code */
                  /* Cursor Right == PCNumLock + PCKeypad6Code */
                  /* Cursor Down == PCNumLock + PCKeypad2Code */
                  /* Del == PCNumLock + PCKeypadDotCode */
                  if (FlagIsClear(KeyFlags, KEY_NUMLOCK_SET))
                  {
                     QueueOneKey(PCNumLockCode);
                     QueueOneKey((UBYTE)(PCNumLockCode | 0x80));
                  }
#if 0
              switch(code)
              {
                 case AmigaCursorUp:
                        outcode = PCKeypad8Code;
                  break;
                 case AmigaCursorLeft:
                        outcode = PCKeypad4Code;
                  break;
                 case AmigaCursorRight:
                        outcode = PCKeypad6Code;
                  break;
                 case AmigaCursorDown:
                        outcode = PCKeypad2Code;
                  break;
                 case AmigaDelCode:
                        outcode = PCKeypadDotCode;
                  break;
              }
#endif
#if 1
                  if (code == AmigaCursorUp)
              {
                     outcode = PCKeypad8Code;
              } else {
                     if (code == AmigaCursorLeft)
                {
                        outcode = PCKeypad4Code;
                } else {
                        if (code == AmigaCursorRight)
                  {
                           outcode = PCKeypad6Code;
                  } else {
                           if (code == AmigaCursorDown)
                     {
                              outcode = PCKeypad2Code;
                     } else {
                              if (code == AmigaDelCode)
                       {
                                 outcode = PCKeypadDotCode;
                       }
                     }
                  }
                 }
              }
#endif
                  QueueOneKey(outcode);
                  QueueOneKey((UBYTE)(outcode | 0x80));
                  if (FlagIsClear(KeyFlags, KEY_NUMLOCK_SET))
                  {
                     QueueOneKey(PCNumLockCode);
                     QueueOneKey((UBYTE)(PCNumLockCode | 0x80));
                  }
               } else
                  if ( (code != 0xFF)
                        && ( (code == (AmigaCursorUp | 0x80) ) 
                        || (code == (AmigaCursorLeft | 0x80) ) 
                        || (code == (AmigaCursorRight | 0x80) ) 
                        || (code == (AmigaDelCode | 0x80) ) 
                        || (code == (AmigaCursorDown | 0x80) ) )) 
                  {
                     /* On the upstroke of the cursor keys, do nothing */
                  } else {
                     /* Else, let's do normal translations */
                     if (KeyFlags & (KEY_AMIGA | KEY_AMIGAPENDING))
                     {
                        /* While the Right AMIGA key is down, let's try for
                         * the special translations.
                         */
                        if (code == AmigaNCode)
                        {
                           /* AMIGARIGHT N == Num Lock */
                           QueueOneKey(PCNumLockCode);
                           SetFlag(KeyFlags, KEY_AMIGAPENDING); 
                           ToggleFlag(KeyFlags, KEY_NUMLOCK_SET); 
                        } else {
                           if (code == (AmigaNCode | 0x80))
                           {
                              /* AMIGARIGHT N up == Num Lock up */
                              QueueOneKey((UBYTE)(PCNumLockCode | 0x80));
                              ClearFlag(KeyFlags, KEY_AMIGAPENDING);
                           } else {
                              if (code == AmigaSCode)
                              {
                                 /* AMIGARIGHT S == Scroll Lock */
                                 QueueOneKey(PCScrollLockCode);
                                 SetFlag(KeyFlags, KEY_AMIGAPENDING); 
                              } else {
                                 if (code == (AmigaSCode | 0x80))
                                 {
                                    /* AMIGARIGHT S up == Scroll Lock up */
                                    QueueOneKey((UBYTE)(PCScrollLockCode | 0x80));
                                    ClearFlag(KeyFlags, KEY_AMIGAPENDING); 
                                 } else {
                                    if (code == AmigaPlusCode)
                                    {
                                       /* AMIGARIGHT + == + on PC num keypad */
                                       QueueOneKey(PCPlusCode);
                                       SetFlag(KeyFlags, KEY_AMIGAPENDING); 
                                    } else {
                                       if (code == (AmigaPlusCode | 0x80))
                                       {
                                          /* AMIGARIGHT + up == + up on num keypad */
                                          QueueOneKey((UBYTE)(PCPlusCode | 0x80));
                                          ClearFlag(KeyFlags, KEY_AMIGAPENDING); 
                                       } else {
                                          if (code == AmigaPCode)
                                          {
                                             /* AMIGARIGHT P == PrtScr */
                                             QueueOneKey(PCPtrScrCode);
                                             SetFlag(KeyFlags, KEY_AMIGAPENDING); 
                                          } else {
                                             if (code == (AmigaPCode | 0x80))
                                             {
                                                /* AMIGARIGHT P up == PrtScr up */
                                                QueueOneKey((UBYTE)(PCPtrScrCode | 0x80));
                                                ClearFlag(KeyFlags, KEY_AMIGAPENDING); 
                                             } else {
                                                goto INELEGANT_SO_BUGGER_OFF;
                                  }
                                }
                              }
                           }
                         }
                       }
                     }
                  }
                     } else {
                        /* Output this key using normal table lookup */
INELEGANT_SO_BUGGER_OFF:
                        /* ... after just one more special case, Prt Scr */
                        /* Rudolf sez no:if (code == AmigaPrtScrCode) QueueOneKey(PCLeftShiftCode);*/

                        if(code==0x43)    /* key pad ENTER down */
                        {
                           /*
                           QueueOneKey(0xE0);
                           QueueOneKey(0x1C);
                           */
                           QueueOneKey(0x6c);
                           return;
                        }
                        if(code==0xC3)    /* keypad ENTER up */
                        {
                           /*
                           QueueOneKey(0xE0);
                           QueueOneKey(0x9C);
                           */
                           QueueOneKey(0xec);
                           return;
                        }
                        outcode = PCRawKeyTable[code & 0x7F];
                        outcode = (outcode & 0x7F) | (code & 0x80);
                        if ((outcode & 0x7F) != 0x7F)
                           QueueOneKey(outcode);

                        if (outcode == PCNumLockCode)
                        {
                           ToggleFlag(KeyFlags, KEY_NUMLOCK_SET); 
                        }
                        /* Rudolf sez no:if (code == AmigaPrtScrCode) QueueOneKey(PCLeftShiftCode | 0x80);*/
                     }
                  }
}

#else
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

#if 0
  $38  ; The PC Alt code
  $3A  ; The PC Caps Lock code
  $1D  ; The PC Ctrl code
  $2A  ; The PC Left Shift code
  $45  ; The PC Num Lock code
  $4E  ; The PC + (on the numeric keypad) code
  $37  ; The PC Ptr Scr * code
  $36  ; The PC Right Shift code
  $46  ; The PC Scroll Lock code
  $0D  ; The PC Tilde code
  $08  ; The PC Bar code
  $0C  ; The PC BackDash code
  $2B  ; The PC BackSlash code
#endif

struct KeySeq {
      ULONG State;
     UBYTE Code;
   };

#define MAXCODE (0x68)

/*** US Keyboard ***/
static struct KeySeq A2PCNoShift[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {NOSHIFT,0x29}, /* Tilde */
   {NOSHIFT,0x02}, /* 1 */
   {NOSHIFT,0x03}, /* 2 */
   {NOSHIFT,0x04}, /* 3 */
   {NOSHIFT,0x05}, /* 4 */
   {NOSHIFT,0x06}, /* 5 */
   {NOSHIFT,0x07}, /* 6 */
   {NOSHIFT,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {NOSHIFT,0x09}, /* 8 */
   {NOSHIFT,0x0A}, /* 9 */
   {NOSHIFT,0x0B}, /* 0 */
   {NOSHIFT,0x0C}, /* - */
   {NOSHIFT,0x0D}, /* = */
   {NOSHIFT,0x2B}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {NOSHIFT,0x10}, /* q */
   {NOSHIFT,0x11}, /* w */
   {NOSHIFT,0x12}, /* e */
   {NOSHIFT,0x13}, /* r */
   {NOSHIFT,0x14}, /* t */
   {NOSHIFT,0x15}, /* y */
   {NOSHIFT,0x16}, /* u */
   {NOSHIFT,0x17}, /* i */
   
   /*; 18 - 1F*/
   {NOSHIFT,0x18}, /* o */
   {NOSHIFT,0x19}, /* p */
   {NOSHIFT,0x1A}, /* [ */
   {NOSHIFT,0x1B}, /* ] */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xCF}, /* Keypad 1 */
   {NOSHIFT,0xD0}, /* Keypad 2 */
   {NOSHIFT,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {NOSHIFT,0x1E}, /* a */
   {NOSHIFT,0x1F}, /* s */
   {NOSHIFT,0x20}, /* d */
   {NOSHIFT,0x21}, /* f */
   {NOSHIFT,0x22}, /* g */
   {NOSHIFT,0x23}, /* h */
   {NOSHIFT,0x24}, /* j */
   {NOSHIFT,0x25}, /* k */
   
   /*; 28 - 2F*/
   {NOSHIFT,0x26}, /* l */
   {NOSHIFT,0x27}, /* ; */
   {NOSHIFT,0x28}, /* ' */
   {NOSHIFT,0x2B}, /* No Key! */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xCB}, /* Keypad 4 */
   {NOSHIFT,0xCC}, /* Keypad 5 */
   {NOSHIFT,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {NOSHIFT,0x56}, /* No Key! */
   {NOSHIFT,0x2C}, /* z */
   {NOSHIFT,0x2D}, /* x */
   {NOSHIFT,0x2E}, /* c */
   {NOSHIFT,0x2F}, /* v */
   {NOSHIFT,0x30}, /* b */
   {NOSHIFT,0x31}, /* n */
   {NOSHIFT,0x32}, /* m */
   
   /*; 38 - 3F*/
   {NOSHIFT,0x33}, /* , */
   {NOSHIFT,0x34}, /* . */
   {NOSHIFT,0x35}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xD3}, /* Keypad . */
   {NOSHIFT,0xC7}, /* Keypad 7 */
   {NOSHIFT,0xC8}, /* Keypad 8 */
   {NOSHIFT,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {NOSHIFT,0x39}, /* SPACE BAR */
   {NOSHIFT,0x0E}, /* BKSP */
   {NOSHIFT,0x0F}, /* TAB */
   {NOSHIFT,0x1C}, /* Keypad Enter */
   {NOSHIFT,0x1C}, /* RETURN */
   {NOSHIFT,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {NOSHIFT,0xFF},
   
   /*; 48 - 4F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0x4A}, /* Keypad - */
   {NOSHIFT,0xFF},
   {NOSHIFT,0x48}, /* Up Arrow */
   {NOSHIFT,0x50}, /* Down Arrow */
   {NOSHIFT,0x4D}, /* Right Arrow */
   {NOSHIFT,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {NOSHIFT,0x3B}, /* F1 */
   {NOSHIFT,0x3C}, /* F2 */
   {NOSHIFT,0x3D}, /* F3 */
   {NOSHIFT,0x3E}, /* F4 */
   {NOSHIFT,0x3F}, /* F5 */
   {NOSHIFT,0x40}, /* F6 */
   {NOSHIFT,0x41}, /* F7 */
   {NOSHIFT,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {NOSHIFT,0x43}, /* F9 */
   {NOSHIFT,0x44}, /* F10 */
   {LSHIFT, 0x0a}, /* NumL */
   {LSHIFT, 0x0b}, /* ScrL */
   {NOSHIFT,0x35}, /* Pause */
   {NOSHIFT,0x37}, /* PrtSc */
   {NOSHIFT,0x4E}, /* Keypad + */
   {NOSHIFT,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {NOSHIFT,0x2A}, /* LSHIFT */
   {NOSHIFT,0x36}, /* RSHIFT */
   {NOSHIFT,0x3A}, /* CAPSLOCK */
   {NOSHIFT,0x1D}, /* CONTROL */
   {NOSHIFT,0x38}, /* LALT */
   {NOSHIFT,0x38}, /* RALT */
   {NOSHIFT,0xFF}, /* LAMIGA */
   {NOSHIFT,0xFF}, /* RAMIGA */
#if 0   

   /*; 68 - 6F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 70 - 77*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 78 - 7F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
#endif   

   };

/************/
/************/
/************/

static struct KeySeq A2PCShift[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {LSHIFT,0x29}, /* Tilde */
   {LSHIFT,0x02}, /* 1 */
   {LSHIFT,0x03}, /* 2 */
   {LSHIFT,0x04}, /* 3 */
   {LSHIFT,0x05}, /* 4 */
   {LSHIFT,0x06}, /* 5 */
   {LSHIFT,0x07}, /* 6 */
   {LSHIFT,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {LSHIFT,0x09}, /* 8 */
   {LSHIFT,0x0A}, /* 9 */
   {LSHIFT,0x0B}, /* 0 */
   {LSHIFT,0x0C}, /* - */
   {LSHIFT,0x0D}, /* = */
   {LSHIFT,0x2B}, /* / */
   {LSHIFT,0xFF},
   {LSHIFT,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {LSHIFT,0x10}, /* q */
   {LSHIFT,0x11}, /* w */
   {LSHIFT,0x12}, /* e */
   {LSHIFT,0x13}, /* r */
   {LSHIFT,0x14}, /* t */
   {LSHIFT,0x15}, /* y */
   {LSHIFT,0x16}, /* u */
   {LSHIFT,0x17}, /* i */
   
   /*; 18 - 1F*/
   {LSHIFT,0x18}, /* o */
   {LSHIFT,0x19}, /* p */
   {LSHIFT,0x1A}, /* [ */
   {LSHIFT,0x1B}, /* ] */
   {LSHIFT,0xFF},
   {LSHIFT,0xCF}, /* Keypad 1 */
   {LSHIFT,0xD0}, /* Keypad 2 */
   {LSHIFT,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {LSHIFT,0x1E}, /* a */
   {LSHIFT,0x1F}, /* s */
   {LSHIFT,0x20}, /* d */
   {LSHIFT,0x21}, /* f */
   {LSHIFT,0x22}, /* g */
   {LSHIFT,0x23}, /* h */
   {LSHIFT,0x24}, /* j */
   {LSHIFT,0x25}, /* k */
   
   /*; 28 - 2F*/
   {LSHIFT,0x26}, /* l */
   {LSHIFT,0x27}, /* ; */
   {LSHIFT,0x28}, /* ' */
   {LSHIFT,0x2B}, /* No Key! */
   {LSHIFT,0xFF},
   {LSHIFT,0xCB}, /* Keypad 4 */
   {LSHIFT,0xCC}, /* Keypad 5 */
   {LSHIFT,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {LSHIFT,0x56}, /* No Key! */
   {LSHIFT,0x2C}, /* z */
   {LSHIFT,0x2D}, /* x */
   {LSHIFT,0x2E}, /* c */
   {LSHIFT,0x2F}, /* v */
   {LSHIFT,0x30}, /* b */
   {LSHIFT,0x31}, /* n */
   {LSHIFT,0x32}, /* m */
   
   /*; 38 - 3F*/
   {LSHIFT,0x33}, /* , */
   {LSHIFT,0x34}, /* . */
   {LSHIFT,0x35}, /* / */
   {LSHIFT,0xFF},
   {LSHIFT,0xD3}, /* Keypad . */
   {LSHIFT,0xC7}, /* Keypad 7 */
   {LSHIFT,0xC8}, /* Keypad 8 */
   {LSHIFT,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {LSHIFT,0x39}, /* SPACE BAR */
   {LSHIFT,0x0E}, /* BKSP */
   {LSHIFT,0x0F}, /* TAB */
   {LSHIFT,0x1C}, /* Keypad Enter */
   {LSHIFT,0x1C}, /* RETURN */
   {LSHIFT,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {LSHIFT,0xFF},
   
   /*; 48 - 4F*/
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0x4A}, /* Keypad - */
   {LSHIFT,0xFF},
   {LSHIFT,0x48}, /* Up Arrow */
   {LSHIFT,0x50}, /* Down Arrow */
   {LSHIFT,0x4D}, /* Right Arrow */
   {LSHIFT,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {LSHIFT,0x3B}, /* F1 */
   {LSHIFT,0x3C}, /* F2 */
   {LSHIFT,0x3D}, /* F3 */
   {LSHIFT,0x3E}, /* F4 */
   {LSHIFT,0x3F}, /* F5 */
   {LSHIFT,0x40}, /* F6 */
   {LSHIFT,0x41}, /* F7 */
   {LSHIFT,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {LSHIFT,0x43}, /* F9 */
   {LSHIFT,0x44}, /* F10 */
   {LSHIFT,0x0a}, /* NumL */
   {LSHIFT,0x0b}, /* ScrL */
   {NOSHIFT,0x35}, /* Pause */
   {LSHIFT,0x37}, /* PrtSc */
   {LSHIFT,0x4E}, /* Keypad + */
   {LSHIFT,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {LSHIFT,0x2A}, /* LSHIFT */
   {LSHIFT,0x36}, /* RSHIFT */
   {LSHIFT,0x3A}, /* CAPSLOCK */
   {LSHIFT,0x1D}, /* CONTROL */
   {LSHIFT,0x38}, /* LALT */
   {LSHIFT,0x38}, /* RALT */
   {LSHIFT,0xFF}, /* LAMIGA */
   {LSHIFT,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   

   /*; 70 - 77*/
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   

   /*; 78 - 7F*/
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
   {LSHIFT,0xFF},
#endif   

   };

/************/
/************/
/************/

static struct KeySeq A2PCCapsLock[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {CAPSLOCK,0x29}, /* Tilde */
   {CAPSLOCK,0x02}, /* 1 */
   {CAPSLOCK,0x03}, /* 2 */
   {CAPSLOCK,0x04}, /* 3 */
   {CAPSLOCK,0x05}, /* 4 */
   {CAPSLOCK,0x06}, /* 5 */
   {CAPSLOCK,0x07}, /* 6 */
   {CAPSLOCK,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {CAPSLOCK,0x09}, /* 8 */
   {CAPSLOCK,0x0A}, /* 9 */
   {CAPSLOCK,0x0B}, /* 0 */
   {CAPSLOCK,0x0C}, /* - */
   {CAPSLOCK,0x0D}, /* = */
   {CAPSLOCK,0x2B}, /* / */
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {CAPSLOCK,0x10}, /* q */
   {CAPSLOCK,0x11}, /* w */
   {CAPSLOCK,0x12}, /* e */
   {CAPSLOCK,0x13}, /* r */
   {CAPSLOCK,0x14}, /* t */
   {CAPSLOCK,0x15}, /* y */
   {CAPSLOCK,0x16}, /* u */
   {CAPSLOCK,0x17}, /* i */
   
   /*; 18 - 1F*/
   {CAPSLOCK,0x18}, /* o */
   {CAPSLOCK,0x19}, /* p */
   {CAPSLOCK,0x1A}, /* [ */
   {CAPSLOCK,0x1B}, /* ] */
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xCF}, /* Keypad 1 */
   {CAPSLOCK,0xD0}, /* Keypad 2 */
   {CAPSLOCK,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {CAPSLOCK,0x1E}, /* a */
   {CAPSLOCK,0x1F}, /* s */
   {CAPSLOCK,0x20}, /* d */
   {CAPSLOCK,0x21}, /* f */
   {CAPSLOCK,0x22}, /* g */
   {CAPSLOCK,0x23}, /* h */
   {CAPSLOCK,0x24}, /* j */
   {CAPSLOCK,0x25}, /* k */
   
   /*; 28 - 2F*/
   {CAPSLOCK,0x26}, /* l */
   {CAPSLOCK,0x27}, /* ; */
   {CAPSLOCK,0x28}, /* ' */
   {CAPSLOCK,0x2B}, /* No Key! */
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xCB}, /* Keypad 4 */
   {CAPSLOCK,0xCC}, /* Keypad 5 */
   {CAPSLOCK,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {CAPSLOCK,0x56}, /* No Key! */
   {CAPSLOCK,0x2C}, /* z */
   {CAPSLOCK,0x2D}, /* x */
   {CAPSLOCK,0x2E}, /* c */
   {CAPSLOCK,0x2F}, /* v */
   {CAPSLOCK,0x30}, /* b */
   {CAPSLOCK,0x31}, /* n */
   {CAPSLOCK,0x32}, /* m */
   
   /*; 38 - 3F*/
   {CAPSLOCK,0x33}, /* , */
   {CAPSLOCK,0x34}, /* . */
   {CAPSLOCK,0x35}, /* / */
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xD3}, /* Keypad . */
   {CAPSLOCK,0xC7}, /* Keypad 7 */
   {CAPSLOCK,0xC8}, /* Keypad 8 */
   {CAPSLOCK,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {CAPSLOCK,0x39}, /* SPACE BAR */
   {CAPSLOCK,0x0E}, /* BKSP */
   {CAPSLOCK,0x0F}, /* TAB */
   {CAPSLOCK,0x1C}, /* Keypad Enter */
   {CAPSLOCK,0x1C}, /* RETURN */
   {CAPSLOCK,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {CAPSLOCK,0xFF},
   
   /*; 48 - 4F*/
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0x4A}, /* Keypad - */
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0x48}, /* Up Arrow */
   {CAPSLOCK,0x50}, /* Down Arrow */
   {CAPSLOCK,0x4D}, /* Right Arrow */
   {CAPSLOCK,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {CAPSLOCK,0x3B}, /* F1 */
   {CAPSLOCK,0x3C}, /* F2 */
   {CAPSLOCK,0x3D}, /* F3 */
   {CAPSLOCK,0x3E}, /* F4 */
   {CAPSLOCK,0x3F}, /* F5 */
   {CAPSLOCK,0x40}, /* F6 */
   {CAPSLOCK,0x41}, /* F7 */
   {CAPSLOCK,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {CAPSLOCK,0x43}, /* F9 */
   {CAPSLOCK,0x44}, /* F10 */
   {LSHIFT,  0x0a}, /* NumL */
   {LSHIFT,  0x0b}, /* ScrL */
   {NOSHIFT, 0x35}, /* Pause */
   {CAPSLOCK,0x37}, /* PrtSc */
   {CAPSLOCK,0x4E}, /* Keypad + */
   {CAPSLOCK,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {CAPSLOCK,0x2A}, /* LSHIFT */
   {CAPSLOCK,0x36}, /* RSHIFT */
   {CAPSLOCK,0x3A}, /* CAPSLOCK */
   {CAPSLOCK,0x1D}, /* CONTROL */
   {CAPSLOCK,0x38}, /* LALT */
   {CAPSLOCK,0x38}, /* RALT */
   {CAPSLOCK,0xFF}, /* LAMIGA */
   {CAPSLOCK,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   

   /*; 70 - 77*/
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   

   /*; 78 - 7F*/
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
   {CAPSLOCK,0xFF},
#endif   

   };

/************/
/************/
/************/

static struct KeySeq A2PCAltGr[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {CONTROL|ALT,0x29}, /* Tilde */
   {CONTROL|ALT,0x02}, /* 1 */
   {CONTROL|ALT,0x03}, /* 2 */
   {CONTROL|ALT,0x04}, /* 3 */
   {CONTROL|ALT,0x05}, /* 4 */
   {CONTROL|ALT,0x06}, /* 5 */
   {CONTROL|ALT,0x07}, /* 6 */
   {CONTROL|ALT,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {CONTROL|ALT,0x09}, /* 8 */
   {CONTROL|ALT,0x0A}, /* 9 */
   {CONTROL|ALT,0x0B}, /* 0 */
   {CONTROL|ALT,0x0C}, /* - */
   {CONTROL|ALT,0x0D}, /* = */
   {CONTROL|ALT,0x2B}, /* / */
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {CONTROL|ALT,0x10}, /* q */
   {CONTROL|ALT,0x11}, /* w */
   {CONTROL|ALT,0x12}, /* e */
   {CONTROL|ALT,0x13}, /* r */
   {CONTROL|ALT,0x14}, /* t */
   {CONTROL|ALT,0x15}, /* y */
   {CONTROL|ALT,0x16}, /* u */
   {CONTROL|ALT,0x17}, /* i */
   
   /*; 18 - 1F*/
   {CONTROL|ALT,0x18}, /* o */
   {CONTROL|ALT,0x19}, /* p */
   {CONTROL|ALT,0x1A}, /* [ */
   {CONTROL|ALT,0x1B}, /* ] */
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xCF}, /* Keypad 1 */
   {CONTROL|ALT,0xD0}, /* Keypad 2 */
   {CONTROL|ALT,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {CONTROL|ALT,0x1E}, /* a */
   {CONTROL|ALT,0x1F}, /* s */
   {CONTROL|ALT,0x20}, /* d */
   {CONTROL|ALT,0x21}, /* f */
   {CONTROL|ALT,0x22}, /* g */
   {CONTROL|ALT,0x23}, /* h */
   {CONTROL|ALT,0x24}, /* j */
   {CONTROL|ALT,0x25}, /* k */
   
   /*; 28 - 2F*/
   {CONTROL|ALT,0x26}, /* l */
   {CONTROL|ALT,0x27}, /* ; */
   {CONTROL|ALT,0x28}, /* ' */
   {CONTROL|ALT,0x2B}, /* No Key! */
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xCB}, /* Keypad 4 */
   {CONTROL|ALT,0xCC}, /* Keypad 5 */
   {CONTROL|ALT,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {CONTROL|ALT,0x56}, /* No Key! */
   {CONTROL|ALT,0x2C}, /* z */
   {CONTROL|ALT,0x2D}, /* x */
   {CONTROL|ALT,0x2E}, /* c */
   {CONTROL|ALT,0x2F}, /* v */
   {CONTROL|ALT,0x30}, /* b */
   {CONTROL|ALT,0x31}, /* n */
   {CONTROL|ALT,0x32}, /* m */
   
   /*; 38 - 3F*/
   {CONTROL|ALT,0x33}, /* , */
   {CONTROL|ALT,0x34}, /* . */
   {CONTROL|ALT,0x35}, /* / */
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xD3}, /* Keypad . */
   {CONTROL|ALT,0xC7}, /* Keypad 7 */
   {CONTROL|ALT,0xC8}, /* Keypad 8 */
   {CONTROL|ALT,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {CONTROL|ALT,0x39}, /* SPACE BAR */
   {CONTROL|ALT,0x0E}, /* BKSP */
   {CONTROL|ALT,0x0F}, /* TAB */
   {CONTROL|ALT,0x1C}, /* Keypad Enter */
   {CONTROL|ALT,0x1C}, /* RETURN */
   {CONTROL|ALT,0x01}, /* ESC */
   {CONTROL|ALT,0xD3}, /* DEL */
   {CONTROL|ALT,0xFF},
   
   /*; 48 - 4F*/
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0x4A}, /* Keypad - */
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0x48}, /* Up Arrow */
   {CONTROL|ALT,0x50}, /* Down Arrow */
   {CONTROL|ALT,0x4D}, /* Right Arrow */
   {CONTROL|ALT,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {CONTROL|ALT,0x3B}, /* F1 */
   {CONTROL|ALT,0x3C}, /* F2 */
   {CONTROL|ALT,0x3D}, /* F3 */
   {CONTROL|ALT,0x3E}, /* F4 */
   {CONTROL|ALT,0x3F}, /* F5 */
   {CONTROL|ALT,0x40}, /* F6 */
   {CONTROL|ALT,0x41}, /* F7 */
   {CONTROL|ALT,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {CONTROL|ALT,0x43}, /* F9 */
   {CONTROL|ALT,0x44}, /* F10 */
   {LSHIFT,     0x0a}, /* NumL */
   {LSHIFT,     0x0b}, /* ScrL */
   {NOSHIFT,    0x35}, /* Pause */
   {CONTROL|ALT,0x37}, /* PrtSc */
   {CONTROL|ALT,0x4E}, /* Keypad + */
   {CONTROL|ALT,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {CONTROL|ALT,0x2A}, /* LSHIFT */
   {CONTROL|ALT,0x36}, /* RSHIFT */
   {CONTROL|ALT,0x3A}, /* CAPSLOCK */
   {CONTROL|ALT,0x1D}, /* CONTROL */
   {CONTROL|ALT,0x38}, /* LALT */
   {CONTROL|ALT,0x38}, /* RALT */
   {CONTROL|ALT,0xFF}, /* LAMIGA */
   {CONTROL|ALT,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   

   /*; 70 - 77*/
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   

   /*; 78 - 7F*/
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
   {CONTROL|ALT,0xFF},
#endif   

   };

/************/
/************/
/************/

static struct KeySeq A2PCControl[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {CONTROL,0x29}, /* Tilde */
   {CONTROL,0x02}, /* 1 */
   {CONTROL,0x03}, /* 2 */
   {CONTROL,0x04}, /* 3 */
   {CONTROL,0x05}, /* 4 */
   {CONTROL,0x06}, /* 5 */
   {CONTROL,0x07}, /* 6 */
   {CONTROL,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {CONTROL,0x09}, /* 8 */
   {CONTROL,0x0A}, /* 9 */
   {CONTROL,0x0B}, /* 0 */
   {CONTROL,0x0C}, /* - */
   {CONTROL,0x0D}, /* = */
   {CONTROL,0x2B}, /* / */
   {CONTROL,0xFF},
   {CONTROL,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {CONTROL,0x10}, /* q */
   {CONTROL,0x11}, /* w */
   {CONTROL,0x12}, /* e */
   {CONTROL,0x13}, /* r */
   {CONTROL,0x14}, /* t */
   {CONTROL,0x15}, /* y */
   {CONTROL,0x16}, /* u */
   {CONTROL,0x17}, /* i */
   
   /*; 18 - 1F*/
   {CONTROL,0x18}, /* o */
   {CONTROL,0x19}, /* p */
   {CONTROL,0x1A}, /* [ */
   {CONTROL,0x1B}, /* ] */
   {CONTROL,0xFF},
   {CONTROL,0xCF}, /* Keypad 1 */
   {CONTROL,0xD0}, /* Keypad 2 */
   {CONTROL,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {CONTROL,0x1E}, /* a */
   {CONTROL,0x1F}, /* s */
   {CONTROL,0x20}, /* d */
   {CONTROL,0x21}, /* f */
   {CONTROL,0x22}, /* g */
   {CONTROL,0x23}, /* h */
   {CONTROL,0x24}, /* j */
   {CONTROL,0x25}, /* k */
   
   /*; 28 - 2F*/
   {CONTROL,0x26}, /* l */
   {CONTROL,0x27}, /* ; */
   {CONTROL,0x28}, /* ' */
   {CONTROL,0x2B}, /* No Key! */
   {CONTROL,0xFF},
   {CONTROL,0xCB}, /* Keypad 4 */
   {CONTROL,0xCC}, /* Keypad 5 */
   {CONTROL,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {CONTROL,0x56}, /* No Key! */
   {CONTROL,0x2C}, /* z */
   {CONTROL,0x2D}, /* x */
   {CONTROL,0x2E}, /* c */
   {CONTROL,0x2F}, /* v */
   {CONTROL,0x30}, /* b */
   {CONTROL,0x31}, /* n */
   {CONTROL,0x32}, /* m */
   
   /*; 38 - 3F*/
   {CONTROL,0x33}, /* , */
   {CONTROL,0x34}, /* . */
   {CONTROL,0x35}, /* / */
   {CONTROL,0xFF},
   {CONTROL,0xD3}, /* Keypad . */
   {CONTROL,0xC7}, /* Keypad 7 */
   {CONTROL,0xC8}, /* Keypad 8 */
   {CONTROL,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {CONTROL,0x39}, /* SPACE BAR */
   {CONTROL,0x0E}, /* BKSP */
   {CONTROL,0x0F}, /* TAB */
   {CONTROL,0x1C}, /* Keypad Enter */
   {CONTROL,0x1C}, /* RETURN */
   {CONTROL,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {CONTROL,0xFF},
   
   /*; 48 - 4F*/
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0x4A}, /* Keypad - */
   {CONTROL,0xFF},
   {CONTROL,0x48}, /* Up Arrow */
   {CONTROL,0x50}, /* Down Arrow */
   {CONTROL,0x4D}, /* Right Arrow */
   {CONTROL,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {CONTROL,0x3B}, /* F1 */
   {CONTROL,0x3C}, /* F2 */
   {CONTROL,0x3D}, /* F3 */
   {CONTROL,0x3E}, /* F4 */
   {CONTROL,0x3F}, /* F5 */
   {CONTROL,0x40}, /* F6 */
   {CONTROL,0x41}, /* F7 */
   {CONTROL,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {CONTROL,0x43}, /* F9 */
   {CONTROL,0x44}, /* F10 */
   {LSHIFT ,0x0a}, /* NumL */
   {LSHIFT, 0x0b}, /* ScrL */
   {NOSHIFT,0x35}, /* Pause */
   {CONTROL,0x37}, /* PrtSc */
   {CONTROL,0x4E}, /* Keypad + */
   {CONTROL,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {CONTROL,0x2A}, /* LSHIFT */
   {CONTROL,0x36}, /* RSHIFT */
   {CONTROL,0x3A}, /* CAPSLOCK */
   {CONTROL,0x1D}, /* CONTROL */
   {CONTROL,0x38}, /* LALT */
   {CONTROL,0x38}, /* RALT */
   {CONTROL,0xFF}, /* LAMIGA */
   {CONTROL,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   

   /*; 70 - 77*/
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   

   /*; 78 - 7F*/
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
   {CONTROL,0xFF},
#endif   

   };
/************/
/************/
/************/
static struct KeySeq A2PCNumLock[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {NUMLOCK,0x29}, /* Tilde */
   {NUMLOCK,0x02}, /* 1 */
   {NUMLOCK,0x03}, /* 2 */
   {NUMLOCK,0x04}, /* 3 */
   {NUMLOCK,0x05}, /* 4 */
   {NUMLOCK,0x06}, /* 5 */
   {NUMLOCK,0x07}, /* 6 */
   {NUMLOCK,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {NUMLOCK,0x09}, /* 8 */
   {NUMLOCK,0x0A}, /* 9 */
   {NUMLOCK,0x0B}, /* 0 */
   {NUMLOCK,0x0C}, /* - */
   {NUMLOCK,0x0D}, /* = */
   {NUMLOCK,0x2B}, /* / */
   {NUMLOCK,0xFF},
   {NUMLOCK,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {NUMLOCK,0x10}, /* q */
   {NUMLOCK,0x11}, /* w */
   {NUMLOCK,0x12}, /* e */
   {NUMLOCK,0x13}, /* r */
   {NUMLOCK,0x14}, /* t */
   {NUMLOCK,0x15}, /* y */
   {NUMLOCK,0x16}, /* u */
   {NUMLOCK,0x17}, /* i */
   
   /*; 18 - 1F*/
   {NUMLOCK,0x18}, /* o */
   {NUMLOCK,0x19}, /* p */
   {NUMLOCK,0x1A}, /* [ */
   {NUMLOCK,0x1B}, /* ] */
   {NUMLOCK,0xFF},
   {NUMLOCK,0xCF}, /* Keypad 1 */
   {NUMLOCK,0xD0}, /* Keypad 2 */
   {NUMLOCK,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {NUMLOCK,0x1E}, /* a */
   {NUMLOCK,0x1F}, /* s */
   {NUMLOCK,0x20}, /* d */
   {NUMLOCK,0x21}, /* f */
   {NUMLOCK,0x22}, /* g */
   {NUMLOCK,0x23}, /* h */
   {NUMLOCK,0x24}, /* j */
   {NUMLOCK,0x25}, /* k */
   
   /*; 28 - 2F*/
   {NUMLOCK,0x26}, /* l */
   {NUMLOCK,0x27}, /* ; */
   {NUMLOCK,0x28}, /* ' */
   {NUMLOCK,0x2B}, /* No Key! */
   {NUMLOCK,0xFF},
   {NUMLOCK,0xCB}, /* Keypad 4 */
   {NUMLOCK,0xCC}, /* Keypad 5 */
   {NUMLOCK,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {NUMLOCK,0x56}, /* No Key! */
   {NUMLOCK,0x2C}, /* z */
   {NUMLOCK,0x2D}, /* x */
   {NUMLOCK,0x2E}, /* c */
   {NUMLOCK,0x2F}, /* v */
   {NUMLOCK,0x30}, /* b */
   {NUMLOCK,0x31}, /* n */
   {NUMLOCK,0x32}, /* m */
   
   /*; 38 - 3F*/
   {NUMLOCK,0x33}, /* , */
   {NUMLOCK,0x34}, /* . */
   {NUMLOCK,0x35}, /* / */
   {NUMLOCK,0xFF},
   {NUMLOCK,0xD3}, /* Keypad . */
   {NUMLOCK,0xC7}, /* Keypad 7 */
   {NUMLOCK,0xC8}, /* Keypad 8 */
   {NUMLOCK,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {NUMLOCK,0x39}, /* SPACE BAR */
   {NUMLOCK,0x0E}, /* BKSP */
   {NUMLOCK,0x0F}, /* TAB */
   {NUMLOCK,0x1C}, /* Keypad Enter */
   {NUMLOCK,0x1C}, /* RETURN */
   {NUMLOCK,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {NUMLOCK,0xFF},
   
   /*; 48 - 4F*/
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0x4A}, /* Keypad - */
   {NUMLOCK,0xFF},
   {NUMLOCK,0x48}, /* Up Arrow */
   {NUMLOCK,0x50}, /* Down Arrow */
   {NUMLOCK,0x4D}, /* Right Arrow */
   {NUMLOCK,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {NUMLOCK,0x3B}, /* F1 */
   {NUMLOCK,0x3C}, /* F2 */
   {NUMLOCK,0x3D}, /* F3 */
   {NUMLOCK,0x3E}, /* F4 */
   {NUMLOCK,0x3F}, /* F5 */
   {NUMLOCK,0x40}, /* F6 */
   {NUMLOCK,0x41}, /* F7 */
   {NUMLOCK,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {NUMLOCK,0x43}, /* F9 */
   {NUMLOCK,0x44}, /* F10 */
   {LSHIFT, 0x0a}, /* NumL */
   {LSHIFT, 0x0b}, /* ScrL */
   {NOSHIFT,0x35}, /* Pause */
   {NUMLOCK,0x37}, /* PrtSc */
   {NUMLOCK,0x4E}, /* Keypad + */
   {NUMLOCK,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {NUMLOCK,0x2A}, /* LSHIFT */
   {NUMLOCK,0x36}, /* RSHIFT */
   {NUMLOCK,0x3A}, /* CAPSLOCK */
   {NUMLOCK,0x1D}, /* CONTROL */
   {NUMLOCK,0x38}, /* LALT */
   {NUMLOCK,0x38}, /* RALT */
   {NUMLOCK,0xFF}, /* LAMIGA */
   {NUMLOCK,0xFF}, /* RAMIGA */
#if 0   

   /*; 68 - 6F*/
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   

   /*; 70 - 77*/
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   

   /*; 78 - 7F*/
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
   {NUMLOCK,0xFF},
#endif   

   };
/************/
/************/
/************/
static struct KeySeq A2PCShiftCapsLock[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {LSHIFT|CAPSLOCK,0x29}, /* Tilde */
   {LSHIFT|CAPSLOCK,0x02}, /* 1 */
   {LSHIFT|CAPSLOCK,0x03}, /* 2 */
   {LSHIFT|CAPSLOCK,0x04}, /* 3 */
   {LSHIFT|CAPSLOCK,0x05}, /* 4 */
   {LSHIFT|CAPSLOCK,0x06}, /* 5 */
   {LSHIFT|CAPSLOCK,0x07}, /* 6 */
   {LSHIFT|CAPSLOCK,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {LSHIFT|CAPSLOCK,0x09}, /* 8 */
   {LSHIFT|CAPSLOCK,0x0A}, /* 9 */
   {LSHIFT|CAPSLOCK,0x0B}, /* 0 */
   {LSHIFT|CAPSLOCK,0x0C}, /* - */
   {LSHIFT|CAPSLOCK,0x0D}, /* = */
   {LSHIFT|CAPSLOCK,0x2B}, /* / */
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {LSHIFT|CAPSLOCK,0x10}, /* q */
   {LSHIFT|CAPSLOCK,0x11}, /* w */
   {LSHIFT|CAPSLOCK,0x12}, /* e */
   {LSHIFT|CAPSLOCK,0x13}, /* r */
   {LSHIFT|CAPSLOCK,0x14}, /* t */
   {LSHIFT|CAPSLOCK,0x15}, /* y */
   {LSHIFT|CAPSLOCK,0x16}, /* u */
   {LSHIFT|CAPSLOCK,0x17}, /* i */
   
   /*; 18 - 1F*/
   {LSHIFT|CAPSLOCK,0x18}, /* o */
   {LSHIFT|CAPSLOCK,0x19}, /* p */
   {LSHIFT|CAPSLOCK,0x1A}, /* [ */
   {LSHIFT|CAPSLOCK,0x1B}, /* ] */
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xCF}, /* Keypad 1 */
   {LSHIFT|CAPSLOCK,0xD0}, /* Keypad 2 */
   {LSHIFT|CAPSLOCK,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {LSHIFT|CAPSLOCK,0x1E}, /* a */
   {LSHIFT|CAPSLOCK,0x1F}, /* s */
   {LSHIFT|CAPSLOCK,0x20}, /* d */
   {LSHIFT|CAPSLOCK,0x21}, /* f */
   {LSHIFT|CAPSLOCK,0x22}, /* g */
   {LSHIFT|CAPSLOCK,0x23}, /* h */
   {LSHIFT|CAPSLOCK,0x24}, /* j */
   {LSHIFT|CAPSLOCK,0x25}, /* k */
   
   /*; 28 - 2F*/
   {LSHIFT|CAPSLOCK,0x26}, /* l */
   {LSHIFT|CAPSLOCK,0x27}, /* ; */
   {LSHIFT|CAPSLOCK,0x28}, /* ' */
   {LSHIFT|CAPSLOCK,0x2B}, /* No Key! */
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xCB}, /* Keypad 4 */
   {LSHIFT|CAPSLOCK,0xCC}, /* Keypad 5 */
   {LSHIFT|CAPSLOCK,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {LSHIFT|CAPSLOCK,0x56}, /* No Key! */
   {LSHIFT|CAPSLOCK,0x2C}, /* z */
   {LSHIFT|CAPSLOCK,0x2D}, /* x */
   {LSHIFT|CAPSLOCK,0x2E}, /* c */
   {LSHIFT|CAPSLOCK,0x2F}, /* v */
   {LSHIFT|CAPSLOCK,0x30}, /* b */
   {LSHIFT|CAPSLOCK,0x31}, /* n */
   {LSHIFT|CAPSLOCK,0x32}, /* m */
   
   /*; 38 - 3F*/
   {LSHIFT|CAPSLOCK,0x33}, /* , */
   {LSHIFT|CAPSLOCK,0x34}, /* . */
   {LSHIFT|CAPSLOCK,0x35}, /* / */
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xD3}, /* Keypad . */
   {LSHIFT|CAPSLOCK,0xC7}, /* Keypad 7 */
   {LSHIFT|CAPSLOCK,0xC8}, /* Keypad 8 */
   {LSHIFT|CAPSLOCK,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {LSHIFT|CAPSLOCK,0x39}, /* SPACE BAR */
   {LSHIFT|CAPSLOCK,0x0E}, /* BKSP */
   {LSHIFT|CAPSLOCK,0x0F}, /* TAB */
   {LSHIFT|CAPSLOCK,0x1C}, /* Keypad Enter */
   {LSHIFT|CAPSLOCK,0x1C}, /* RETURN */
   {LSHIFT|CAPSLOCK,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {LSHIFT|CAPSLOCK,0xFF},
   
   /*; 48 - 4F*/
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0x4A}, /* Keypad - */
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT,0x48}, /* Up Arrow */
   {LSHIFT,0x50}, /* Down Arrow */
   {LSHIFT,0x4D}, /* Right Arrow */
   {LSHIFT,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {LSHIFT|CAPSLOCK,0x3B}, /* F1 */
   {LSHIFT|CAPSLOCK,0x3C}, /* F2 */
   {LSHIFT|CAPSLOCK,0x3D}, /* F3 */
   {LSHIFT|CAPSLOCK,0x3E}, /* F4 */
   {LSHIFT|CAPSLOCK,0x3F}, /* F5 */
   {LSHIFT|CAPSLOCK,0x40}, /* F6 */
   {LSHIFT|CAPSLOCK,0x41}, /* F7 */
   {LSHIFT|CAPSLOCK,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {LSHIFT|CAPSLOCK,0x43}, /* F9 */
   {LSHIFT|CAPSLOCK,0x44}, /* F10 */
   {LSHIFT,         0x0a}, /* NumL */
   {LSHIFT,         0x0b}, /* ScrL */
   {NOSHIFT,        0x35}, /* Pause */
   {LSHIFT|CAPSLOCK,0x37}, /* PrtSc */
   {LSHIFT|CAPSLOCK,0x4E}, /* Keypad + */
   {LSHIFT|CAPSLOCK,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {LSHIFT|CAPSLOCK,0x2A}, /* LSHIFT */
   {LSHIFT|CAPSLOCK,0x36}, /* RSHIFT */
   {LSHIFT|CAPSLOCK,0x3A}, /* CAPSLOCK */
   {LSHIFT|CAPSLOCK,0x1D}, /* CONTROL */
   {LSHIFT|CAPSLOCK,0x38}, /* LALT */
   {LSHIFT|CAPSLOCK,0x38}, /* RALT */
   {LSHIFT|CAPSLOCK,0xFF}, /* LAMIGA */
   {LSHIFT|CAPSLOCK,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   

   /*; 70 - 77*/
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   

   /*; 78 - 7F*/
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
   {LSHIFT|CAPSLOCK,0xFF},
#endif   

   };
/************/
/************/
/************/
static struct KeySeq A2PCShiftNumLock[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {LSHIFT|NUMLOCK,0x29}, /* Tilde */
   {LSHIFT|NUMLOCK,0x02}, /* 1 */
   {LSHIFT|NUMLOCK,0x03}, /* 2 */
   {LSHIFT|NUMLOCK,0x04}, /* 3 */
   {LSHIFT|NUMLOCK,0x05}, /* 4 */
   {LSHIFT|NUMLOCK,0x06}, /* 5 */
   {LSHIFT|NUMLOCK,0x07}, /* 6 */
   {LSHIFT|NUMLOCK,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {LSHIFT|NUMLOCK,0x09}, /* 8 */
   {LSHIFT|NUMLOCK,0x0A}, /* 9 */
   {LSHIFT|NUMLOCK,0x0B}, /* 0 */
   {LSHIFT|NUMLOCK,0x0C}, /* - */
   {LSHIFT|NUMLOCK,0x0D}, /* = */
   {LSHIFT|NUMLOCK,0x2B}, /* / */
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {LSHIFT|NUMLOCK,0x10}, /* q */
   {LSHIFT|NUMLOCK,0x11}, /* w */
   {LSHIFT|NUMLOCK,0x12}, /* e */
   {LSHIFT|NUMLOCK,0x13}, /* r */
   {LSHIFT|NUMLOCK,0x14}, /* t */
   {LSHIFT|NUMLOCK,0x15}, /* y */
   {LSHIFT|NUMLOCK,0x16}, /* u */
   {LSHIFT|NUMLOCK,0x17}, /* i */
   
   /*; 18 - 1F*/
   {LSHIFT|NUMLOCK,0x18}, /* o */
   {LSHIFT|NUMLOCK,0x19}, /* p */
   {LSHIFT|NUMLOCK,0x1A}, /* [ */
   {LSHIFT|NUMLOCK,0x1B}, /* ] */
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xCF}, /* Keypad 1 */
   {LSHIFT|NUMLOCK,0xD0}, /* Keypad 2 */
   {LSHIFT|NUMLOCK,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {LSHIFT|NUMLOCK,0x1E}, /* a */
   {LSHIFT|NUMLOCK,0x1F}, /* s */
   {LSHIFT|NUMLOCK,0x20}, /* d */
   {LSHIFT|NUMLOCK,0x21}, /* f */
   {LSHIFT|NUMLOCK,0x22}, /* g */
   {LSHIFT|NUMLOCK,0x23}, /* h */
   {LSHIFT|NUMLOCK,0x24}, /* j */
   {LSHIFT|NUMLOCK,0x25}, /* k */
   
   /*; 28 - 2F*/
   {LSHIFT|NUMLOCK,0x26}, /* l */
   {LSHIFT|NUMLOCK,0x27}, /* ; */
   {LSHIFT|NUMLOCK,0x28}, /* ' */
   {LSHIFT|NUMLOCK,0x2B}, /* No Key! */
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xCB}, /* Keypad 4 */
   {LSHIFT|NUMLOCK,0xCC}, /* Keypad 5 */
   {LSHIFT|NUMLOCK,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {LSHIFT|NUMLOCK,0x56}, /* No Key! */
   {LSHIFT|NUMLOCK,0x2C}, /* z */
   {LSHIFT|NUMLOCK,0x2D}, /* x */
   {LSHIFT|NUMLOCK,0x2E}, /* c */
   {LSHIFT|NUMLOCK,0x2F}, /* v */
   {LSHIFT|NUMLOCK,0x30}, /* b */
   {LSHIFT|NUMLOCK,0x31}, /* n */
   {LSHIFT|NUMLOCK,0x32}, /* m */
   
   /*; 38 - 3F*/
   {LSHIFT|NUMLOCK,0x33}, /* , */
   {LSHIFT|NUMLOCK,0x34}, /* . */
   {LSHIFT|NUMLOCK,0x35}, /* / */
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xD3}, /* Keypad . */
   {LSHIFT|NUMLOCK,0xC7}, /* Keypad 7 */
   {LSHIFT|NUMLOCK,0xC8}, /* Keypad 8 */
   {LSHIFT|NUMLOCK,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {LSHIFT|NUMLOCK,0x39}, /* SPACE BAR */
   {LSHIFT|NUMLOCK,0x0E}, /* BKSP */
   {LSHIFT|NUMLOCK,0x0F}, /* TAB */
   {LSHIFT|NUMLOCK,0x1C}, /* Keypad Enter */
   {LSHIFT|NUMLOCK,0x1C}, /* RETURN */
   {LSHIFT|NUMLOCK,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {LSHIFT|NUMLOCK,0xFF},
   
   /*; 48 - 4F*/
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0x4A}, /* Keypad - */
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT,0x48}, /* Up Arrow */
   {LSHIFT,0x50}, /* Down Arrow */
   {LSHIFT,0x4D}, /* Right Arrow */
   {LSHIFT,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {LSHIFT|NUMLOCK,0x3B}, /* F1 */
   {LSHIFT|NUMLOCK,0x3C}, /* F2 */
   {LSHIFT|NUMLOCK,0x3D}, /* F3 */
   {LSHIFT|NUMLOCK,0x3E}, /* F4 */
   {LSHIFT|NUMLOCK,0x3F}, /* F5 */
   {LSHIFT|NUMLOCK,0x40}, /* F6 */
   {LSHIFT|NUMLOCK,0x41}, /* F7 */
   {LSHIFT|NUMLOCK,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {LSHIFT|NUMLOCK,0x43}, /* F9 */
   {LSHIFT|NUMLOCK,0x44}, /* F10 */
   {LSHIFT,        0x0a}, /* NumL */
   {LSHIFT,        0x0b}, /* ScrL */
   {NOSHIFT,       0x35}, /* Pause */
   {LSHIFT|NUMLOCK,0x37}, /* PrtSc */
   {LSHIFT|NUMLOCK,0x4E}, /* Keypad + */
   {LSHIFT|NUMLOCK,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {LSHIFT|NUMLOCK,0x2A}, /* LSHIFT */
   {LSHIFT|NUMLOCK,0x36}, /* RSHIFT */
   {LSHIFT|NUMLOCK,0x3A}, /* CAPSLOCK */
   {LSHIFT|NUMLOCK,0x1D}, /* CONTROL */
   {LSHIFT|NUMLOCK,0x38}, /* LALT */
   {LSHIFT|NUMLOCK,0x38}, /* RALT */
   {LSHIFT|NUMLOCK,0xFF}, /* LAMIGA */
   {LSHIFT|NUMLOCK,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   

   /*; 70 - 77*/
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   

   /*; 78 - 7F*/
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
   {LSHIFT|NUMLOCK,0xFF},
#endif   

   };
/************/
/************/
/************/
static struct KeySeq A2PCCapsLockNumLock[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {CAPSLOCK|NUMLOCK,0x29}, /* Tilde */
   {CAPSLOCK|NUMLOCK,0x02}, /* 1 */
   {CAPSLOCK|NUMLOCK,0x03}, /* 2 */
   {CAPSLOCK|NUMLOCK,0x04}, /* 3 */
   {CAPSLOCK|NUMLOCK,0x05}, /* 4 */
   {CAPSLOCK|NUMLOCK,0x06}, /* 5 */
   {CAPSLOCK|NUMLOCK,0x07}, /* 6 */
   {CAPSLOCK|NUMLOCK,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {CAPSLOCK|NUMLOCK,0x09}, /* 8 */
   {CAPSLOCK|NUMLOCK,0x0A}, /* 9 */
   {CAPSLOCK|NUMLOCK,0x0B}, /* 0 */
   {CAPSLOCK|NUMLOCK,0x0C}, /* - */
   {CAPSLOCK|NUMLOCK,0x0D}, /* = */
   {CAPSLOCK|NUMLOCK,0x2B}, /* / */
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {CAPSLOCK|NUMLOCK,0x10}, /* q */
   {CAPSLOCK|NUMLOCK,0x11}, /* w */
   {CAPSLOCK|NUMLOCK,0x12}, /* e */
   {CAPSLOCK|NUMLOCK,0x13}, /* r */
   {CAPSLOCK|NUMLOCK,0x14}, /* t */
   {CAPSLOCK|NUMLOCK,0x15}, /* y */
   {CAPSLOCK|NUMLOCK,0x16}, /* u */
   {CAPSLOCK|NUMLOCK,0x17}, /* i */
   
   /*; 18 - 1F*/
   {CAPSLOCK|NUMLOCK,0x18}, /* o */
   {CAPSLOCK|NUMLOCK,0x19}, /* p */
   {CAPSLOCK|NUMLOCK,0x1A}, /* [ */
   {CAPSLOCK|NUMLOCK,0x1B}, /* ] */
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xCF}, /* Keypad 1 */
   {CAPSLOCK|NUMLOCK,0xD0}, /* Keypad 2 */
   {CAPSLOCK|NUMLOCK,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {CAPSLOCK|NUMLOCK,0x1E}, /* a */
   {CAPSLOCK|NUMLOCK,0x1F}, /* s */
   {CAPSLOCK|NUMLOCK,0x20}, /* d */
   {CAPSLOCK|NUMLOCK,0x21}, /* f */
   {CAPSLOCK|NUMLOCK,0x22}, /* g */
   {CAPSLOCK|NUMLOCK,0x23}, /* h */
   {CAPSLOCK|NUMLOCK,0x24}, /* j */
   {CAPSLOCK|NUMLOCK,0x25}, /* k */
   
   /*; 28 - 2F*/
   {CAPSLOCK|NUMLOCK,0x26}, /* l */
   {CAPSLOCK|NUMLOCK,0x27}, /* ; */
   {CAPSLOCK|NUMLOCK,0x28}, /* ' */
   {CAPSLOCK|NUMLOCK,0x2B}, /* No Key! */
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xCB}, /* Keypad 4 */
   {CAPSLOCK|NUMLOCK,0xCC}, /* Keypad 5 */
   {CAPSLOCK|NUMLOCK,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {CAPSLOCK|NUMLOCK,0x56}, /* No Key! */
   {CAPSLOCK|NUMLOCK,0x2C}, /* z */
   {CAPSLOCK|NUMLOCK,0x2D}, /* x */
   {CAPSLOCK|NUMLOCK,0x2E}, /* c */
   {CAPSLOCK|NUMLOCK,0x2F}, /* v */
   {CAPSLOCK|NUMLOCK,0x30}, /* b */
   {CAPSLOCK|NUMLOCK,0x31}, /* n */
   {CAPSLOCK|NUMLOCK,0x32}, /* m */
   
   /*; 38 - 3F*/
   {CAPSLOCK|NUMLOCK,0x33}, /* , */
   {CAPSLOCK|NUMLOCK,0x34}, /* . */
   {CAPSLOCK|NUMLOCK,0x35}, /* / */
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xD3}, /* Keypad . */
   {CAPSLOCK|NUMLOCK,0xC7}, /* Keypad 7 */
   {CAPSLOCK|NUMLOCK,0xC8}, /* Keypad 8 */
   {CAPSLOCK|NUMLOCK,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {CAPSLOCK|NUMLOCK,0x39}, /* SPACE BAR */
   {CAPSLOCK|NUMLOCK,0x0E}, /* BKSP */
   {CAPSLOCK|NUMLOCK,0x0F}, /* TAB */
   {CAPSLOCK|NUMLOCK,0x1C}, /* Keypad Enter */
   {CAPSLOCK|NUMLOCK,0x1C}, /* RETURN */
   {CAPSLOCK|NUMLOCK,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {CAPSLOCK|NUMLOCK,0xFF},
   
   /*; 48 - 4F*/
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0x4A}, /* Keypad - */
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0x48}, /* Up Arrow */
   {CAPSLOCK|NUMLOCK,0x50}, /* Down Arrow */
   {CAPSLOCK|NUMLOCK,0x4D}, /* Right Arrow */
   {CAPSLOCK|NUMLOCK,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {CAPSLOCK|NUMLOCK,0x3B}, /* F1 */
   {CAPSLOCK|NUMLOCK,0x3C}, /* F2 */
   {CAPSLOCK|NUMLOCK,0x3D}, /* F3 */
   {CAPSLOCK|NUMLOCK,0x3E}, /* F4 */
   {CAPSLOCK|NUMLOCK,0x3F}, /* F5 */
   {CAPSLOCK|NUMLOCK,0x40}, /* F6 */
   {CAPSLOCK|NUMLOCK,0x41}, /* F7 */
   {CAPSLOCK|NUMLOCK,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {CAPSLOCK|NUMLOCK,0x43}, /* F9 */
   {CAPSLOCK|NUMLOCK,0x44}, /* F10 */
   {LSHIFT,          0x0a}, /* NumL */
   {LSHIFT,          0x0b}, /* ScrL */
   {NOSHIFT,         0x35}, /* Pause */
   {CAPSLOCK|NUMLOCK,0x37}, /* PrtSc */
   {CAPSLOCK|NUMLOCK,0x4E}, /* Keypad + */
   {CAPSLOCK|NUMLOCK,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {CAPSLOCK|NUMLOCK,0x2A}, /* LSHIFT */
   {CAPSLOCK|NUMLOCK,0x36}, /* RSHIFT */
   {CAPSLOCK|NUMLOCK,0x3A}, /* CAPSLOCK */
   {CAPSLOCK|NUMLOCK,0x1D}, /* CONTROL */
   {CAPSLOCK|NUMLOCK,0x38}, /* LALT */
   {CAPSLOCK|NUMLOCK,0x38}, /* RALT */
   {CAPSLOCK|NUMLOCK,0xFF}, /* LAMIGA */
   {CAPSLOCK|NUMLOCK,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   

   /*; 70 - 77*/
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   

   /*; 78 - 7F*/
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
   {CAPSLOCK|NUMLOCK,0xFF},
#endif   

   };
/************/
/************/
/************/
static struct KeySeq A2PCShiftCapsLockNumLock[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x29}, /* Tilde */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x02}, /* 1 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x03}, /* 2 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x04}, /* 3 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x05}, /* 4 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x06}, /* 5 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x07}, /* 6 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x08}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x09}, /* 8 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x0A}, /* 9 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x0B}, /* 0 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x0C}, /* - */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x0D}, /* = */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x2B}, /* / */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xD2}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x10}, /* q */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x11}, /* w */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x12}, /* e */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x13}, /* r */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x14}, /* t */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x15}, /* y */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x16}, /* u */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x17}, /* i */
   
   /*; 18 - 1F*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x18}, /* o */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x19}, /* p */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x1A}, /* [ */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x1B}, /* ] */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xCF}, /* Keypad 1 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xD0}, /* Keypad 2 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xD1}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x1E}, /* a */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x1F}, /* s */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x20}, /* d */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x21}, /* f */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x22}, /* g */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x23}, /* h */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x24}, /* j */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x25}, /* k */
   
   /*; 28 - 2F*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x26}, /* l */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x27}, /* ; */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x28}, /* ' */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x2B}, /* No Key! */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xCB}, /* Keypad 4 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xCC}, /* Keypad 5 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xCD}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x56}, /* No Key! */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x2C}, /* z */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x2D}, /* x */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x2E}, /* c */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x2F}, /* v */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x30}, /* b */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x31}, /* n */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x32}, /* m */
   
   /*; 38 - 3F*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x33}, /* , */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x34}, /* . */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x35}, /* / */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xD3}, /* Keypad . */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xC7}, /* Keypad 7 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xC8}, /* Keypad 8 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xC9}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x39}, /* SPACE BAR */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x0E}, /* BKSP */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x0F}, /* TAB */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x1C}, /* Keypad Enter */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x1C}, /* RETURN */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x01}, /* ESC */
   {NOSHIFT,0x53}, /* DEL */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   
   /*; 48 - 4F*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0x4A}, /* Keypad - */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT,0x48}, /* Up Arrow */
   {LSHIFT,0x50}, /* Down Arrow */
   {LSHIFT,0x4D}, /* Right Arrow */
   {LSHIFT,0x4B}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x3B}, /* F1 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x3C}, /* F2 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x3D}, /* F3 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x3E}, /* F4 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x3F}, /* F5 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x40}, /* F6 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x41}, /* F7 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x42}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x43}, /* F9 */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x44}, /* F10 */
   {LSHIFT,                 0x0a}, /* NumL */
   {LSHIFT,                 0x0b}, /* ScrL */
   {NOSHIFT,                0x35}, /* Pause */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x37}, /* PrtSc */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x4E}, /* Keypad + */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x46}, /* HELP */
   
   /*; 60 - 67*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0x2A}, /* LSHIFT */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x36}, /* RSHIFT */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x3A}, /* CAPSLOCK */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x1D}, /* CONTROL */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x38}, /* LALT */
   {LSHIFT|CAPSLOCK|NUMLOCK,0x38}, /* RALT */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF}, /* LAMIGA */
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   

   /*; 70 - 77*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   

   /*; 78 - 7F*/
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
   {LSHIFT|CAPSLOCK|NUMLOCK,0xFF},
#endif   

   };
/************/
/************/
/************/
static struct KeySeq A2PCLeftAmiga[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {NOSHIFT,0xFF}, /* Tilde */
   {NOSHIFT,0xFF}, /* 1 */
   {NOSHIFT,0xFF}, /* 2 */
   {NOSHIFT,0xFF}, /* 3 */
   {NOSHIFT,0xFF}, /* 4 */
   {NOSHIFT,0xFF}, /* 5 */
   {NOSHIFT,0xFF}, /* 6 */
   {NOSHIFT,0xFF}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {NOSHIFT,0xFF}, /* 8 */
   {NOSHIFT,0xFF}, /* 9 */
   {NOSHIFT,0xFF}, /* 0 */
   {NOSHIFT,0xFF}, /* - */
   {NOSHIFT,0xFF}, /* = */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {NOSHIFT,0xFF}, /* q */
   {NOSHIFT,0xFF}, /* w */
   {NOSHIFT,0xFF}, /* e */
   {NOSHIFT,0xFF}, /* r */
   {NOSHIFT,0xFF}, /* t */
   {NOSHIFT,0xFF}, /* y */
   {NOSHIFT,0xFF}, /* u */
   {NOSHIFT,0xFF}, /* i */
   
   /*; 18 - 1F*/
   {NOSHIFT,0xFF}, /* o */
   {NOSHIFT,0xFF}, /* p */
   {NOSHIFT,0xFF}, /* [ */
   {NOSHIFT,0xFF}, /* ] */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 1 */
   {NOSHIFT,0xFF}, /* Keypad 2 */
   {NOSHIFT,0xFF}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {NOSHIFT,0xFF}, /* a */
   {NOSHIFT,0xFF}, /* s */
   {NOSHIFT,0xFF}, /* d */
   {NOSHIFT,0xFF}, /* f */
   {NOSHIFT,0xFF}, /* g */
   {NOSHIFT,0xFF}, /* h */
   {NOSHIFT,0xFF}, /* j */
   {NOSHIFT,0xFF}, /* k */
   
   /*; 28 - 2F*/
   {NOSHIFT,0xFF}, /* l */
   {NOSHIFT,0xFF}, /* ; */
   {NOSHIFT,0xFF}, /* ' */
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 4 */
   {NOSHIFT,0xFF}, /* Keypad 5 */
   {NOSHIFT,0xFF}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF}, /* z */
   {NOSHIFT,0xFF}, /* x */
   {NOSHIFT,0xFF}, /* c */
   {NOSHIFT,0xFF}, /* v */
   {NOSHIFT,0xFF}, /* b */
   {NOSHIFT,0xFF}, /* n */
   {NOSHIFT,0xFF}, /* m */
   
   /*; 38 - 3F*/
   {NOSHIFT,0xFF}, /* , */
   {NOSHIFT,0xFF}, /* . */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad . */
   {NOSHIFT,0xFF}, /* Keypad 7 */
   {NOSHIFT,0xFF}, /* Keypad 8 */
   {NOSHIFT,0xFF}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {NOSHIFT,0xFF}, /* SPACE BAR */
   {NOSHIFT,0xFF}, /* BKSP */
   {NOSHIFT,0xFF}, /* TAB */
   {NOSHIFT,0xFF}, /* Keypad Enter */
   {NOSHIFT,0xFF}, /* RETURN */
   {NOSHIFT,0xFF}, /* ESC */
   {NOSHIFT,0xFF}, /* DEL */
   {NOSHIFT,0xFF},
   
   /*; 48 - 4F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad - */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Up Arrow */
   {NOSHIFT,0xFF}, /* Down Arrow */
   {NOSHIFT,0xFF}, /* Right Arrow */
   {NOSHIFT,0xFF}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {NOSHIFT,0xFF}, /* F1 */
   {NOSHIFT,0xFF}, /* F2 */
   {NOSHIFT,0xFF}, /* F3 */
   {NOSHIFT,0xFF}, /* F4 */
   {NOSHIFT,0xFF}, /* F5 */
   {NOSHIFT,0xFF}, /* F6 */
   {NOSHIFT,0xFF}, /* F7 */
   {NOSHIFT,0xFF}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {NOSHIFT,0xFF}, /* F9 */
   {NOSHIFT,0xFF}, /* F10 */
   {NOSHIFT,0x45}, /* NumL */
   {NOSHIFT,0x46}, /* ScrL */
   {NOSHIFT,0xFF}, /* Pause */
   {NOSHIFT,0xFF}, /* PrtSc */
   {NOSHIFT,0xFF}, /* Keypad + */
   {NOSHIFT,0xFF}, /* HELP */
   
   /*; 60 - 67*/
   {NOSHIFT,0xFF}, /* LSHIFT */
   {NOSHIFT,0xFF}, /* RSHIFT */
   {NOSHIFT,0xFF}, /* CAPSLOCK */
   {NOSHIFT,0xFF}, /* CONTROL */
   {NOSHIFT,0xFF}, /* LALT */
   {NOSHIFT,0xFF}, /* RALT */
   {NOSHIFT,0xFF}, /* LAMIGA */
   {NOSHIFT,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 70 - 77*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 78 - 7F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
#endif

   };
/************/
/************/
/************/
static struct KeySeq A2PCShiftLeftAmiga[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {NOSHIFT,0xFF}, /* Tilde */
   {NOSHIFT,0xFF}, /* 1 */
   {NOSHIFT,0xFF}, /* 2 */
   {NOSHIFT,0xFF}, /* 3 */
   {NOSHIFT,0xFF}, /* 4 */
   {NOSHIFT,0xFF}, /* 5 */
   {NOSHIFT,0xFF}, /* 6 */
   {NOSHIFT,0xFF}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {NOSHIFT,0xFF}, /* 8 */
   {NOSHIFT,0xFF}, /* 9 */
   {NOSHIFT,0xFF}, /* 0 */
   {NOSHIFT,0xFF}, /* - */
   {NOSHIFT,0xFF}, /* = */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {NOSHIFT,0xFF}, /* q */
   {NOSHIFT,0xFF}, /* w */
   {NOSHIFT,0xFF}, /* e */
   {NOSHIFT,0xFF}, /* r */
   {NOSHIFT,0xFF}, /* t */
   {NOSHIFT,0xFF}, /* y */
   {NOSHIFT,0xFF}, /* u */
   {NOSHIFT,0xFF}, /* i */
   
   /*; 18 - 1F*/
   {NOSHIFT,0xFF}, /* o */
   {NOSHIFT,0xFF}, /* p */
   {NOSHIFT,0xFF}, /* [ */
   {NOSHIFT,0xFF}, /* ] */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 1 */
   {NOSHIFT,0xFF}, /* Keypad 2 */
   {NOSHIFT,0xFF}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {NOSHIFT,0xFF}, /* a */
   {NOSHIFT,0xFF}, /* s */
   {NOSHIFT,0xFF}, /* d */
   {NOSHIFT,0xFF}, /* f */
   {NOSHIFT,0xFF}, /* g */
   {NOSHIFT,0xFF}, /* h */
   {NOSHIFT,0xFF}, /* j */
   {NOSHIFT,0xFF}, /* k */
   
   /*; 28 - 2F*/
   {NOSHIFT,0xFF}, /* l */
   {NOSHIFT,0xFF}, /* ; */
   {NOSHIFT,0xFF}, /* ' */
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 4 */
   {NOSHIFT,0xFF}, /* Keypad 5 */
   {NOSHIFT,0xFF}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF}, /* z */
   {NOSHIFT,0xFF}, /* x */
   {NOSHIFT,0xFF}, /* c */
   {NOSHIFT,0xFF}, /* v */
   {NOSHIFT,0xFF}, /* b */
   {NOSHIFT,0xFF}, /* n */
   {NOSHIFT,0xFF}, /* m */
   
   /*; 38 - 3F*/
   {NOSHIFT,0xFF}, /* , */
   {NOSHIFT,0xFF}, /* . */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad . */
   {NOSHIFT,0xFF}, /* Keypad 7 */
   {NOSHIFT,0xFF}, /* Keypad 8 */
   {NOSHIFT,0xFF}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {NOSHIFT,0xFF}, /* SPACE BAR */
   {NOSHIFT,0xFF}, /* BKSP */
   {NOSHIFT,0xFF}, /* TAB */
   {NOSHIFT,0xFF}, /* Keypad Enter */
   {NOSHIFT,0xFF}, /* RETURN */
   {NOSHIFT,0xFF}, /* ESC */
   {NOSHIFT,0xFF}, /* DEL */
   {NOSHIFT,0xFF},
   
   /*; 48 - 4F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad - */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Up Arrow */
   {NOSHIFT,0xFF}, /* Down Arrow */
   {NOSHIFT,0xFF}, /* Right Arrow */
   {NOSHIFT,0xFF}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {NOSHIFT,0xFF}, /* F1 */
   {NOSHIFT,0xFF}, /* F2 */
   {NOSHIFT,0xFF}, /* F3 */
   {NOSHIFT,0xFF}, /* F4 */
   {NOSHIFT,0xFF}, /* F5 */
   {NOSHIFT,0xFF}, /* F6 */
   {NOSHIFT,0xFF}, /* F7 */
   {NOSHIFT,0xFF}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {NOSHIFT,0xFF}, /* F9 */
   {NOSHIFT,0xFF}, /* F10 */
   {LSHIFT, 0x45}, /* NumL */
   {LSHIFT, 0x46}, /* ScrL */
   {NOSHIFT,0xFF}, /* Pause */
   {NOSHIFT,0xFF}, /* PrtSc */
   {NOSHIFT,0xFF}, /* Keypad + */
   {NOSHIFT,0xFF}, /* HELP */
   
   /*; 60 - 67*/
   {NOSHIFT,0xFF}, /* LSHIFT */
   {NOSHIFT,0xFF}, /* RSHIFT */
   {NOSHIFT,0xFF}, /* CAPSLOCK */
   {NOSHIFT,0xFF}, /* CONTROL */
   {NOSHIFT,0xFF}, /* LALT */
   {NOSHIFT,0xFF}, /* RALT */
   {NOSHIFT,0xFF}, /* LAMIGA */
   {NOSHIFT,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 70 - 77*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 78 - 7F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
#endif   

   };
/************/
/************/
/************/
static struct KeySeq A2PCRightAmiga[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {NOSHIFT,0xFF}, /* Tilde */
   {NOSHIFT,0xFF}, /* 1 */
   {NOSHIFT,0xFF}, /* 2 */
   {NOSHIFT,0xFF}, /* 3 */
   {NOSHIFT,0xFF}, /* 4 */
   {NOSHIFT,0xFF}, /* 5 */
   {NOSHIFT,0xFF}, /* 6 */
   {NOSHIFT,0xFF}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {NOSHIFT,0xFF}, /* 8 */
   {NOSHIFT,0xFF}, /* 9 */
   {NOSHIFT,0xFF}, /* 0 */
   {NOSHIFT,0xFF}, /* - */
   /*{NOSHIFT,0xFF},*/ /* = */
   {NOSHIFT,0x4E}, /* +=Keypad + */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {NOSHIFT,0xFF}, /* q */
   {NOSHIFT,0xFF}, /* w */
   {NOSHIFT,0xFF}, /* e */
   {NOSHIFT,0xFF}, /* r */
   {NOSHIFT,0xFF}, /* t */
   {NOSHIFT,0xFF}, /* y */
   {NOSHIFT,0xFF}, /* u */
   {NOSHIFT,0xFF}, /* i */
   
   /*; 18 - 1F*/
   {NOSHIFT,0xFF}, /* o */
   /*{NOSHIFT,0xFF},*/ /* p */
   {NOSHIFT,0x37}, /* p=PrtSc */
   {NOSHIFT,0xFF}, /* [ */
   {NOSHIFT,0xFF}, /* ] */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 1 */
   {NOSHIFT,0xFF}, /* Keypad 2 */
   {NOSHIFT,0xFF}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {NOSHIFT,0xFF}, /* a */
   /*{NOSHIFT,0xFF},*/ /* s */
   {NOSHIFT,0x46}, /* s=ScrL */
   {NOSHIFT,0xFF}, /* d */
   {NOSHIFT,0xFF}, /* f */
   {NOSHIFT,0xFF}, /* g */
   {NOSHIFT,0xFF}, /* h */
   {NOSHIFT,0xFF}, /* j */
   {NOSHIFT,0xFF}, /* k */
   
   /*; 28 - 2F*/
   {NOSHIFT,0xFF}, /* l */
   {NOSHIFT,0xFF}, /* ; */
   {NOSHIFT,0xFF}, /* ' */
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 4 */
   {NOSHIFT,0xFF}, /* Keypad 5 */
   {NOSHIFT,0xFF}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF}, /* z */
   {NOSHIFT,0xFF}, /* x */
   {NOSHIFT,0xFF}, /* c */
   {NOSHIFT,0xFF}, /* v */
   {NOSHIFT,0xFF}, /* b */
   /*{NOSHIFT,0xFF}, *//* n */
   {NOSHIFT,0x45}, /* n=NumL */
   {NOSHIFT,0xFF}, /* m */
   
   /*; 38 - 3F*/
   {NOSHIFT,0xFF}, /* , */
   {NOSHIFT,0xFF}, /* . */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad . */
   {NOSHIFT,0xFF}, /* Keypad 7 */
   {NOSHIFT,0xFF}, /* Keypad 8 */
   {NOSHIFT,0xFF}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {NOSHIFT,0xFF}, /* SPACE BAR */
   {NOSHIFT,0xFF}, /* BKSP */
   {NOSHIFT,0xFF}, /* TAB */
   {NOSHIFT,0xFF}, /* Keypad Enter */
   {NOSHIFT,0xFF}, /* RETURN */
   {NOSHIFT,0xFF}, /* ESC */
   {NOSHIFT,0xFF}, /* DEL */
   {NOSHIFT,0xFF},
   
   /*; 48 - 4F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad - */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Up Arrow */
   {NOSHIFT,0xFF}, /* Down Arrow */
   {NOSHIFT,0xFF}, /* Right Arrow */
   {NOSHIFT,0xFF}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {NOSHIFT,0xFF}, /* F1 */
   {NOSHIFT,0xFF}, /* F2 */
   {NOSHIFT,0xFF}, /* F3 */
   {NOSHIFT,0xFF}, /* F4 */
   {NOSHIFT,0xFF}, /* F5 */
   {NOSHIFT,0xFF}, /* F6 */
   {NOSHIFT,0xFF}, /* F7 */
   {NOSHIFT,0xFF}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {NOSHIFT,0xFF}, /* F9 */
   {NOSHIFT,0xFF}, /* F10 */
   {NOSHIFT,0xFF}, /* NumL */
   {NOSHIFT,0xFF}, /* ScrL */
   {NOSHIFT,0xFF}, /* Pause */
   {NOSHIFT,0xFF}, /* PrtSc */
   {NOSHIFT,0xFF}, /* Keypad + */
   {NOSHIFT,0xFF}, /* HELP */
   
   /*; 60 - 67*/
   {NOSHIFT,0xFF}, /* LSHIFT */
   {NOSHIFT,0xFF}, /* RSHIFT */
   {NOSHIFT,0xFF}, /* CAPSLOCK */
   {NOSHIFT,0xFF}, /* CONTROL */
   {NOSHIFT,0xFF}, /* LALT */
   {NOSHIFT,0xFF}, /* RALT */
   {NOSHIFT,0xFF}, /* LAMIGA */
   {NOSHIFT,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 70 - 77*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 78 - 7F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
#endif

   };
/************/
/************/
/************/
static struct KeySeq A2PCShiftRightAmiga[MAXCODE]={
   /***********/
   /*; 00 - 07*/
   /***********/
   {NOSHIFT,0xFF}, /* Tilde */
   {NOSHIFT,0xFF}, /* 1 */
   {NOSHIFT,0xFF}, /* 2 */
   {NOSHIFT,0xFF}, /* 3 */
   {NOSHIFT,0xFF}, /* 4 */
   {NOSHIFT,0xFF}, /* 5 */
   {NOSHIFT,0xFF}, /* 6 */
   {NOSHIFT,0xFF}, /* 7 */
   
   /***********/
   /*; 08 - 0F*/
   /***********/
   {NOSHIFT,0xFF}, /* 8 */
   {NOSHIFT,0xFF}, /* 9 */
   {NOSHIFT,0xFF}, /* 0 */
   {NOSHIFT,0xFF}, /* - */
   /*{NOSHIFT,0xFF},*/ /* = */
   {LSHIFT,0x4E}, /* +=Keypad + */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 0 */
   
   /*; 10 - 17*/
   {NOSHIFT,0xFF}, /* q */
   {NOSHIFT,0xFF}, /* w */
   {NOSHIFT,0xFF}, /* e */
   {NOSHIFT,0xFF}, /* r */
   {NOSHIFT,0xFF}, /* t */
   {NOSHIFT,0xFF}, /* y */
   {NOSHIFT,0xFF}, /* u */
   {NOSHIFT,0xFF}, /* i */
   
   /*; 18 - 1F*/
   {NOSHIFT,0xFF}, /* o */
   /*{NOSHIFT,0xFF},*/ /* p */
   {LSHIFT,0x37}, /* p=PrtSc */
   {NOSHIFT,0xFF}, /* [ */
   {NOSHIFT,0xFF}, /* ] */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 1 */
   {NOSHIFT,0xFF}, /* Keypad 2 */
   {NOSHIFT,0xFF}, /* Keypad 3 */
   
   /*; 20 - 27*/
   {NOSHIFT,0xFF}, /* a */
   /*{NOSHIFT,0xFF},*/ /* s */
   {LSHIFT, 0x46}, /* s=ScrL */
   {NOSHIFT,0xFF}, /* d */
   {NOSHIFT,0xFF}, /* f */
   {NOSHIFT,0xFF}, /* g */
   {NOSHIFT,0xFF}, /* h */
   {NOSHIFT,0xFF}, /* j */
   {NOSHIFT,0xFF}, /* k */
   
   /*; 28 - 2F*/
   {NOSHIFT,0xFF}, /* l */
   {NOSHIFT,0xFF}, /* ; */
   {NOSHIFT,0xFF}, /* ' */
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad 4 */
   {NOSHIFT,0xFF}, /* Keypad 5 */
   {NOSHIFT,0xFF}, /* Keypad 6 */
   
   /*; 30 - 37*/
   {NOSHIFT,0xFF}, /* No Key! */
   {NOSHIFT,0xFF}, /* z */
   {NOSHIFT,0xFF}, /* x */
   {NOSHIFT,0xFF}, /* c */
   {NOSHIFT,0xFF}, /* v */
   {NOSHIFT,0xFF}, /* b */
   /*{NOSHIFT,0xFF},*/ /* n */
   {LSHIFT, 0x45}, /* n=NumL */
   {NOSHIFT,0xFF}, /* m */
   
   /*; 38 - 3F*/
   {NOSHIFT,0xFF}, /* , */
   {NOSHIFT,0xFF}, /* . */
   {NOSHIFT,0xFF}, /* / */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad . */
   {NOSHIFT,0xFF}, /* Keypad 7 */
   {NOSHIFT,0xFF}, /* Keypad 8 */
   {NOSHIFT,0xFF}, /* Keypad 9 */
   
   /*; 40 - 47*/
   {NOSHIFT,0xFF}, /* SPACE BAR */
   {NOSHIFT,0xFF}, /* BKSP */
   {NOSHIFT,0xFF}, /* TAB */
   {NOSHIFT,0xFF}, /* Keypad Enter */
   {NOSHIFT,0xFF}, /* RETURN */
   {NOSHIFT,0xFF}, /* ESC */
   {NOSHIFT,0xFF}, /* DEL */
   {NOSHIFT,0xFF},
   
   /*; 48 - 4F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Keypad - */
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF}, /* Up Arrow */
   {NOSHIFT,0xFF}, /* Down Arrow */
   {NOSHIFT,0xFF}, /* Right Arrow */
   {NOSHIFT,0xFF}, /* Left Arrow */
   
   /***********/
   /*; 50 - 57*/
   /***********/
   {NOSHIFT,0xFF}, /* F1 */
   {NOSHIFT,0xFF}, /* F2 */
   {NOSHIFT,0xFF}, /* F3 */
   {NOSHIFT,0xFF}, /* F4 */
   {NOSHIFT,0xFF}, /* F5 */
   {NOSHIFT,0xFF}, /* F6 */
   {NOSHIFT,0xFF}, /* F7 */
   {NOSHIFT,0xFF}, /* F8 */
   
   /***********/
   /*; 58 - 5F*/
   /***********/
   {NOSHIFT,0xFF}, /* F9 */
   {NOSHIFT,0xFF}, /* F10 */
   {LSHIFT, 0xFF}, /* NumL */
   {LSHIFT, 0xFF}, /* ScrL */
   {NOSHIFT,0xFF}, /* Pause */
   {NOSHIFT,0xFF}, /* PrtSc */
   {NOSHIFT,0xFF}, /* Keypad + */
   {NOSHIFT,0xFF}, /* HELP */
   
   /*; 60 - 67*/
   {NOSHIFT,0xFF}, /* LSHIFT */
   {NOSHIFT,0xFF}, /* RSHIFT */
   {NOSHIFT,0xFF}, /* CAPSLOCK */
   {NOSHIFT,0xFF}, /* CONTROL */
   {NOSHIFT,0xFF}, /* LALT */
   {NOSHIFT,0xFF}, /* RALT */
   {NOSHIFT,0xFF}, /* LAMIGA */
   {NOSHIFT,0xFF}, /* RAMIGA */
   
#if 0
   /*; 68 - 6F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 70 - 77*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   

   /*; 78 - 7F*/
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
   {NOSHIFT,0xFF},
#endif   

   };

#if 0
  $52  ; The PC Numeric Keypad 0 key (insert key)
  $4F  ; The PC Numeric Keypad 1 key (end key)
  $50  ; The PC Numeric Keypad 2 key (down-cursor key)
  $51  ; The PC Numeric Keypad 3 key (page-down key)
  $4B  ; The PC Numeric Keypad 4 key (left-cursor key)
  $4C  ; The PC Numeric Keypad 5 key
  $4D  ; The PC Numeric Keypad 6 key (right-cursor key)
  $47  ; The PC Numeric Keypad 7 key (home key)
  $48  ; The PC Numeric Keypad 8 key (up-cursor key)
  $49  ; The PC Numeric Keypad 9 key (page-up key)
  $53  ; The PC Numeric Keypad Dot   (del key)

  $4C  ; The Amiga Up-Cursor code
  $4F  ; The Amiga Left-Cursor code
  $4E  ; The Amiga Right-Cursor code
  $4D  ; The Amiga Down-Cursor code
  $46  ; The Amiga DEL key 

#endif


VOID QueuePCRawKey(message)
struct IntuiMessage *message;
{   
   UWORD  code,key;
   UBYTE  outcode;
   USHORT qualifier;
   static ULONG State=NOSHIFT,OldState=NOSHIFT,NewState=NOSHIFT;
   ULONG AmigaState=NOSHIFT;
   BOOL PCReset=FALSE;
   BOOL Keypad;

   code      = message->Code&0xff;
   key=code&0x7f;
   qualifier = message->Qualifier;

#if 0
   /*********/
   /* Debug */
   /*********/
   if(code&0x80)
      kprintf("AMIGA code=%ld 0x%02lx UP ",(ULONG)code&0x7f,(ULONG)code&0x7f);
   else
      kprintf("AMIGA code=%ld 0x%02lx DN ",(ULONG)code,(ULONG)code);

#endif

   /**********************************/
   /* Determine Amiga Keyboard State */
   /**********************************/
   if(qualifier&IEQUALIFIER_LSHIFT)
      AmigaState|=LSHIFT;

   if(qualifier&IEQUALIFIER_RSHIFT)
      AmigaState|=RSHIFT;

   if(qualifier&IEQUALIFIER_CAPSLOCK)
      AmigaState|=CAPSLOCK;

   if(qualifier&IEQUALIFIER_CONTROL)
      AmigaState|=CONTROL;

   if(qualifier&IEQUALIFIER_LALT)
      AmigaState|=ALT;

   if(qualifier&IEQUALIFIER_RALT)
   {
      AmigaState|=ALT;
      AmigaState|=CONTROL;
   }

   if(qualifier&IEQUALIFIER_LCOMMAND)
      AmigaState|=LAMIGA;

   if(qualifier&IEQUALIFIER_RCOMMAND)
      AmigaState|=RAMIGA;

/*
   if(qualifier&IEQUALIFIER_NUMERICPAD)
      AmigaState|=kprintf("NUMERICPAD ");

   if(qualifier&IEQUALIFIER_REPEAT)
      AmigaState|=kprintf("REPEAT ");

   if(qualifier&IEQUALIFIER_INTERRUPT)
      AmigaState|=kprintf("INTERRUPT ");

   if(qualifier&IEQUALIFIER_MULTIBROADCAST)
      AmigaState|=kprintf("MULTIBROADCAST ");
*/
   if(State&NUMLOCK)
      AmigaState|=NUMLOCK;
   else
      AmigaState&=~NUMLOCK;

   D(PrintState(AmigaState,"AmigaState     = "); )
   D(PrintState(State,     "PCState        = "); )

   if(AmigaState!=State)
   {
      State=ChangeState(AmigaState,State);
      D(PrintState(State,  "ReSync PCState = ");)
   }

   /**********************************/
   /* Update modifier State variable */
   /**********************************/
   if(key<MAXCODE)
   switch(key)
   {
      /********/
      /* CTRL */
      /********/
      case 0x63:
      /************/
      /* CAPSLOCK */
      /************/
      case 0x62:
      /**********/
      /* LSHIFT */
      /**********/
      case 0x60:
      /**********/
      /* RSHIFT */
      /**********/
      case 0x61: 
      /*******/
      /* ALT */
      /*******/
      case 0x64:
      /*********/
      /* ALTGR */
      /*********/
      case 0x65: 
      /**********/
      /* LAMIGA */
      /**********/
      case 0x66: 
      /**********/
      /* RAMIGA */
      /**********/
      case 0x67: 
         break;

      default:
         switch(State)
         {
            case NOSHIFT:
               /*kprintf("noshift\n");*/
               NewState= A2PCNoShift[code & 0x7F].State;
               outcode = A2PCNoShift[code & 0x7F].Code;
               break;

            case LSHIFT:
            case RSHIFT:
            case LSHIFT|RSHIFT:
               /*kprintf("shift\n");*/
               NewState= A2PCShift[code & 0x7F].State;
               outcode = A2PCShift[code & 0x7F].Code;
               break;

            case CAPSLOCK:
               /*kprintf("CAPSLOCK\n");*/
               NewState= A2PCCapsLock[code & 0x7F].State;
               outcode = A2PCCapsLock[code & 0x7F].Code;
               break;

            case CONTROL|ALT:
            case CONTROL|ALT|CAPSLOCK:
            case CONTROL|ALT|NUMLOCK:
            case CONTROL|ALT|NUMLOCK|CAPSLOCK:
               /*kprintf("altgr\n");*/
               NewState= A2PCAltGr[code & 0x7F].State;
               outcode = A2PCAltGr[code & 0x7F].Code;
			   if(State&CAPSLOCK)
			   {
			      NewState|=CAPSLOCK;
			   }
			   if(State&NUMLOCK)
			   {
			      NewState|=NUMLOCK;
			   }
               if(outcode==0xD3/* Del code */)
               {
                  PCReset=TRUE;
               }
               break;

            case CONTROL:
               /*kprintf("control\n");*/
               NewState= A2PCControl[code & 0x7F].State;
               outcode = A2PCControl[code & 0x7F].Code;
               break;

            case NUMLOCK:
               /*kprintf("numlock\n");*/
               NewState= A2PCNumLock[code & 0x7F].State;
               outcode = A2PCNumLock[code & 0x7F].Code;
               break;

            case LSHIFT|CAPSLOCK:
            case RSHIFT|CAPSLOCK:
            case LSHIFT|RSHIFT|CAPSLOCK:
               /*kprintf("shift capslock\n");*/
               NewState= A2PCShiftCapsLock[code & 0x7F].State;
               outcode = A2PCShiftCapsLock[code & 0x7F].Code;
               break;

            case LSHIFT|NUMLOCK:
            case RSHIFT|NUMLOCK:
            case LSHIFT|RSHIFT|NUMLOCK:
               /*kprintf("shift numlock\n");*/
               NewState= A2PCShiftNumLock[code & 0x7F].State;
               outcode = A2PCShiftNumLock[code & 0x7F].Code;
               break;

            case CAPSLOCK|NUMLOCK:
               /*kprintf("capslock numlock\n");*/
               NewState= A2PCCapsLockNumLock[code & 0x7F].State;
               outcode = A2PCCapsLockNumLock[code & 0x7F].Code;
               break;

            case LSHIFT|CAPSLOCK|NUMLOCK:
            case RSHIFT|CAPSLOCK|NUMLOCK:
            case LSHIFT|RSHIFT|CAPSLOCK|NUMLOCK:
               /*kprintf("shift capslock numlock\n");*/
               NewState= A2PCShiftCapsLockNumLock[code & 0x7F].State;
               outcode = A2PCShiftCapsLockNumLock[code & 0x7F].Code;
               break;

            case LAMIGA:
            case LAMIGA|CAPSLOCK:
            case LAMIGA|NUMLOCK:
            case LAMIGA|NUMLOCK|CAPSLOCK:
               NewState= A2PCLeftAmiga[code & 0x7F].State;
			   if(State&CAPSLOCK)
			   {
			      NewState|=CAPSLOCK;
			   }
			   if(State&NUMLOCK)
			   {
			      NewState|=NUMLOCK;
			   }
               outcode = A2PCLeftAmiga[code & 0x7F].Code;
               break;

            case LSHIFT|LAMIGA:
            case RSHIFT|LAMIGA:
            case LSHIFT|RSHIFT|LAMIGA:
            case LSHIFT|LAMIGA|CAPSLOCK:
            case RSHIFT|LAMIGA|CAPSLOCK:
            case LSHIFT|RSHIFT|LAMIGA|CAPSLOCK:
            case LSHIFT|LAMIGA|NUMLOCK:
            case RSHIFT|LAMIGA|NUMLOCK:
            case LSHIFT|RSHIFT|LAMIGA|NUMLOCK:
            case LSHIFT|LAMIGA|NUMLOCK|CAPSLOCK:
            case RSHIFT|LAMIGA|NUMLOCK|CAPSLOCK:
            case LSHIFT|RSHIFT|LAMIGA|NUMLOCK|CAPSLOCK:
               NewState= A2PCShiftLeftAmiga[code & 0x7F].State;
			   if(State&CAPSLOCK)
			   {
			      NewState|=CAPSLOCK;
			   }
			   if(State&NUMLOCK)
			   {
			      NewState|=NUMLOCK;
			   }
               outcode = A2PCShiftLeftAmiga[code & 0x7F].Code;
               break;
            case RAMIGA:
            case RAMIGA|CAPSLOCK:
            case RAMIGA|NUMLOCK:
            case RAMIGA|NUMLOCK|CAPSLOCK:
               NewState= A2PCRightAmiga[code & 0x7F].State;
			   if(State&CAPSLOCK)
			   {
			      NewState|=CAPSLOCK;
			   }
			   if(State&NUMLOCK)
			   {
			      NewState|=NUMLOCK;
			   }
               outcode = A2PCRightAmiga[code & 0x7F].Code;
               break;

            case LSHIFT|RAMIGA:
            case RSHIFT|RAMIGA:
            case LSHIFT|RSHIFT|RAMIGA:
            case LSHIFT|RAMIGA|CAPSLOCK:
            case RSHIFT|RAMIGA|CAPSLOCK:
            case LSHIFT|RSHIFT|RAMIGA|CAPSLOCK:
            case LSHIFT|RAMIGA|NUMLOCK:
            case RSHIFT|RAMIGA|NUMLOCK:
            case LSHIFT|RSHIFT|RAMIGA|NUMLOCK:
            case LSHIFT|RAMIGA|NUMLOCK|CAPSLOCK:
            case RSHIFT|RAMIGA|NUMLOCK|CAPSLOCK:
            case LSHIFT|RSHIFT|RAMIGA|NUMLOCK|CAPSLOCK:
               NewState= A2PCShiftRightAmiga[code & 0x7F].State;
			   if(State&CAPSLOCK)
			   {
			      NewState|=CAPSLOCK;
			   }
			   if(State&NUMLOCK)
			   {
			      NewState|=NUMLOCK;
			   }
               outcode = A2PCShiftRightAmiga[code & 0x7F].Code;
               break;

            default:
               /*kprintf("NO CODE!\n");*/
               NewState=State;
               outcode = A2PCNoShift[code & 0x7F].Code;
               break;
      }
	  if(NewState&ALTK)
	  {
	     /*NewState&=~ALTK;*/
	     Keypad=TRUE;
		 if(code&0x80)
		    outcode=0xff;
	 	 D( kprintf("Keypad=TRUE\n");)
	  } else {
	     Keypad=FALSE;
	 	 D( kprintf("Keypad=FALSE\n");)
	  }
      if(outcode!=0xff)
      {
         D(PrintState(State,"Output Code OldState ");)

         OldState=State;
         State=ChangeState(NewState,State);

         D(PrintState(State,"OutPut Code NewState ");)

         if(Keypad)
		 {
		    if(!(code&0x80)) /* only send the sequence on the down stroke */
			{
			   D( kprintf("KP1,2,3\n"); )
			   QueueOneKey(KeyPadCode(outcode/100L));
			   QueueOneKey(KeyPadCode(outcode/100L)|0x80);
			   QueueOneKey(KeyPadCode((outcode%100)/10L));
			   QueueOneKey(KeyPadCode((outcode%100)/10L)|0x80);
			   QueueOneKey(KeyPadCode(outcode%10));
			   QueueOneKey(KeyPadCode(outcode%10)|0x80);
			}
		    D( kprintf("ALTK\n"); )
		 } else {
            outcode = (outcode & 0x7F) | (code & 0x80);
            QueueOneKey(outcode);
            D(
            if(outcode&0x80)
               kprintf("outcode=%ld 0x%lx UP\n",
			      (ULONG)outcode&0x7f,
			      (ULONG)outcode&0x7f);
            else
               kprintf("outcode=%ld 0x%lx DN\n",
			      (ULONG)outcode,
			      (ULONG)outcode);
            )
		 }



         if(PCReset)
         {
            State=NOSHIFT;
         } else {

            State=ChangeState(OldState,State);

            if((!(NewState&=CONTROL))&&
               (outcode==0x45)) /* NumLock */
            {
               if(State&NUMLOCK)
               {
                  State&=~NUMLOCK;
               } else {
                  State|=NUMLOCK;
               }
            }
         }
        D(PrintState(State,"OutPut Code RestoredState ");)
      }
      break;
   }
   PCCurrentState=State;
   D(kprintf("\n");)

}
ULONG ChangeState(ULONG NewState,ULONG CurrentState)
{
   if((NewState&NUMLOCK)!=(CurrentState&NUMLOCK))
   {
      if(CurrentState&NUMLOCK)
     {
        CurrentState&=~NUMLOCK;
         QueueOneKey(0x45);
         QueueOneKey(0x45|0x80);
     } else {
        CurrentState|=NUMLOCK;
         QueueOneKey(0x45);
         QueueOneKey(0x45|0x80);
     }
   }
   if((CurrentState&ALT)&&(!(NewState&ALT)))
   {
      CurrentState&=~ALT;
      QueueOneKey(0x38|0x80);
   }

   if((NewState&CONTROL)!=(CurrentState&CONTROL))
   {
      if(CurrentState&CONTROL)
      {
         CurrentState&=~CONTROL;
         QueueOneKey(0x1d|0x80);
      } else {
         CurrentState|=CONTROL;
         QueueOneKey(0x1d);
      }
   }
   if((!(CurrentState&ALT))&&(NewState&ALT))
   {
      CurrentState|=ALT;
      QueueOneKey(0x38);
   }
#if 0
   if((NewState&ALT)!=(CurrentState&ALT))
   {
      if(CurrentState&ALT)
      {
         CurrentState&=~ALT;
         QueueOneKey(0x38|0x80);
      } else {
         CurrentState|=ALT;
         QueueOneKey(0x38);
      }
   }
#endif
/*
   if((NewState&ALTGR)!=(CurrentState&ALTGR))
   {
      if(CurrentState&ALTGR)
     {
        CurrentState&=~ALTGR;
     } else {
        CurrentState|=ALTGR;
     }
   }
*/
   if((NewState&LAMIGA)!=(CurrentState&LAMIGA))
   {
      if(CurrentState&LAMIGA)
     {
        CurrentState&=~LAMIGA;
     } else {
        CurrentState|=LAMIGA;
     }
   }
   if((NewState&RAMIGA)!=(CurrentState&RAMIGA))
   {
      if(CurrentState&RAMIGA)
     {
        CurrentState&=~RAMIGA;
     } else {
        CurrentState|=RAMIGA;
     }
   }
   if((NewState&CAPSLOCK)!=(CurrentState&CAPSLOCK))
   {
     if(CurrentState&CAPSLOCK)
       {
          CurrentState&=~CAPSLOCK;
            QueueOneKey(0x3a);
            QueueOneKey(0x3a|0x80);
       } else {
          CurrentState|=CAPSLOCK;
            QueueOneKey(0x3a);
            QueueOneKey(0x3a|0x80);
       }
   }
   if(((NewState&LSHIFT)||(NewState&RSHIFT))!=
     ((CurrentState&LSHIFT)||(CurrentState&RSHIFT)))
   {
      /* Should be shifted but isnt */
      if(((NewState&LSHIFT)||(NewState&RSHIFT)))
     {
       if(NewState&LSHIFT)
       {
           CurrentState|=LSHIFT;
            QueueOneKey(0x2a);
       } 
       if(NewState&RSHIFT)
       {
           CurrentState|=RSHIFT;
            QueueOneKey(0x36);
       } 
     } else {
        if(CurrentState&RSHIFT)
       {
          CurrentState&=~RSHIFT;
            QueueOneKey(0x36|0x80);
       }
       if(CurrentState&LSHIFT)
       {
          CurrentState&=~LSHIFT;
            QueueOneKey(0x2a|0x80);
       }
     }
   }
   return(CurrentState);
}
#endif
BOOL ReadKeymap(VOID)
{
   FILE *fp;
   /*UBYTE b,l,i;*/
   UBYTE buf[80];
   ULONG l1,l2,l3,l4,r;

   if((fp=fopen("sys:pc/system/PCWKeymap","r"))==NULL)
      return(FALSE);

   while(fgets(buf,sizeof(buf),fp))
   {
      r=sscanf(buf,"%lx %lx %lx %lx\n",&l1,&l2,&l3,&l4);
     if(r==4)
     {
        D(kprintf("r=%1ld l1=0x%02lx l2=0x%04lx l3=0x%04lx l4=0x%02lx\n",r,l1,l2,l3,l4);)
		if( ((UBYTE)l1)<MAXCODE)
        switch(l2)
       {
          case NOSHIFT:
            A2PCNoShift[(UBYTE)l1].State=(ULONG)l3;
            A2PCNoShift[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case LSHIFT:
            A2PCShift[(UBYTE)l1].State=(ULONG)l3;
            A2PCShift[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case CAPSLOCK:
            A2PCCapsLock[(UBYTE)l1].State=(ULONG)l3;
            A2PCCapsLock[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case CONTROL|ALT:
            A2PCAltGr[(UBYTE)l1].State=(ULONG)l3;
            A2PCAltGr[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case CONTROL:
            A2PCControl[(UBYTE)l1].State=(ULONG)l3;
            A2PCControl[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case NUMLOCK:
            A2PCNumLock[(UBYTE)l1].State=(ULONG)l3;
            A2PCNumLock[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case LSHIFT|CAPSLOCK:
            A2PCShiftCapsLock[(UBYTE)l1].State=(ULONG)l3;
            A2PCShiftCapsLock[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case LSHIFT|NUMLOCK:
            A2PCShiftNumLock[(UBYTE)l1].State=(ULONG)l3;
            A2PCShiftNumLock[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case CAPSLOCK|NUMLOCK:
            A2PCCapsLockNumLock[(UBYTE)l1].State=(ULONG)l3;
            A2PCCapsLockNumLock[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         case LSHIFT|CAPSLOCK|NUMLOCK:
            A2PCShiftCapsLockNumLock[(UBYTE)l1].State=(ULONG)l3;
            A2PCShiftCapsLockNumLock[(UBYTE)l1].Code =(UBYTE)l4;
            break;
         default:
            break;
       }
     }
   }

   fclose(fp);
   return(TRUE);
}
#if 0
VOID PrintState(ULONG State,UBYTE *s)
{
   kprintf("%s",s);
   if(State&CONTROL)
      kprintf("CONTROL ");
   if(State&CAPSLOCK)
      kprintf("CAPSLOCK ");
   if(State&LSHIFT)
      kprintf("LSHIFT ");
   if(State&RSHIFT)
      kprintf("RSHIFT ");
   if(State&ALT)
      kprintf("ALT ");
   /*if(State&ALTGR)
      kprintf("ALTGR ");*/
   if(State&LAMIGA)
      kprintf("LAMIGA ");
   if(State&RAMIGA)
      kprintf("RAMIGA ");
   if(State&NUMLOCK)
      kprintf("NUMLOCK ");
   kprintf("\n");
}
#endif
ULONG KeyPadCode(UBYTE code)
{
   /*D(kprintf("KP %ld\n",code);)*/
   switch(code&0x7f)
   {
      case 0:
	     D(kprintf("KP 0\n");)
	     return(0x52);
      case 1:
	     D(kprintf("KP 1\n");)
	     return(0x4f);
      case 2:
	     D(kprintf("KP 2\n");)
	     return(0x50);
      case 3:
	     D(kprintf("KP 3\n");)
	     return(0x51);
      case 4:
	     D(kprintf("KP 4\n");)
	     return(0x4b);
      case 5:
	     D(kprintf("KP 5\n");)
	     return(0x4c);
      case 6:
	     D(kprintf("KP 6\n");)
	     return(0x4d);
      case 7:
	     D(kprintf("KP 7\n");)
	     return(0x47);
      case 8:
	     D(kprintf("KP 8\n");)
	     return(0x48);
      case 9:
	     D(kprintf("KP 9\n");)
	     return(0x49);
   }
}
