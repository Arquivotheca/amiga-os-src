
	TTL '$Id: begin.asm,v 1.11 93/10/01 10:07:00 jjszucs Exp $'

*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	         SERIAL DEVICE DRIVER            **
*	   **	   --------------------------------	 **
*	   **						 **
*	   ************************************************


**********************************************************************
*								     *
*   Copyright 1984,1985,1989 Commodore Amiga Inc.All rights reserved.*
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*								     *
**********************************************************************


**********************************************************************
*
*   Source Control:
*
*	$Id: begin.asm,v 1.11 93/10/01 10:07:00 jjszucs Exp $
*
*	$Locker:  $
*
*	$Log:	begin.asm,v $
* Revision 1.11  93/10/01  10:07:00  jjszucs
* Calls DISABLE and ENABLE macros with ExecBase in A6 to ensure
* reliable disabling/enabling of interrupts.
* 
* Revision 1.10  91/05/30  21:21:28  bryce
* The stop-bit mask calulation (sDoMask) was calculated before the
* parity is set up.  This resulted in parity changes taking effect
* on the _second_ open of the serial.device (affecting initial open,
* and open after use of preferences).  Old bug, spagehtti code.
*
* Revision 1.9  91/04/03  22:26:22  bryce
* Support MARK & SPACE parity from preferences.  Turns out the that in
* addition to no support for MARK & SPACE, the code was real damaged.
* A previous program could force M&S on a later running program.  Aghghg!
* What crummy, broken, damaged code!
*
* Revision 1.8  91/01/12  20:38:41  bryce
* #@&!~($%! RCS 4.0 switch.   Change Header to Id
*
* Revision 1.7  90/12/01  20:06:21  bryce
* Fix use of A1 after call to OpenDevice()
*
* Revision 1.6  89/04/19  18:47:43  bryce
* Fix some autodocs & short branches
*
* Revision 1.5  89/04/12  13:51:55  bryce
* V1.31 release, with Y2 and Ariadne software fixes
*
* Revision 1.4  89/03/08  22:20:44  bryce
* V1.3.1 release.  Trash IO_DEVICE on close (it's no longer valid)
*
* Revision 1.3  89/02/07  15:57:11  bryce
* V1.3 maintainance release
* Restrict unit number on open
* fix bug where AllocMem() in open would cause Expunge() of self
* Try (and fail) to fix resource hogging bug
*
* Revision 1.2  89/01/24  21:21:24  bryce
* Disallow opens of unit numbers >1
* Fix a bug where the device could be expunged DURING an open!
*
* Revision 33.5  86/05/30  22:58:42  barry
* The device now restores the DDR and data registers correctly
*
* Revision 33.4  86/05/09  02:17:16  barry
* corrections to auto-docs.
*
* Revision 33.3  86/05/04  10:53:19  barry
* more changes to auto-docs
*
* Revision 33.2  86/05/02  13:42:07  barry
* small changes to auto-docs
*
* Revision 33.1  86/04/29  14:18:56  barry
* Initial version under RCS
*
*
*	Revision 1.3  85/04/24  12:46:01  tomp
*	*** empty log message ***
*
*	Revision 1.2  85/04/24  12:39:14  tomp
*	fix init to restore exec ptr .
*
*	Revision 1.1  85/04/23  20:28:04  tomp
*	Initial revision
*
*
**********************************************************************

    SECTION     serial.device

    NOLIST

****** Included Files ***********************************************

    INCLUDE	'exec/types.i'
    INCLUDE	'exec/nodes.i'
    INCLUDE	'exec/lists.i'
    INCLUDE	'exec/strings.i'
    INCLUDE	'exec/memory.i'
    INCLUDE	'exec/interrupts.i'
    INCLUDE	'exec/ports.i'
    INCLUDE	'exec/libraries.i'
    INCLUDE	'exec/devices.i'
    INCLUDE	'exec/resident.i'
    INCLUDE	'exec/initializers.i'
    INCLUDE	'exec/io.i'
    INCLUDE	'exec/tasks.i'
    INCLUDE	'exec/execbase.i'
    INCLUDE	'exec/ables.i'
    INCLUDE	'exec/errors.i'
    INCLUDE     'hardware/intbits.i'
    INCLUDE     'hardware/cia.i'
    INCLUDE     'devices/timer.i'
    INCLUDE     'resources/misc.i'
    INCLUDE     'intuition/intuition.i'

    INCLUDE     'assembly.i'
    INCLUDE     'serialdev.i'

    LIST

****** Imported Globals *********************************************

    TASK_ABLES

    XREF       	_serper
    XREF       	_serdatr
    XREF       	_serdat
    XREF       	_intena
    XREF       	_intreq
    XREF	_adkcon

    XREF        cmdReset
    XREF        cmdBeginRead
    XREF        cmdBeginWrite
    XREF        cmdBeginClear
    XREF        cmdStop
    XREF        cmdStart
    XREF        cmdBeginFlush
    XREF        cmdQuery
    XREF        cmdBreak
    XREF        cmdBeginSetP
    XREF        sCalcBaud
    XREF        sDoMasks
    XREF        _ciab

****** Imported Functions *******************************************

    EXTERN_SYS AllocMem
    EXTERN_SYS OpenLibrary
    EXTERN_SYS OpenDevice
*    EXTERN_SYS OpenResource
    EXTERN_SYS CloseDevice
    EXTERN_SYS CloseLibrary
    EXTERN_SYS FreeMem
    EXTERN_SYS SetIntVector
    EXTERN_SYS RemPort
    EXTERN_SYS ReplyMsg
    EXTERN_SYS GetPrefs
    EXTERN_SYS AbortIO

****** Exported *****************************************************

    XDEF        sBIODone
    XDEF        serName
    XDEF        miscName
    XDEF        serOpen
    XDEF        serAbortIO
    XDEF        sDoAbort
    XDEF        sAbortTimer
    XDEF        GetBuf
    XDEF        functions

**********************************************************************
*
*			Device Initial Data
*
**********************************************************************

*------ Device Name ----------------------------------------------

serName:       dc.b 'serial.device',0
timerName:     dc.b 'timer.device',0
intuiName:     dc.b 'intuition.library',0
miscName:      dc.b 'misc.resource',0
		CNOP 0,2


*****************************************************************
*
*			Device Functions
*
*****************************************************************

DEFFUNC	    MACRO
	    DC.W    \1-functions
	    ENDM

functions:
	    DC.W    -1
	    DEFFUNC serOpen
	    DEFFUNC serClose
	    DEFFUNC serExpunge
	    DEFFUNC serNOP
	    DEFFUNC serBeginIO
	    DEFFUNC serAbortIO
	    DC.W    -1

serNOP:
	    RTS
*
******* serial.device/OpenDevice ***********************************
*
*   NAME
*       OpenDevice -- Request an opening of the serial device.
*
*   SYNOPSIS
*       error = OpenDevice("serial.device", unit, ioRequest, flags)
*       D0                  A0              D0    A1         D1
*
*	BYTE OpenDevice(STRPTR, ULONG, struct IOExtSer *, ULONG);
*
*   FUNCTION
*       This is an exec call.  Exec will search for the serial.device, and
*       if found, will pass this call on to the device.
*
*       Unless the shared-access bit (bit 5 of io_SerFlags) is set,
*       exclusive use is granted and no other access to that unit is
*       allowed until the owner closes it.  All the serial-specific fields
*       in the ioRequest are initialized to their most recent values (or
*       the Preferences default, for the first time open).
*
*       If support of 7-wire handshaking (i.e. RS232-C CTS/RTS protocol)
*       is required, use the serial.device/SDCMD_SETPARAMS command.
*	This feature should also be specified at inital OpenDevice() time.
*
*   INPUTS
*       "serial.device"	- pointer to literal string "serial.device"
*       unit      	- Must be zero, or a user setable unit number.
*			  (This field is used by multiple port controllers)
*                         Zero specifies the default serial port.
*       ioRequest 	- pointer to an ioRequest block of size io_ExtSerSize
*                         to be initialized by the serial.device.
*                   	  (see devices/serial.h for the definition)
*                     	  NOTE use of io_SerFlags (see FUNCTION above)
*		IMPORTANT: The ioRequest block MUST be of size io_ExtSerSize,
*			   and zeroed (with the exeptions as noted)!
*       flags     	- Must be zero for future compatibility
*
*   RESULTS
*       D0 	  - same as io_Error
*       io_Error  - If the Open succeded, then io_Error will be null.
*           	    If the Open failed, then io_Error will be non-zero.
*       io_Device - A pointer to whatever device will handle the calls
*                   for this unit.  This pointer may be different depending
*                   on what unit is requested.
*
*   BUGS
*	If 7-wire handshaking is specified, a timeout "feature" is enabled.
*	If the device holds off the computer for more than about 30-60
*	seconds, the device will return the write request with the error
*	SerErr_TimerErr.  Don't depend on this, however.  If you want a timeout,
*	set up the timer.device and wait for either timer, or serial IO to
*	complete.
*
*	On open, the serial.device allocates the misc.resource for the
*	serial port.  It does not return it until the serial.device is
*	expunged from memory.  It should return it when no more openers
*	exist.   This code can force a specified device to try and
*	expunge.  Of course, if the device is in use nothing will happen:
*
*	#include "exec/types.h"
*	#include "exec/execbase.h"
*	#include "proto/exec.h"
*
*	void FlushDevice(char *);
*	extern struct ExecBase *SysBase;
*
*	void main()
*	{
*		FlushDevice("serial.device");	/* or parallel.device */
*	}
*
*	/*
*	 * Attempts to flush the named device out of memory.
*	 * If it fails, no status is returned; examination of
*	 * the problem will reveal that information has no
*	 * valid use after the Permit().
*	 */
*	void FlushDevice(name)
*	char  *name;
*	{
*	struct Device *result;
*
*	    Forbid();
*	    if( result=(struct Device *)FindName(&SysBase->DeviceList,name) )
*		RemDevice(result);
*	    Permit();
*	}
*
*
*   SEE ALSO
*	serial.device/CloseDevice
*	serial.device/SDCMD_SETPARAMS
*	devices/serial.h
*
************************************************************************
*
serOpen:
            ; OPEN
*
            CLR.B     IO_ERROR(A1)
            MOVEQ.L   #1,D1
	    CMP.L     D1,D0		;:bryce:Test unit number
            BHI.S     sNoOpen
            TST.W     LIB_OPENCNT(A6)
            BEQ.S     serBufPrep	;First open...
            BTST.B    #SERB_SHARED,UNSER_FLAGS(A6)
            BEQ.S     sNoOpen
            BTST.B    #SERB_SHARED,IO_SERFLAGS(A1)
            BNE       sOK10		;Subsequent opens...
sNoOpen:
            MOVE.B    #SerErr_DevBusy,IO_ERROR(A1)
            RTS


;-----------------The first open comes to here--------------------------------

serBufPrep:
          MOVEM.L   A3/D2,-(SP)

            CLR.B     UNSER_FLAG2(A6)
	    CLR.B     UNSER_STATUS(A6)
	    MOVE.W    #SER_TIMEOUT,UNSER_TIMEOUT(A6) ;init CTS/RTS timeout
            LEA       serReadQueue(A6),A0
            NEWLIST   A0
            LEA       serWriteQueue(A6),A0
            NEWLIST   A0

;----start of get perferences--------------------------------
;BUG-bryce:Some poor assumptions are made here...
;:KLUDGE:bryce/bart:If AllocMem causes an expunge, we loop back into our
;own expunge code, and crash!!!!
;:FIXED:bryce:If alloc fails, Intuition is left open

            MOVE.L    A1,-(SP)
             MOVE.L    #$100,D0
             MOVEQ     #MEMF_PUBLIC,D1       ; get nice no-bullshit memory
	     ADDQ.W    #1,LIB_OPENCNT(A6)
             LINKLIB   _LVOAllocMem,UNSER_EXLIB(A6)
	     SUBQ.W    #1,LIB_OPENCNT(A6)
             MOVE.L    D0,A0
             MOVE.L    A0,D2
            MOVE.L    (SP)+,A1
	    TST.L     D0
            BEQ       sBBadOpen

            MOVE.L    A1,-(SP)
             MOVEQ     #33,D0
             LEA       intuiName(PC),A1
             LINKLIB   _LVOOpenLibrary,UNSER_EXLIB(A6) ; open Intuition
             MOVE.L    D0,A3
             MOVE.L    #$C0,D0
             MOVE.L    D2,A0
             LINKLIB   _LVOGetPrefs,A3      ; if opened, get Pref baud offset
             MOVE.L    A3,A1
             LINKLIB   _LVOCloseLibrary,UNSER_EXLIB(A6)
            MOVE.L    (SP)+,A1
        ;[d2-pointer to prefs]

;------------------Deal with preferences
;[A3 will hold pointer to prefs]

            MOVE.L    D2,A3
            MOVE.W    pf_BaudRate(A3),D0
            LSL.W     #1,D0
	    LEA	      sBaudTable(PC),A0
            MOVE.W    0(A0,D0.W),UNSER_BAUD+2(A6) ; convert offset to baud
            CLR.W     UNSER_BAUD(A6)
*  Install other pref's here
            MOVE.B    #8,UNSER_WWLCNT(A6)   ; pref write bits
            MOVE.B    pf_SerRWBits(A3),D0
            BTST.B    #0,D0
            BEQ.S     sDoPrefs10
                MOVE.B    #7,UNSER_WWLCNT(A6)
sDoPrefs10:
            MOVE.B    #8,D1
            LSR.B     #4,D0
            SUB.B     D0,D1
            MOVE.B    D1,UNSER_RWLCNT(A6)    ; pref read bits
            MOVE.B    #1,UNSER_SBLCNT(A6)
            MOVE.B    pf_SerStopBuf(A3),D0     ; pref stop bits
            BTST.B    #4,D0
            BEQ.S     sDoPrefs20
                MOVE.B    #2,UNSER_SBLCNT(A6)
sDoPrefs20:
            MOVE.B    UNSER_WWLCNT(A6),IO_WRITELEN(A1)
            MOVE.B    UNSER_RWLCNT(A6),IO_READLEN(A1)
            MOVE.B    UNSER_SBLCNT(A6),IO_STOPBITS(A1)
;            BSR       sDoMasks		(this was the wrong place... -Bryce)

* setup bufsize, parity, hshake
            MOVE.B    pf_SerStopBuf(A3),D0
            AND.W     #$000F,D0
            LSL.W     #1,D0
            LEA       sBufTable(PC),A0
            CLR.L     UNSER_RBUFLEN(A6)
            MOVE.W    0(A0,D0.W),UNSER_RBUFLEN+2(A6) ; convert to bufsize


            MOVE.B    pf_SerParShk(A3),D0      ; D0 for parity...
            MOVE.B    D0,D1                    ; D1 for handshake...


            BCLR.B    #SERB_PARTY_ON,UNSER_FLAGS(A6)
            BCLR.B    #SEXTB_MSPON,UNSER_EXTFLAGS+3(A6)
            LSR.B     #4,D0		;0-NONE 1-EVEN 2-ODD 3-MARK 4-SPACE
            BEQ.S     no_parity
                BSET.B    #SERB_PARTY_ON,UNSER_FLAGS(A6)

		SUBQ.B	  #1,D0
		BNE.S     party_not_even
                BCLR.B    #SERB_PARTY_ODD,UNSER_FLAGS(A6)	;EVEN

party_not_even:	SUBQ.B	  #1,D0
		BNE.S     party_not_odd
                BSET.B    #SERB_PARTY_ODD,UNSER_FLAGS(A6)	;ODD

party_not_odd:	SUBQ.B	  #1,D0
		BNE.S     party_not_mark
		BSET.B    #SEXTB_MSPON,UNSER_EXTFLAGS+3(A6)	;MARK
		BSET.B    #SEXTB_MARK,UNSER_EXTFLAGS+3(A6)

party_not_mark:	SUBQ.B	  #1,D0
		BNE.S     party_not_space
		BSET.B    #SEXTB_MSPON,UNSER_EXTFLAGS+3(A6)	;SPACE
		BCLR.B    #SEXTB_MARK,UNSER_EXTFLAGS+3(A6)
party_not_space:
no_parity:


            AND.B     #$0F,D1		;0-XON, 1-RTS, 2-NONE
            BEQ.S     sDoPrefs35
                BSET.B    #SERB_XDISABLED,UNSER_FLAGS(A6)
                CMP.B     #SHSHAKE_RTS,D1
                BNE.S     sDoPrefs37
                BSET.B    #SERB_7WIRE,UNSER_FLAGS(A6)
                BRA.S     sBP3
sDoPrefs35:	BCLR.B    #SERB_XDISABLED,UNSER_FLAGS(A6)
sDoPrefs37:	BCLR.B    #SERB_7WIRE,UNSER_FLAGS(A6)
sBP3:


            BSR       sCalcBaud
            BSR       sDoMasks


            MOVE.L    UNSER_RBUFLEN(A6),D2  ; get latest read buffer size
            CLR.L     D0
            MOVE.L    A3,D1
            BEQ.S     sBP7
		;[A3-prefs buffer]
                MOVE.L    #$100,D0 ;Trick-use this to free the preferences buffer
sBP7:
            BSR       GetBuf                ; setup read buffer
            TST.L     D0
            BNE.S     sBOpenTimer

;FIXED-bryce:Good comment on this line!  If there was an error, this used to
;bomb out :-).  It probably still is not quite correct.
;           BEQ       sOpenExit             ; if err, bomb out


sBBadOpen:
            MOVE.B    #SerErr_BufErr,IO_ERROR(A1)
            MOVEM.L   (SP)+,A3/D2
            MOVE.L    A6,D0
            RTS



;---preferences and the buffer have been allocated.

sBOpenTimer:                                 ; OPEN TIMER subroutine
            MOVE.L    A1,-(SP)
            LEA       IORBSTV(A6),A1
            LEA       timerName(PC),A0
            MOVEQ.L   #UNIT_VBLANK,D0
            CLR.L     D1
            LINKLIB   _LVOOpenDevice,UNSER_EXLIB(A6)
            TST.L     D0
            BEQ.S     sOK3
            MOVE.L    (SP)+,A1
            MOVE.B    #SerErr_TimerErr,IO_ERROR(A1)
            BRA.S     sOK5
sOK3:
	    ;This used to depend on the returned value of A1.  Bad!
	    ;UNSER_TIMER is never used, by the way.
            MOVE.L    IORBSTV+IO_DEVICE(A6),UNSER_TIMER(A6)
            MOVE.L    IORBSTV+IO_DEVICE(A6),IOWBSTV+IO_DEVICE(A6)
            MOVE.L    IORBSTV+IO_UNIT(A6),IOWBSTV+IO_UNIT(A6)
            MOVE.L    (SP)+,A1
sOK5:
            MOVEM.L   (SP)+,A3/D2
            TST.B     IO_ERROR(A1)
            BNE       sCloseDNB                    ; close devices and buffer
            MOVE.W    #(INTF_RBF),_intreq          ; clear old read int rqsts
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intreq ; always a pending write
            MOVE.W    #(INTF_SETCLR!INTF_RBF),_intena ; I'll read anything now...
            BTST.B    #SERB_SHARED,IO_SERFLAGS(A1)
            BEQ.S     sOK10
            BSET.B    #SERB_SHARED,UNSER_FLAGS(A6)


;-------------Shared reopens come here

sOK10:
            MOVE.B    UNSER_FLAGS(A6),IO_SERFLAGS(A1)
	    ;UNSER_EXTFLAGS is handled below
            TST.B     IO_ERROR(A1)
            BNE.S     sOK15
	    ADDQ.W    #1,LIB_OPENCNT(A6)

*           ; ASSERT RTS, DTR. Setup CTS and DSR as input.

;:FIXED:bryce-Y2 computing bug. The DISABLE macro expects ExecBase in A6, or
;as the first argument.
            move.l	UNSER_EXLIB(A6),a0
	    DISABLE	a0

            MOVE.B    _ciab+ciaddra,UNSER_DDRA(A6)
            AND.B     #((~(SerRegBits))&$FF),_ciab+ciaddra
            MOVE.B    _ciab+ciapra,UNSER_PRA(A6) ; read 8 bits, mask later
            OR.B      #(CIAF_COMDTR+CIAF_COMRTS),_ciab+ciaddra
            AND.B     #((~(CIAF_COMRTS+CIAF_COMDTR+CIAF_COMCD))&($FF)),_ciab+ciapra
            AND.B     #((~(CIAF_COMCTS+CIAF_COMDSR+CIAF_COMCD))&($FF)),_ciab+ciaddra
	    ENABLE	a0
*                         ; Set CTS and DSR to input
*
sOK15:
            MOVE.L    UNSER_XON(A6),IO_CTLCHAR(A1) ; show user control char's
	    MOVE.L    UNSER_RBUFLEN(A6),IO_RBUFLEN(A1)
	    CLR.W     IO_STATUS(A1)
	    MOVE.L    UNSER_RWLCNT(A6),IO_READLEN(A1) ;length read,write,stop
	    MOVE.L    UNSER_BAUD(A6),IO_BAUD(A1)       ; show real baud rate
	    MOVE.L    UNSER_BRKTIME(A6),IO_BRKTIME(A1) ; break duration
	    MOVE.L    UNSER_EXTFLAGS(A6),IO_EXTFLAGS(A1)
            MOVE.L  TERMARRAY_0+UNSER_TARRAY(A6),TERMARRAY_0+IO_TERMARRAY(A1)
            MOVE.L  TERMARRAY_1+UNSER_TARRAY(A6),TERMARRAY_1+IO_TERMARRAY(A1)
sOpenExit:
            RTS
*
* D0 - old buffer length
* A3 - old buffer pointer
* D2 - requested buffer length
* Returns new buffer pointer
*
;FIXED-used to loose the buffer every so often.
GetBuf:          ; gets a buffer for serPort reads, frees old buffer
*
            MOVEM.L   A1/A3/D2,-(SP)
;FIXED-bryce:Assumed that a move to an address register sets the CC's (again!)
	    MOVE.L    A3,D1
            BEQ.S     GB10     ; if no current buffer, skip FreeMem routine
            TST.L     D0
            BEQ.S     GB10
            MOVE.L    A3,A1
            LINKLIB   _LVOFreeMem,UNSER_EXLIB(A6) ; else free up current bfr
GB10:
            MOVE.L    D2,D0                 ; new bufsize in bytes
            BEQ.S     GB20
            MOVEQ     #MEMF_PUBLIC,D1       ; get nice no-bullshit memory
	    ADDQ.W    #1,LIB_OPENCNT(A6)
            LINKLIB   _LVOAllocMem,UNSER_EXLIB(A6)
	    SUBQ.W    #1,LIB_OPENCNT(A6)
            TST.L     D0
            BEQ.S     GB20
            MOVE.L    D2,UNSER_RBUFLEN(A6)
GB20:
            MOVE.L    D0,UNSER_RBUF(A6)
            MOVE.L    D0,UNSER_RBUFGET(A6)
            MOVE.L    D0,UNSER_RBUFPUT(A6)
            MOVE.L    D0,UNSER_RBUFMAX(A6)
            ADD.L     D2,UNSER_RBUFMAX(A6)
            CLR.L     UNSER_RBUFCNT(A6)
            CLR.L     UNSER_RBUFOD(A6)
            LSR.L     #1,D2
            MOVE.L    D2,UNSER_XONTHRESH(A6) ; xON's when buffer half empty
            LSR.L     #1,D2
            MOVE.L    D2,UNSER_XOFFTHRESH(A6) ; xOFF's when buffer 1/4 empty
            MOVEM.L   (SP)+,A1/A3/D2
            TST.L     D0
            RTS
*
sBaudTable:
	    DC.W      112
	    DC.W      300
	    DC.W      1200
	    DC.W      2400
	    DC.W      4800
	    DC.W      9600
	    DC.W      19200
	    DC.W      31250
*
sBufTable:
	    DC.W      512
	    DC.W      1024
	    DC.W      2048
	    DC.W      4096
	    DC.W      8000
	    DC.W      16000
*
******* serial.device/CloseDevice *********************************
*
*   NAME
*       CloseDevice -- close the serial port
*
*   SYNOPSIS
*       CloseDevice(deviceNode)
*                    A1
*   FUNCTION
*       This is an exec call that terminates communication with the
*       serial device.  Upon closing, the device's input buffer is freed.
*
*       Note that all IORequests MUST be complete before closing.
*	If any are pending, your program must AbortIO() then WaitIO()
*       to complete them.
*
*   INPUTS
*       deviceNode - pointer to the device node, set by Open
*
*   SEE ALSO
*       serial.device/OpenDevice
*
************************************************************************
*

*   Entry conditions:
*
*       A1          -   IOExtSer
*       A6          -   device base

serClose:           ; CLOSE
*
            MOVEQ     #0,D0
            MOVE.L    D0,IO_DEVICE(A1)	;Trash IO_DEVICE
            MOVE.B    D0,IO_ERROR(A1)
	    TST.W     LIB_OPENCNT(A6)
	    BEQ       serCExit
            SUBQ.W    #1,LIB_OPENCNT(A6)
            BNE       serCExit        ; if last one out...
*                                     ; ... close allocated devices and buffers
sCloseDNB:
            CLR.L     UNSER_WCURIOR(A6)
            CLR.L     UNSER_RCURIOR(A6)
            CLR.L     UNSER_RQCNT(A6)  ; NOTE that this longword clears
*                                      ; BOTH read and write queues. CAREFUL!
*                     ; Restore serial regs to original state

;replaced the commented code so that the non-serial bits don't get hit
;when ciaddra and ciapra are restored to the old settings.
;Barry..... 30-May-1986		report B2793.

;Modified DISABLE call to use ExecBase in A0
;jjszucs    1 Oct 1993
        MOVE.L      	UNSER_EXLIB(A6),a0  ;ExecBase -> A0
	    DISABLE         A0                  ;Disable interrupts (using A0 as ExecBase)

	    MOVE.B	_ciab+ciaddra,D0	; read contents of DDR
	    AND.B	#$07,D0			; Don't change these bits
	    ORI.B	#SerRegBits,D0		; Do change these bits
	    MOVE.B	D0,_ciab+ciaddra	; set the DDR

	    MOVE.B	_ciab+ciapra,D0		; read data register
	    MOVE.B	UNSER_PRA(A6),D1	; read old PRA
	    AND.B	#SerRegBits,D1		; Use these bits, only.
	    AND.B	#$07,D0			; Keep these bits
	    OR.B	D1,D0			; merge old & new bits
	    MOVE.B	D0,_ciab+ciapra		; restore the bits

	    MOVE.B	_ciab+ciaddra,D0	; read contents of DDR
	    AND.B	#$07,D0			; Don't change these bits
	    MOVE.B	UNSER_DDRA(A6),D1	; read old DDR settings
	    AND.B	#SerRegBits,D1		; Only use 'serial' bits
	    OR.B	D1,D0			; merge old & new bits
	    MOVE.B	D0,_ciab+ciaddra	; restore 'serial' bits in DDR

;            AND.B     #SerRegBits,_ciab+ciaddra ; write only serial's bits
;            MOVE.B    UNSER_PRA(A6),D0          ; Of _prae bits to restore,
;            AND.B     #SerRegBits,D0            ;  write only serial's ones
;            AND.B     D0,_ciab+ciapra

;            MOVE.B    UNSER_DDRA(A6),D0         ; restore data direction
;            AND.B     #SerRegBits,D0            ;  of all serial's bits
;            AND.B     D0,_ciab+ciaddra           ; write now.

;Modified ENABLE call to use ExecBase in A0
;jjszucs    1 Oct 1993
        MOVE.L          UNSER_EXLIB(A6),A0  ;ExecBase -> A0
	    ENABLE          A0                  ;Enable interrupts (using A0 as ExecBase)

; This is the end of the restoration changes. Barry 30-may-1986

*
            CLR.W     LIB_OPENCNT(A6)
            LEA       IOWBSTV(A6),A0  ; abort any unfinished ...
            BSR       sAbortTimer
            LEA       IORBSTV(A6),A0  ;  ... timer requests
            BSR       sAbortTimer
            MOVE.L    A1,-(SP)
            LEA       IORBSTV(A6),A1  ;  close brk's MsgPorts
            LINKLIB   _LVOCloseDevice,UNSER_EXLIB(A6)
            MOVE.L    (SP)+,A1
            BCLR.B    #SERB_SHARED,UNSER_FLAGS(A6)
            MOVEM.L   A1/A3/D2,-(SP)
            MOVE.L    UNSER_RBUFLEN(A6),D0 ; store user's latest len's
            CLR.L     D2                   ; don't need new ones
            MOVE.L    UNSER_RBUF(A6),A3
            BSR       GetBuf               ; free up read and write buffers
            MOVEM.L   (SP)+,A1/A3/D2
            MOVE.W    #(INTF_TBE!INTF_RBF),_intena ; disable serial intrpts
            BTST.B    #SERB_EXPUNGE,UNSER_FLAG2(A6)
            BEQ.S     serCExit
            JSR       LIB_EXPUNGE(A6)      ; if rqsted, expunge device
serCExit:
            RTS
*
*
*
******* serial.device/BeginIO **************************************
*
*   NAME
*       BeginIO(ioRequest),deviceNode -- start up an I/O process
*                A1        A6
*   FUNCTION
*       This is a direct function call to the device.  It is intended for
*       more advanced programmers.  See exec's DoIO() and SendIO() for
*       the normal method of calling devices.
*
*       This function initiates a I/O request made to the serial
*       device. Other than read or write, the functions are performed
*       synchronously, and do not depend on any interrupt handling
*       logic (or it's associated discontinuities), and hence should
*       be performed as IO_QUICK.
*       With some exceptions, reads and writes are merely initiated by
*       BeginIO, and thusly return to the caller as begun, not completed.
*       Completion is signalled via the standard ReplyMsg routine.
*       Multiple requests are handled via FIFO queueing.
*       One exception to this non-QUICK handling of reads and writes
*       is for READS when:
*         - IO_QUICK bit is set
*         - There are no pending read requests
*         - There is already enough data in the input buffer to satisfy
*           this I/O Request immediately.
*       In this case, the IO_QUICK flag is not cleared, and the request
*       is completed by the time it returns to the caller. There is no
*       ReplyMsg or signal bit activity in this case.
*
*   INPUTS
*       ioRequest  -- pointer to an I/O Request Block of size
*           io_ExtSerSize (see serial.i for size/definition),
*           containing a valid command in io_Command to process,
*           as well as the command's other required parameters.
*       deviceNode -- pointer to the "serial.device", as found in
*           the IO_DEVICE of the ioRequest.
*
*   RESULTS
*       io_Error -- if the BeginIO succeded, then Error will be null.
*           If the BeginIO failed, then the Error will be non-zero.
*           I/O errors won't be reported until the io completes.
*
*   SEE ALSO
*	devices/serial.h
*
************************************************************************
*
serBeginIO:         ; BEGIN I/O
*
	    MOVE.B    #NT_MESSAGE,LN_TYPE(A1)  ; during request processing.
            ANDI.B    #~(IOF_ACTIVE!IOF_QUEUED!IOF_ABORT),IO_FLAGS(A1)
	    MOVE.W    IO_COMMAND(A1),D0
	    CMP.W     #SER_DEVFINISH,D0
	    BHI.S     badCmd
            CLR.B     IO_ERROR(A1)
	    LSL.W     #2,D0
	    LEA	      cmdTable(PC),A0
	    MOVE.L    0(A0,D0.W),A0
	    JMP	      (A0)
sBIODone:
            MOVE.W    #(IOF_QUICK!IOF_ACTIVE!IOF_QUEUED),D1
            AND.B     IO_FLAGS(A1),D1         ; if quick,or active I/O,
            BNE.S     sBIOExit                ;  exit as is.
            LINKLIB   _LVOReplyMsg,UNSER_EXLIB(A6) ; not qik,send RMsg to caller
sBIOExit:
            RTS
***

cmdTable:
	    DC.L      badCmd
	    DC.L      cmdReset
	    DC.L      cmdBeginRead
	    DC.L      cmdBeginWrite
	    DC.L      badCmd                      ; UPDATE not supported
	    DC.L      cmdBeginClear
	    DC.L      cmdStop
	    DC.L      cmdStart
	    DC.L      cmdBeginFlush
	    DC.L      cmdQuery
	    DC.L      cmdBreak
	    DC.L      cmdBeginSetP
badCmd:
	    MOVE.B    #IOERR_NOCMD,IO_ERROR(A1)
            BRA       sBIODone
*
******* serial.device/AbortIO **************************************
*
*   NAME
*       AbortIO(ioRequest) -- abort an I/O request
*               A1
*
*   FUNCTION
*      This is an exec.library call.
*
*      This function attempts to aborts a specified read or write request.
*      If the request is active, it is stopped immediately. If the request is
*      queued, it is painlessly removed.  The request will be returned
*      in the same way any completed request it.
*
*      After AbortIO(), you must generally do a WaitIO().
*
*   INPUTS
*      iORequest  -- pointer to the IORequest Block that is to be aborted.
*
*   RESULTS
*       io_Error -- if the Abort succeded, then io_Error will be #IOERR_ABORTED
*           (-2) and the request will be flagged as aborted (bit 5 of
*           io_Flags is set).  If the Abort failed, then the Error will be zero.
*
*   BUGS
*       Previous to version 34, the serial.device would often hang when
*       aborting CTS/RTS handshake requests.  This was the cause of the
*       incorrect assumption that AbortIO() does not need to be followed
*       by a wait for a reply (or a WaitIO()).
*
************************************************************************
*
*
serAbortIO:         ; ABORT SPECIFIED I/O OPERATION
*
*******************************************************************

	    BSET.B    #SERB_IRQLOCK,UNSER_FLAG2(A6)
            MOVE.W    #(INTF_RBF!INTF_TBE),_intena
            BSR.S     sDoAbort
            MOVE.W    #(INTF_SETCLR!INTF_RBF!INTF_TBE),_intena
	    BCLR.B    #SERB_IRQLOCK,UNSER_FLAG2(A6)
            RTS
sDoAbort:
            CLR.B     IO_ERROR(A1)
            CMP.W     #CMD_READ,IO_COMMAND(A1)
            BEQ.S     sAbRead
            CMP.W     #CMD_WRITE,IO_COMMAND(A1)
            BEQ       sAbWrite        ; if write ...
            CMP.W     #SDCMD_BREAK,IO_COMMAND(A1)
            BEQ.S     sAbWrite        ;  ... or write break,skip down
            BRA       sAbExit10       ; else, go away.
sAbRead:
            CMPA.L    UNSER_RCURIOR(A6),A1
            BNE.S     sAbRdQd
            BCLR.B    #SERB_RBUSY,UNSER_FLAG2(A6) ; tell int handler current IO
            CLR.L     UNSER_RCURIOR(A6)
            BRA.S     sAbExit         ; is done, deQue next IO_Rqst
sAbRdQd:
            BTST.B    #IOB_QUEUED,IO_FLAGS(A1)
            BEQ       sAbExit10
            MOVE.L    A1,-(SP)
            REMOVE
            MOVE.L    (SP)+,A1
            SUBQ.W    #1,UNSER_RQCNT(A6)
            BRA.S     sAbExit
sAbWrite:
            CMPA.L    UNSER_WCURIOR(A6),A1
            BNE.S     sAbWrQd
            CMP.W     #SDCMD_BREAK,IO_COMMAND(A1)
            BNE.S     sAbWrite5

;---we have been asked to abort a BREAK---
            BCLR.B    #IOSTB_WRITINGBRK,UNSER_STATUS(A6)
            BEQ.S     sAbWrite5
            MOVE.L    #_adkcon,A0
            MOVE.W    #(ADKCONF_UARTBRK),(A0) ; stop break signal
;FIXED-bryce: Used to try and abort the hardware register pointed to by A0
;FIXED-bryce: It forgot to abort CTS/RTS timer requests!
sAbWrite5:
            LEA       IOWBSTV(A6),A0  ; Get write timer IORequest
            BSR.S     sAbortTimer

            CLR.L     UNSER_WCURIOR(A6)
            BCLR.B    #SERB_WEOL,UNSER_FLAG2(A6)
            BRA.S     sAbExit
sAbWrQd:
            BTST.B    #IOB_QUEUED,IO_FLAGS(A1)
            BEQ       sAbExit10
;FIXED-bryce: Old code forgot to save A1 before the REMOVE.  This has the
;effect of aborting the previous IORequest in the list.  The REMOVE macro
;is dangerous like this... it should force the programmer to specify what
;registers to destroy.
	    MOVE.L    A1,-(A7)
            REMOVE
	    MOVE.L    (A7)+,A1
            SUBQ.W    #1,UNSER_WQCNT(A6)
sAbExit:
            BSET.B    #IOB_ABORT,IO_FLAGS(A1)
            AND.B     #(~(IOF_ACTIVE+IOF_QUEUED))&$FF,IO_FLAGS(A1)
            MOVE.B    #IOERR_ABORTED,IO_ERROR(A1)
*            MOVE.W    #(INTF_SETCLR!INTF_RBF!INTF_TBE),_intena ; re-enable
            BTST.B    #IOB_QUICK,IO_FLAGS(A1)
            BNE.S     sAbExit10
            LINKLIB   _LVOReplyMsg,UNSER_EXLIB(A6)
sAbExit10:
            RTS
*
sAbortTimer:
            MOVE.L    A1,-(SP)
            MOVE.L    A0,A1
            LINKLIB   _LVOAbortIO,UNSER_EXLIB(A6)
            MOVE.L    (SP)+,A1
            RTS
*
*****i* serial.device/function/Expunge **************************************
*
*   NAME
*       Expunge -- Free all system resources and dependencies
*
*   FUNCTION
*       This function deallocates all memory and functionality associated
*       with the serial device. This includes the data section for
*       the serial device, the break-related message ports, read and
*       write queues and buffers, and interrupt vector attachments. If
*       the device is currently closed, Expunge takes place immediately.
*       If it is Open, the Expunge cannot take place.
*
*   RESULTS
*       Error -- if the Expunge succeded, then Error will be null.
*           If the Expunge failed, then the Error will be non-zero.
*
************************************************************************
*
*
serExpunge:         ; GIVE UP ALL RESOURCES AND RUN AWAY
*
*******************************************************************
*
            MOVEQ.L   #0,D0
            TST.W     LIB_OPENCNT(A6)
            BEQ.S     sE10
            ;BSET.B    #SERB_EXPUNGE,UNSER_FLAG2(A6)
            RTS
*
sE10:
            MOVE.L    A6,-(SP)
            MOVE.L    UNSER_MISC(A6),A6
            MOVE.L    #MR_SERIALBITS,D0
            JSR       MR_FREEMISCRESOURCE(A6)
            MOVE.L    #MR_SERIALPORT,D0
            JSR       MR_FREEMISCRESOURCE(A6)   ; give up ownership of serial
            MOVE.L    (SP)+,A6                  ;  hardware resources
            MOVE.L    #INTB_RBF,D0
            MOVE.L    UNSER_OLDRBF(A6),A1
            LINKLIB   _LVOSetIntVector,UNSER_EXLIB(A6)
            MOVE.L    #INTB_TBE,D0
            MOVE.L    UNSER_OLDTBE(A6),A1
            LINKLIB   _LVOSetIntVector,UNSER_EXLIB(A6)
            MOVE.L    UNSER_SEGLIST(A6),-(SP)
            LEA       RdBrkMp(A6),A1
            LINKLIB   _LVORemPort,UNSER_EXLIB(A6)
            LEA       WrBrkMp(A6),A1
            LINKLIB   _LVORemPort,UNSER_EXLIB(A6)
;Seeing a REMOVE after a freemem made my teeth hurt!  (We are in a Forbid()
;so it really does not matter).
            MOVE.L    A6,A1
            REMOVE
            MOVE.L    A6,A1
            CLR.L     D0
            MOVE.W    LIB_NEGSIZE(A6),D0
            SUBA.L    D0,A1
            ADD.W     LIB_POSSIZE(A6),D0
            LINKLIB   _LVOFreeMem,UNSER_EXLIB(A6)
            MOVE.L    (SP)+,D0
            RTS
*
    END
