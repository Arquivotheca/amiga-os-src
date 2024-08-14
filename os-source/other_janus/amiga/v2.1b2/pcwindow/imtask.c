
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
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/console.h>
#include <janus/janusreg.h>
#include <janus/janusvar.h>
#include <janus/jfuncs.h>

#define  DBG_INPUT_MONITOR_ENTER                         0
#define  DBG_INPUT_MONITOR_OFFSETS                       0
#define  DBG_QUEUE_PC_RAW_KEY_ENTER                      0
#define  DBG_QUEUE_ONE_KEY_ENTER                         0
#define  DBG_SEND_ONE_KEY_ENTER                          0
#define  DBG_SEND_ONE_KEY_ADDRESSES                      0
#define  DBG_CLOSE_INPUT_MONITOR_TASK_ENTER              0
#define  DBG_IRVING_FORCES_FF_DOWN_ZAPHODS_THROAT_ENTER  0

#define  INPUT_BUFFERSIZE    6
struct   Library *ConsoleDevice = NULL; 
struct   IOStdReq IOStdReq = {0};
struct   InputEvent InputEvent = {0};
UBYTE    InputBuffer[INPUT_BUFFERSIZE];

/* === Keyboard Events Variables ======================================== */
SHORT    KeyBufferNextSend = 0;
SHORT    KeyBufferNextSlot = 0;
UBYTE    KeyBuffer[KEYBUFFER_SIZE];
BOOL     PCWantsKey = FALSE;
SHORT    KeyFlags = NULL;
SHORT    InputFlags = NULL;
SHORT    MoveStartX, MoveStartY;
UBYTE    *KeyboardAddress;
LONG     KeyDelaySeconds = 0;
LONG     KeyDelayMicros = 5000;
ULONG    PCWantsKeySignal = 0;

/* === InputFlags definitions =========================================== */
#define IN_SELECT_MODE  0x0001
#define IN_MOVE_MODE    0x0002

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

#if (DEBUG1 & DBG_INPUT_MONITOR_ENTER)
   kprintf("imtask.c:     InputMonitor(VOID)\n");
#endif

   inputPort = NULL;
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

   inputSignal = 1 << inputPort->mp_SigBit;


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

   timerSignal = 1 << timerPort->mp_SigBit;

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
   UBYTE code;
   UBYTE outcode, outkey;
   USHORT qualifier;

#if (DEBUG1 & DBG_QUEUE_PC_RAW_KEY_ENTER)
   kprintf("imtask.c:     QueuePCRawKey(message=0x%lx)\n",message);
   kprintf("imtask.c:     QueuePCRawKey: code = 0x%lx)\n",message->Code);
#endif
           
   code = message->Code & 0xFF;
   qualifier = message->Qualifier;

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
               if ((qualifier & ALTKEYS) == 0) /* If both alts are up */
               {
                  QueueOneKey((UBYTE)(PCAltCode | 0x80));
                  ClearFlag(KeyFlags, KEY_ALTKEY); 
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
                     outkey = PCTildeCode;
                  else
                     outkey = PCBarCode;
               } else {
                  if (code == AmigaTildeBackDash)
                     outkey = PCBackDashCode;
                  else
                     outkey = PCBackSlashCode;
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
                           QueueOneKey(0x79);
                           return;
                        }
                        if(code==0xC3)    /* keypad ENTER up */
                        {
                           /*
                           QueueOneKey(0xE0);
                           QueueOneKey(0x9C);
                           */
                           QueueOneKey(0xF9);
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

#if (DEBUG1 & DBG_QUEUE_ONE_KEY_ENTER)
   kprintf("imtask.c:     QueueOneKey(0x%lx)\n",code);
#endif

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
   REGISTER UBYTE *janus;

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
		    Wait(1 << timerPort->mp_SigBit);
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
