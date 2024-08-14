**********************************************************************
*
*			---------------
*   stddevice.i		RAWINPUT MODULE		standard device defines
*			---------------
*
*   Copyright 1985, 1987, 1991 Commodore-Amiga Inc.
*
*   Source Control	$Locker:  $
*
*   $Log:	stddevice.i,v $
*   Revision 35.6  91/05/03  10:41:19  bryce
*   Shuffle dd_NumCommands.  LIB_SIZE is misaligned.  Putting a word
*   afterword magically aligns dd_ExecBase, etc. etc.
*   
*   Revision 35.5  91/04/09  13:19:41  darren
*   Document that Beginio.asm no longer supports CmdBytes > 0;
*   not needed at this point, and Beginio.asm modified to be faster
*   within QueueCommand() for MIDI performance.
*   
*   Revision 35.4  90/04/13  12:46:11  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 35.3  90/04/02  16:30:25  kodiak
*   back to using dd_ExecBase, not SYSBASE (4)
*   
*   Revision 35.2  90/04/02  13:01:23  kodiak
*   for rcs 4.x header change
*   
*   Revision 35.1  88/08/02  12:24:00  kodiak
*   remove unused dd_ExecBase (using 4 instead)
*   
*   Revision 35.0  87/10/26  11:34:27  kodiak
*   initial from V34, but w/ stripped log
*   
**********************************************************************
	IFND	STDDEVICE_I
STDDEVICE_I	SET	1

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/libraries.i"

*------ IO_FLAGS -----------------------------------------------------
	BITDEF	IO,QUEUED,4	; command is queued to be performed
	BITDEF	IO,CURRENT,5	; command is being performed
	BITDEF	IO,SERVICING,6	; command is being actively performed
	BITDEF	IO,DONE,7	; command is done

*------ du_Flags -----------------------------------------------------
	BITDEF	DU,STOPPED,0	; commands are not to be performed

*------ DeviceUnit ---------------------------------------------------
*	array of current & pending command queues, a command is
*	removed after it is done, it is ACTIVE while in progress unless
*	it is quick.  The queues are message ports.
du_Flags	EQU	LN_PRI	; various unit flags

*------ dd_CmdBytes --------------------------------------------------
*
*   if <  0, this command executes immediately, ignoring STOPPED.
*   if >= 0, this command is queued on the list at
*		*IO_UNIT + ((n-1)*MP_SIZE)
*	if it cannot be satisfied quickly
*
* !!!!NOTE!!!! support for CmdByte greater than 0 removed from
*              Beginio.asm for MIDI performance - not needed
*              at time of change!
*


 STRUCTURE	DeviceData,LIB_SIZE
    UWORD	dd_NumCommands	; the number of commands supported
    APTR	dd_ExecBase	; SysBase
    APTR	dd_Segment	; A0 when initialized
    APTR	dd_CmdVectors	; command table for device commands
    APTR	dd_CmdBytes	; bytes describing which command queue
    LABEL	dd_SIZEOF

	ENDC
