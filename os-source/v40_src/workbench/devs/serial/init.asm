
	TTL '$Id: init.asm,v 1.5 91/01/12 20:39:03 bryce Exp $'

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
*	$Id: init.asm,v 1.5 91/01/12 20:39:03 bryce Exp $
*
*	$Locker:  $
*
*	$Log:	init.asm,v $
* Revision 1.5  91/01/12  20:39:03  bryce
* #@&!~($%! RCS 4.0 switch.   Change Header to Id
* 
* Revision 1.4  90/12/01  20:07:12  bryce
* Fix use of A1 after call to AddPort()  [was "safe" at least until
* Enqueue() changes...]
* 
* Revision 1.3  89/04/12  13:52:53  bryce
* V1.31 release, with Y2 and Ariadne software fixes
* 
* Revision 1.2  89/02/07  15:59:45  bryce
* V1.3 maintainance release
* 
* Revision 33.3  86/07/21  17:50:20  andy
* moved timeout counter initialization
* 
* Revision 33.2  86/04/29  16:22:03  barry
* *** empty log message ***
* 
* Revision 33.1  86/04/29  14:19:04  barry
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
    INCLUDE     'devices/timer.i'
    INCLUDE     'resources/misc.i'

    INCLUDE     'assembly.i'  
    INCLUDE     'serialdev.i'  

    LIST

****** Imported Globals *********************************************

    XREF       	_serper
    XREF       	_serdatr
    XREF       	_serdat
    XREF       	_intena
    XREF       	_intreq

    XREF       	REVNUM
    XREF       	VERNUM
    XREF       	serName
    XREF       	miscName
    XREF       	functions
    XREF       	cmdRead
    XREF       	cmdWrite
    XREF       	ReadSI
    XREF       	WriteSI
    XREF       	cUnbrkNow
    XREF       	CheckRdBrk
    XREF	serIdent

****** Imported Functions *******************************************

    EXTERN_SYS MakeLibrary
    EXTERN_SYS OpenLibrary
    EXTERN_SYS OpenResource
    EXTERN_SYS SetIntVector
    EXTERN_SYS AddDevice
    EXTERN_SYS AddPort

****** Exported *****************************************************

    XDEF        serInit
    XDEF        devStructInit
    XDEF        paramInit

*
****** serial.device/Init ************************************************
*
*   NAME
*       Init -- initialize the serial port
*
*   FUNCTION
*       This function is called only by exec at this time.
*
*   INPUTS
*       A0 set to deviceNode (where to build the "serial.device" struct)
*
************************************************************************
*
*---------------------------------------------------------------- 
*
*   Initialization definitions
*
*----------------------------------------------------------------  

devStructInit:
***  what about initializing all these fields???
            INITBYTE    LN_TYPE,NT_DEVICE
	    INITLONG    LN_NAME,serName
	    INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	    INITWORD    LIB_VERSION,VERNUM
	    INITWORD    LIB_REVISION,REVNUM
	    INITLONG	LIB_IDSTRING,serIdent
paramInit:
            INITLONG    UNSER_XON,SER_CTL   ; careful !! does 4 char's at once
            INITLONG    UNSER_BAUD,SER_DBAUD
            INITWORD    UNSER_SPEED,SER_DSPEED
            INITWORD    UNSER_READBITS,SER_DRBITS
            INITWORD    UNSER_WRITEBITS,SER_DWBITS
            INITWORD    UNSER_STOPBITS,SER_DSBITS
            INITLONG    UNSER_RWLCNT,SER_DSBX
            INITLONG    UNSER_BRKTIME,SER_DBRKTIME
            INITLONG    UNSER_RBUFLEN,SER_DRBFSIZE
            INITLONG    UNSER_PARITY,ParityMagic
            DC.W        0  
serInit:
            MOVEM.L   A0/A2,-(SP)
            LEA       functions(PC),A0
            LEA       devStructInit(PC),A1
            SUB.L     A2,A2
            MOVE.L    #UNSER_SIZE,D0
            LINKLIB   _LVOMakeLibrary,A6
            MOVEM.L   (SP)+,A0/A2
            TST.L     D0
            BEQ       sIErr
            MOVE.L    A6,D1
            MOVEM.L   A3/A6,-(SP)
            MOVE.L    D0,A6                    ; pointer to ser data
            MOVE.L    A0,UNSER_SEGLIST(A6)     ; pointer to segment list
            MOVE.L    D1,UNSER_EXLIB(A6)       ; pointer to exec
	    MOVE.W    #SER_TIMEOUT,UNSER_TIMEOUT(A6)   ; init CTS timeout

***       ; Add handling for misc resources
            LEA       miscName(PC),A1
            LINKLIB   _LVOOpenResource,UNSER_EXLIB(A6)  ; open Misc Resource
            MOVE.L    D0,UNSER_MISC(A6)
            BEQ       sIErr
            LEA       serName(PC),A1
            MOVE.L    #MR_SERIALPORT,D0
            MOVE.L    A6,-(SP)
            MOVE.L    UNSER_MISC(A6),A6
            JSR       MR_ALLOCMISCRESOURCE(A6)  ; get ownership of serial
            TST.L     D0                        ;  hardware resources
            BNE       sIR10
            MOVE.L    #MR_SERIALBITS,D0
;FIXED-bryce:Old code forgot to set up A1 with the name before this call
            LEA       serName(PC),A1
            JSR       MR_ALLOCMISCRESOURCE(A6)  ; get ownership (2nd half)
sIR10:
            MOVE.L    (SP)+,A6
            TST.L     D0                        ; if failed, go restore
            BEQ.S     sIR20                     ;  stack and bomb out
            MOVEM.L   (SP)+,A3/A6
            BRA       sIErr
sIR20:
***
            LEA       WriteSoftInt(A6),A0      ; soft int for io-rqst
            LEA       WriteSI,A3               ;  completion on WRITE
            BSR.S     serDoSI10
            LEA       ReadSoftInt(A6),A0       ; soft int for io-rqst
            LEA       ReadSI,A3                ;  completion on READ
            BSR.S     serDoSI10
            BRA.S     sI10
*
*	A0 - SoftInt
*	A1 - MsgPort pointer
*	A3 - IS_CODE
*	A6 - IS_DATA
*
serDoSI:
            MOVE.L    A0,MP_SOFTINT(A1)
*	A0 - SoftInt
*	A3 - IS_CODE
*	A6 - IS_DATA
serDoSI10:
            MOVE.B    #16,LN_PRI(A0)
            MOVE.L    #serName,LN_NAME(A0)
            MOVE.L    A3,IS_CODE(A0)
            MOVE.L    A6,IS_DATA(A0)
            RTS

*
*	A1 - MsgPort [preserved]
*
serDoAP:    MOVE.L    A1,-(SP)
            MOVE.L    #serName,LN_NAME(A1)
            CLR.B     LN_PRI(A1)
            MOVE.B    #PA_SOFTINT,MP_FLAGS(A1) ; SoftInt handling for
            MOVE.B    #NT_MSGPORT,LN_TYPE(A1)  ;  MsgPort.
            LINKLIB   _LVOAddPort,UNSER_EXLIB(A6) 
	    MOVE.L    (SP)+,A1
            RTS
*
sI10:
            LEA       WrBrkMp(A6),A1           ; MsgPort for WRITE Break
            BSR.S     serDoAP                  ; add port
            LEA       WrBrkSoftInt(A6),A0
*	    [A1- preserved MsgPort]
            LEA       cUnbrkNow,A3
            BSR       serDoSI
            LEA       IOWBSTV(A6),A0            ; IORqst for break handling
            MOVE.B    #NT_REPLYMSG,LN_TYPE(A0)
            LEA       WrBrkMp(A6),A3
            MOVE.L    A3,MN_REPLYPORT(A0)
*
            LEA       RdBrkMp(A6),A1           ; MsgPort for READ Break
            BSR       serDoAP                  ; add port
            LEA       RdBrkSoftInt(A6),A0
*	    [A1- preserved MsgPort]
            LEA       CheckRdBrk,A3
            BSR       serDoSI
            LEA       IORBSTV(A6),A0            ; IORqst for break handling
            MOVE.B    #NT_REPLYMSG,LN_TYPE(A0)
            LEA       RdBrkMp(A6),A3
            MOVE.L    A3,MN_REPLYPORT(A0)

*	serSetIntVec:
            MOVE.L    A6,A1
            LINKLIB   _LVOAddDevice,UNSER_EXLIB(A6)
            LEA       serReadStruct(A6),A1
            MOVEQ     #INTB_RBF,D0
            MOVE.L    #cmdRead,IS_CODE(A1)
            BSR.S     sSIV
            MOVE.L    D0,UNSER_OLDRBF(A6)
            LEA       serWriteStruct(A6),A1
            MOVE.L    #cmdWrite,IS_CODE(A1)
            MOVEQ     #INTB_TBE,D0
            BSR.S     sSIV
            MOVE.L    D0,UNSER_OLDTBE(A6)
            MOVE.W    #(INTF_TBE!INTF_RBF),_intena ; disable intrpts for now
            MOVEQ     #1,D0                     ; unclear D0
*	sSexit:
            MOVEM.L   (SP)+,A3/A6               ; restore A6 to exec
            BRA.S     sIExit
sSIV:
            MOVE.L    A6,IS_DATA(A1)
            MOVE.L    #serName,LN_NAME(A1)
            MOVE.B    #NT_INTERRUPT,LN_TYPE(A1)
            LINKLIB   _LVOSetIntVector,UNSER_EXLIB(A6)
sIExit:
            RTS
*
sIErr:
            CLR.L     D0
            BRA.S     sIExit
*

    END 
