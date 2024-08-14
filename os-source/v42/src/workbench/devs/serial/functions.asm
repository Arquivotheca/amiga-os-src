
	TTL '$Id: functions.asm,v 1.4 91/01/12 20:38:55 bryce Exp $'

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
*	$Id: functions.asm,v 1.4 91/01/12 20:38:55 bryce Exp $
*
*	$Locker:  $
*
*	$Log:	functions.asm,v $
* Revision 1.4  91/01/12  20:38:55  bryce
* #@&!~($%! RCS 4.0 switch.   Change Header to Id
* 
* Revision 1.3  89/04/19  18:48:07  bryce
* Fix some autodocs & short branches
* 
* Revision 1.2  89/02/07  15:58:46  bryce
* V1.3 maintainance release
* 
* Revision 33.5  86/07/21  17:47:16  andy
* *** empty log message ***
* 
* Revision 33.4  86/07/18  19:34:54  barry
* *** empty log message ***
* 
* Revision 33.3  86/05/04  10:59:07  barry
* more changes to auto-docs
* 
* Revision 33.2  86/05/02  13:43:11  barry
* small changes to auto-docs
* 
* Revision 33.1  86/04/29  14:19:00  barry
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
    INCLUDE	'exec/errors.i'
    INCLUDE     'hardware/intbits.i'
    INCLUDE     'hardware/cia.i'
    INCLUDE     'devices/timer.i'

    INCLUDE     'assembly.i'  
    INCLUDE     'serialdev.i'  

    LIST
  
****** Imported Globals *********************************************

    XREF       	_serper
    XREF       	_serdatr
    XREF       	_serdat
    XREF       	_intena
    XREF       	_intreq
    XREF	_adkcon
    XREF       	cmdSetParams
    XREF        sDoAbort 
    XREF       	paramInit
    XREF       	cWQueIt
    XREF       	serOpen 
    XREF        sBIODone
    XREF        cRSendXoff
    XREF        cRSendXon
    XREF        sAbortTimer

****** Imported Functions *******************************************

    EXTERN_SYS InitStruct
    EXTERN_SYS ReplyMsg 

****** Exported *****************************************************

    XDEF        cmdReset
    XDEF        cmdClear
    XDEF        cmdBeginClear
    XDEF        cmdStart
    XDEF        cmdStop
    XDEF        cmdFlush
    XDEF        cmdBeginFlush
    XDEF        cmdQuery
    XDEF        cmdBreak
    XDEF        cBrkNow
    XDEF        cUnbrkNow
    XDEF        StartWriteTimer

**********************************************************************
*
******* serial.device/CMD_RESET **********************************
*
*   NAME
*       Reset -- reinitializes the serial port
*
*   FUNCTION
*       This command resets the serial port to its freshly initialized
*       condition. It aborts all I/O requests both queued and current,
*       relinquishes the current buffer, obtains a new default sized
*       buffer, and sets the port's flags and parameters to their 
*       boot-up time default values. The functions places the reset
*       parameter values in the ioRequest block.
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      CMD_RESET
*
*   RESULTS
*       Error -- if the Reset succeded, then Error will be null.
*           If the Reset failed, then the Error will be non-zero.
*
************************************************************************
*
cmdReset:           ; RESET
*
*----------------------------------------------------------------

            MOVE.W    #(INTF_RBF!INTF_TBE),_intena ; disable serial ints
            BSR       cmdFlush       ; abort all queued IORqst's
            MOVE.L    A1,-(SP)
            MOVE.L    UNSER_RCURIOR(A6),D0
            BEQ.S     cReset10
            MOVE.L    D0,A1
            BSR       sDoAbort
cReset10:
            MOVE.L    UNSER_WCURIOR(A6),D0
            BEQ.S     cReset20
            MOVE.L    D0,A1
            BSR       sDoAbort
cReset20:
            AND.B     #SERF_SHARED,UNSER_FLAGS(A6)
            CLR.B     UNSER_FLAG2(A6)
            MOVE.L    (SP)+,A1
*         ; Reset Device Parameters
            MOVE.L    #SER_CTL,IO_CTLCHAR(A1)
            MOVE.L    #SER_DBAUD,IO_BAUD(A1)
            MOVE.L    #SER_DSBX,IO_READLEN(A1)
            MOVE.L    #SER_DBRKTIME,IO_BRKTIME(A1)
            MOVE.L    #SER_DRBFSIZE,IO_RBUFLEN(A1)
            LEA       IO_TERMARRAY(A1),A0
            CLR.L     (A0)+
            CLR.L     (A0)
            BSR       cmdSetParams           ; reset to original parameters
            BSR.S     cmdClear       ; purge device's buffers
            MOVE.W    #(INTF_SETCLR!INTF_RBF!INTF_TBE),_intena ; re-enable
            BRA       sBIODone
*
******* serial.device/CMD_CLEAR *************************************
*
*   NAME
*       Clear -- clear the serial port buffers
*
*   FUNCTION
*       This command resets the serial port's read buffer pointers.
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      CMD_CLEAR
*
*   RESULTS
*       Error -- If the Clear succeded, then io_Error will be null.
*                If the Clear failed, then the io_Error will be non-zero.
*
************************************************************************
*
cmdBeginClear:      ; BeginIO's Clear enrty point
            BSR.S     cmdClear
            BRA       sBIODone
*
*---------------------------------------------------------------- 
*
cmdClear:           ; CLEAR READ BUFFER
*
*---------------------------------------------------------------- 
*
            MOVE.L    UNSER_RBUF(A6),UNSER_RBUFPUT(A6)            
            MOVE.L    UNSER_RBUF(A6),UNSER_RBUFGET(A6)            
            CLR.L     UNSER_RBUFCNT(A6)
            RTS
*
******* serial.device/CMD_STOP **************************************
*
*   NAME
*       Stop -- pause all current I/O over the serial port
*
*   FUNCTION
*       This command halts all current I/O on the serial port by 
*       sending an xOFF to the "other side", and submitting a "logical
*       xOFF" to "our side", if/when appropriate to current activity.
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      CMD_STOP
*
*   RESULTS
*
*   SEE ALSO
*       serial.device/CMD_START
*
************************************************************************
*
*----------------------------------------------------------------
*
cmdStop:            ; PAUSE CURRENT I/O  
*
*---------------------------------------------------------------- 
*
            MOVE.L    UNSER_WCURIOR(A6),D0
            BEQ.S     cStopRead
            BSET.B    #SERB_XOFFWSET,UNSER_FLAG2(A6)
cStopRead:
            MOVE.L    UNSER_RCURIOR(A6),D0
            BEQ.S     cStopExit
            BSR       cRSendXoff
cStopExit:            
            BRA       sBIODone
            
*
******* serial.device/CMD_START *************************************
*
*   NAME
*       Start -- restart paused I/O over the serial port
*
*   FUNCTION
*       This function restarts all current I/O on the serial port by 
*       sending an xON to the "other side", and submitting a "logical
*       xON" to "our side", if/when appropriate to current activity.
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      CMD_START
*
*   RESULTS
*
*   SEE ALSO
*       serial.device/CMD_STOP
*
************************************************************************

cmdStart:           ; RESTART PAUSED I/O
*
*---------------------------------------------------------------- 
*
            BCLR.B    #SERB_XOFFWSET,UNSER_FLAG2(A6)
            MOVE.L    UNSER_RCURIOR(A6),D0
            BEQ.S     cStartWrite
            BSR       cRSendXon
cStartWrite:
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intreq
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intena
cStartExit:            
            BRA       sBIODone
            
*
******* serial.device/CMD_FLUSH ****************************************
*
*   NAME
*       Flush -- clear all queued I/O requests for the serial port
*
*   FUNCTION
*       This command purges the read and write request queues for the
*       serial device. Flush will not affect active requests.
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      CMD_FLUSH
*
*   RESULTS
*       Error -- if the Flush succeded, then io_Error will be null.
*           	 If the Flush failed, then the io_Error will be non-zero.
*
************************************************************************
*
cmdBeginFlush:      ; BeginIO's Flush entry point
            MOVE.W    #(INTF_RBF!INTF_TBE),_intena ; disable serial ints
            BSR       cmdFlush
            MOVE.W    #(INTF_SETCLR!INTF_RBF!INTF_TBE),_intena ; re-enable
            BRA       sBIODone               ; done
*
*---------------------------------------------------------------- 
*
cmdFlush:           ; FLUSH OUT QUEUED REQUESTS
*
*---------------------------------------------------------------- 
*
            MOVE.L    A1,-(SP)
            LEA       serReadQueue(A6),A0
            BSR       cFPurgeQ
            LEA       serWriteQueue(A6),A0
            BSR       cFPurgeQ
            CLR.L     UNSER_RQCNT(A6) ; CAREFUL! clearing BOTH R + W Que counts
            MOVE.L    (SP)+,A1
            RTS
cFPurgeQ:
            REMHEAD
            TST.L     D0
            BEQ       cFPQDone 
            MOVE.L    D0,A1
            BSR       cFTellAborted
            BRA.S     cFPurgeQ
cFPQDone:
            RTS
cFTellAborted:
            BSET.B    #IOB_ABORT,IO_FLAGS(A1)
            MOVE.B    #IOERR_ABORTED,IO_ERROR(A1)
            BTST.B    #IOB_QUICK,IO_FLAGS(A1)
            BNE.S     cFTA10
            LINKLIB   _LVOReplyMsg,UNSER_EXLIB(A6) 
cFTA10:
            RTS
            
*
*
******* serial.device/SDCMD_QUERY ****************************************
*
*   NAME
*       Query -- query serial port/line status
*
*   FUNCTION
*       This command return the status of the serial port lines and
*       registers. The number of unread bytes in the serial device's
*       read buffer is shown in io_Actual.
*
*	The break send & received flags are cleared by a query, and
*	whenever a read IORequest is returned with a error
*	in io_Error.
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       preset by OpenDevice
*       io_Unit         preset by OpenDevice
*       io_Command      SDCMD_QUERY
*
*   RESULTS
*       io_Status        BIT  ACTIVE  FUNCTION
*
*              LSB       0    ---    reserved
*                        1    ---    reserved
*                        2    high   parallel "sel" on the A1000
*                                    On the A500 & A2000, "sel" is also
*                                    connected to the serial port's
*                                    "Ring Indicator".  Be cautious when
*                                    making cables.
*                        3    low    Data Set Ready
*                        4    low    Clear To Send
*                        5    low    Carrier Detect
*                        6    low    Ready To Send
*                        7    low    Data Terminal Ready
*              MSB       8    high   hardware overrun
*                        9    high   break sent (most recent output)
*                       10    high   break received (as latest input)
*                       11    high   transmit x-OFFed       
*                       12    high   receive x-OFFed       
*                    13-15    ---    reserved
*
*       io_Actual       set to count of unread input characters
*
*       io_Error -- Query will always succeded.
*
************************************************************************
*
cmdQuery:           ; GET SERIAL PORT STATUS SNAPSHOT
*
*---------------------------------------------------------------- 
*
            MOVE.L    #prae,A0
            MOVE.B    (A0),D0
            BTST.B    #SERB_7WIRE,UNSER_FLAGS(A6)
            BEQ.S     cQry10
	    AND.B     #(~(CIAF_COMCTS))&$FF,D0	;Zero COMCTS
            BTST.B    #SERB_XOFFRSET,UNSER_FLAG2(A6)
            BEQ.S     cQry10
            BSET.B    #CIAB_COMCTS,D0
cQry10:     AND.B     #SerQueryBits,D0
            MOVE.B    D0,IO_STATUS+1(A1)
            MOVE.B    UNSER_STATUS(A6),IO_STATUS(A1)
            AND.B     #SerStatMask,IO_STATUS(A1)
            AND.B #(~(IOSTF_READBREAK+IOSTF_WROTEBREAK))&$FF,UNSER_STATUS(A6)
            MOVE.L    UNSER_RBUFCNT(A6),IO_ACTUAL(A1)
            BRA       sBIODone
*
*
******* serial.device/SDCMD_BREAK ****************************************
*
*   NAME
*       Break -- send a break signal over the serial line
*
*   FUNCTION
*       This command sends a break signal (serial line held low for an
*       extended period) out the serial port. For the built-in port,
*       This is accomplished by setting the UARTBRK bit of regisrer ADKCON.
*
*       After a duration (user specifiable via setparams, default 250000
*       microseconds) the bit is reset and the signal discontinued.
*       If the QUEUEDBRK bit of io_SerFlags is set in the io_Request 
*       block, the request is placed at the back of the write-request 
*       queue and executed in turn. If the QUEUEDBRK bit is not set, 
*       the break is started immediately, control returns to the 
*       caller, and the timer discontinues the signal after the 
*       duration is completed. Be aware that calling BREAK may
*	affect other commands such as ABORT, FLUSH, STOP, START, etc...
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       set by OpenDevice
*       io_Unit         set by OpenDevice
*       io_Command      SDCMD_BREAK
*       io_Flags        set/reset IO_QUICK per above description
*
*   RESULTS
*       Error -- if the Break succeded, then Error will be null.
*           If the Break failed, then the Error will be non-zero.
*
************************************************************************
*
cmdBreak:           ; SEND BREAK SIGNAL OUT SERIAL PORT
*
*---------------------------------------------------------------- 
*
            BCLR.B    #IOB_QUICK,IO_FLAGS(A1)
            BTST.B    #SERB_QUEUEDBRK,IO_SERFLAGS(A1) ; que if port busy?
            BEQ.S     cBrkNow
            TST.W     UNSER_WQCNT(A6)
            BNE       cWQueIt            ; que it and BRA  sBIODone
            TST.L     UNSER_WCURIOR(A6)
            BNE       cWQueIt
cBrkNow:                            ; send break now and go away
;FIXED-bryce:State bit was not set until *after* the timer request was posted.
;FIXED-bryce:Impossible error check for with wrong value in A1 at the time.
            BSET.B    #IOSTB_WRITINGBRK,UNSER_STATUS(A6)
            MOVE.L    UNSER_BRKTIME(A6),D0
            BSR       StartWriteTimer     ; start timer for break duration
            MOVE.L    A1,UNSER_WCURIOR(A6)
            MOVE.L    #_adkcon,A0
            MOVE.W    #(ADKCONF_SETCLR!ADKCONF_UARTBRK),(A0)
            RTS

*************************          ; Timer interrupt comes here
;FIXED-bryce:This int can blow away AbortIO's attempt to lock out ints. KLUDGE
cUnbrkNow:  BCLR.B    #IOSTB_WRITINGBRK,UNSER_STATUS(A1)
            BEQ.S     cUnbrkNow20              ; not break, just re-enable
            MOVEM.L   A6/A1,-(SP)
             MOVE.L    A1,A6
             MOVE.L    UNSER_WCURIOR(A6),A1
             BEQ.S     cUnbrkNow10              ; if aborted, fall out.
             CMP.W     #SDCMD_BREAK,IO_COMMAND(A1)
             BNE.S     cUnbrkNow10              ; if aborted, fall out.
             BSET.B    #IOSTB_WROTEBREAK,UNSER_STATUS(A6)
             CLR.L     UNSER_WCURIOR(A6)
             MOVE.L    #_adkcon,A0
             MOVE.W    #(ADKCONF_UARTBRK),(A0)
             LINKLIB   _LVOReplyMsg,UNSER_EXLIB(A6) ; go tell completion
cUnbrkNow10:
            MOVEM.L   (SP)+,A6/A1
cUnbrkNow20:
            BTST.B    #SERB_IRQLOCK,UNSER_FLAG2(A1)
            BNE.S     cUno_do
            MOVE.W    #(INTF_SETCLR!INTF_TBE),_intena   ; re-enable TBE int
cUno_do:    RTS
*
*


;FIXED-bryce:This used to start the same write request multiple times. Bad
;news, and the reason for one of the CTS/RTS lockups.

StartWriteTimer:      ; PASS wait time in D0
            MOVEM.L   A6/A1,-(SP)
            MOVE.W    #(INTF_TBE),_intena ; disable serial write ints
             LEA       IOWBSTV(A6),A1
             CMP.B     #NT_MESSAGE,LN_TYPE(A1)
             BEQ.S     sW_already	;Already a pending timer?...
             CLR.L     IOTV_TIME+TV_SECS(A1)
	     CMP.L     #600,D0
             BHI.S     sW_long
	     MOVE.L    #600,D0
sW_long:     OR.B      #$FF,D0	;Or in some extra time
             MOVE.L    D0,IOTV_TIME+TV_MICRO(A1)
             MOVE.W    #TR_ADDREQUEST,IO_COMMAND(A1)
	     CLR.B     IO_FLAGS(A1)  ;unset io_quick
	     MOVE.L    IO_DEVICE(A1),A6
             JSR       DEV_BEGINIO(A6)
            MOVEM.L   (SP)+,A1/A6
            RTS

sW_already: MOVEM.L   (SP)+,A1/A6
            RTS
*
    END
