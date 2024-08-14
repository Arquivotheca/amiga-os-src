/*****************************************************************************
*
*	$Id: lowlevelbase.h,v 40.1 93/03/08 10:45:17 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	lowlevelbase.h,v $
 * Revision 40.1  93/03/08  10:45:17  Jim2
 * Added fields to support the patched EasyRequest.
 * 
 * Revision 40.0  93/03/02  13:11:10  Jim2
 * Renamed from GameBase.h.
 *
 * Revision 39.11  93/01/19  15:50:54  Jim2
 * Removed language from structure.  Added nesting count for killing
 * input.device.
 *
 * Revision 39.10  93/01/18  13:37:03  Jim2
 * Changed field that contains CD default behavior.
 *
 * Revision 39.9  93/01/15  14:06:33  Jim2
 * Added fields from ReadJoyPort manipulation of gameport.device.
 * Removed the fields used for keyboard repeat.
 *
 * Revision 39.8  93/01/13  14:53:27  Jim2
 * Added structures for autorepeating the keyboard.
 *
 * Revision 39.7  93/01/07  14:28:08  Jim2
 * Cleaned up of the comments and the field ordering.
 *
 * Revision 39.6  93/01/05  12:15:45  Jim2
 * Added fields for keyboard support.
 *
 * Revision 39.5  92/12/17  18:17:49  Jim2
 * Added the fields needed to support the routines for Timer.asm.
 *
 * Revision 39.4  92/12/14  15:03:06  Jim2
 * *** empty log message ***
 *
 * Revision 39.3  92/12/11  14:17:10  Jim2
 * Removed the list of process context and added nesting
 * fields for the owning of the system and the killinf
 * of requesters.
 *
 * Revision 39.2  92/12/09  18:13:42  Jim2
 * Added OpenCnt to the GBPI record to guard against someone
 * opening the library multiple times and our throwing away
 * this information on the first exit.
 *
 * Revision 39.1  92/12/08  16:51:56  Jim2
 * Added all of the fields.
 *
 * Revision 39.0  92/12/07  16:06:07  Jim2
 * Initial release prior to testing.
 *
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************/

#ifndef LOWLEVELBASE_H
#define LOWLEVELBASE_H

#ifndef EXEC_LISTS_H
    #include <exec/lists.h>
#endif

#ifndef EXEC_LIBRARIES_H
    #include <exec/libraries.h>
#endif

#ifndef EXEC_SEMAPHORES_H
    #include <exec/semaphores.h>
#endif

#ifndef	EXEC_INTERRUPTS_H
    #include <exec/interrupts.h>
#endif

#ifndef DEVICES_TIMER_H
    #include <devices/timer.i>
#endif

struct llct_CIATimer
{
    struct Interrupt llct_TimerInt;		//Interrupt structure.  (word sized)
    APTR	llct_Resource;			//Timer within the CIA where the interrupt is hung.
    UWORD	llct_WhichTimer;		//CIA resource where the interrupt is hung.
} ;

struct LowLevelBase
{
    struct Library	ll_LibNode;		//(word sized)
    UBYTE	ll_TaskPri;			//Orginal Task priority, if a task owns the system; 127 otherwise.
    UBYTE	ll_NestOwn;			//Nest count for owning the system; starts at -1.
    UBYTE	ll_NestReq;			//Nest count for killing requesters; starts at -1.
    UBYTE	ll_NestKillIn;			//Nest count for killing input.device; starts at -1.
    UBYTE	ll_NestReqAll;			//Next count for killing requesters including desire of nonvolatile.library; starts at -1
    UBYTE	ll_Pad0;			//Get to long word alignment.
    UWORD	ll_Pad1;			//Matching single WORD, but let's keep the padding bytes together.
    UWORD	ll_DefaultCDReboot;		//CD Reboot behaviour reported at init time.
    ULONG	ll_ER_Location;			//Pointer to the EasyRequest code that is to execute.
    ULONG	ll_ER_move;			//move ll_ER_Location(pc),-(sp)
    ULONG	ll_ER_clear_rts;		//CLEAR d0/rts
    ULONG	ll_LastKey;			//Last key code read from the keyboard.
    ULONG	ll_EClockConversion;		//EClocks per second, used for converting from milliseconds to EClocks.
    ULONG	ll_Port0State;			//State information on Port 0.  Used in type determination.
    ULONG	ll_Port1State;			//State information on Port 1.  Used in type determination.
    APTR	ll_SegList;			//Segment Pointer for the game library.
    APTR	ll_EasyRequest;			//Original LVO for Intuition EasyRequest.
    struct Library	*ll_ExecBase;		//Pointer to exec.library.
    struct Library	*ll_UtilBase;		//Pointer to utility.library.
    struct Library	*ll_NVBase;		//Pointer to nonvolatile.library.
    struct Task	*ll_SystemOwner;		//Pointer to the task that has taken over part of the system via SystemControlA.
    struct MsgPort	*ll_WaitPort;		//Pointer to port used in shutting down the startup animation.
    struct Device	*ll_TimerDevice;	//Pointer to timer.device.
    struct Device	*ll_KBDevice;		//Pointer to keyboard.device.
    struct Interrupt	*ll_KBOrig;		//Pointer to the original keyboard interrupt.
    struct Interrupt	*ll_RJP_Saved_IS;	//Pointer to the original gameport interrrupt.
    struct Interrupt	*ll_VBlankInt;		//Pointer to the user's VBlank interrupt.
    struct llct_CIATimer	ll_CIATimer;	//Structure used for allocating a CIA timer interrupt.
    struct MinList	ll_KBInterrupts;	//List of extended keyboard interrupts.
    struct SignalSemaphore	ll_RJP_VBlank;	//Interrupt structure for our replacement gameport interrupt. (word sized)
    struct Interrupt	ll_KBExtend;		//Interrupt structure for our replacment keyboard interrupt.  (word sized)
};

#define LOWLEVELNAME	"lowlevel.library"

#endif
