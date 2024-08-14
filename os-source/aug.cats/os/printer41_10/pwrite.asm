	TTL    '$Id $'
**********************************************************************
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*	printer private write command
*
*   Source Control
*   --------------
*   $Id: pwrite.asm,v 1.20 91/07/15 17:01:33 darren Exp $
*
*   $Locker:  $
*
*   $Log:	pwrite.asm,v $
*   Revision 1.20  91/07/15  17:01:33  darren
*   First V38 release - localization support.  See requester.asm for
*   requester code.
*   
*   Revision 1.19  91/02/14  15:26:01  darren
*   Autodoc stuff
*   
*   Revision 1.18  91/02/12  11:47:03  darren
*   Spelling error - fixed
*   
*   Revision 1.17  90/10/04  13:32:00  darren
*   Clear IO_PARSTATUS before
*   calling parallel.device query.
*   If it fails, well we have a number of
*   sorts, though in any case we dont really
*   know what wrong (in theory it never fails, but...)
*   
*   Revision 1.16  90/10/01  15:55:53  darren
*   Fix - doesnt read hardware
*   anymore to get status of SEL & POUT
*   bits.
*   
*   Revision 1.15  90/07/25  21:30:02  valentin
*   Changed $Header to $Id
*   
*   Revision 1.14  90/07/25  17:33:43  valentin
*   RCS directory change
*   
*   Revision 1.14  90/07/25  09:37:40  valentin
*   Made small modification for aborted requests... the chkBothReady label
*   ,
*   
*   Revision 1.13  90/04/30  12:50:12  daveb
*   changed all occurances of 'topaz 9' to 'topaz 8' for the
*   printer trouble requester.
*   
*   Revision 1.12  90/04/06  19:24:59  daveb
*   for rcs 4.x header change
*   
*   Revision 1.11  89/09/07  12:08:53  daveb
*   added comment about netprint.device kludge at 'timeoutAlert'
*   
*   Revision 1.10  88/08/18  15:36:38  daveb
*   added some comments
*   
*   Revision 1.9  88/08/17  10:48:35  daveb
*   made changes to autodocs
*   
*   Revision 1.8  88/06/14  16:28:40  daveb
*   added entry point for cancelling parallel/serial requests
*   V1.3 Gamma 18
*   
*   Revision 1.7  88/05/17  15:53:16  daveb
*   removed comments from what Bryce and Dave B thought was redundant code.
*   It IS redundant code BUT because of some magic going on IS required else
*   the printing hangs at some point.  Oh well... (sigh).
*   V1.3 Gamma 15
*   
*   Revision 1.6  88/05/16  14:09:02  daveb
*   V1.3 Gamma 15
*   
*   Revision 1.5  88/05/16  11:18:50  daveb
*   removed AbortIO kludge since the new serial device no longer needs it.
*   V1.3 Gamma 15
*   
*   Revision 1.4  87/11/18  10:41:08  daveb
*   V1.3 Gamma 3 release
*   
*   Revision 1.3  87/11/17  11:16:32  daveb
*   added code to abort if a write of length 0 is requested
*   V1.3 Gamma 3 release
*   
*   Revision 1.2  87/09/09  06:54:23  daveb
*   added code to get around the serial device abort bug
*   
*   Revision 1.1  87/08/31  16:12:24  daveb
*   added code to do a CheckIO after an AbortIO
*   this fixes the code hanging in WaitIO (on serial printers) bug
*   
*   Revision 1.0  87/08/26  12:13:49  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/21  17:27:38  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:49:54  daveb
*   added to rcs
*   
*   Revision 32.3  86/07/25  11:04:18  andy
*   device no longer crashes when there is a -1 in the Process
*   WindowPrt field and the device tries to put up a requester
*   
*   Revision 32.2  86/06/24  16:44:29  andy
*   Added DISABLE/ENABLE call around code which directly checks the
*   cia, and plays with its ddr
*   
*   Revision 32.1  86/02/11  15:56:26  kodiak
*   update PWrite documentation
*   
*   Revision 1.5  85/10/09  23:59:56  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.4  85/10/01  18:46:05  kodiak
*   use unknown message for serial timeout.
*   
*   Revision 1.3  85/09/25  16:48:09  kodiak
*   use IOR variable to check for existing error (caused by abort)
*   
*   Revision 1.2  85/09/25  15:32:42  kodiak
*   remove unused XREFs
*   
*   Revision 1.1  85/09/23  14:21:53  kodiak
*   codify PDERR_CANCEL as explicit error for timeout
*   
*   Revision 1.0  85/09/23  14:20:30  kodiak
*   added to rcs for updating in version 1
*   
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ables.i"
	INCLUDE		"exec/interrupts.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/errors.i"
	INCLUDE		"exec/strings.i"
	INCLUDE    	"exec/execbase.i"


	INCLUDE		"hardware/cia.i"
	INCLUDE		"devices/timer.i"
	INCLUDE		"devices/parallel.i"
	INCLUDE		"libraries/dosextens.i"

	INCLUDE		"macros.i"
	INCLUDE		"prtbase.i"
	INCLUDE		"printer.i"

PRINTER_DEVICE		EQU	1
	INCLUDE		"localestr/devs.i"

*------ Imported Names -----------------------------------------------

        XREF		_intena
	XREF		_IOPQUERY	;IOEXTPar struct for Query

*------ Imported Functions -------------------------------------------

	XREF_EXE	Forbid
	XREF_EXE	GetMsg
	XREF_EXE	Permit
	XREF_EXE	Wait
	XREF_EXE	WaitIO
	XREF_EXE	CheckIO
	XREF_EXE	DoIO
	XREF		_SysBase

	XREF_ITU	AutoRequest
	XREF		_IntuitionBase

	XREF		_ciab

	XREF		_PD
	XREF		_PED
	XREF		_IOR
	XREF		_OpenTask


	XREF		_TroubleNotify	;requester.asm

*------ Exported Functions -------------------------------------------

	XDEF		PWrite
	XDEF		_PWrite
	XDEF		_PBothReady
	XDEF		cancel
	XDEF		abortTimeout

******* printer.device/PWrite ****************************************
*
*   NAME
*	PWrite -- internal write to printer port
*
*   SYNOPSIS
*	error = (*PrinterData->pd_PWrite)(buffer, length);
*	D0                                  A0      D0
*
*   FUNCTION
*	PWrite writes 'length' bytes directly to the printer.  This
*	function is generally called by printer drivers to send
*	their buffer(s) to the printer.
*
*	This function is accessed by referencing off the PrinterData (PD)
*	structure.  Below is a code fragment to show how to do get access
*	to a pointer to the PrinterData (PD) structure.
*
*	#include <exec/types.h>
*	#include <devices/printer.h>
*	#include <devices/prtbase.h>
*
*	struct IODRPReq PReq;
*	struct PrinterData *PD;
*	struct PrinterExtendedData *PED;
*
*	/* open the printer device (any version); if it opened... */
*	if (OpenDevice("printer.device", 0, &PReq, 0) == NULL) {
*
*	    /* get pointer to printer data structure */
*	    PD = (struct PrinterData *)PReq.io_Device;
*
*	    /* write something directly to the printer */
*	    (*PD->pd_PWrite)("Hello world\n", 12);
*
*	    CloseDevice(&PReq); /* close the printer device */
*	}
*
**********************************************************************
_PWrite:				; 'C' entry point
		MOVE.L	4(A7),A0	; buffer
		MOVE.L	8(A7),D0	; length
		BSR.S	PWrite		; perform the write
		RTS

PWrite:					; 'Assembly' entry point
		TST.L	D0		; null length?
		BEQ.S	wEnd2		; yes, so exit (returning 0)
		MOVE.L	A6,-(A7)
		MOVE.L	_PD,A6
		MOVEM.L	D0/A0,-(A7)
		BSR.S	pOneReady	; wait for 1 of the io requests
		TST.B	D0		; to become available.
		BNE.S	error1		; timeout cancel?
		MOVEM.L	(A7)+,D0/A0	; yes, so error out.
		BSET	#PB_IOR0,pd_Flags(A6) ; is req0 available?
		BNE.S	checkIOR1	; no, so use req1.
		LEA	pd_IOR0(A6),A1	; yes, use req0.
					; (already flagged as in use)

loadIOR:	; sendio off a request to write to the ser/par port.
		MOVE.L	D0,IO_LENGTH(A1)
		MOVE.L	A0,IO_DATA(A1)
		MOVE.W	#CMD_WRITE,IO_COMMAND(A1)
		BSR	sendIO
		BSR.S	pOneReady
wEnd:
		MOVE.L	(A7)+,A6
wEnd2:
		RTS

error1:
		ADDQ.L	#8,A7
		BRA.S	wEnd

checkIOR1:	; use req1.
		BSET	#PB_IOR1,pd_Flags(A6)	; flag that req1 is in use.
		LEA	pd_IOR1(A6),A1		; use req1.
		BRA.S	loadIOR


;---------------------------------------------------------------------
pOneReady:	; wait for 1 of the io requests to become available
		MOVE.L	_IOR,A0
		MOVEQ	#0,D0
		MOVE.B	IO_ERROR(A0),D0
		BNE.S	oRts
		MOVE.B	pd_Flags(A6),D0
		NOT.B	D0
		ANDI.B	#PF_IOR0!PF_IOR1,D0
		BEQ.S	waitOneReady
		MOVEQ	#0,D0
oRts:
		RTS

waitOneReady:
		BSR	postTimeout
reWaitOne:
		LINKEXE	Forbid
		MOVEQ	#0,D0
		MOVE.B	pd_IORPort+MP_SIGBIT(A6),D1
		BSET	D1,D0
		LINKEXE	Wait
		LINKEXE	Permit
		MOVE.L	pd_IORPort+MP_MSGLIST(A6),A0
		TST.L	(A0)		; check for false alarm
		BEQ.S	reWaitOne
	    ;------ See which output I/O request has completed
getOnePort:
		LEA	pd_IORPort(A6),A0
		LINKEXE	GetMsg			;get msg
		TST.L	D0			;is it null?
		BEQ.S	pOneReady		;yes

oCheckIOR0:
		LEA	pd_IOR0(A6),A1		;get req1
		CMP.L	A1,D0			;req1 msg?
		BNE.S	oCheckIOR1		;no
		BCLR	#PB_IOR0,pd_Flags(A6)	;yes, clear flag
		BSR.S	abortTimeout		;abort timeout request
		BRA.S	getOnePort		;check for more msgs

oCheckIOR1:
		LEA	pd_IOR1(A6),A1		;get req2
		CMP.L	A1,D0			;req2 msg?
		BNE.S	oCheckTIOR		;no
		BCLR	#PB_IOR1,pd_Flags(A6)	;yes, clear flag
		BSR.S	abortTimeout		;abort timeout request
		BRA.S	getOnePort		;check for more msgs

oCheckTIOR:
		LEA	pd_TIOR(A6),A1		;get timer req
		CMP.L	A1,D0			;timer msg?
		BNE.S	oCheckErr		;no
		BSR	timeoutAlert		;yes, put up requestor
		TST.L	D0			;retry?
		BEQ.S	getOnePort		;yes, check for more msgs
		RTS				;no, exit

oCheckErr:
		BSR.S	abortTimeout		;abort timeout request
		BRA	pOneReady


;---------------------------------------------------------------------
postTimeout:
		LEA	pd_TIOR(A6),A1
		MOVE.W	#TR_ADDREQUEST,IO_COMMAND(A1)
		MOVE.L	_PED,A0
		MOVE.L	ped_TimeoutSecs(A0),IOTV_TIME+TV_SECS(A1)
		CLR.L	IOTV_TIME+TV_MICRO(A1)
sendIO:
		CLR.B	IO_FLAGS(A1)
		MOVE.L	A6,-(A7)
		MOVE.L	IO_DEVICE(A1),A6
		JSR	DEV_BEGINIO(A6)
		MOVE.L	(A7)+,A6
		RTS


;---------------------------------------------------------------------
abortTimeout:
		LEA	pd_TIOR(A6),A1		; get ptr to io req.
; this code essentially does a CheckIO.  It is redundant since AbortIO
; on an already aborted request does nothin.  HOWEVER, THERE MUST BE SOME
; MAGIC GOING ON HERE SINCE IF WE COMMENT OUT THE NEXT THREE LINES
; (WHICH SHOULD BE OK) THE PRINTING HANGS AT SOME POINT.  THIS SHOULD BE
; INVESTIGATED SOME DAY.  PERHAPS ITS A BUG IN THE TIMER DEVICE.
		CMPI.B	#NT_REPLYMSG,LN_TYPE(A1)	; is this a reply msg?
		BNE.S	abortIO			; no, so abort the request.
		RTS
abortIO:
		MOVEM.L	A1/A6,-(A7)		; save reqs used.
		MOVE.L  IO_DEVICE(A1),A6	; get ptr to device.
		JSR DEV_ABORTIO(A6)	; call device specific AbortIO routine.
		MOVEM.L	(A7)+,A1/A6		; restore regs used.
; next 5 lines is for serial device bug
; they have been commented out as of V1.3 Gamma 15 as the new serial device
; (for V1.3) no longer needs this kludge
;		MOVE.L	A1,-(A7)		; save reqs used.
;		LINKEXE	CheckIO			; check on the i/o.
;		MOVE.L	(A7)+,A1		; restore regs used.
;		TST.L	D0			; is the i/o still active?
;		BEQ	10$	; yes, so don't wait for abort to finish.

		LINKEXE	WaitIO			; wait for the abort to finish.
10$		RTS

;---------------------------------------------------------------------
_PBothReady:
		MOVE.L	A6,-(A7)
chkBothReady:					; added for bug fix  -Val
		MOVE.L	_IOR,A0
		MOVEQ	#0,D0
		MOVE.B	IO_ERROR(A0),D0
		BNE.S	bRts
		MOVE.L	_PD,A6
; chkBothReady:					; removed for bug fix  -Val
		MOVE.B	pd_Flags(A6),D0
		ANDI.B	#PF_IOR0!PF_IOR1,D0
		BNE.S	waitBothReady
		MOVEQ	#0,D0
bRts:
		MOVE.L	(A7)+,A6
		RTS

waitBothReady:
		BSR	postTimeout
reWaitBoth:
		LINKEXE	Forbid
		MOVEQ	#0,D0
		MOVE.B	pd_IORPort+MP_SIGBIT(A6),D1
		BSET	D1,D0
		LINKEXE	Wait
		LINKEXE	Permit
		MOVE.L	pd_IORPort+MP_MSGLIST(A6),A0
		TST.L	(A0)		; check for false alarm
		BEQ.S	reWaitBoth
	    ;------ See which output I/O request has completed
getBothPort:
		LEA	pd_IORPort(A6),A0
		LINKEXE	GetMsg
		TST.L	D0
		BEQ.S	chkBothReady

bCheckIOR0:
		LEA	pd_IOR0(A6),A1
		CMP.L	A1,D0
		BNE.S	bCheckIOR1
		BCLR	#PB_IOR0,pd_Flags(A6)
		BSR	abortTimeout
		BRA.S	getBothPort

bCheckIOR1:
		LEA	pd_IOR1(A6),A1
		CMP.L	A1,D0
		BNE.S	bCheckTIOR
		BCLR	#PB_IOR1,pd_Flags(A6)
		BSR	abortTimeout
		BRA.S	getBothPort

bCheckTIOR:
		LEA	pd_TIOR(A6),A1
		CMP.L	A1,D0
		BNE.S	bCheckErr
		BSR	timeoutAlert
		TST.L	D0
		BEQ.S	getBothPort
		BRA	bRts

bCheckErr:
		BSR	abortTimeout
		BRA	chkBothReady


;---------------------------------------------------------------------
timeoutAlert:
;		BRA	timeout	; kludge: assume user hit cancel.

;                         ^
;			  |
; This should have been pointing to "cancel:", not "timeout:". This was the
; cause of the parallel requests not getting aborted, in the special A3000
; launch version of the printer device.


		;------ AutoRequest(Window,Body,PText,NText,PFlag,NFlag,W,H)
		;------            (  A0,   A1,   A2,   A3,   D0,  D1, D2,D3)
		MOVEM.L	D2/D3/A2/A3,-(A7)
		CMP.B	#SERIAL_PRINTER,pd_Preferences+pf_PrinterPort(A6)
; David Berezowski - Sept. 5/89
; - no change was made to the code here but a note of warning to be heeded.
;   the 'netprint.device' kludge assumes that this device is a variation
;   on a parallel port driver.  Thus when this code branches to
;   'readParallelStatus' (if 'pd_Preferences+pf_PrinterPort(A6)' is not
;   '#SERIAL_PRINTER') it is correct.  ie. we want to use the read parallel
;   port status code when netprint.device is selected.

		BNE.S	readParallelStatus

		MOVEQ	#MSG_PRI_DEV_UNKNOWN-MSG_PRI_DEV_OFFLINE,d1


		BRA.S	constructMessage

readParallelStatus:
****************************************************************************
**
** The printer.device use to read the parallel hardware directly
** 
** Replaced with code to send a query request to the printer.device, or
** netprint.device.  We have to assume that netprint.device supports a
** query function like this one.  See query.c for validation of
** this assumption (QUERY of the printer.device queries serial.device,
** or parallel/netprint.device.
**

		LEA	_IOPQUERY,A1
		LEA	pd_IOR0(A6),A0
		MOVE.L	IO_DEVICE(A0),IO_DEVICE(A1)
		MOVE.L	IO_UNIT(A0),IO_UNIT(A1)

		MOVE.W	#PDCMD_QUERY,IO_COMMAND(A1)
		CLR.B	IO_PARSTATUS(A1)	;in case it fails

		LINKEXE	DoIO

		LEA	_IOPQUERY,A1

		MOVEQ	#00,D1
		MOVE.B	IO_PARSTATUS(A1),D1

	;-- Mask out all but the SEL & PAPER OUT bits

	IFNE	CIAF_PRTRSEL-4
	FAIL	"CIAF_PRTRSEL not bit 2, recode"
	ENDC

	IFNE	CIAF_PRTRPOUT-2
	FAIL	"CIAF_PRTRPOUT not bit 1, recode"
	ENDC


		AND.B	#CIAF_PRTRSEL+CIAF_PRTRPOUT,D1

		LSR.W	#1,D1			;move to bits 1 and 0

constructMessage:
		MOVE.L	_OpenTask,A0
		CMP.B	#NT_PROCESS,LN_TYPE(A0)
		BNE.S	noProcess
		MOVE.L	pr_WindowPtr(A0),A0
		CMPA	#-1,A0		;-1 as WindowPtr ?
		BNE.S	itPatch		;no, give the requester
		CLR.L  	D0		;clear
		BRA.S	itPatch1	;as if cancel were hit

noProcess:
		SUBA.L	A0,A0
itPatch:

		;window ptr in A0, message # in D1

		BSR	_TroubleNotify

itPatch1:
		MOVEM.L	(A7)+,D2/D3/A2/A3
		TST.L	D0			;cancel (0-yes, 1-no)
		BNE.S	continue		;no
cancel:
		BCLR	#PB_IOR0,pd_Flags(A6)	;is req1 in use?
		BEQ.S	aCheckIOR1		;no
		LEA	pd_IOR0(A6),A1		;yes, get ptr in A1
		BSR	abortIO			;abort req1

aCheckIOR1:
		BCLR	#PB_IOR1,pd_Flags(A6)	;is req2 in use?
		BEQ.S	timeout			;no
		LEA	pd_IOR1(A6),A1		;yes, get ptr in A1
		BSR	abortIO			;abort req2

timeout:
		MOVEQ	#PDERR_CANCEL,D0
		RTS

continue:
		MOVEQ	#0,D0
		RTS

		END
