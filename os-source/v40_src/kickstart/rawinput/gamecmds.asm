	TTL	'$Id: gamecmds.asm,v 36.10 91/02/12 10:28:01 darren Exp $
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*	gameport device commands
*
*   Source Control
*   --------------
*   $Id: gamecmds.asm,v 36.10 91/02/12 10:28:01 darren Exp $
*
*   $Locker:  $
*
*   $Log:	gamecmds.asm,v $
*   Revision 36.10  91/02/12  10:28:01  darren
*   Fix spelling error - AUTODOCS
*   
*   Revision 36.9  90/04/13  12:43:32  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.8  90/04/09  09:43:51  kodiak
*   avoid implicit $4 reference by converting DISABLE/ENABLE to NOFETCH form
*   
*   Revision 36.7  90/04/02  12:56:29  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.6  90/01/29  09:29:17  kodiak
*   rename [Gg]ameportTrigger to [Gg]amePortTrigger in autodocs
*   
*   Revision 36.5  90/01/04  13:27:22  kodiak
*   polish autodoc headers
*   
*   Revision 36.4  89/11/10  13:39:03  kodiak
*   make gameport exclusive access
*   
*   Revision 36.3  89/07/12  13:27:42  kodiak
*   have input task ReadEvent multiple events from gameport
*   (and have it work)
*   
*   Revision 36.2  88/11/03  12:35:13  kodiak
*   use AUTOINIT in ResidentTag
*   
*   Revision 35.0  87/10/26  11:30:30  kodiak
*   initial from V34, but w/ stripped log
*   
*   
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/strings.i"
	INCLUDE		"exec/interrupts.i"
	INCLUDE		"exec/ables.i"
	INCLUDE		"exec/errors.i"

	INCLUDE		"hardware/custom.i"

	INCLUDE		"devices/timer.i"
	INCLUDE		"devices/inputevent.i"

	INCLUDE		"gameport.i"

	INCLUDE		"macros.i"
	INCLUDE		"gpdata.i"


*------ Imported Names -----------------------------------------------

	XREF		_custom
	XREF		_intena


*------ Imported Functions -------------------------------------------

	XREF_PGR	AllocPotBits
	XREF_PGR	FreePotBits

	XREF		EndCommand


*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

	XDEF		GDClear
	XDEF		GDReadEvent
	XDEF		GDAskCType
	XDEF		GDSetCType
	XDEF		GDAskTrigger
	XDEF		GDSetTrigger

	XDEF		GDCBClear
	XDEF		GDCBReadEvent
	XDEF		GDCBAskCType
	XDEF		GDCBSetCType
	XDEF		GDCBAskTrigger
	XDEF		GDCBSetTrigger

*****i* gameport.device/CMD_CLEAR ************************************
*
*   NAME
*	Clear - clear gameport input buffer
*
*   FUNCTION
*	Remove from the input buffer any gameport reports waiting to
*	satisfy read requests.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_CLEAR
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
GDCBClear	EQU	-1

GDClear:
		move.l	IO_UNIT(a1),a0
		move.l	dd_ExecBase(a6),a1
		DISABLE	a1,NOFETCH
		clr.w	gu_BufHead(a0)
		clr.w	gu_BufTail(a0)
		ENABLE	a1,NOFETCH
		move.l	a3,a1		; restore IORequest
		bra	EndCommand


******* gameport.device/GPD_READEVENT ********************************
*
*   NAME
*	GPD_READEVENT -- Return the next game port event.
*
*   FUNCTION
*	Read game port events from the game port and put them in the
*	data area of the iORequest.  If there are no pending game port
*	events, this command will not be satisfied, but if there are
*	some events, but not as many as can fill IO_LENGTH, the
*	request will be satisfied with those currently available.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	GPD_READEVENT
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	the size of the io_Data area in bytes: there
*			are sizeof(inputEvent) bytes per input event.
*	io_Data		a buffer area to fill with input events.  The
*			fields of the input event are:
*	    ie_NextEvent
*			links the events returned
*	    ie_Class
*			is IECLASS_RAWMOUSE
*	    ie_SubClass
*			is 0 for the left, 1 for the right game port
*	    ie_Code 
*			contains any gameport button reports.  No
*			report is indicated by the value 0xff.
*	    ie_Qualifier 
*			only the relative and button bits are set
*	    ie_X, ie_Y
*			the x and y values for this report, in either
*			relative or absolute device dependent units.
*	    ie_TimeStamp 
*			the delta time since the last report, given
*			not as a standard timestamp, but as the frame
*			count in the TV_SECS field.
*
*   RESULTS
*	This function sets the error field in the iORequest, and fills
*	the iORequest with the next game port events (but not partial
*	events).
*
*   SEE ALSO
*	gameport.device/SetCType, gameport.device/SetTrigger
*
**********************************************************************
GDCBReadEvent	EQU	0

GDReadEvent:
		movem.l d2-d4/a2-a4,-(a7)
		move.l	a1,a3
		move.l	IO_DATA(a3),a2
		move.l	IO_UNIT(a3),a4

		move.l	dd_ExecBase(a6),a1
		DISABLE	a1,NOFETCH

		;------ check if ReadEvent already running here
		bset	#IOB_SERVICING,IO_FLAGS(a3)
		bne	readAlreadyRunning

		;------ check for room in ReadEvent
		moveq	#ie_SIZEOF,d1
		moveq	#0,d2
		move.l	IO_LENGTH(a3),d3
		sub.l	d1,d3
		blt	readTooSmall

		;------ check for data for ReadEvent
		move.w	gu_BufHead(a4),d0
		cmp.w	gu_BufTail(a4),d0
		beq	readPending

readLoop:

		;------	grab critical data from buffer while disabled
		move.b	gu_BufQueue(a4,d0.w),ie_Code+1(a2)
		move.b	gu_BufQueue+1(a4,d0.w),ie_Qualifier(a2)
		move.l  gu_BufQueue+2(a4,d0.w),ie_X(a2)	; and ie_Y
		move.w  gu_BufQueue+6(a4,d0.w),ie_TimeStamp+2(a2)
		addq.w	#8,d0

		;------ increment buffer head
		andi.w	#(GPBUFSIZE-1),d0
		move.w	d0,gu_BufHead(a4)

		move.l	dd_ExecBase(a6),a1
		ENABLE	a1,NOFETCH

		;------ finish filling in input event
		clr.b	ie_Code(a2)		; clear high byte
		clr.b	ie_Qualifier+1(a2)	; clear low byte
		clr.w	ie_TimeStamp(a2)	; clear high word
		clr.l	ie_TimeStamp+4(a2)	; clear low long
		move.b	#IECLASS_RAWMOUSE,ie_Class(a2)
		btst    #UB_RIGHTPORT,gu_Flags(a4)
		beq.s   leftPort
		move.b  #1,ie_SubClass(a2)
		bra.s   bumpEvent
leftPort:
		clr.b   ie_SubClass(a2)

bumpEvent:
		add.l	d1,a2
		move.l	a2,ie_NextEvent-ie_SIZEOF(a2)
		add.l	d1,d2			;increment actual

		;------ check for more space in I/O request data area
		sub.l	d1,d3
		blt.s	readDone

		move.l	dd_ExecBase(a6),a1
		DISABLE	a1,NOFETCH

		;------ get next data from buffer
		move.w	gu_BufHead(a4),d0
		cmp.w	gu_BufTail(a4),d0
		beq.s	readEnableDone

		bra	readLoop


readPending:
		andi.b	#~(IOF_SERVICING!IOF_QUICK),IO_FLAGS(a3)

readAlreadyRunning:
		move.l	dd_ExecBase(a6),a1
		ENABLE	a1,NOFETCH

		movem.l (a7)+,d2-d4/a2-a4
		rts

readTooSmall:
		move.l	dd_ExecBase(a6),a1
		ENABLE	a1,NOFETCH
		moveq	#IOERR_BADLENGTH,d0
		bra.s	readEndCommand

readEnableDone:
		move.l	dd_ExecBase(a6),a1
		ENABLE	a1,NOFETCH

readDone:
*	    ;------ the destination for the read is full or read exhausted
		clr.l	ie_NextEvent-ie_SIZEOF(a2)
		move.l	d2,IO_ACTUAL(a3)
		moveq	#0,d0

readEndCommand:
		move.l	a3,a1
		bsr	EndCommand

readNextCommand:
		move.l	MP_MSGLIST(a4),a1

		movem.l (a7)+,d2-d4/a2-a4

		tst.l	(a1)
		bne	GDReadEvent

		rts


******* gameport.device/GPD_ASKCTYPE *********************************
*
*   NAME
*	GPD_ASKCTYPE -- Acquire the current game port controller type
*
*   FUNCTION
*	This command identifies the type of controller at the game
*	port, so that the signals at the port may be properly
*	interpreted.  The controller type has been set by a previous
*	SetCType.
*
*	This command always executes immediately.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	GPD_ASKCTYPE
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	at least 1
*	io_Data		the address of the byte variable for the
*			result
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	This quite simply puts the contents of the correct game port
*	variable into IO_DATA.
*
*---------------------------------------------------------------------
GDCBAskCType	EQU	-1

GDAskCType:
		move.l	IO_LENGTH(a1),d0
		subq.w	#1,d0
		bcs	lenErr

		move.l	IO_UNIT(a1),a0
		move.b	gu_Type(a0),d1
		move.l	IO_DATA(a1),a0
		move.b	d1,(a0)+
		moveq	#1,d0
		move.l	d0,IO_ACTUAL(a1)
		moveq	#0,d0
		bra	EndCommand


******* gameport.device/GPD_SETCTYPE *********************************
*
*   NAME
*	GPD_SETCTYPE -- Set the current game port controller type
*
*   FUNCTION
*	This command sets the type of device at the game port, so that
*	the signals at the port may be properly interpreted.  The port
*	can also be turned off, so that no reports are generated.
*
*	This command always executes immediately.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	GPD_SETCTYPE
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	1
*	io_Data		the address of the byte variable describing
*			the controller type, as per the equates in
*			the gameport include file
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	This stores the IO_DATA into the correct game port variable,
*	and initializes the port.
*
*---------------------------------------------------------------------
GDCBSetCType	EQU	-1

GDSetCType:
		move.l	IO_LENGTH(a3),d0
		subq.w	#1,d0
		bcs	lenErr

*	    ;------ get the controller type
		movem.l d2/a2,-(sp)
		move.l	IO_UNIT(a3),a2

		move.l	dd_ExecBase(a6),a1
		DISABLE	a1,NOFETCH		;protect from VERTB interrupt
						; and other potgo accessors

		move.l	IO_DATA(a3),a0
		move.b	(a0),d0
		ext.w	d0
		cmpi.w	#GPCT_ALLOCATED,d0
		blt	setFail
		cmpi.w	#MAXCONTROLLER,d0
		bgt	setFail

		move.b	d0,gu_Type(a2)

	    ;------ zero the trigger conditions
		lea	gu_AccumTimeout(a2),a0
		moveq	#gt_SIZEOF/2-1,d1
initTLoop:
		clr.w	(a0)+
		dbf	d1,initTLoop


*	    ;------ initialize for the new controller type
		clr.w	gu_LastSave(a2)
		clr.w	gu_CurrSave(a2)
		move.b	gu_Flags(a2),d1
		and.w	#4,d1		    ;get 0 for left, 4 for right
		or.b	#UF_INITIALIZE!UF_RELATIVE,gu_Flags(a2)

*	    ;-- do port-type specific initialization code
		lsl.w	#2,d0
		move.l	cInitTable(d0.w),a0
		jmp	(a0)

		dc.l	potFree
cInitTable:
		dc.l	potFree
		dc.l	potReadPulseInit
		dc.l	potReadRelInit
		dc.l	potReadAbsInit

potFree:
		bsr.s	potClear
		bsr.s	potReallocBits
		bra.s	setCEnd

potReallocBits:
		move.w	d0,gd_PotAlloc(a6)
		btst	#4,d0
		beq.s	allocBits
		bset	#0,d0
allocBits:
		LINKPGR	AllocPotBits
		move.w	d0,gd_PotMask(a6)
		rts

potClear:
		move.w	gd_PotMask(a6),d0
		move.w	d1,d2
		LINKPGR	FreePotBits
		move.w	d2,d1
		move.w	#$0F01,d0
		lsl.w	d1,d0
		not.w	d0
		and.w	d0,gd_PotGo(a6)
		and.w	gd_PotAlloc(a6),d0
		rts

potReadPulseInit:
		move.l	#_custom,a0
		lsr.w	#1,d1
		move.w	joy0dat(a0,d1.W),gu_CurrSave(a2)
		add.w	d1,d1

potReadRelInit:
		bsr.s	potClear
		move.w	#$0F00,d2
		lsl.w	d1,d2
		or.w	d2,d0
		or.w	d2,gd_PotGo(a6)
		bsr.s	potReallocBits
		bra.s	potChkAlloc

potReadAbsInit:
		bclr	#UB_RELATIVE,gu_Flags(a2)
		bra.s	potReadRelInit

potChkAlloc:
		and.w	d2,d0
		cmp.w	d2,d0
		bne.s	setFail
		bra.s	setCEnd

setFail:
		move.b	#GPDERR_SETCTYPE,IO_ERROR(a3)

setCEnd:
		move.l	dd_ExecBase(a6),a1
		ENABLE	a1,NOFETCH

		moveq	#1,d0
		move.l	d0,IO_ACTUAL(a3)
		movem.l (sp)+,d2/a2
		moveq	#0,d0
		move.l	a3,a1
		bra	EndCommand


******* gameport.device/GPD_ASKTRIGGER *******************************
*
*   NAME
*	GPD_ASKTRIGGER -- Inquire the conditions for a game port report
*
*   FUNCTION
*	This command inquires what conditions must be met by a game
*	port unit before a pending Read request will be satisfied.
*	These conditions, called triggers, are independent -- that
*	any one occurs is sufficient to queue a game port report to
*	the Read queue.	 These conditions are set by SetTrigger.
*
*	This command always executes immediately.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	GPD_ASKTRIGGER
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	sizeof(gamePortTrigger)
*	io_Data		a structure of type GamePortTrigger, which
*			has the following elements
*	    gpt_Keys -
*		    GPTB_DOWNKEYS set if button down transitions
*		    trigger a report, and GPTB_UPKEYS set if button up
*		    transitions trigger a report
*	    gpt_Timeout	-
*		    a time which, if exceeded, triggers a report;
*		    measured in vertical blank units (60/sec)
*	    gpt_XDelta	-
*		    a distance in x which, if exceeded, triggers a
*		    report
*	    gpt_YDelta	-
*		    a distance in x which, if exceeded, triggers a
*		    report
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	This quite simply stores the correct game port trigger info
*	into the buffer pointed to by IO_DATA.
*
*---------------------------------------------------------------------
GDCBAskTrigger	EQU	-1

GDAskTrigger:
		move.l	IO_LENGTH(a1),d0
		subq.w	#gt_SIZEOF,d0
		bcs.s	lenErr

		movem.l a2,-(sp)
		move.l	IO_UNIT(a1),a2
		lea	gu_Timeout(a2),a2
		move.l	IO_DATA(a1),a0
		moveq	#gt_SIZEOF/2-1,d0
askTLoop:
		move.w	(a2)+,(a0)+
		dbf	d0,askTLoop

		movem.l (sp)+,a2
		moveq	#gt_SIZEOF,d0
		move.l	d0,IO_ACTUAL(a1)
		moveq	#0,d0
		bra	EndCommand


******* gameport.device/GPD_SETTRIGGER *******************************
*
*   NAME
*	GPD_SETTRIGGER -- Set the conditions for a game port report
*
*   FUNCTION
*	This command sets what conditions must be met by a game
*	port unit before a pending Read request will be satisfied.
*	These conditions, called triggers, are independent -- that
*	any one occurs is sufficient to queue a game port report to
*	the Read queue.	 These conditions are inquired with
*	AskTrigger.
*
*	This command always executes immediately.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	GPD_SETTRIGGER
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	sizeof(gamePortTrigger)
*	io_Data		a structure of type GamePortTrigger, which
*			has the following elements
*	    gpt_Keys -
*		    GPTB_DOWNKEYS set if button down transitions
*		    trigger a report, and GPTB_UPKEYS set if button up
*		    transitions trigger a report
*	    gpt_Timeout	-
*		    a time which, if exceeded, triggers a report;
*		    measured in vertical blank units (60/sec)
*	    gpt_XDelta	-
*		    a distance in x which, if exceeded, triggers a
*		    report
*	    gpt_YDelta	-
*		    a distance in x which, if exceeded, triggers a
*		    report
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	This quite simply stores the correct game port trigger info
*	into the buffer pointed to by IO_DATA.
*
*---------------------------------------------------------------------
GDCBSetTrigger	EQU	-1

GDSetTrigger:
		move.l	IO_LENGTH(a1),d0
		subq.w	#gt_SIZEOF,d0
		bcs.s	lenErr

		movem.l a2,-(sp)
		move.l	IO_UNIT(a1),a2
		lea	gu_Timeout(a2),a2
		move.l	IO_DATA(a1),a0
		moveq	#gt_SIZEOF/2-1,d0
*	    ;------ no need to disable, trigger can be mixed
setTLoop:
		move.w	(a0)+,(a2)+
		dbf	d0,setTLoop

		movem.l (sp)+,a2
		moveq	#gt_SIZEOF,d0
		move.l	d0,IO_ACTUAL(a1)
		moveq	#0,d0
		bra	EndCommand

lenErr:
		moveq	#IOERR_BADLENGTH,d0
		bra	EndCommand

		END
