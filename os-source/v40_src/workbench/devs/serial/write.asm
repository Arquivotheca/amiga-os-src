	TTL '$Id: write.asm,v 1.3 91/01/12 20:39:26 bryce Exp $'

*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	         SERIAL DEVICE DRIVER            **
*	   **	   --------------------------------	 **
*	   **						 **
*	   ************************************************


**********************************************************************
*								     *
*   Copyright 1984, 1985, Commodore Amiga Inc. All rights reserved.  *
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
*	$Id: write.asm,v 1.3 91/01/12 20:39:26 bryce Exp $
*
*	$Locker:  $
*
*	$Log:	write.asm,v $
* Revision 1.3  91/01/12  20:39:26  bryce
* #@&!~($%! RCS 4.0 switch.   Change Header to Id
* 
* Revision 1.2  89/02/07  16:01:28  bryce
* V1.3 maintainance release
* 
* Revision 33.7  86/07/21  17:49:55  andy
* moved timeout counter initialization
* 
* Revision 33.6  86/07/18  19:35:49  andy
* Added timeout flag on fixed CTS/RTS handshaking
* 
* Revision 33.5  86/07/18  19:34:38  barry
* *** empty log message ***
* 
* Revision 33.4  86/05/30  23:08:24  barry
* *** empty log message ***
* 
* Revision 33.3  86/05/09  02:24:18  barry
* corrections to auto-docs.
* 
* Revision 33.2  86/05/02  13:46:48  barry
* small changes to auto-docs
* 
* Revision 33.1  86/04/29  14:19:19  barry
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
    INCLUDE     'devices/timer.i'

    INCLUDE     'assembly.i'  
    INCLUDE     'serialdev.i'  

    LIST

****** Imported Globals *********************************************

    XREF       	_serdat
    XREF       	_intena
    XREF       	_intreq
    XREF       	_ciab
    XREF        cBrkNow
    XREF        sBIODone
    XREF        StartWriteTimer
    XREF        rCzechParty

***** Imported Functions *******************************************

    EXTERN_SYS ReplyMsg 
;   EXTERN_SYS Cause

****** Exported *****************************************************

    XDEF        cmdBeginWrite
    XDEF        cmdWrite
    XDEF        cWQueIt
    XDEF        WriteSI

**********************************************************************
*
******* serial.device/CMD_WRITE *************************************
*
*   NAME
*       Write -- send output to serial port
*
*   FUNCTION
*       This command causes a stream of characters to be written out
*       the serial port. The number of characters is specified in
*       io_Length, unless -1 is used, in which case output is sent until
*       a null(0x00) is encountered. 
*
*   IO REQUEST
*       io_Message      must have mn_ReplyPort initialized
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      CMD_WRITE
*       io_Flags        Set IOF_QUICK to try quick I/O
*       io_Length       number of characters to transmit, or if set
*                       to -1 transmit until null encountered in buffer
*       io_Data         pointer to block of data to transmit
*
*   RESULTS
*       Error -- if the Write succeded, then io_Error will be null.
*           If the Write failed, then the io_Error will be non-zero.
*
*   SEE ALSO
*	serial.device/SDCMD_SETPARAMS
*
************************************************************************
*
cmdBeginWrite:      ; WRITE
*
*----------------------------------------------------------------
*                                                 ; ENTRY from BEGIN I/O
            CLR.L     IO_ACTUAL(A1)
            TST.L     IO_LENGTH(A1)
            BEQ       sBIODone			  ; go directly to exit IO


            BCLR.B    #IOB_QUICK,IO_FLAGS(A1)     ; never quick I/O
;FIXED-bryce: this list is shared with the hardware interrupt.  We must turn
;that off before playing with it.
;FIXED-bryce:This is not protected against timer interrupts starting things up
;again
            BSET.B    #SERB_IRQLOCK,UNSER_FLAG2(A6) ;Lock out timer danger
	    MOVE.W    #INTF_TBE,_intena		  ;Disable TBE interrupt
             TST.W     UNSER_WQCNT(A6)             ; if some already queued,
             BNE.S     cWQueIt                     ;  queue this rqst too
             TST.L     UNSER_WCURIOR(A6)           ; if some already queued,
             BNE.S     cWQueIt                     ;  queue this rqst too

;--Do immediately!
	     BSET.B    #IOB_ACTIVE,IO_FLAGS(A1)
             CLR.L     UNSER_WLEN(A6)
             MOVE.L    IO_DATA(A1),UNSER_WDATA(A6) ; Queued I/O joins in here
             MOVE.L    A1,UNSER_WCURIOR(A6)        ; store current IORqst address
             MOVE.W    #(INTF_SETCLR!INTF_TBE),_intreq ; trigger the write
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intena  ;unlock irq
	    BCLR.B    #SERB_IRQLOCK,UNSER_FLAG2(A6) ;unlock timer
;BUG-bryce:He sets up a software interrupt that goes *nowhere*????
WriteSI:    RTS


;QUEUE writes here
cWQueIt:     BSET.B    #IOB_QUEUED,IO_FLAGS(A1)    ; flag as queued
             LEA       serWriteQueue(A6),A0
             ADDTAIL 				;A1 not destroyed                   
             ADDQ.W    #1,UNSER_WQCNT(A6)
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intena	;Enable TBE interrupt
	    BCLR.B    #SERB_IRQLOCK,UNSER_FLAG2(A6)	;unlock timer
;FIXED-bryce:Used to reply to a message--right after queueing it!!
	    RTS



*********************************            
	CNOP 0,4 ;ADD-bryce:Align to longword for 68020/030 folks
cmdWrite:   ; HARDWARE INTERRUPT (TBE) PROCESSING (i.e. write a (another) char)
*********************************
            TST.B     UNSER_XDATA(A1)
            BNE       cWSendXctl
            MOVEM.L   A6/D2,-(SP)
            MOVE.L    A1,A6
            BTST.B    #IOSTB_WRITINGBRK,UNSER_STATUS(A6) ; if woken while breaking
            BNE       cWExitx                     ; go back to sleep.
            TST.L     UNSER_WCURIOR(A6)           ; if no current,
            BEQ       cBWNext                     ;  check for any other.
cWNextChar:                                       ; else next char.
            MOVE.L    UNSER_WCURIOR(A6),A1        ; store current IORqst ptr
            BTST.B    #IOB_ABORT,IO_FLAGS(A1)
            BNE       cBWNext
            BTST.B    #SERB_XOFFWSET,UNSER_FLAG2(A6) ; if Xmit halted,
            BNE       cWExitx                      ; wait for xOn
            BTST.B    #SERB_7WIRE,UNSER_FLAGS(A6)  ; if handshake active
            BEQ.S     cWN5

;-----CTS/RTS handling
            MOVE.B    _ciab+ciapra,D0
            AND.B     #(CIAF_COMCTS!CIAF_COMDSR),D0
            BEQ.S     cWN5 ;both handshake and DSR ok

            BTST.B    #CIAB_COMDSR,D0
            BEQ.S     CWLOk
            MOVE.B    #SerErr_NoDSR,IO_ERROR(A1) ;Clear Z
            BRA       cWDone                       ; err = DSR bad

;FIXED-bryce:This did a BTST where a compare was needed.  No DSR errors were
;ever reported by the serial.device because of it
;           BTST.B    #SerErr_NoDSR,IO_ERROR(A1)   

CWLOk:      SUBQ.W    #1,UNSER_TIMEOUT(A6)         ;time enough for handshake ?
            BNE.S     cWNotDone
            MOVE.B    #SerErr_TimerErr,IO_ERROR(A1)
            BRA       cWDone   

cWNotDone:  MOVEQ     #0,D0
            MOVE.W    UNSER_SPEED(A6),D0           ; hardware xOFF found
            LSL.L     #MagicShift,D0               ; wait for 90 baud cycles
;StartWriteTimer disables tbe ints.  It leaves any current int pending
            BSR       StartWriteTimer              ; go post wake-up, retry
            BRA       cWExit                       ;  pending int later.

;^^^^^CTS/RTS handling

cWN5:       MOVE.W    #SER_TIMEOUT,UNSER_TIMEOUT(A6)	  ; reinit CTS timeout
            MOVE.L    UNSER_WDATA(A6),A0
            CLR.W     D0
	    MOVE.B    (A0),D0
            MOVE.L    IO_LENGTH(A1),D1
            BPL.S     cWN7
            BTST.B    #SERB_WEOL,UNSER_FLAG2(A6)  ; if EOL = 0,
            BEQ.S     cWN8
            SUBQ.L    #1,UNSER_WLEN(A6)           ;  correct length here.
            BRA       cWDone
cWN7:
            CMP.L     UNSER_WLEN(A6),D1
            BLS       cWDone
cWN8:
            MOVE.W    #(INTF_TBE),_intreq         ; clear TBE intrpt request
            BCLR.B    #SERB_WEOL,UNSER_FLAG2(A6)
            AND.W     UNSER_WRITEBITS(A6),D0      ; allow only correct # bits
            BNE.S     cWN10
            BSET.B    #SERB_WEOL,UNSER_FLAG2(A6)  ; flag as EOF (null) char.
cWN10:            
            BTST.B    #SERB_PARTY_ON,UNSER_FLAGS(A6) ; parity check enabled?
            BEQ.S     cWPOk
            BTST.B    #SEXTB_MSPON,UNSER_EXTFLAGS+3(A6) ; mark-space ?
            BEQ.S     cWP18
            BTST.B    #SEXTB_MARK,UNSER_EXTFLAGS+3(A6) ; if space, leave unset
            BNE.S     cWPSetIt
            BRA.S     cWPOk
cWP18:
*            MOVE.B    D0,D1
*            ASR.B     #4,D1
*            EOR.B     D0,D1
*	    MOVE.L    UNSER_PARITY(A6),D2
*	    BTST.B    #SERB_PARTY_ODD,UNSER_FLAGS(A6)
*	    BEQ.S     cWN20
*	    NOT.L     D2
*cWN20:
*	    BTST.L    D1,D2
            JSR       rCzechParty
            BEQ.S     cWPOk
cWPSetIt:
            MOVE.B    UNSER_WWLCNT(A6),D1
            BSET.L    D1,D0
cWPOk:
            OR.W      UNSER_STOPBITS(A6),D0       ; set stop bit(s)
	    MOVE.W    D0,_serdat
	    ADDQ.L    #1,UNSER_WDATA(A6)          ; next byte
            ADDQ.L    #1,UNSER_WLEN(A6)   
            BTST.B    #SERB_EOFMODE,IO_SERFLAGS(A1) ; check for EOF ?
            BEQ.S     cWSB15
            LEA       UNSER_TARRAY(A6),A0
            MOVEQ     #TERMARRAY_SIZE-1,D1
cWEofNextChar:
            CMP.B     (A0)+,D0                  ; match this EOF char ?
            DBCC      D1,cWEofNextChar          ; if less or equ fall thru
            BEQ.S     cWDone
cWSB15:
            MOVE.L    IO_LENGTH(A1),D0
            BMI.S     cWExit
            CMP.L     UNSER_WLEN(A6),D0
            BGE.S     cWExit

cWDone:
*******
	    MOVE.L    UNSER_WLEN(A6),IO_ACTUAL(A1)
            BCLR.B    #SERB_WEOL,UNSER_FLAG2(A6)
            CLR.L     UNSER_WCURIOR(A6)
            LINKLIB   _LVOReplyMsg,UNSER_EXLIB(A6) ; not qik,send RMsg
            BRA.S     cWExit                      ;  go away till next
cBWNext:                                          ; get next IO_Rqst
            TST.W     UNSER_WQCNT(A6)             ; if nothing else queued,
            BEQ.S     cWExitx                     ;  go away till next
            LEA       serWriteQueue(A6),A0        ; so, we get next write IOR
            REMHEAD                    
            MOVE.L    D0,A1                       ; address next IO_Rqst block
            MOVE.L    D0,UNSER_WCURIOR(A6)
            SUBQ.W    #1,UNSER_WQCNT(A6)
            BSET.B    #IOB_ACTIVE,IO_FLAGS(A1)    ; this IORqst now active,
            BCLR.B    #IOB_QUEUED,IO_FLAGS(A1)    ;  no longer queued.
            CLR.L     UNSER_WLEN(A6)
            MOVE.L    IO_DATA(A1),UNSER_WDATA(A6)
            CMP.W     #SDCMD_BREAK,IO_COMMAND(A1) ; if queued break, go do
            BNE.S     cWExit
            BSR       cBrkNow                     ;  and disable for duration.
cWExit:     MOVEM.L   (SP)+,A6/D2		; ints still enabled
            RTS


cWExitx:    MOVE.W    #(INTF_TBE),_intena      ; disable til next IORqst
            MOVEM.L   (SP)+,A6/D2
            RTS   


cWSendXctl: MOVE.W    #(INTF_TBE),_intreq      ; clear TBE intrpt request
            CLR.W     D1
            MOVE.B    UNSER_XDATA(A1),D1       ; note dev-ptr in A1, not A6
            OR.W      UNSER_STOPBITS(A1),D1
            CLR.B     UNSER_XDATA(A1)
            MOVE.W    D1,_serdat
            TST.L     UNSER_WCURIOR(A1)
            BNE.S     cWSX10
            TST.W     UNSER_WQCNT(A1)
            BNE.S     cWSX10
            MOVE.W    #(INTF_TBE),_intena      ; disable til next IORqst
cWSX10:     RTS

            END

