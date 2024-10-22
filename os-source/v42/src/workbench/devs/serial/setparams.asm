

	TTL '$Id: setparams.asm,v 1.4 91/01/12 20:39:19 bryce Exp $'

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
*	$Id: setparams.asm,v 1.4 91/01/12 20:39:19 bryce Exp $
*
*	$Locker:  $
*
*	$Log:	setparams.asm,v $
* Revision 1.4  91/01/12  20:39:19  bryce
* #@&!~($%! RCS 4.0 switch.   Change Header to Id
* 
* Revision 1.3  89/04/12  13:53:55  bryce
* V1.31 release, with Y2 and Ariadne software fixes
* 
* Revision 1.2  89/02/07  16:01:10  bryce
* V1.3 maintainance release
* 
*
* Revision 34.1  86/07/21  17:47:35  andy
* Adjusted the BaudMagic constant for PAL
*
* Revision 33.5  86/07/21  17:47:35  andy
* *** empty log message ***
* 
* Revision 33.4  86/05/09  02:20:31  barry
* corrections to auto-docs.
* 
* Revision 33.3  86/05/04  11:00:33  barry
* more changes to auto-docs
* 
* Revision 33.2  86/05/02  13:45:03  barry
* small changes to auto-docs
* 
* Revision 33.1  86/04/29  14:19:16  barry
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
    INCLUDE	'exec/execbase.i'
    INCLUDE	'exec/ports.i'
    INCLUDE	'exec/libraries.i'
    INCLUDE	'exec/devices.i'
    INCLUDE	'exec/resident.i'
    INCLUDE	'exec/initializers.i'
    INCLUDE	'exec/io.i'
    INCLUDE     'hardware/intbits.i'
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

    XREF       	GetBuf
    XREF       	sBIODone
    XREF       	cRSendXon

****** Imported Functions *******************************************


****** Exported *****************************************************
*
    XDEF        cmdSetParams
    XDEF        cmdBeginSetP
    XDEF        sCalcBaud
    XDEF        sDoMasks
*
******* serial.device/SDCMD_SETPARAMS ********************************
*
*   NAME
*       SetParams -- change parameters for the serial port
*
*   FUNCTION
*       This command allows the caller to change parameters for the
*       serial device. Except for xON-xOFF enable/disable, it will
*       reject a setparams call if any reads or writes are active 
*       or pending. 
*          Note specifically:
*
*    1. Valid input for io_Baud is between 112 and 292000 baud inclusive;
*       asynchronous i/o above 32KB (especially on a busy system) may
*       be ambitious.
*    2. The EOFMODE and QUEUEDBRK bits of io_SerFlags can be set/reset 
*       in the io_Rqst block without a call to SetParams. The SHARED 
*       and 7WIRE bits of io_SerFlags can be used in OpenDevice calls.
*       ALL OTHER PARAMETERS CAN ONLY BE CHANGED BY THE SetParams 
*       COMMAND.
*    3. RBufLen must be at least 64.  The buffer may be any multiple of
*	64 bytes.
*    4. If not used, io_ExtFlags MUST be set to zero.
*    5. xON-xOFF is by default enabled. The XDISABLED bit is the only
*       parameter that can be changed via a SetParams call while the
*       device is active. Note that this will return the value
*       SerErr_DevBusy in the io_Error field.
*	
*	xON/xOFF handshaking is inappropriate for certain binary transfer
*	protocalls, such as Xmodem.  The binary data might contain the
*	xON (ASCII 17) and xOFF (ASCII 19) characters.
*
*    6. If trying to run MIDI, you should set the RAD_BOOGIE bit of
*       io_SerFlags to eliminate unneeded overhead. Specifically, this skips
*       checks for parity, x-OFF handling, character lengths other than
*       8 bits, and testing for a break signal. Setting RAD_BOOGIE will
*       also set the XDISABLED bit.
*       Note that writing data (that's already in MIDI format) at MIDI rates
*       is easily accomplished. Using this driver alone for MIDI reads may,
*       however, may not be reliable, due to MIDI timestamping requirements,
*       and possibility of overruns in a busy multitasking and/or display
*       intensive environment.
*    7. If you select mark or space parity (see io_ExtFlags in serial.h),
*       this will cause the SERB_PARTY_ON bit to be set, and the setting
*       of SERB_PARTY_ODD to be ignored.
*    8. For best results, set the RAD_BOOGIE flag whenever possible.  See
*	#6 for details.
*    9. Note that at this time parity is *not* calculated for the xON-xOFF
*	characters.  If you have a system that is picky about the parity of
*	these, you must set your own xON-xOFF characters in io_CtlChar.
*   10. 7WIRE (CTS/RTS) handshake is bi-directional.  The external side
*	is expected to drop CTS several character times before the external
*	buffer is full.  The Amiga will drop RTS several character times
*	before the Amiga's buffer is full.
*
*
*   IO REQUEST
*       io_Message      mn_ReplyPort initialized
*       io_Device       preset by OpenDevice
*       io_Unit         preset by OpenDevice
*       io_Command      SDCMD_SETPARAMS (0x0B)
*  			NOTE that the following fields are filled in by Open 
*                       to reflect the serial device's current configuration.
*       io_CtlChar      a longword containing byte values for the
*                       xON,xOFF,INQ,ACK fields (respectively)
*                       (INQ/ACK not used at this time)
*       io_RBufLen      length in bytes of input buffer
*  			NOTE that any change in buffer size causes the
*                       current buffer to be deallocated and a new,
*                       correctly sized one to be allocated. Thusly,
*                       the CONTENTS OF THE OLD BUFFER ARE LOST.
*       io_ExtFlags     additional serial flags (bitdefs in devices/serial.h)
*			mark & space parity may be specified here.
*       io_Baud         baud rate for reads AND writes. (See 1 above)
*       io_BrkTime      duration of break signal in MICROseconds
*       io_TermArray    ASCII descending-ordered 8-byte array of
*                       termination characters. If less than 8 chars
*                       used, fill out array w/lowest valid value.
*                       Terminators are checked only if EOFMODE bit of
*                       io_Serflags is set. (e.g. x512F040303030303 )
*       io_ReadLen      number of bits in read word (1-8) not including parity
*       io_WriteLen     number of bits in write word (1-8) "      "       "
*       io_StopBits     number of stop bits (0, 1 or 2)
*       io_SerFlags     see devices/serial.h for bit equates, NOTE that x00
*                       yields exclusive access, xON/OFF-enabled, no
*                       parity checking, 3-wire protocol and TermArray
*                       inactive.
*
*   RESULTS
*       Error -- if the SetParams succeded, then Error will be null.
*           If the SetParams failed, then the Error will be non-zero.
*
*   SEE ALSO
*	exec/OpenDevice
*
************************************************************************
*
cmdBeginSetP:       ; BeginIO's SetParams entry point
*
*---------------------------------------------------------------- 
*
            BTST.B    #SERB_XDISABLED,IO_SERFLAGS(A1)
            BEQ.S     cSPX
            BSET.B    #SERB_XDISABLED,UNSER_FLAGS(A6) ; if no more x-ON/OFF
            BCLR.B    #SERB_XOFFWSET,UNSER_FLAG2(A6)  ;  clear current set
            BTST.B    #SERB_XOFFRSET,UNSER_FLAG2(A6)  ;  clear current set
            BEQ.S     cSBTest
            BCLR.B    #SERB_XDISABLED,IO_SERFLAGS(A1)
            BSR       cRSendXon
            BSET.B    #SERB_XDISABLED,IO_SERFLAGS(A1)
            BRA.S     cSBTest
cSPX:
            BCLR.B    #SERB_XDISABLED,UNSER_FLAGS(A6)
cSBTest:
            BTST.B    #SERB_RBUSY,UNSER_FLAG2(A6)  ; if I/O busy,...
            BNE.S     cSBusy             
            TST.L     UNSER_WCURIOR(A6)
            BNE.S     cSBusy             
            BTST.B    #IOB_ACTIVE,IO_FLAGS(A1)     ;  ... or queued ...
	    BNE.S     cSBusy
            TST.L     UNSER_RQCNT(A6)  ; NOTE that this longword test checks
            BEQ.S     sP10          ; BOTH read and write queues. CAREFUL!
cSBusy:
            MOVE.B    #SerErr_DevBusy,IO_ERROR(A1) ; busy, don't set params
            BRA       sP10Done
sP10:
            MOVE.W    #(INTF_TBE!INTF_RBF),_intena ; disable ints during setp.
            BSR       cmdSetParams            
            MOVE.W    #(INTF_SETCLR!INTF_TBE!INTF_RBF),_intena ; reenable ints
sP10Done:
            BRA       sBIODone
*
cmdSetParams:       ; SET SERIAL PORT PARAMETERS
*
            MOVEM.L   A3/D2,-(SP)
            TST.B     IO_READLEN(A1)
            BEQ       ParamErr
            CMP.B     #CharMax,IO_READLEN(A1)
            BGT       ParamErr
            TST.B     IO_WRITELEN(A1)
            BEQ       ParamErr
            CMP.B     #CharMax,IO_WRITELEN(A1)
            BGT       ParamErr
            CMP.B     #StopMax,IO_STOPBITS(A1)
            BGT       ParamErr
	    MOVE.L    IO_BAUD(A1),D0   
            CMP.L     #MinBaud,D0
            BLT       ParamErr
            CMP.L     #MaxBaud,D0
            BGT       ParamErr
	    TST.L     IO_CTLCHAR(A1)           ; if input is 0, 
            BEQ       ParamErr                 ;  err.
            MOVE.B    IO_READLEN(A1),UNSER_RWLCNT(A6) 
            MOVE.B    IO_WRITELEN(A1),UNSER_WWLCNT(A6)
            MOVE.B    IO_STOPBITS(A1),UNSER_SBLCNT(A6)
            MOVE.L    IO_CTLCHAR(A1),UNSER_XON(A6) ; store new control char's
            MOVE.L    IO_EXTFLAGS(A1),UNSER_EXTFLAGS(A6)
            BTST.B    #SERB_RAD_BOOGIE,IO_SERFLAGS(A1)
            BEQ.S     sP20
            BSET.B    #SERB_XDISABLED,IO_SERFLAGS(A1)
sP20:
            BTST.B    #SEXTB_MSPON,IO_EXTFLAGS+3(A1)
            BEQ.S     sP30
            BSET.B    #SERB_PARTY_ON,IO_SERFLAGS(A1)
sP30:
            MOVE.B    UNSER_FLAGS(A6),D0
            AND.B     #SERF_SHARED,D0
            MOVE.B    IO_SERFLAGS(A1),UNSER_FLAGS(A6)
            OR.B      D0,UNSER_FLAGS(A6)
            MOVE.B    UNSER_FLAGS(A6),IO_SERFLAGS(A1)
	    MOVE.L    IO_BAUD(A1),UNSER_BAUD(A6)
            BSR       sCalcBaud
            MOVE.L   TERMARRAY_0+IO_TERMARRAY(A1),TERMARRAY_0+UNSER_TARRAY(A6)
            MOVE.L   TERMARRAY_1+IO_TERMARRAY(A1),TERMARRAY_1+UNSER_TARRAY(A6)
            BSR       sDoMasks
sP50:                                  ; BREAK SIGNAL DURATION
            TST.L     IO_BRKTIME(A1)
            BEQ       ParamErr
            MOVE.L    IO_BRKTIME(A1),UNSER_BRKTIME(A6)
*
sP60:                                  ; READ BUFFER
            MOVE.L    UNSER_RBUFLEN(A6),D0
            CMP.L     IO_RBUFLEN(A1),D0   ; if buffer rqstd same as current,
            BEQ.S     setParamsExit       ;  skip down
            MOVE.L    IO_RBUFLEN(A1),D2
            CMP.L     #MIN_RBUFSIZE,D2    ; if buffer less than minimum
            BLT.S     ParamErrBuf         ;  error.
            MOVE.L    UNSER_RBUF(A6),A3
	;[D0-old size A3-old loc D2-new size ; Z-status] 
            BSR       GetBuf              ; else free current and get new
            TST.L     D0
            BNE.S     setParamsExit
ParamErrBuf:
	    MOVE.B    #SerErr_BufErr,IO_ERROR(A1)
setParamsExit:
	    MOVEM.L   (SP)+,A3/D2 
            RTS
***
sDoMasks:
            CLR.L     D0
            CLR.L     D1
            MOVE.B    IO_WRITELEN(A1),D0
	    TST.B     IO_STOPBITS(A1)
	    BEQ.S     sDM30
            BSET.L    D0,D1
            CMP.B     #2,IO_STOPBITS(A1)
            BNE.S     sDM30
            LSL.W     #1,D1
            BSET.L    D0,D1
sDM30:
            BTST.B    #SERB_PARTY_ON,UNSER_FLAGS(A6)
            BEQ.S     sDM40
            LSL.W     #1,D1
sDM40:
            MOVE.W    D1,UNSER_STOPBITS(A6)
            BSR       serGetMask 
            MOVE.W    D0,UNSER_WRITEBITS(A6)
            CLR.W     D0
            MOVE.B    IO_READLEN(A1),D0
            BSR       serGetMask 
            MOVE.W    D0,UNSER_READBITS(A6)
            RTS
*
serGetMask:
            BTST.B    #SERB_PARTY_ON,UNSER_FLAGS(A6)
            BNE.S     sGM10
            SUBQ.W    #1,D0
sGM10:
	    LSL.W     #1,D0 
	    LEA       WrdLenTable(PC),A0
            MOVE.W    0(A0,D0.W),D0
	    RTS   
*
ParamErr:
	    MOVE.B    #SerErr_InvParam,IO_ERROR(A1)
	    BRA.S     setParamsExit

WrdLenTable:
	    DC.W      $0001
	    DC.W      $0003
	    DC.W      $0007
	    DC.W      $000F
	    DC.W      $001F
	    DC.W      $003F
	    DC.W      $007F
	    DC.W      $00FF
            DC.W      $01FF
            DC.W      $03FF

sCalcBaud: ; you must be kidding ! DON'T mess with this mess unless you ...
            MOVE.L    UNSER_BAUD(A6),D0
            MOVE.L    D0,D1
            LSL.L     #3,D0
            SUB.L     D1,D0
;Multiply baud * 7   ((BAUD * 8)-BAUD)

	    MOVE.L    A1,-(SP)		; save register so we can play
	    MOVE.L    UNSER_EXLIB(A6),A1
	    CLEAR     D1
	    MOVE.B    PowerSupplyFrequency(A1),D1
;BUG-bryce:BaudMagic is wrong for NTSC.  Since it has been like this from V1.0,
;I won't mess with it at this time
    	    MOVE.L    #BaudMagic,A1 ; Another MAGIC constant: DON'T MESS
					; assume NTSC, though
	    CMP.B     #50,D1
	    BNE	      1$
	    MOVE.L    #PALBaudMagic,A1 ; Another MAGIC constant: DON'T MESS
1$          MOVE.L    A1,D1
            MOVE.L    (SP)+,A1		; done with it now

	    CMPI.L    #$FFFF,D0
            BLE.S     sCB10
            LSR.L     #5,D0
            DIVU.W    D0,D1
            AND.L     #$0000FFFF,D1
            LSR.L     #5,D1
            BRA.S     sCB20
sCB10:
            DIVU.W    D0,D1
sCB20:
            MOVE.W    D1,UNSER_SPEED(A6) ; SPEED = # of _serper cycles
            MOVE.W    UNSER_SPEED(A6),D0
            BCLR.B    #SERB_9BITRD,UNSER_FLAG2(A6)
            CMPI.B    #8,UNSER_RWLCNT(A6)
            BNE.S     sCB30
            BTST.B    #SERB_PARTY_ON,UNSER_FLAGS(A6)
            BEQ.S     sCB30
;FIXED-bryce:Used to OR.W with a *bit number*.  8 bits with parity would not
;work because of it.
            BSET.L    #SERB_PER9W,D0               ; if 9bit receive, set HOB
            BSET.B    #SERB_9BITRD,UNSER_FLAG2(A6) ; 8 bits PLUS parity
sCB30:
            MOVE.W    D0,_serper
            RTS
    END
