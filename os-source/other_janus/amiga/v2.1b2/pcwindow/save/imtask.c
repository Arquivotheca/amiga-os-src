
/* *** imtask.c **************************************************************
 * 
 * Input Monitor Task for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 24 Jul 88   -RJ         Added the cursor key support for Henri
 * 26 Jan 88   -RJ         Started adding the mouse and scroll stuff to the 
 *                         logic of the input monitor
 * 15 Oct 87   - RJ        Added call to timer (variable duration) between 
 *                         sending keys to PC.  Fixes a problem for Rudolf 
 *                         and hopefully can be used by Clipboard stuff too.
 *                         All changes should be marked TIMER
 * 1 Mar 86    =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

#include <janus/janusbase.h>
#include <janus/memory.h>
#include <janus/janusreg.h>
#include <janus/janusvar.h>
#include <janus/services.h>


/*NOSCROLL extern   ULONG   ScrollSignal;*/



#define INPUT_BUFFERSIZE    6
struct   ConsoleDevice *ConsoleDevice = NULL;
struct   IOStdReq IOStdReq = {0};
struct   InputEvent InputEvent = {0};
UBYTE   InputBuffer[INPUT_BUFFERSIZE];



/* === Keyboard Events Variables ======================================== */
SHORT   KeyBufferNextSend = 0;
SHORT   KeyBufferNextSlot = 0;
UBYTE   KeyBuffer[KEYBUFFER_SIZE];
BOOL   PCWantsKey = FALSE;
SHORT   KeyFlags = NULL;
SHORT   InputFlags = NULL;
SHORT   MoveStartX, MoveStartY;
UBYTE   *KeyboardAddress;
LONG   KeyDelaySeconds = 0;
LONG   KeyDelayMicros = 5000;
ULONG   PCWantsKeySignal = 0;



/* === InputFlags definitions =========================================== */
#define IN_SELECT_MODE   0x0001
#define IN_MOVE_MODE   0x0002



void InputMonitor();
void QueuePCRawKey();
void QueueOneKey();
void SendOneKey();
void IrvingForcesFFDownZaphodsThroat();
void CloseInputMonitorTask();
 
void InputMonitor()
/* The input monitor task spends its energy monitoring the main input
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
 */
{
    REGIST   ULONG wakeUpBits;
    ULONG   waitBits, inputSignal;
    ULONG   timerSignal;
    REGIST   struct IntuiMessage *message;
    REGIST   struct SuperWindow *superwindow;
    struct   Window *inputWindow;
    REGIST   struct Display *display;
    struct   MsgPort *inputPort;
    struct   MsgPort *timerPort;
    struct    IOStdReq *timerMsg;
    BOOL   timerSet, pcrawkey;
    SHORT   i;


    inputPort = NULL;
    timerPort = NULL;
    timerMsg = NULL;
    timerSet = NULL;


#ifdef AZTEC
    /* This gets A4 from a memory location.  This is needed because
     * Manx has implemented relative-addressing using A4.  Task object files
     * should be clustered around the call to SaveA4();
     */
    GetA4();
#endif


/* === Critical Section.  Must be completed in toto ==================== */ 
    Forbid();

    InputTask = FindTask(0);
    InputSuicideSignal = ZAllocSignal();

    Permit();
/* === End of Critical Section ========================================== */

/*NOSCROLL     SetupScroll();*/
/*???    MakePCMouseConnection();*/

    if (OpenDevice("console.device", -1, &IOStdReq, 0) == 0)
   {
   ConsoleDevice = (struct ConsoleDevice *)IOStdReq.io_Device;
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

    if (OpenDevice(TIMERNAME, UNIT_MICROHZ, timerMsg, 0) != 0)
   {
   MyAlert(ALERT_INCOMPLETESYSTEM, NULL);
   CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet);
   }

    timerSignal = 1 << timerPort->mp_SigBit;

    /* Establish the PC interrupt stuff */
    KeyboardAddress = (UBYTE *)((ULONG)GetJanusStart() + (ULONG)IOREG_OFFSET);
    KeyboardAddress += ((ULONG)JanusBase->jb_KeyboardRegisterOffset);

#ifdef DEBUG
   kprintf("Board offset = 0x%lx\n",GetJanusStart());
   kprintf("Register offset = 0x%lx\n",JanusBase->jb_KeyboardRegisterOffset);
   kprintf("Keyboard offset = 0x%lx\n",KeyboardAddress);
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
/*???kprintf("On entry, RESET flag was set\n");*/
   ClearFlag(JanusAmigaWA->ja_AmigaState, AMIGA_NUMLOCK_RESET);
/*???   QueueOneKey(PCNumLockCode);*/
/*???   QueueOneKey(PCNumLockCode | 0x80);*/
   SetFlag(KeyFlags, KEY_NUMLOCK_SET);
   }
    else if (FlagIsSet(JanusAmigaWA->ja_AmigaState, AMIGA_NUMLOCK_SET))
   {
/*???kprintf("On entry SET flag was set\n");*/
   SetFlag(KeyFlags, KEY_NUMLOCK_SET);
   }
/*???else kprintf("On entry, neither RESET nor SET was set\n");*/

/*NOSCROLL     waitBits = inputSignal | PCWantsKeySignal | InputSuicideSignal */
/*NOSCROLL        | timerSignal | ScrollSignal;*/
    waitBits = inputSignal | PCWantsKeySignal | InputSuicideSignal 
       | timerSignal;

    FOREVER
   {
   wakeUpBits = Wait(waitBits);

   if (wakeUpBits & InputSuicideSignal) 
       CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet);

/*NOSCROLL    if (wakeUpBits & ScrollSignal) */
/*NOSCROLL        Scroll();*/

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
         i = RawKeyConvert(&InputEvent, &InputBuffer[0], INPUT_BUFFERSIZE, NULL);
         if ( (i == 3)
            && (InputBuffer[0] == 0x9B)
            && (InputBuffer[1] == 0x3F)
            && (InputBuffer[2] == 0x7E) )
             pcrawkey = FALSE;
         }
          }
      else pcrawkey = FALSE;

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
          }
      else
          {
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
             }
         else if (message->Code == SELECTUP)
             {
             ClearFlag(InputFlags, IN_SELECT_MODE);
             ClearFlag(InputFlags, IN_MOVE_MODE);
             }
         }

          else if (message->Class == GADGETDOWN)
         {
         SetFlag(InputFlags, IN_SELECT_MODE);
         MoveStartX = message->MouseX;
         MoveStartY = message->MouseY;
         }
          else if (message->Class == GADGETUP)
         {
         ClearFlag(InputFlags, IN_SELECT_MODE);
         ClearFlag(InputFlags, IN_MOVE_MODE);
         }

          if (message->Class == MOUSEMOVE)
         {
         if (FlagIsSet(InputFlags, IN_SELECT_MODE))
             if (FlagIsClear(InputFlags, IN_MOVE_MODE))
            if ( (ABS(message->MouseX - MoveStartX) 
               > CHAR_WIDTH)
               || (ABS(message->MouseY - MoveStartY) 
               > CHAR_HEIGHT) )
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
              if (inputWindow
                  == superwindow->DisplayWindow)
                 {
                 if (display->TaskPort)
               PutMsg(display->TaskPort,(struct Message *) message);
                 Unlock(&display->DisplayLock);
                 goto WHEW;
                 }   /* end of finding our window */
              superwindow = superwindow->NextWindow;
              }   /* end of superwindow while-loop */
                Unlock(&display->DisplayLock);
                }   /* end of examining one display */
             display = display->NextDisplay;
             }   /* end of display while-loop */
WHEW: /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
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



void QueuePCRawKey(message)
REGISTER struct IntuiMessage *message;
{   
    REGISTER UBYTE code;
    REGISTER UBYTE outcode, outkey;
    REGISTER USHORT qualifier;
           
    code = message->Code & 0xFF;
    qualifier = message->Qualifier;

/*???kprintf("code=$%lx\n", code);*/

    if (qualifier & AMIGARIGHT) SetFlag(KeyFlags, KEY_AMIGA);
    else ClearFlag(KeyFlags, KEY_AMIGA);

    if ((code == AmigaCapsLockCode) || (code == (AmigaCapsLockCode | 0x80)))
   {
   /* Thus sprach Dieter.   Both CAPSLOCK DOWN
    * and CAPSLOCK UP should generate both CAPSLOCK DOWN and CAPSLOCK UP.
    */
   QueueOneKey(PCCapsLockCode);
   QueueOneKey(PCCapsLockCode | 0x80);
   }
    else if ((code == AmigaLeftAltCode) || (code == AmigaRightAltCode))
   {
   if ((KeyFlags & KEY_ALTKEY) == 0)
       {
       /* We don't have ALT down yet, so transmit one now */
       QueueOneKey(PCAltCode);
       SetFlag(KeyFlags, KEY_ALTKEY);
       }
   }
    else if ((code == (AmigaLeftAltCode | 0x80)) 
       || (code == (AmigaRightAltCode | 0x80)))
   {
   if (KeyFlags & KEY_ALTKEY) /* If we've seen an alt-down */
       if ((qualifier & ALTKEYS) == 0) /* If both alts are up */
      {
      QueueOneKey(PCAltCode | 0x80);
      ClearFlag(KeyFlags, KEY_ALTKEY); 
      }
   }
    else if ( (code != 0xFF)
       && ((code == AmigaTildeBackDash) || (code == AmigaBarBackSlash)) )
   {
   QueueOneKey(PCCtrlCode);
   QueueOneKey(PCAltCode);
   if (FlagIsSet(qualifier, SHIFTY))
       {
       if (code == AmigaTildeBackDash) outkey = PCTildeCode;
       else outkey = PCBarCode;
       }
   else
       {
       if (code == AmigaTildeBackDash) outkey = PCBackDashCode;
       else outkey = PCBackSlashCode;
       }
   QueueOneKey(outkey);
   QueueOneKey(outkey | 0x80);
   QueueOneKey(PCAltCode | 0x80);
   QueueOneKey(PCCtrlCode | 0x80);
   }
    else if ( (code != 0xFF) 
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
/*???   if (FlagIsSet(KeyFlags, KEY_NUMLOCK_SET))*/
   if (FlagIsClear(KeyFlags, KEY_NUMLOCK_SET))
      {
/*???kprintf("SET was clear?\n");*/
      QueueOneKey(PCNumLockCode);
      QueueOneKey(PCNumLockCode | 0x80);
      }
   if (code == AmigaCursorUp) outcode = PCKeypad8Code;
   else if (code == AmigaCursorLeft) outcode = PCKeypad4Code;
   else if (code == AmigaCursorRight) outcode = PCKeypad6Code;
   else if (code == AmigaCursorDown) outcode = PCKeypad2Code;
   else if (code == AmigaDelCode) outcode = PCKeypadDotCode;
   QueueOneKey(outcode);
   QueueOneKey(outcode | 0x80);
/*???   if (FlagIsSet(KeyFlags, KEY_NUMLOCK_SET))*/
   if (FlagIsClear(KeyFlags, KEY_NUMLOCK_SET))
      {
      QueueOneKey(PCNumLockCode);
      QueueOneKey(PCNumLockCode | 0x80);
      }
   }
    else if ( (code != 0xFF) 
      && ( (code == (AmigaCursorUp | 0x80) ) 
        || (code == (AmigaCursorLeft | 0x80) ) 
        || (code == (AmigaCursorRight | 0x80) ) 
        || (code == (AmigaDelCode | 0x80) ) 
        || (code == (AmigaCursorDown | 0x80) ) )) 
   {
   /* On the upstroke of the cursor keys, do nothing */
   }
    else
   {
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
/*???kprintf("if (code == AmigaNCode) ToggleFlag(KeyFlags, KEY_NUMLOCK_SET); \n");*/
      ToggleFlag(KeyFlags, KEY_NUMLOCK_SET); 
      }
       else if (code == (AmigaNCode | 0x80))
      {
      /* AMIGARIGHT N up == Num Lock up */
      QueueOneKey(PCNumLockCode | 0x80);
      ClearFlag(KeyFlags, KEY_AMIGAPENDING);
      }
       else if (code == AmigaSCode)
      {
      /* AMIGARIGHT S == Scroll Lock */
      QueueOneKey(PCScrollLockCode);
      SetFlag(KeyFlags, KEY_AMIGAPENDING); 
      }
       else if (code == (AmigaSCode | 0x80))
      {
      /* AMIGARIGHT S up == Scroll Lock up */
      QueueOneKey(PCScrollLockCode | 0x80);
      ClearFlag(KeyFlags, KEY_AMIGAPENDING); 
      }
       else if (code == AmigaPlusCode)
      {
      /* AMIGARIGHT + == + on PC num keypad */
      QueueOneKey(PCPlusCode);
      SetFlag(KeyFlags, KEY_AMIGAPENDING); 
      }
       else if (code == (AmigaPlusCode | 0x80))
      {
      /* AMIGARIGHT + up == + up on num keypad */
      QueueOneKey(PCPlusCode | 0x80);
      ClearFlag(KeyFlags, KEY_AMIGAPENDING); 
      }
       else if (code == AmigaPCode)
      {
      /* AMIGARIGHT P == PrtScr */
      QueueOneKey(PCPtrScrCode);
      SetFlag(KeyFlags, KEY_AMIGAPENDING); 
      }
       else if (code == (AmigaPCode | 0x80))
      {
      /* AMIGARIGHT P up == PrtScr up */
      QueueOneKey(PCPtrScrCode | 0x80);
      ClearFlag(KeyFlags, KEY_AMIGAPENDING); 
      }
       /* I'm in a pissed mood.  Some self-mutilation ... */
       else goto INELEGANT_SO_BUGGER_OFF;
       }
   else
       {
       /* Output this key using normal table lookup */
INELEGANT_SO_BUGGER_OFF:
       /* ... after just one more special case, Prt Scr */
/* Rudolf sez no:if (code == AmigaPrtScrCode) QueueOneKey(PCLeftShiftCode);*/

       outcode = PCRawKeyTable[code & 0x7F];
       outcode = (outcode & 0x7F) | (code & 0x80);
/*???kprintf("code=$%lx ", (ULONG)(code));*/
/*???kprintf("outcode=$%lx\n", (ULONG)(outcode));*/
       if ((outcode & 0x7F) != 0x7F) QueueOneKey(outcode);

       if (outcode == PCNumLockCode)
/*???{*/
      ToggleFlag(KeyFlags, KEY_NUMLOCK_SET); 
/*???kprintf("if (outcode == PCNumLockCode) ToggleFlag(KeyFlags, SET);\n");*/
/*???}*/

/* Rudolf sez no:if (code == AmigaPrtScrCode) QueueOneKey(PCLeftShiftCode | 0x80);*/
       }
   }
}

      

void QueueOneKey(code)
REGISTER UBYTE code;
{
    REGISTER SHORT i;

#ifdef DEBUG
   kprintf("imtask.c: QueueOneKey(0x%lx)\n",code);
#endif

    i = (KeyBufferNextSlot + 1) & KEYBUFFER_MASK; 
    if (i == KeyBufferNextSend)
   {
   /* Oops, buffer overflow.  If this is the down transition, beep */
   if ((code & 0x80) == 0)       
       DisplayBeep(NULL);
   }
    else
   {
   KeyBuffer[KeyBufferNextSlot] = code;
   KeyBufferNextSlot = i;
   } 

    if (PCWantsKey) SendOneKey();      
}



void SendOneKey()
/* The caller should correctly ascertain that there's a key to send
 * before calling this routine.  This allows this routine to avoid
 * checking if there's a key to send, which optimizes the case where
 * somebody already knows there's one to send (for instance, when one
 * was just stuffed into the buffer while the PC is waiting for a key).
 */  
{
    REGISTER UBYTE *janus;

#ifdef DEBUG
   kprintf("imtask.c: SendOneKey()\n");
#endif

    janus = (UBYTE *)GetJanusStart();

/*???if ((KeyBuffer[KeyBufferNextSend] & 0x80) == 0)*/
/*???   kprintf("$%lx\n", KeyBuffer[KeyBufferNextSend]);*/

    /* Write the data to the PC keycode-input register */
    *(KeyboardAddress) = KeyBuffer[KeyBufferNextSend];

#ifdef DEBUG
   kprintf("imtask.c: SendOneKey Janus = 0x%lx\n",janus);
   kprintf("imtask.c: SendOneKey KeyboardAddress = 0x%lx\n",KeyboardAddress);
#endif


    /* Tell the PC that we've got a key ready in the register.  Writing
     * this value to this register causes a keyboard interrupt on the PC.
     */
    *(janus + PCINTGEN_OFFSET) = JPCKEYINT;

    PCWantsKey = FALSE;

    KeyBufferNextSend = (KeyBufferNextSend + 1) & KEYBUFFER_MASK;
}


void CloseInputMonitorTask(inputPort, timerPort, timerMsg, timerSet)
REGISTER struct MsgPort *inputPort;
REGISTER struct MsgPort *timerPort;
REGISTER struct IOStdReq *timerMsg;
REGISTER BOOL timerSet;
/* TIMER:  added closing the timer to this routine */
{
    REGISTER struct Task *task;

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
       if (GetMsg(timerPort) == NULL) Wait(1 << timerPort->mp_SigBit);
   CloseDevice(timerMsg);
   DeleteStdIO(timerMsg);
   }
    if (timerPort) DeletePort(timerPort);

    if (PCKeySig) CleanupJanusSig(PCKeySig);
    ZFreeSignal(PCWantsKeySignal);

/*NOSCROLL     CleanupScroll();*/
/*???    UnmakePCMouseConnection();*/

    task = InputTask;
    InputTask = NULL;

    RemTask(task);
}


void IrvingForcesFFDownZaphodsThroat()
/* This is retained for historical purposes only */
{
#ifdef NOTDEFINED
    UBYTE *janus;

    if (IrvingToggle)
   {
   janus = GetJanusStart();

   /* Write the data to the PC keycode-input register */
   *(janus + KeyboardOffset) = 0xFF;

   /* Tell the PC that we've got a key ready in the register.  Writing
    * this value to this register causes a keyboard interrupt on the PC.
    */
   *(janus + PCIntGenOffset) = JPCKEYINT;
   }
#endif
}


