*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		Device Driver Support		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,91 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
*   Source Control:
*
*	$Id: devices.asm,v 39.1 93/03/05 11:10:26 mks Exp $
*
*	$Log:	devices.asm,v $
* Revision 39.1  93/03/05  11:10:26  mks
* Added kludge device names...
* 
* Revision 39.0  91/10/15  08:26:39  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"

    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"memory.i"
    INCLUDE	"lists.i"
    INCLUDE	"ports.i"
    INCLUDE	"libraries.i"
    INCLUDE	"execbase.i"
    INCLUDE	"ables.i"
    INCLUDE	"errors.i"
    INCLUDE	"io.i"

    INCLUDE	"calls.i"
    LIST

;****** Exported *****************************************************

	XDEF	AddDevice
	XDEF	OpenDevice
	XDEF	CloseDevice
	XDEF	DoIO
	XDEF	SendIO
	XDEF	WaitIO
	XDEF	CheckIO
	XDEF	AbortIO
	XDEF	CreateIORequest
	XDEF	DeleteIORequest
	XDEF	CreateMsgPort
	XDEF	DeleteMsgPort


;****** Imported Globals *********************************************

    INT_ABLES
    TASK_ABLES

    EXTERN_BASE DeviceList
    EXTERN_BASE ThisTask


;****** Imported Functions *******************************************

    EXTERN_CODE AddNode 		* #DEPENDENCY#
    EXTERN_CODE RemLibrary		* #DEPENDENCY#

    EXTERN_SYS	AllocMem
    EXTERN_SYS	FreeMem
    EXTERN_SYS	Wait
    EXTERN_SYS	PutMsg
    EXTERN_SYS	GetMsg
    EXTERN_SYS	ReplyMsg
    EXTERN_SYS	SumLibrary
    EXTERN_SYS	SendIO
    EXTERN_SYS	WaitIO
    EXTERN_SYS	AllocSignal
    EXTERN_SYS	FreeSignal
    EXTERN_SYS	FindName




*****o* exec.library/AddDevice *********************************************
*
*   NAME
*	AddDevice -- add a device to the system
*
*   SYNOPSIS
*	AddDevice(device)
*		  A1
*
*   FUNCTION
*	This function adds a new device to the system, making it
*	available to other programs.  The device should be ready to be
*	opened at this time.
*
*   INPUTS
*	device - pointer to a properly initialized device node
*
*   SEE ALSO
*	RemDevice, OpenDevice, CloseDevice, MakeLibrary
*
**********************************************************************
*	Could be made common with AddLibrary

AddDevice:
	    LEA     DeviceList(A6),A0
	    BSR     AddNode
	    JMPSELF SumLibrary	;JSR/RTS


*****o* exec.library/RemDevice *********************************************
*
*   NAME
*	RemDevice -- remove a device from the system
*
*   SYNOPSIS
*	error = RemDevice(device)
*	D0		  A1
*
*   FUNCTION
*	This function removes an existing device from the system.
*	This function deletes the device from the device name list,
*	so no new opens can occur.
*
*   INPUTS
*	device - pointer to a device node
*
*   RESULTS
*	error - zero if successful, else an error is returned
*
*   SEE ALSO
*	AddDevice
*
**********************************************************************
*
*	Two options are recommended -- either delaying the actual
*	expunging of the device until all users have closed it, or
*	replacing all entry points will "null" routines that always
*	return errors.
*

;
;RemDevice moved to libraries.asm
;
;RemDevice: BRA     RemLibrary


*****o* exec.library/OpenDevice ********************************************
*
*   NAME
*	OpenDevice -- gain access to a device
*
*   SYNOPSIS
*	error = OpenDevice(devName, unitNumber, iORequest, flags)
*	D0		   A0	    D0		A1	   D1
*
*   FUNCTION
*	This function opens the named device/unit and initializes
*	the given I/O request block.
*
*   INPUTS
*	devName - requested device name
*
*	unitNumber - the unit number to open on that device.  The
*		format of the unit number is device specific.
*
*	iORequest - the I/O request block to be returned with
*		appropriate fields initialized.
*
*	flags - additional driver specific information.  This is
*		sometimes used to request opening a device with
*		exclusive access.
*
*   RESULTS
*	error - zero if successful, else an error is returned
*
*   SEE ALSO
*	CloseDevice
*
**********************************************************************

	;A0-name A1-request D0-unitNumber D1-flags

OpenDevice: MOVE.L  A2,-(SP)
	    MOVE.L  A1,A2
	    MOVE.B  #IOERR_OPENFAIL,IO_ERROR(A2)	;preset error

	    MOVEM.L D0/D1,-(SP)         * save for later

*	    ------- find the device node:
	    MOVE.L  A0,A1		* name string
;
; Hack: For ROM SPACE the NULL device name is timer.device...
;                            1 device name is input.device...
;
		move.l	a1,d0		; Check for NULL...
		bne.s	od_NotTimer	; If not timer, we skip...
		lea	od_Timer(pc),a1	; timer.device name...
od_NotTimer:	subq.l	#1,d0		; Check for 1...
		bne.s	od_NotInput	; If not input, we skip...
		lea	od_Input(pc),a1	; input.device name...
od_NotInput:
;
	    LEA     DeviceList(A6),A0
	    FORBID
od_next:    MOVE.L  A1,-(SP)
	    JSRSELF FindName		* search for device (via vector)
	    MOVE.L  (SP)+,A1
	    MOVE.L  D0,A0
	    MOVE.L  A0,IO_DEVICE(A2)
	    BEQ.S   od_error
	    CLR.B   IO_ERROR(A2)
*	    ------- end find

od_error:   MOVEM.L (SP)+,D0/D1

	    TST.B   IO_ERROR(A2)
	    BNE.S   od_error2

*	    ------- call the open routine
	    CLR.L   IO_UNIT(A2)
	    MOVE.L  A2,A1
	    LINKLIB LIB_OPEN,A0
*	    ------- return the error field:

od_error2:  MOVE.B  IO_ERROR(A2),D0
	    EXT.W   D0
	    EXT.L   D0
	    BEQ.S   od_exit
	    CLR.L   IO_DEVICE(A2)       ;:bryce - zero IO_DEVICE on fail
od_exit:    MOVE.L  (SP)+,A2
	    JMP_PERMIT
;
od_Timer:	dc.b	'timer.device',0
od_Input:	dc.b	'input.device',0

*****o* exec.library/CloseDevice *******************************************
*
*   NAME
*	CloseDevice -- conclude access to a device
*
*   SYNOPSIS
*	CloseDevice(iORequest)
*		    A1
*
*   FUNCTION
*	This function informs the system that access to a
*	device/unit previously opened has been concluded.  The
*	device may perform certain house-cleaning operations.  The
*	I/O request structure is now free to be recycled.
*
*   INPUTS
*	iORequest - pointer to an I/O request structure
*
*   SEE ALSO
*	OpenDevice
*
**********************************************************************
*
*	I'd like to trash IO_DEVICE after close, but it breaks too
*	many programs.  Sigh, another one for IO_Torture.
*

CloseDevice:
		FORBID
		  MOVE.L  A6,-(SP)
		  MOVE.L  IO_DEVICE(A1),A6
		  MOVE.L  A6,D0
		  BEQ.S   cd_NoClose	;Set D0 for exit (ramlib SegList)
		  JSR	  LIB_CLOSE(A6)
cd_NoClose:	  MOVE.L  (SP)+,A6
		JMP_PERMIT


*****o* exec.library/SendIO ************************************************
*
*   NAME
*	SendIO -- initiate an I/O command
*
*   SYNOPSIS
*	SendIO(iORequest)
*	       A1
*
*   FUNCTION
*	This function requests the device driver to initiate the
*	command specified in the given I/O request.  The device
*	will return regardless of whether the I/O has completed.
*
*   INPUTS
*	iORequest - pointer to an I/O request
*
*   SEE ALSO
*	DoIO, WaitIO
*
********************************************************************

SendIO:
	CLR.B	IO_FLAGS(A1)
	BEGINIO
	RTS


*****o* exec.library/DoIO **************************************************
*
*   NAME
*	DoIO -- perform an I/O command and wait for completion
*
*   SYNOPSIS
*	error = DoIO(iORequest)
*	D0	     A1
*
*   FUNCTION
*	This function requests a device driver to perform the I/O
*	command specified in the I/O request.  This function will
*	always block until the I/O request is completed.
*
*   INPUTS
*	iORequest - pointer to a properly initialized I/O request
*
*   RESULTS
*	error - see WaitIO
*
*   SEE ALSO
*	SendIO, WaitIO
*
**********************************************************************
*
*   NOTE: Replacing WaitIO softvector requires DoIO replacement
*

DoIO:
	    MOVE.L  A1,-(SP)
	    MOVE.B  #IOF_QUICK,IO_FLAGS(A1)
	    BEGINIO
	    MOVE.L  (SP)+,A1
*	    FALL    THROUGH


*****o* exec.library/WaitIO ************************************************
*
*   NAME
*	WaitIO -- wait for completion of an I/O request
*
*   SYNOPSIS
*	error = WaitIO(iORequest)
*	D0	       A1
*
*   FUNCTION
*	This function waits for the specified I/O request to
*	complete.  If the I/O has already completed, this function
*	will return immediately.
*
*	This function should be used with care, as it does not
*	return until the I/O request completes; if the I/O never
*	completes, this function will never return, and your task
*	will hang.  If this situation is a possibility, it is
*	safer to use the Wait function, which will return when
*	any particular signal is received.  This is how I/O timeouts
*	can be properly handled.
*
*   INPUTS
*	iORequest - pointer to an I/O request block
*
*   RESULTS
*	error - zero if successful, else an error is returned
*
*   SEE ALSO
*	SendIO
*
**********************************************************************

WaitIO:
*	    ------- is it a quick I/O? (totally synchronous)
	    BTST.B  #IOB_QUICK,IO_FLAGS(A1)
	    BNE.S   wi_done

	    MOVE.L  A2,-(SP)
	    MOVE.L  A1,A2

*	    ------- wait for a reply message:
	    MOVE.L  MN_REPLYPORT(A2),A0
	    MOVE.B  MP_SIGBIT(A0),D1
	    CLEAR   D0
	    BSET    D1,D0
	    DISABLE
wi_wait:
	    CMP.B   #NT_REPLYMSG,LN_TYPE(A2)
	    BEQ.S   wi_complete
	    JSRSELF Wait (D0)           * D0 not affected
	    BRA.S   wi_wait
wi_complete:
*	    ------- unlink the IO request from the port:
	    MOVE.L  A2,A1
	    REMOVE
	    IFNE    TORTURE_TEST
		move.l	#$DEAD0031,LN_SUCC(a2)
		move.l	#$DEAD0041,LN_PRED(a2)
	    ENDC
	    ENABLE

	    MOVE.L  A2,A1
	    MOVE.L  (SP)+,A2

wi_done:
	    MOVE.B  IO_ERROR(A1),D0
	    EXT.W   D0
	    EXT.L   D0
	    RTS

*****o* exec.library/CheckIO ************************************************
*
*   NAME
*	CheckIO -- get the IO request status
*
*   SYNOPSIS
*	result = CheckIO(iORequest)
*	D0	       A1
*
*   FUNCTION
*	This function determines the current state of an I/O
*	request and returns FALSE if the I/O has not yet completed.
*	This function effectively hides the internals of the I/O
*	completion mechanism.
*
*	If the I/O has completed, CheckIO will not remove the
*	returned IORequest from the reply port.  This should
*	be performed with Remove.
*
*	This function SHOULD NOT be used to busy loop, waiting
*	for an IO to complete.
*
*   INPUTS
*	iORequest - pointer to an I/O request block
*
*   RESULTS
*	result - null if I/O is still in progress.  Otherwise
*		D0 points to the IORequest block.
*
**********************************************************************

CheckIO:
*	    ------- is it a quick I/O? (totally synchronous)
	    BTST.B  #IOB_QUICK,IO_FLAGS(A1)
	    BNE.S   gi_doneMsg
	    CMP.B   #NT_REPLYMSG,LN_TYPE(A1)
	    BEQ.S   gi_doneMsg
	    CLEAR   D0		;still pending
	    RTS
gi_doneMsg:
	    MOVE.L  A1,D0	;already done
	    RTS

****------- unlink the IO request from the port:
****gi_doneMsg:
****	    MOVE.L  A1,D0
****	    DISABLE
****	    REMOVE			* will leave a pending signal
****	    ENABLE
****	    RTS

*****o* exec.library/AbortIO ***************************************************
*
*   NAME
*	AbortIO - attempt to abort an in-progress I/O request
*
*   SYNOPSIS
*	error = AbortIO(iORequest)
*	D0		A1
*
*   FUNCTION
*	Attempt to abort an I/O request started previously with
*	SendIO.  Depending on the device and the state of the
*	request, it may not be possible to abort a given I/O
*	request.
*
*	If for some reason the device cannot abort the request,
*	it should return an error code in D0.
*
*   INPUTS
*	iORequest - pointer to an I/O request block.
*
*   RESULTS
*	error - zero if successful, else an error is returned
*
*   SEE ALSO
*	SendIO, WaitIO
*
********************************************************************************

AbortIO:
	    ABORTIO
	    RTS

*	    BSET.B  #IOB_ABORT,IO_FLAGS(A1) * indicate attempt was made
*	    DISABLE
*	    BTST.B  #IOB_QUEUED,IO_FLAGS(A1)
*	    BEQ.S   ab_device
*	    BCLR.B  #IOB_ACTIVE,IO_FLAGS(A1)
*	    MOVE.L  A1,D1
*	    REMOVE
*	    ENABLE
*	    MOVE.L  D1,A1
*	    JSRSELF ReplyMsg (A1)
*	    CLEAR   D0
*	    BRA.S   ab_exit
*ab_device:
*	    LINKLIB DEV_ABORTIO,IO_DEVICE(A1) * returns D0
*	    ENABLE
*ab_exit:
*	    RTS



*****o* exec.library/CreateIORequest *****************************************
*
*   NAME
*	CreateIORequest() -- create an IORequest structure
*
*   SYNOPSIS
*	ioReq = CreateIORequest( ioReplyPort, size );
*	                         A0           D0
*
*	struct IORequest *CreateIORequest(struct MsgPort *, ULONG);
*
**********************************************************************
*
*	A0 - memory
*	D2 - Size
*	D3 - Port
*

CreateIORequest:
		movem.l d2/d3,-(sp)		;PUSHM  d2/d3
		move.l	d0,d2			;[D2-Size]
		move.l	a0,d3			;[D3-Port]
		beq.s	cir_noport
			move.l	#MEMF_CLEAR!MEMF_PUBLIC,d1
			JSRSELF AllocMem			;(D0/D1)
			move.l	d0,a0
			tst.l	d0
			beq.s	cir_fail
				;Mark as "finished"
				move.b	#NT_REPLYMSG,LN_TYPE(a0)
				move.l	d3,MN_REPLYPORT(a0)
				;Size of memory alloc (WORD!)
				move.w	d2,MN_LENGTH(a0)
cir_noport:
cir_fail:	move.l	a0,d0
		movem.l (sp)+,d3/d2		;POPM
		rts				;D0=return


*****o* exec.library/DeleteIORequest **************************************
*
*   NAME
*	DeleteIORequest() - Free a request made by CreateIORequest()
*
*   SYNOPSIS
*	DeleteIORequest( ioReq );
*	                 a0
*
*	void DeleteIORequest(struct IORequest *);
*
*   FUNCTION
*	Frees up an IO request as allocated by CreateIORequest().
*
*   INPUTS
*	ioReq - A pointer to the IORequest block to be freed, or NULL.
*		This function used the ln_Length field to determine how
*		much memory to free.
*
*   SEE ALSO
*	CreateIORequest()
*
*************************************************************************

DeleteIORequest:
		move.l	a0,d0
		beq.s	dir_exit
		moveq	#-1,d0			;destroy old request
		move.l	d0,LN_SUCC(a0)
		move.l	d0,IO_DEVICE(a0)	;:TODO: Null.device
		moveq	#0,d0
		move.w	MN_LENGTH(a0),d0
		move.l	a0,a1
		JSRSELF	FreeMem ;(a1)(d0)	;exit
dir_exit:	rts



*****o* exec.library/CreateMsgPort ****************************************
*
*   NAME
*	CreateMsgPort - Allocate and initialize a new message port  (V36)
*
*   SYNOPSIS
*	CreateMsgPort()
*
*	struct MsgPort * CreateMsgPort(void);
*
*   FUNCTION
*	Allocates and initializes a new message port.  The message list
*	of the new port will be prepared for use (via NewList).  A signal
*	bit will be allocated, and the port will be set to signal your
*	task when a message arrives (PA_SIGNAL).
*
*	You *must* use DeleteMsgPort() to delete ports created with
*	CreateMsgPort()!
*
*   RESULT
*	MsgPort - A new MsgPort structure ready for use, or NULL if out of
*		memory or signals.  If you wish to add this port to the public
*		port list, fill in the ln_Name and ln_Pri fields, then call
*		AddPort().  Don't forget RemPort()!
*
*   SEE ALSO
*	DeleteMsgPort(), exec/AddPort(), exec/ports.h, amiga.lib/CreatePort()
*
*****************************************************************************
*
*	(sp) - port address
*	 A0  - port address
*

CreateMsgPort:	moveq	#MP_SIZE,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		JSRSELF AllocMem
		move.l	d0,-(sp)			;[Stack has pointer]
		beq.s	cmp_NoMemory

		moveq	#-1,d0
		JSRSELF AllocSignal
		;[D0-so far unchecked signal bit]

		move.l	(sp),a0				;get copy of pointer
		move.b	#NT_MSGPORT,LN_TYPE(a0)
		move.b	#PA_SIGNAL,MP_FLAGS(a0)		;Port Action: signal
		move.b	d0,MP_SIGBIT(a0)		;signal bit number
		bmi.s	cmp_NoSignal
		move.l	ThisTask(a6),MP_SIGTASK(a0)	;task ID to be signaled
		lea.l	MP_MSGLIST(a0),a1
		NEWLIST a1
cmp_NoMemory:
cmp_Exit:	move.l	(sp)+,d0	;return value (see above)
		rts			;exit

cmp_NoSignal:	moveq	#MP_SIZE,d0
		move.l	a0,a1		;get copy of pointer
		JSRSELF FreeMem		;(a1,d0)
		clr.l	(sp)		;set return code to NULL
		bra.s	cmp_Exit


*****o* exec.library/DeleteMsgPort *******************************************
*
*   NAME
*	DeleteMsgPort - Free a message port created by CreateMsgPort
*
*   SYNOPSIS
*	DeleteMsgPort(msgPort)
*		      a0
*
*	void DeleteMsgPort(struct MsgPort *);
*
*   FUNCTION
*	Frees a message port created by CreateMsgPort.	All messages that
*	may have been attached to this port must have already been
*	replied to.
*
*   INPUTS
*	msgPort - A message port.  NULL for no action.
*
*   SEE ALSO
*	CreateMsgPort()
*
******************************************************************************
*	A0 & A1 - msgPort at different times

DeleteMsgPort:	move.l	a0,-(sp)                ;[stack has pointer]
		beq.s	dmp_NullAction
		moveq	#0,d0
		move.b	MP_SIGBIT(a0),d0
		JSRSELF FreeSignal
		move.l	(sp),a1                 ;register switch
		moveq	#-1,d0
		move.l	d0,MP_MSGLIST+LH_HEAD(a1) ;rape, pillage, destroy!
		move.l	d0,LN_SUCC(a1)            ; make request unusable!
		moveq	#MP_SIZE,d0
		JSRSELF FreeMem
dmp_NullAction: addq.l	#4,sp
		rts				;exit

	END
