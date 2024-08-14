
/* ***************************************************************************
 * 
 * Input Monitor Task for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 1 Mar 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"			    



/* === Keyboard Events Variables ======================================== */
SHORT KeyBufferNextSend = 0;
SHORT KeyBufferNextSlot = 0;
UBYTE KeyBuffer[KEYBUFFER_SIZE];  
BOOL PCWantsKey = FALSE;
SHORT KeyFlags = NULL;


 
InputMonitor()
/* The input monitor task spends its energy monitoring the main input
 * port for the Zaphod tasks.  When input events arrive, the following
 * evaluation is made:
 *
 *	- if the input is not RAWKEY, it is transmitted to the appropriate
 *	  display task
 *	- if the input is RAWKEY, this input is translated into a PC-style
 *	  keycode and then sent via the Janus interrupt manager to the PC
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
 * This task is created by the others.	When another task wants to find
 * this one but finds that this one doesn't exist, it creates this one.
 *
 * If the SuicideSignal is received, this task quietly suicides.
 */
{
    ULONG waitBits, wakeUpBits, inputSignal, PCWantsKeySignal;
    struct IntuiMessage *message;
    struct SuperWindow *superwindow;
    struct Window *inputWindow;
    struct Display *display;
    struct MsgPort *inputPort;


/* === Critical Section.  Must be completed in toto ==================== */ 
    Forbid();

    InputTask = FindTask(0);
    InputSuicideSignal = ZAllocSignal();

    Permit();
/* === End of Critical Section ========================================== */

    if ((inputPort = CreatePort(INPUT_PORT_NAME, 0)) == NULL)
	{
	Alert(ALERT_NO_INPUTPORT, NULL);
	CloseInputMonitorTask(NULL);
	}

    inputSignal = 1 << inputPort->mp_SigBit;

    /* Establish the PC interrupt stuff */
    KeyBufferNextSend = KeyBufferNextSlot = 0;
    PCWantsKey = TRUE;
    QueueOneKey(0xFF);

    /* Queue up some events that will "clear" the PC keyboard state */
    QueueOneKey(PCCtrlCode | 0x80);  /* CTRL UP */
    QueueOneKey(PCAltCode | 0x80);  /* ALT UP */
    QueueOneKey(PCLeftShiftCode | 0x80);  /* Left SHIFT UP */
    QueueOneKey(PCRightShiftCode | 0x80);  /* Right SHIFT UP */

    ClearFlag(KeyFlags, KEY_ALTKEY);
    ClearFlag(KeyFlags, KEY_AMIGAPENDING);

    /* When the PC is ready to receive a rawkey event, this task will
     * receive this signal.
     */
    PCWantsKey = FALSE; /* PC doesn't want a key until I say so */
    PCWantsKeySignal = ZAllocSignal();

#ifdef JANUS

    if ( (PCKeySig = SetupJanusSig(JSERV_ENBKB,
	    StupidCreateSignalNumber(PCWantsKeySignal), 0, 0)) == NULL)
	{
	Alert(ALERT_NO_JANUS_SIG, NULL);
	CloseInputMonitorTask(inputPort);
	}

#endif
 
    waitBits = inputSignal | PCWantsKeySignal | InputSuicideSignal;

    FOREVER
	{
	wakeUpBits = Wait(waitBits);

	if (wakeUpBits & InputSuicideSignal) CloseInputMonitorTask(inputPort);

	if (wakeUpBits & PCWantsKeySignal)
	    {
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
	    while (message = GetMsg(inputPort))
		{
		/* I've put the Forbid() inside the loop intentionally.
		 * This will allow other urgent tasks to get a time slice.
		 * This is program politesse.
		 */
		Forbid();
		inputWindow = message->IDCMPWindow;

		switch (message->Class)
		    {
		    case RAWKEY:
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
			ReplyMsg(message);
			break;

		    default:
			if (Lock(&ZaphodDisplay.DisplayListLock))
			    {
			    Forbid();
			    display = ZaphodDisplay.FirstDisplay;
			    while (display)
				{
				if (Lock(&display->DisplayLock))
				   {
				   superwindow = display->FirstWindow;
				   while (superwindow)
				      {
				      if (inputWindow
					       == superwindow->DisplayWindow)
					 {
					 if (display->TaskPort)
					    PutMsg(display->TaskPort, message);
					 break;
					 }
				      superwindow = superwindow->NextWindow;
				      } 
				   Unlock(&display->DisplayLock);
				   }
				display = display->NextDisplay;
				}
			    Permit();
			    Unlock(&ZaphodDisplay.DisplayListLock);
			    }
			break;
		    }

		Permit();
		}
	    }
	}
}



QueuePCRawKey(message)
struct IntuiMessage *message;
{   
    UBYTE code, outcode;
    USHORT qualifier;
		     
    code = message->Code & 0xFF;
    qualifier = message->Qualifier;

    if (qualifier & AMIGARIGHT) SetFlag(KeyFlags, KEY_AMIGA);
    else ClearFlag(KeyFlags, KEY_AMIGA);

    if ((code == AmigaCapsLockCode) || (code == (AmigaCapsLockCode | 0x80)))
	{
	/* Thus sprach Dieter.	Both CAPSLOCK DOWN
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
	    }

	else
	    {
	    /* Output this key using normal table lookup */
	    outcode = PCRawKeyTable[code & 0x7F];
	    outcode = (outcode & 0x7F) | (code & 0x80);
	    if ((outcode & 0x7F) != 0x7F)
		QueueOneKey(outcode);
	    }
	}
}

	   
QueueOneKey(code)
UBYTE code;
{
    SHORT i;

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


SendOneKey()
/* The caller should correctly ascertain that there's a key to send
 * before calling this routine.  This allows this routine to avoid
 * checking if there's a key to send, which optimizes the case where
 * somebody already knows there's one to send (for instance, when one
 * was just stuffed into the buffer while the PC is waiting for a key).
 */  
{
    UBYTE *janus;

    PCWantsKey = FALSE;

#ifdef JANUS
    janus = GetJanusStart();

    /* Write the data to the PC keycode-input register */
    *(janus + KEYBOARD_OFFSET) = KeyBuffer[KeyBufferNextSend];

    /* Tell the PC that we've got a key ready in the register.  Writing
     * this value to this register causes a keyboard interrupt on the PC.
     */
    *(janus + PCINTGEN_OFFSET) = JPCKEYINT;
#else
    janus = 0; /* To prevent a compile-time warning */
    kprintf("\n<%lx>", KeyBuffer[KeyBufferNextSend]);
#endif

    KeyBufferNextSend = (KeyBufferNextSend + 1) & KEYBUFFER_MASK;	 
}


CloseInputMonitorTask(inputPort)
struct MsgPort *inputPort;
{
    struct Task *task;

    Forbid();

    if (inputPort)
	{
	DrainPort(inputPort);
	DeletePort(inputPort);
	}

#ifdef JANUS
    if (PCKeySig) CleanupJanusSig(PCKeySig);
#endif

    task = InputTask;
    InputTask = NULL;

    RemTask(task);
}


IrvingForcesFFDownZphdsThroat()
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




