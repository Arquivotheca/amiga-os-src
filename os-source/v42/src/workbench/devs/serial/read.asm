
	TTL '$Id: read.asm,v 1.8 93/10/01 10:07:53 jjszucs Exp $'

*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	         SERIAL DEVICE DRIVER            **
*	   **	   --------------------------------	 **
*	   **						 **
*	   ************************************************


**********************************************************************
*								     *
*   Copyright 1984,85,89  Commodore-Amiga Inc. All rights reserved.  *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated.				     *
*								     *
**********************************************************************


**********************************************************************
*
*   Source Control:
*
*	$Id: read.asm,v 1.8 93/10/01 10:07:53 jjszucs Exp $
*
*	$Locker:  $
*
*	$Log:	read.asm,v $
* Revision 1.8  93/10/01  10:07:53  jjszucs
* BTST.B #8,D0 instructions changed to BTST.L #8,D0 to allow
* native assembly (using HX68).
* 
* Revision 1.7  91/01/12  20:39:10  bryce
* #@&!~($%! RCS 4.0 switch.   Change Header to Id
*
* Revision 1.6  89/04/19  18:49:22  bryce
* Fix some autodocs & short branches
*
* Revision 1.5  89/04/12  13:53:18  bryce
* V1.31 release, with Y2 and Ariadne software fixes
*
* Revision 1.4  89/03/08  22:21:16  bryce
* V1.3.1 release.
*
* Revision 1.2  89/02/07  16:00:07  bryce
* V1.3 maintainance release
* Fix MAJOR bug that's been here forever.  Device would crash
* on mismatched baud rates.
*
* Revision 33.6  86/07/21  17:46:56  andy
* *** empty log message ***
*
* Revision 33.5  86/05/10  17:11:03  barry
* Changed the way that we read data.The data is now read before
* the RBF interrupt request is cleared.
*
* Revision 33.4  86/05/04  10:59:43  barry
* more changes to auto-docs
*
* Revision 33.3  86/05/02  13:43:42  barry
* small changes to auto-docs
*
* Revision 33.2  86/04/29  15:32:35  barry
* Fixed the problem with the read & write pointers getting out of
* line when serial received a break
*
* Revision 33.1  86/04/29  14:19:06  barry
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
    INCLUDE     'exec/nodes.i'
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
    INCLUDE     'hardware/intbits.i'
    INCLUDE     'hardware/cia.i'
    INCLUDE	'hardware/custom.i'
    INCLUDE     'devices/timer.i'

    INCLUDE     'assembly.i'
    INCLUDE     'serialdev.i'

    LIST

****** Imported Globals *********************************************

    XREF       	_serper
    XREF       	_serdatr
    XREF       	_serdat
    XREF       	_intena
    XREF       	_ciab
    XREF       	serAbortIO
    XREF       	sBIODone
    XREF       	sAbortTimer

****** Imported Functions *******************************************

    EXTERN_SYS ReplyMsg
    EXTERN_SYS Cause
    EXTERN_SYS AbortIO

*
****** Exported *****************************************************

    XDEF       	cmdBeginRead
    XDEF       	cmdRead
    XDEF       	CheckRdBrk
    XDEF       	ReadSI
    XDEF       	cRSendXoff
    XDEF       	cRSendXon
    XDEF       	rCzechParty

    XDEF        endMarker

*****************************************************************
*
*			Device Commands
*
*
******* serial.device/CMD_READ *****************************************
*
*   NAME
*       Read -- read input from serial port
*
*   FUNCTION
*	This command causes a stream of characters to be read in from
*	the serial port buffer.  The number of characters is specified
*	in io_Length.
*
*	The Query function can be used to check how many characters
*	are currently waiting in the serial port buffer.  If more characters
*	are requested than are currently available, the ioRequest
*	will be queued until it can be satisfied.
*
*
*	The best way to handle reads is to first Query to get the number
*	of characters currently in the buffer.  Then post a read request
*	for that number of characters (or the maximum size of your buffer).
*
*	If zero characters are in the buffer, post a request
*	for 1 character.  When at least one is ready, the device will return
*	it.  Now start over with another Query.
*
*	Before the program exits, it must be sure to AbortIO() then WaitIO()
*	any outstanding ioRequests.
*
*   IO REQUEST
*       io_Message      A mn_ReplyPort is required
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      CMD_READ
*       io_Flags        If the IOB_QUICK bit is set, read will try
*			to complete the IO quickly
*       io_Length       number of characters to receive.
*       io_Data         pointer to buffer
*
*   RESULTS
*       Error -- if the Read succeded, then io_Error will be null.
*           If the Read failed, then io_Error will be non-zero.
*	     io_Error will indicate problems such as parity mismatch,
*	     break, and buffer overrun.
*
*   SEE ALSO
*	serial.device/SDCMD_QUERY
*	serial.device/SDCMD_SETPARAMS
*
*   BUGS
*	Having multiple outstanding read IORequests at any one time will
*	probably fail.
*
*	Old documentation mentioned a mode where io_Length was set to -1.
*	If you want a NULL terminated read, use the io_TermArray instead.
*
************************************************************************
*
cmdBeginRead:       ; READ
*
*----------------------------------------------------------------
*                                                ; ENTRY from BEGIN I/O
            CLR.L     IO_ACTUAL(A1)
            MOVE.L    IO_LENGTH(A1),D1
            BEQ       sBIODone                    ; if null request, return.
            TST.W     UNSER_RQCNT(A6)             ; leave as QUICK if nothing
            BNE.S     cBR5                        ;  queued ...
            TST.L     UNSER_RCURIOR(A6)           ;  ... or active ...
            BNE.S     cBR5
            CMP.L     UNSER_RBUFCNT(A6),D1        ;  ... and enough already
            BLS.S     cBR10                       ;  read in to fill io_Rqst
cBR5:
            BCLR.B    #IOB_QUICK,IO_FLAGS(A1)     ; flag as not quick I/O
cBR10:
;BUG-bryce:Hard interrupt uses same list... this sould be protected from that
            BSET.B    #IOB_QUEUED,IO_FLAGS(A1)    ; flag as queued
            LEA       serReadQueue(A6),A0
            ADDTAIL
            ADDQ.W    #1,UNSER_RQCNT(A6)
sBRExit:
            LEA       ReadSoftInt(A6),A1
            LINKLIB   _LVOCause,UNSER_EXLIB(A6)       ; deque rqst
            RTS

*** SOFTWARE INTERRUPT HANDLER
ReadSI:
***************************************
*
            MOVEM.L   A6/D2,-(SP)
	    MOVE.L    A1,A6
            MOVE.L    UNSER_RCURIOR(A6),D0
            BNE.S     RSICurRqst
RSIGetIoRqst:
            TST.W     UNSER_RQCNT(A6)
            BLE       RSIExit
            LEA       serReadQueue(A6),A0        ; so, we deQ next read IORqst
            REMHEAD
            MOVE.L    D0,A1
            MOVE.L    A1,UNSER_RCURIOR(A6)        ; store next IORqst address
            SUBQ.W    #1,UNSER_RQCNT(A6)
            CLR.L     UNSER_RLEN(A6)
            BSET.B    #IOB_ACTIVE,IO_FLAGS(A1)    ; flag as active,
            BCLR.B    #IOB_QUEUED,IO_FLAGS(A1)    ;  no longer queued
            MOVE.L    IO_DATA(A1),UNSER_RDATA(A6)
            BSET.B    #SERB_RBUSY,UNSER_FLAG2(A6)
            MOVE.L    IO_LENGTH(A1),D0
            BRA.S     RSICheckBuf
RSICurRqst:
            MOVE.L    D0,A1
RSICheckBuf:
            TST.L     UNSER_RBUFCNT(A6)
            BEQ       RSIExit
            MOVE.L    UNSER_XONTHRESH(A6),D1
            CMP.L     UNSER_RBUFCNT(A6),D1       ; if buffer emptying,
            BLE.S     rB5
	    BTST.B    #SERB_XOFFRSET,UNSER_FLAG2(A6) ; and sender x-OFFed now
            BEQ.S     rB5
            BSR       cRSendXon
rB5:
            MOVE.L    UNSER_RBUFGET(A6),A0
            MOVE.B    (A0)+,D0
            CMP.L     UNSER_RBUFMAX(A6),A0       ; if at end of buffer,
            BLT.S     rB10
            MOVE.L    UNSER_RBUF(A6),A0          ;  wrap-around to beginning
rB10:
            SUBQ.L    #1,UNSER_RBUFCNT(A6)
            MOVE.L    A0,UNSER_RBUFGET(A6)
            SUBQ.L    #1,A0
	    CMPA.L    UNSER_RBUF(A6),A0		 ; Barry... 25-Apr-86 added
	    BNE.B     NoWrap			 ; checking for wrap-around
	    MOVE.L    UNSER_RBUFMAX(A6),A0	 ; on these lines.
NoWrap      CMPA.L    UNSER_RBUFOD(A6),A0
            BEQ       rBOverDose
RSICheckParity:
            BTST.B    #SERB_PARTY_ON,UNSER_FLAGS(A6) ; parity check enabled?
            BEQ.S     RSICheckEof
            BTST.B    #SERB_9BITRD,UNSER_FLAG2(A6)    ; if 9bit, all done.
            BNE.S     RSICheckEof
            BTST.B    #SEXTB_MSPON,UNSER_EXTFLAGS+3(A6)  ; mark-space ?
            BEQ.S     rP5
            MOVE.B    UNSER_RWLCNT(A6),D1
            BCLR.B    D1,D0                            ; check parity bit
            BNE.S     rPGotMark
            BTST.B    #SEXTB_MARK,UNSER_EXTFLAGS+3(A6) ; space ok?
            BNE.S     rPErr
            BRA.S     rPOk
rPGotMark:
            BTST.B    #SEXTB_MARK,UNSER_EXTFLAGS+3(A6) ; mark ok?
            BEQ.S     rPErr
            BRA.S     RSICheckEof
rP5:
            BSR.S     rCzechParty                      ; check parity
            BEQ.S     rPOk
rPErr:
            MOVE.B    #SerErr_ParityErr,IO_ERROR(A1)
            BRA.S     RSIReply
rPOk:
            MOVE.B    UNSER_RWLCNT(A6),D1
            BCLR.L    D1,D0
RSICheckEof:
            MOVE.L    IO_LENGTH(A1),D1
            MOVE.L    UNSER_RDATA(A6),A0
            MOVE.B    D0,(A0)                    ; store next buffered char
            BNE.S     rE10
            TST.L     D1
            BMI.S     RSIReply
rE10:
            ADDQ.L    #1,IO_ACTUAL(A1)
            ADDQ.L    #1,UNSER_RDATA(A6)
            CMP.L     IO_ACTUAL(A1),D1
            BLS.S     RSIReply                   ; enough read ?
rE20:
            BTST.B    #SERB_EOFMODE,IO_SERFLAGS(A1) ;should we check for EOF ?
            BEQ       RSICheckBuf
            LEA       UNSER_TARRAY(A6),A0
            MOVEQ     #TERMARRAY_SIZE-1,D1
rEofNextChar:
            CMP.B     (A0)+,D0                  ; match this EOF char ?
            DBCC      D1,rEofNextChar
            BNE       RSICheckBuf
***
RSIReply:
***    When rqst done:
            CLR.L     UNSER_RCURIOR(A6)
            BCLR.B    #SERB_RBUSY,UNSER_FLAG2(A6) ; else blk done:clear busy
	    BCLR.B    #IOB_ACTIVE,IO_FLAGS(A1)    ; flag IOR as done
            BTST.B    #IOB_QUICK,IO_FLAGS(A1)     ; if quick, skip Reply
            BNE       RSIGetIoRqst
            LINKLIB   _LVOReplyMsg,UNSER_EXLIB(A6) ; not qik,send RMsg to caller
            BRA       RSIGetIoRqst
*
RSIExit:
            MOVEM.L   (SP)+,A6/D2
            RTS

rCzechParty:                ; What ?? ...oh, check odd-even style parity.
            MOVE.B    D0,D1
            ASR.B     #4,D1
            EOR.B     D0,D1
            AND.B     #$0F,D1
            MOVE.L    UNSER_PARITY(A6),D2
            BTST.B    #SERB_PARTY_ODD,UNSER_FLAGS(A6)
            BEQ.S     rCP10
	    NOT.L     D2
rCP10:
            BTST.L    D1,D2
            RTS
*
rBOverDose:      ; encountered fatal error or break in input buffer
            BCLR.B    #IOSTB_READBREAK,UNSER_STATUS(A6) ; was it a break ??
            BEQ.S     rBNotBreak
            MOVE.B    #SerErr_DetectedBreak,IO_ERROR(A1)
            BRA.S     RSIReplyErr
rBNotBreak:
            BCLR.B    #IOSTB_PARITYERR,UNSER_STATUS(A6) ; was it parity ??
            BEQ.S     rBNotPErr
            MOVE.B    #SerErr_ParityErr,IO_ERROR(A1)
            BRA.S     RSIReplyErr
rBNotPErr:
            MOVE.B    #SerErr_BufOverflow,IO_ERROR(A1)  ; buffer overrun ??
            BTST.B    #IOSTB_OVERRUN,UNSER_STATUS(A6)
            BEQ.S     RSIReplyErr
            MOVE.B    #SerErr_LineErr,IO_ERROR(A1)      ; data overrun ??
***
RSIReplyErr:
            CLR.L     UNSER_RBUFOD(A6)        ; found err spot, clear for next
;:bryce-FIXED- was IOSTB_ instead of IOSTF_
;           AND.B     #(~(IOSTB_PARITYERR+IOSTB_OVERRUN))&$FF,UNSER_STATUS(A6)
            AND.B     #(~(IOSTF_PARITYERR+IOSTF_OVERRUN))&$FF,UNSER_STATUS(A6)
            BRA     RSIReply                ; all reset ok, go tell error


*******************************************************************************
cROverrun:                   ; data overrun: flag as such and send x-OFF
            MOVE.W    #(INTF_RBF),intreq(a0)	; clear RBF interrupt req
            BSET.B    #IOSTB_OVERRUN,UNSER_STATUS(A6)
            BRA       cRODErr      ; go flag as input-buf-char-invalid

**********************
	CNOP 0,4 ;ADD-bryce align to longword for 68020/030 folks
cmdRead:                                     ; INTERRUPTS (RBF) join in here
**********************
;FIXED-bryce:Coded to take better advantage of values already in registers
;(It still saves A6 semi-needlesly)

            MOVEM.L   A6/D2,-(SP)
            MOVE.L    A1,A6
            MOVE.W    serdatr(A0),D0              ; finally get to read data
;FIXED-bryce: Due to an extra register write, hardware overrun was never flagged
            BMI.S      cROverrun                ; data overrun
            MOVE.W    #(INTF_RBF),intreq(A0)   ; clear RBF interrupt req
            BTST.B    #SERB_RAD_BOOGIE,UNSER_FLAGS(A6)
	    BNE       cBC		;if high-speed mode,skip down
;BUG-bryce:I don't know how break should be detected... but this is not it.
;FIXED-bryce:First pass at a working break detect
;If we just got a zero, and the incomming serial line is still a zero,
;then there is a good chance this is a break.  Test for it.
            AND.W     #$0FFF,D0
            BEQ       cRGotBreak
            BCLR.B    #IOSTB_READINGBRK,UNSER_STATUS(A6)
            BNE       cRLostBreak
;[back from lostbreak]
cRLBGo:
            AND.W     UNSER_READBITS(A6),D0    ; mask to proper word size
	    BTST.B    #SERB_XDISABLED,UNSER_FLAGS(A6) ; xON/xOFF enabled
            BNE.S     cRBufIt                  ; if not, skip further checks

;------Xon/Xoff checking---------------------------------
            MOVE.B    D0,D1
            BTST.B    #SERB_PARTY_ON,UNSER_FLAGS(A6) ; parity check enabled?
            BEQ.S     cRXT5
              MOVE.B    UNSER_RWLCNT(A6),D2
              BCLR.L    D2,D1                    ; ignore parity for xCTL check
cRXT5:

	    CMP.B     UNSER_XOFF(A6),D1        ; input char = xOFF ?
	    BNE.S     cRXT10                     ; no ?, skip down
              BSET.B    #SERB_XOFFWSET,UNSER_FLAG2(A6) ; flag as Xoffed
              BRA       cRDone10                   ; if x-CTL, don't store.
cRXT10:

	    CMP.B     UNSER_XON(A6),D1           ; input char = xON ?
	    BNE.S     cRBufIt                    ; neither x-ON or OFF
	    BTST.B    #SERB_XOFFWSET,UNSER_FLAG2(A6) ; if not xOffed
	    BEQ       cRDone10                   ; ignore current char
	    BCLR.B    #SERB_XOFFWSET,UNSER_FLAG2(A6) ; else, clear Xoff
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intena ; restart writes
            BRA       cRDone10

;------9 bit handling-----------------------------------
*                            ; NOT a control character
cRBufIt:                     ; got input, put in driver's buffer
            BTST.B    #SERB_9BITRD,UNSER_FLAG2(A6)
            BEQ.S     cBC
            BTST.B    #SEXTB_MSPON,UNSER_EXTFLAGS+3(A6)
            BNE.S     c8MSP
            BSR       rCzechParty                ; check odd-even parity
cRB3:                                            ; shouldn't be set
            BEQ.S     cRB5
;Next instruction changed from BTST.B #8,D0 to BTST.L #8,D0.
;HX68 (correctly) flags testing bit 8 of an 8-bit byte as an error.
;jjszucs    1 Oct 1993
            BTST.L    #8,D0                      ; should be set
            BEQ.S     c8PErr
            BRA.S     cBC
cRB5:                                            ; shouldn't be set
;Next instruction changed from BTST.B #8,D0 to BTST.L #8,D0.
;HX68 (correctly) flags testing bit 8 of an 8-bit byte as an error.
;jjszucs    1 Oct 1993
            BTST.L    #8,D0
            BNE.S     c8PErr
            BRA.S     cBC
c8MSP:                                           ; mark/space parity
            BTST.B    #SEXTB_MARK,UNSER_EXTFLAGS+3(A6)
            BRA.S     cRB3
c8PErr:
            BSET.B    #IOSTB_PARITYERR,UNSER_STATUS(A6)

;-------errors
cRODErr:
            TST.L     UNSER_RBUFOD(A6)           ; If already OD'd,
            BNE.S     cRDone                     ;  don't overwrite previous.
            MOVE.L    UNSER_RBUFPUT(A6),UNSER_RBUFOD(A6) ;last valid data char
            BRA.S     cRDone
;BUG-bryce:What is this doing after a BRA.S ???!!!
;            ADDQ.L    #1,UNSER_RBUFCNT(A6)       ; sumthin to unbuffer
			; 25-Apr-86 Barry... Moved this ADDQ to here from
			;          	     before the TST.L

;--------buffer handling-------------------------------
cBC:
            MOVE.L    UNSER_RBUFLEN(A6),D1
            CMP.L     UNSER_RBUFCNT(A6),D1       ; if full,
            BLE.S     BCFull                     ;  send XOFF, don't store
            MOVE.L    UNSER_RBUFPUT(A6),A0
            MOVE.B    D0,(A0)+                   ; store data char
            MOVE.L    A0,UNSER_RBUFPUT(A6)
            ADDQ.L    #1,UNSER_RBUFCNT(A6)       ; 1 more char in buffer
            CMPA.L    UNSER_RBUFMAX(A6),A0       ; if at end of buffer;
            BLT.S     BC10                       ;  don't store
              MOVE.L    UNSER_RBUF(A6),UNSER_RBUFPUT(A6)
BC10:
            SUB.L     UNSER_XOFFTHRESH(A6),D1
            CMP.L     UNSER_RBUFCNT(A6),D1       ; if not at threshhold,
            BGE.S     cRDone                     ;  exit for next int.
            BRA.S     BC15                       ; else, send XOFF

BCFull:
            TST.L     UNSER_RBUFOD(A6)           ; If already OD'd,
            BNE.S     BC15                       ;  don't overwrite previous.
              MOVE.L    UNSER_RBUFPUT(A6),UNSER_RBUFOD(A6) ; if at very end,flag
BC15:                                      ; this spot as last-valid char.
            BSR.S     cRSendXoff

;---------ping softint---------------------------------------
cRDone:

;---------- check if there are any pending requests before bothering witha
;---------- before starting softint
            TST.L     UNSER_RCURIOR(A6)
            BEQ.S     MayNotNeed
MustDo:
            LEA       ReadSoftInt(A6),A1
            MOVEM.L   (SP)+,A6/D2		;Get registers back now
            JMP       _LVOCause(A6)		;[execbase came from stack]

MayNotNeed:
            TST.W     UNSER_RQCNT(A6)
	    BGT.S     MustDo
;:test
;;bchg.b	#1,$bfe001

cRDone10:
            MOVEM.L   (SP)+,A6/D2
            RTS


***************************************************************************
cRSendXon:
            MOVE.B    UNSER_XON(A6),UNSER_XDATA(A6)  ; restart sender
	    BCLR.B    #SERB_XOFFRSET,UNSER_FLAG2(A6) ; sender x-ON'ed
            BTST.B    #SERB_7WIRE,UNSER_FLAGS(A6)
            BEQ.S     cRSX10
            BCLR.B    #CIAB_COMRTS,_ciab+ciapra      ; hardware xON
            BRA.S     cRSX10
cRSendXoff:
            MOVE.B    UNSER_XOFF(A6),UNSER_XDATA(A6) ; stop data coming in
	    BSET.B    #SERB_XOFFRSET,UNSER_FLAG2(A6) ; sender x-OFF'ed
            BTST.B    #SERB_7WIRE,UNSER_FLAGS(A6)
            BEQ.S     cRSX10
            BSET.B    #CIAB_COMRTS,_ciab+ciapra      ; hardware xOFF
cRSX10:
            BTST.B    #SERB_XDISABLED,UNSER_FLAGS(A6)
            BNE.S     cRSX30
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intena ; enable write of ctl char
            RTS

cRSX30:
            CLR.B     UNSER_XDATA(A6)
            RTS
*
cRGotBreak:         ; all low signal, check if held low for break duration
            BSET.B    #IOSTB_READINGBRK,UNSER_STATUS(A6)
            BNE.S     cRDone10

            LEA       IORBSTV(A6),A1
            CLR.L     IOTV_TIME+TV_SECS(A1)
            MOVE.L    UNSER_BRKTIME(A6),D0
            LSR.L     #1,D0		;Take 1/2 the time
            BNE.S     cRNotZero
            MOVEQ.L   #127,D0		;Too low!
cRNotZero:  MOVE.L    D0,IOTV_TIME+TV_MICRO(A1)
            MOVE.W    #TR_ADDREQUEST,IO_COMMAND(A1)
            LEA.L     IORBSTV(A6),A1
            BEGINIO
            BRA       cRDone10

*
* returned from timed wait, if still break:
* exit flagged as break and cause SoftInt, else ignore
*
CheckRdBrk:
            MOVEM.L   A6/D2,-(SP)
            MOVE.L    A1,A6
            BCLR.B    #IOSTB_READINGBRK,UNSER_STATUS(A6)
            BEQ.S     CRB_exit	;already gone...
            MOVE.W    _serdatr,D0
            AND.W     #$0FFF,D0
            BNE.S     CRB_exit	;no more break here...
            BSET.B    #IOSTB_READBREAK,UNSER_STATUS(A6)
            BRA       cRODErr      ; go flag as input-buf-char-invalid
CRB_exit:
            MOVEM.L   (SP)+,A6/D2
            RTS

*
;:FIXED:-bryce-This was the cause of the "crash at mismatched baud rates"
;bug.  The timer.device's lists would get screwed.  This fix is
;"unusal".  To prevent an unwanted triggering of the softinit, I momentarily
;set MN_REPLYPORT to zero.  This depends on timer.device aborting
;my request syncronously (a good assumption).
cRLostBreak:                       ; didn't work, so abort timer
		BCLR.B	#IOSTB_READINGBRK,UNSER_STATUS(A6)
		LEA.L	IORBSTV(A6),A1
		MOVE.L	MN_REPLYPORT(A1),-(SP)
		CLR.L	MN_REPLYPORT(A1)
		LINKLIB	_LVOAbortIO,UNSER_EXLIB(A6)
		LEA.L	IORBSTV(A6),A1
		MOVE.L	(SP)+,MN_REPLYPORT(A1)
		BRA	cRLBGo	;jump back


  	DS.W 0
endMarker:

    END
