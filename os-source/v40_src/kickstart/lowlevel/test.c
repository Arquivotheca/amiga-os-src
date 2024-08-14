/*****************************************************************************
*
*	$Id: test.c,v 40.8 93/07/30 16:12:09 vertex Exp $
*
******************************************************************************
*
*	$Log:	test.c,v $
 * Revision 40.8  93/07/30  16:12:09  vertex
 * *** empty log message ***
 * 
 * Revision 40.7  93/03/26  17:36:25  Jim2
 * Added test for reading language selection.
 *
 * Revision 40.6  93/03/23  14:42:01  Jim2
 * Added printing of qualifier value when reading joystick rawkeys.
 *
 * Revision 40.5  93/03/12  20:06:23  Jim2
 * Added test for SetLanguageSelection and the new tag on SystemControl.
 *
 * Revision 40.4  93/03/10  12:23:58  Jim2
 * Let's just leave all tests enabled.
 *
 * Revision 40.3  93/03/08  10:47:05  Jim2
 * Added tests for KillRequesters and port 0.
 *
 * Revision 40.2  93/03/04  11:55:33  Jim2
 * Added support for the unknown controller returning button
 * information.
 *
 * Revision 40.1  93/03/02  13:28:03  Jim2
 * The library name has been changed from game to lowlevel.
 *
 * Revision 40.0  93/02/11  17:31:19  Jim2
 * Uses the new single include file rather than the three seperate ones.
 *
 * Revision 1.6  93/01/19  15:52:08  Jim2
 * Added test for killing input.device.
 *
 * Revision 1.5  93/01/19  10:43:59  Jim2
 * Change the tag to match.
 *
 * Revision 1.4  93/01/15  13:56:17  Jim2
 * Changed the timer and keyboard functions.
 *
 * Revision 1.3  93/01/13  13:45:14  Jim2
 * Rewrote keyboard function tests for the rewritten keyboard
 * functions.
 *
 * Revision 1.2  93/01/07  14:27:24  Jim2
 * Added the testing of the keyboard functions and the joyport.
 *
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************/
#include <exec/nodes.h>
#include <exec/tasks.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>

#include "lowlevel_pragmas.h"
#include "lowlevel_protos.h"

#define EXECBASE (*(struct ExecBase **)4)

struct TagItem TestTags[] =
{
    {SCON_CDReboot,    CDReboot_On},
    {SCON_KillReq,     TRUE},
    {SCON_TakeOverSys, TRUE},
    {TAG_DONE, TRUE}
};

struct mySignal
{
    struct Task *Task;
    ULONG Signal;
    ULONG KeyPress;
};


struct ExecBase *SysBase = NULL;

VOID __asm Int(register __a1 ULONG *Data);
VOID __asm Int2(register __a1 struct mySignal *Data, register __d0 UWORD Key);


ULONG cmd(VOID)
{
    struct Library *DOSBase, *LowLevelBase;

    SysBase = EXECBASE;

    DOSBase=OpenLibrary("dos.library", 0);
    if ((LowLevelBase = OpenLibrary("lowlevel.library",0)) != NULL)
    {
	Printf ("Opened library\n");


	if (FALSE)
	{
	    Printf ("\n\n*************************Testing SetLanguageSelection\n");
	    SetLanguageSelection (3);
	}

	if (FALSE)
	{
	    Printf ("\n\n*************************Testing GetLanguageSelection\n\t%lx\n",
	   	GetLanguageSelection ());
	}

	if (FALSE)
	{
	   struct Process *Me;

	   Printf ("\n\n\n***********************Testing SystemControlA\n");
	    Me = (struct Process *) FindTask(NULL);
	    Printf ("About to LowLevelBase Own system\nPriority=%lx ... ",Me->pr_Task.tc_Node.ln_Pri);
	    if (SystemControlA(&(TestTags[2])) == 0)
	    {
	    	Printf(" Yes - %lx ...", Me->pr_Task.tc_Node.ln_Pri);
	    	if (SystemControlA(&(TestTags[1])) == 0)
	    	{
		    Printf("Killed -%lx", Me->pr_Task.tc_Node.ln_Pri);
		    if (SystemControlA(&(TestTags[0])) == 0)
		        Printf ("How did we get at the CD?\n");
		    else
		    	Printf ("Failed with CD %lx\n", Me->pr_Task.tc_Node.ln_Pri);
		    TestTags[1]. ti_Data = FALSE;
		    TestTags[2]. ti_Data = FALSE;
		    if (SystemControlA(&(TestTags[1])) == 0)
		    	Printf ("Rel Kill %lx - ", Me->pr_Task.tc_Node.ln_Pri);
		    else
		    	Printf ("Count unkill %lx -", Me->pr_Task.tc_Node.ln_Pri);
	    	}
	    	else
		    Printf("Once...");
	    	TestTags[2]. ti_Data = FALSE;
	    	if (SystemControlA(&(TestTags[2])) == 0)
		    Printf(" %lx\n", Me->pr_Task.tc_Node.ln_Pri);
	    	else
		    Printf(" unrel %lx -", Me->pr_Task.tc_Node.ln_Pri);
	    }
	}
	if (TRUE)
	{
	    struct Library * IntuitionBase;
	    struct TagItem InputTags[] =
	    {
		{SCON_AddCreateKeys, 1},
		{TAG_DONE, TRUE}
	    };
	    struct TagItem WindowTags[] =
	    {
		{WA_Height, 100},
		{WA_Width, 100},
		{WA_Title, (ULONG)"GameController Converstion"},
		{WA_Flags, WFLG_CLOSEGADGET},
		{WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_RAWKEY},
		{TAG_DONE, TRUE}
	    };

	    Printf ("\n\n\n****************Testing joystick/game controller raw key stuff\n");
	    if ((IntuitionBase = OpenLibrary ("intuition.library",0)) != NULL)
	    {
		if (SystemControlA(InputTags) == 0)
		{
		    struct Window *InputWindow;

		    if ((InputWindow = OpenWindowTagList (NULL, WindowTags)) != NULL)
		    {
			struct IntuiMessage *News;
			BOOL	StayIn = TRUE;

			while (StayIn)
			{
			    if ((News = (struct IntuiMessage *) GetMsg(InputWindow->UserPort)) != NULL)
			    {
				switch (News->Class)
				{
				    case CLOSEWINDOW :
					StayIn = FALSE;
					break;
				    case IDCMP_RAWKEY :
					Printf ("\t");
					if (News->Qualifier !=0)
					    Printf ("%lx", News->Qualifier);
					Printf(" >%lx<\n", News->Code);
					break;
				}
				ReplyMsg((struct Message *) News);
			     }
			}
			CloseWindow (InputWindow);
		    }
		    InputTags[0].ti_Tag = SCON_RemCreateKeys;
		    SystemControlA(InputTags);
		}
	    }
	    else
		Printf("Cannot open Intuition\n");
	}


	if (FALSE)
	{
	    ULONG Count = 0;
	    struct EClockVal Time;
	    APTR Handle;

	    Printf ("\n\n\n****************Testing Timer Interrupts\n");
	    Handle = AddTimerInt(Int, &Count);
	    StartTimerInt(Handle, 10000, FALSE);
	    ElapsedTime(&Time);
	    Delay(500);
	    StopTimerInt(Handle);
	    Printf ("%lx Interrupts @ 10000 in %08lx seconds\n",Count, ElapsedTime(&Time));
	    Count=0;
	    Delay(500);
	    Printf ("%lx Interrupts when stopped for %08lx seconds\n",Count, ElapsedTime(&Time));
	    Count=0;
	    StartTimerInt(Handle, 20000, TRUE);
	    Delay(500);
	    StopTimerInt(Handle);
	    RemTimerInt(Handle);
	    Printf ("%lx Interrupts @20000in %08lx seconds\n",Count, ElapsedTime(&Time));
	}
	if (FALSE)
	{
	    APTR Handle;
	    struct mySignal IntData;
	    BYTE SignalBitNumber;
	    ULONG Key,LastKey=0xFF;

	    Printf ("\n\n\n****************Testing Keyboard Interrupt\n\tHit q to exit\n");
	    IntData.Task = FindTask(NULL);
	    SignalBitNumber = AllocSignal(-1);
	    IntData.Signal = 1 << SignalBitNumber;
	    Handle = AddKBInt(Int2, &IntData);
	    do
	    {
		Wait(IntData.Signal);
		Key=GetKey();
		if (Key != LastKey)
		{
		    Printf("\t >");
		    if ((Key & LLKF_LSHIFT) != 0)
			Printf (" LSHIFT");
		    if ((Key & LLKF_RSHIFT) != 0)
			Printf(" RSHIFT");
		    if ((Key & LLKF_CAPSLOCK) != 0)
			Printf (" CAPSLOCK");
		    if ((Key & LLKF_CONTROL) != 0)
			Printf (" CONTROL");
		    if ((Key & LLKF_LALT) != 0)
			Printf (" LALT");
		    if ((Key & LLKF_RALT) != 0)
			Printf (" RALT");
		    if ((Key & LLKF_LAMIGA) != 0)
			Printf (" LAMIGA");
		    if ((Key & LLKF_RAMIGA) != 0)
		  	Printf (" RAMIGA");
		    if ((Key & 0xFF) == 0xFF)
			Printf (" No key\n");
		    else
			Printf(" %lx<\n", (0xFF & Key));
		    LastKey = Key;
		}
	    }
	    while (Key != 0x10);
	    RemKBInt(Handle);
	    FreeSignal(SignalBitNumber);
	}
	if (FALSE)
	{
	    struct KeyQuery Matrix[16];
	    UBYTE I, J=0;


	    for (I = 0; I < 16; I++)
		Matrix[I]. kq_KeyCode = 32+I;
	    Printf ("\n\n\n****************Testing Keyboard multiple\n\tHit a to exit\n");
	    do
	    {
		if ((J++ % 8) == 0)
		    Printf (" a s d f g h j k l '       4 5 6\n");
		QueryKeys(Matrix, 16);
		for (I = 0; (I < 16); I++)
		    Printf(" %s", (Matrix[I]. kq_Pressed ? "d" : "u"));
		Printf ("\n");
	    }
	    while (!Matrix[0].kq_Pressed);
	}
	if (FALSE)
	{
	    ULONG LastRead = -1, Current;

	    Printf("\n\n\n****************Testing JoyPort 0\n\tHit z to exit");
	    do
	    {
		if (LastRead != (Current = ReadJoyPort(0)))
		{
		    if ((LastRead & JP_TYPE_MASK) != (Current & JP_TYPE_MASK))
		    {
			Printf ("New Controller - ");
			switch (Current & JP_TYPE_MASK)
			{
			    case JP_TYPE_NOTAVAIL :
				Printf ("Port Data Unavailable\n");
				break;
			    case JP_TYPE_GAMECTLR :
				Printf ("Game\n");
				break;
			    case JP_TYPE_MOUSE :
				Printf ("Mouse\n");
				break;
			    case JP_TYPE_JOYSTK :
				Printf ("JoyStick\n");
				break;
			    case JP_TYPE_UNKNOWN :
				Printf ("Unknown\n");
				break;
			    default :
				Printf ("Illegal\n");
				break;
			}
		    }
		    switch (Current & JP_TYPE_MASK)
		    {
			case JP_TYPE_NOTAVAIL :
			    if ((Current & (~JP_TYPE_MASK)) != 0)
				Printf ("So where is the data comming from?\n");
			    break;
			case JP_TYPE_GAMECTLR :
			    Printf ("Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("A ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("B ");
			    if ((Current & JPF_BTN3) != 0)
				Printf ("X ");
			    if ((Current & JPF_BTN4) != 0)
				Printf ("Y ");
			    if ((Current & JPF_BTN5) != 0)
				Printf ("R ");
			    if ((Current & JPF_BTN6) != 0)
				Printf ("L ");
			    if ((Current & JPF_BTN7) != 0)
				Printf ("START ");
			    if ((Current & JPF_UP) != 0)
				Printf ("Up ");
			    if ((Current & JPF_DOWN) != 0)
				Printf ("Down ");
			    if ((Current & JPF_LEFT) != 0)
				Printf ("Left ");
			    if ((Current & JPF_RIGHT) != 0)
				Printf ("Right ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2|JPF_BTN3|
					JPF_BTN4|JPF_BTN5|JPF_BTN6|JPF_BTN7|JPF_UP|JPF_DOWN|
					JPF_LEFT|JPF_RIGHT))) != 0)
				Printf ("INVALID ");
			    Printf ("\n");
			    break;
			case JP_TYPE_MOUSE :
			    Printf ("Mouse Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("Right ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("Left ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2|JP_MVERT_MASK|
					JP_MHORZ_MASK))) != 0)
				Printf ("INVALID ");
			    Printf ("Counters:  Vertical-%02lx  Horizontal-%02lx\n",
					(Current &JP_MVERT_MASK)>>8, (Current&JP_MHORZ_MASK));
			    break;
			case JP_TYPE_JOYSTK :
			    Printf ("Joystick Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("Right ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("Fire ");
			    Printf ("    Directions:  ");
			    if ((Current & JPF_UP) != 0)
				Printf ("Up ");
			    if ((Current & JPF_DOWN) != 0)
				Printf ("Down ");
			    if ((Current & JPF_LEFT) != 0)
				Printf ("Left ");
			    if ((Current & JPF_RIGHT) != 0)
				Printf ("Right ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2|JPF_UP|JPF_DOWN|
					JPF_LEFT|JPF_RIGHT))) != 0)
				Printf ("INVALID ");
			    Printf ("\n");
			    break;
			case JP_TYPE_UNKNOWN :
			    Printf ("Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("Right ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("Left ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2))) != 0)
				Printf ("INVALID ");
			    Printf ("\n");
			    break;
		    }
		LastRead=Current;
		}
	    }
	    while(GetKey() != 0x31);

	}
	if (FALSE)
	{
	    ULONG LastRead = -1, Current;

	    Printf("\n\n\n****************Testing JoyPort 1\n\tHit z to exit");
	    do
	    {
		if (LastRead != (Current = ReadJoyPort(1)))
		{
		    if ((LastRead & JP_TYPE_MASK) != (Current & JP_TYPE_MASK))
		    {
			Printf ("New Controller - ");
			switch (Current & JP_TYPE_MASK)
			{
			    case JP_TYPE_NOTAVAIL :
				Printf ("Port Data Unavailable\n");
				break;
			    case JP_TYPE_GAMECTLR :
				Printf ("Game\n");
				break;
			    case JP_TYPE_MOUSE :
				Printf ("Mouse\n");
				break;
			    case JP_TYPE_JOYSTK :
				Printf ("JoyStick\n");
				break;
			    case JP_TYPE_UNKNOWN :
				Printf ("Unknown\n");
				break;
			    default :
				Printf ("Illegal\n");
				break;
			}
		    }
		    switch (Current & JP_TYPE_MASK)
		    {
			case JP_TYPE_NOTAVAIL :
			    if ((Current & (~JP_TYPE_MASK)) != 0)
				Printf ("So where is the data comming from?\n");
			    break;
			case JP_TYPE_GAMECTLR :
			    Printf ("Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("A ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("B ");
			    if ((Current & JPF_BTN3) != 0)
				Printf ("X ");
			    if ((Current & JPF_BTN4) != 0)
				Printf ("Y ");
			    if ((Current & JPF_BTN5) != 0)
				Printf ("R ");
			    if ((Current & JPF_BTN6) != 0)
				Printf ("L ");
			    if ((Current & JPF_BTN7) != 0)
				Printf ("START ");
			    if ((Current & JPF_UP) != 0)
				Printf ("Up ");
			    if ((Current & JPF_DOWN) != 0)
				Printf ("Down ");
			    if ((Current & JPF_LEFT) != 0)
				Printf ("Left ");
			    if ((Current & JPF_RIGHT) != 0)
				Printf ("Right ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2|JPF_BTN3|
					JPF_BTN4|JPF_BTN5|JPF_BTN6|JPF_BTN7|JPF_UP|JPF_DOWN|
					JPF_LEFT|JPF_RIGHT))) != 0)
				Printf ("INVALID ");
			    Printf ("\n");
			    break;
			case JP_TYPE_MOUSE :
			    Printf ("Mouse Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("Right ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("Left ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2|JP_MVERT_MASK|
					JP_MHORZ_MASK))) != 0)
				Printf ("INVALID ");
			    Printf ("Counters:  Vertical-%02lx  Horizontal-%02lx\n",
					(Current &JP_MVERT_MASK)>>8, (Current&JP_MHORZ_MASK));
			    break;
			case JP_TYPE_JOYSTK :
			    Printf ("Joystick Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("Right ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("Fire ");
			    Printf ("    Directions:  ");
			    if ((Current & JPF_UP) != 0)
				Printf ("Up ");
			    if ((Current & JPF_DOWN) != 0)
				Printf ("Down ");
			    if ((Current & JPF_LEFT) != 0)
				Printf ("Left ");
			    if ((Current & JPF_RIGHT) != 0)
				Printf ("Right ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2|JPF_UP|JPF_DOWN|
					JPF_LEFT|JPF_RIGHT))) != 0)
				Printf ("INVALID ");
			    Printf ("\n");
			    break;
			case JP_TYPE_UNKNOWN :
			    Printf ("Buttons pressed:  ");
			    if ((Current & JPF_BTN1) != 0)
				Printf ("Right ");
			    if ((Current & JPF_BTN2) != 0)
				Printf ("Left ");
			    if ((Current & (~(JP_TYPE_MASK|JPF_BTN1|JPF_BTN2))) != 0)
				Printf ("INVALID ");
			    Printf ("\n");
			    break;
		    }
		LastRead=Current;
		}
	    }
	    while(GetKey() != 0x31);
	}
	if (TRUE)
	{
	    struct TagItem InputTags[] =
	    {
		{SCON_StopInput, TRUE},
		{TAG_DONE, TRUE}
	    };

	    Printf ("\n\n\n****************SystemControlA - StopInput for 30 seconds\n");
	    if (SystemControlA(InputTags) == 0)
		Delay(150);
	    Printf ("Turning on\n");
	    InputTags[0]. ti_Data = FALSE;
	    SystemControlA(InputTags);
	}


	if (FALSE)
	{
	    struct TagItem InputTags[] =
	    {
		{SCON_KillReq, TRUE},
		{TAG_DONE, TRUE}
	    };

	    Printf ("\n\n\n****************SystemControlA - KillRequesters for 30 seconds\n");
	    if (SystemControlA(InputTags) == 0)
		Delay(1500);
	    Printf ("Turning on\n");
	    InputTags[0]. ti_Data = FALSE;
	    SystemControlA(InputTags);
	}


	CloseLibrary(LowLevelBase);
	Printf ("Closed library\n");
    }
    CloseLibrary(DOSBase);
    return (0);
} /* main */


VOID __asm Int(register __a1 ULONG *Data)
{
    (*Data)++;
}

VOID __asm Int2(register __a1 struct mySignal *Data, register __d0 UWORD Key)
{
	Signal(Data->Task, Data->Signal);
}