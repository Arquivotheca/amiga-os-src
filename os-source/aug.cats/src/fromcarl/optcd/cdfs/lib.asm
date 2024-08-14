************************************************************************
****************                                        ****************
****************        -=  CDTV FILE SYSTEM  =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
***   Copyright (C) 1990 Commodore-Amiga, Inc. All rights reserved.  ***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
************************************************************************
***                                                                  ***
***                             History                              ***
***                             -------                              ***
***                                                                  ***
***   21.00 24OCT90 Carl Sassenrath      Beta 1.5 Release            ***
***                                                                  ***
************************************************************************

	include	"exec/types.i"
	include	"exec/memory.i"
	include	"exec/io.i"
	include "stddefs.i"
	include "libraries/dosextens.i"
	INCLUDE	"expansion.lvo"

	public	_DebugLevel
	public	_ExpBase
	public	_DOSBase

***********************************************************
**
** LIBRARY
**
***********************************************************

************************************************************************
***
***  MonthDays Table
***
***	Given a month, this table converts to the total number
***	of days upto the first day of that month.  These are
***	non-leap values and must be adjusted by the main algorithm.
***
***	   Example: March returns 59 days (non-leap)
***
************************************************************************
MonthDays:
	;------	    31,28,31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	dc.w	0,0,31,59,90,120,151,181,212,243,273,304,334,365
	ds.w	0

************************************************************************
***
***  MakeDate
***
***	Make a DOS date-stamp from an ISO directory date array.
***	Adjust properly for leap years and leap centuries.
***	If the year is 0, assume that the ISO date array is not
***	valid and create an invalid DOS date.
***
************************************************************************
	public	_MakeDate
_MakeDate:
		save	d2/d3
		move.l	12(sp),a0	; DOS date stamp
		move.l	16(sp),a1	; ISO date array

	;------	Clear DOS date stamp structure
		clear	d2
		move.l	a0,d0		; save pointer
		move.l	d2,(a0)+
		move.l	d2,(a0)+
		move.l	d2,(a0)+
		move.l	d0,a0		; restore pointer

	;------	Get ISO year:
		move.b	(a1)+,d2	; year from 1900
		beq.s	9$		; not valid

	;------	Is it a leap year?
		move.l	d2,d0
		moveq	#3,d1
		subq.l	#1,d0		
		and.b	d1,d0
		cmp.b	d1,d0
		sne.b	d3		; leap flag
		
	;------	Check for leap century!??!?!?!!!!

	;------	Convert years to days:
		move.l	d2,d0
		sub.l	#78,d0		; base year
		mulu	#365,d0		; works for 179 years
		sub.l	#76,d2		; last leap year before base
		lsr.l	#2,d2		; /4 = number of leaps since base
		add.l	d2,d0

	;------	Convert months to days:
		move.b	(a1)+,d2	; month (upper 24 still clear)
		beq.s	9$		; not valid
		move.l	d2,d1
		lsl.l	#1,d1
		move.w	MonthDays(pc,d1.w),d1

	;------	Check and handle Feb 28:
		cmp.b	#3,d2		; before March?
		bge.s	4$		; no, count leap
		tst.b	d3		; leap flag?
		beq.s	4$		; not leap year anyway
		subq.l	#1,d1		; too early in year to leap
4$:		add.l	d1,d0

	;------	Add day of month:
		move.b	(a1)+,d2	; day (upper 24 still clear)
		add.l	d2,d0
		move.l	d0,(a0)+	; store DOS days

	;------	Get hour and minute:
		clear	d0
		move.b	(a1)+,d0	; hour
		mulu	#60,d0		; to minutes
		move.b	(a1)+,d2	; min (upper 24 still clear)
		add.l	d2,d0
		move.l	d0,(a0)+	; store DOS mins

	;------	Calc ticks:
		clear	d0
		move.b	(a1)+,d0	; second
		mulu	#50,d0		; to ticks
		move.l	d0,(a0)+	; store DOS mins

9$:		restore	d2/d3
		rts

*	Mult(time[0]-78,365) +	/* years */
*	  ((time[0]-76) >> 2) +   /* leaps */
*	    MonthDays[time[1]] +	/* months */
*	      ((leap&&time[1]<3)?-1:0) + /* not yet leap month */
*		+ time[2];		/* day of month */
*	fib->fib_Date.ds_Minute = Mult(time[3],24) + time[4];	*/
*	fib->fib_Date.ds_Tick = Mult(time[5],TICKS_PER_SECOND);	*/

************************************************************************
***
***  MakeDate(dirdate,pvddate)
***
***	Converts an ISO PVD date to that which can be used by
***	the MakeDate function.  If the year is zero a FALSE
***	is returned and the date should be considered invalid.
***
************************************************************************
	public	_ConvertDate
_ConvertDate:
		save	d2/a2
		move.l	12(sp),a2	; ISO dir date form
		move.l	16(sp),a1	; ISO PVD date str pointer

	;------	Convert year:
		moveq	#4,d1		; 4 digit number
		bsr	CharToDec	; D1,A1
		tst.w	d0		; should not be zero
		beq.s	9$
		sub.w	#1900,d0	; get year as offset
		move.b	d0,(a2)+

	;------	Convert month,day,hour,min,sec:
		moveq	#4,d2		; dbf 5 over all
2$:		moveq	#2,d1		; two digit numbers
		bsr	CharToDec	; D1,A1
		move.b	d0,(a2)+
		dbf	d2,2$

		clr.b	(a2)+		; GMT offset

		restore	d2/a2
9$		rts

************************************************************************
***
***  CompDate
***
***	Compare two DOS date stamps for equality. Return TRUE if same.
***
***********************************************************************/
	public	_CompDate
_CompDate:
		movem.l	4(sp),a0/a1
		clear	d0
		cmp.l	(a0)+,(a1)+
		bne.s	9$
		cmp.l	(a0)+,(a1)+
		bne.s	9$
		cmp.l	(a0)+,(a1)+
		bne.s	9$
		moveq.l	#-1,d0
9$:		rts

************************************************************************
***
***  ConvertArgs
***
***	Converts necessary packet args to APTRs from BPTRs.
***	In C:
***		PktArg1 = pkt->dp_Arg1 << ((flags & ARG1) ? 2 : 0);
***
************************************************************************
	public	_ConvertArgs
	public	_Packet,_PktArg1,_PktArg2,_PktArg3,_PktArg4
_ConvertArgs:
	move.l	4(sp),d0		; conversion flags

	move.l	_Packet,a0		; global
	add	#dp_Arg1,a0		; set to ARG1

	move.l	(a0)+,d1		; get Arg1
	lsr	#1,d0			; check first bit
	bcc.s	1$			; not set
	lsl.l	#2,d1			; convert to APTR
1$	move.l	d1,_PktArg1		; save it

	move.l	(a0)+,d1		; get Arg2
	lsr	#1,d0			; check first bit
	bcc.s	2$			; not set
	lsl.l	#2,d1			; convert to APTR
2$	move.l	d1,_PktArg2		; save it

	move.l	(a0)+,d1		; get Arg3
	lsr	#1,d0			; check first bit
	bcc.s	3$			; not set
	lsl.l	#2,d1			; convert to APTR
3$	move.l	d1,_PktArg3		; save it

	move.l	(a0)+,d1		; get Arg4
	lsr	#1,d0			; check first bit
	bcc.s	4$			; not set
	lsl.l	#2,d1			; convert to APTR
4$	move.l	d1,_PktArg4		; save it

	rts

************************************************************************
***
***  CharToDec
***
***	Take N chars of a string and convert to decimal number.
***	Used for the ISO date conversion above.
***
***	Input:
***		D1 == Number of chars
***		A1 -> String
***	Return:
***		D0 == Decimal value
***		A1 -> New string pointer
***
************************************************************************
CharToDec:
		save	d2
		clear	d0
		clear	d2
		bra.s	2$
1$		mulu	#10,d0
		move.b	(a1)+,d2	; get char
		sub.b	#'0',d2		; make into num
		add.l	d2,d0
2$		dbf	d1,1$
		restore	d2
		rts			; result in D0, A2 updated	


*** Mult -- 16 bit multiply
	public _Mult
_Mult:
		move.l	4(sp),d0
		move.l	8(sp),d1
		muls	d1,d0
		rts

	public _CopyMem		;source,target,count
_CopyMem:
		move.l	4(sp),a0
		move.l	8(sp),a1
		move.l	12(sp),d0
		exec	CopyMem
		rts

	public	_MakeVector
_MakeVector:
		move.l	d2,-(sp)
		move.l	8(sp),d0
		move.l	#MEMF_CLEAR|MEMF_PUBLIC,d1
		addq.l	#4,d0
		addq.l	#8,d0		; V23.1 (for DMA overreading...)
		move.l	d0,d2
		exec	AllocMem
		tst.l	d0
		beq.s	1$
		move.l	d0,a0
		move.l	d2,(a0)+	; save size
		move.l	a0,d0
1$		move.l	(sp)+,d2
		rts

	public	_FreeVector
_FreeVector:
		move.l	4(sp),d0
		beq.s	1$
		move.l	d0,a1
		move.l	-(a1),d0	; size of block
		exec	FreeMem
1$:		rts

	public	_AvailMem
_AvailMem:
		move.l	4(sp),d1
		exec	AvailMem
		rts

	public	_ClearMem
_ClearMem:				; for small mem areas
		move.l	4(sp),a0
		move.l	8(sp),d0
		moveq.l	#0,d1
		bra.s	2$
1$:		move.b	d1,(a0)+
2$:		dbf	d0,1$
		rts

	public	_Forbid
_Forbid:
		exec	Forbid
		rts

	public	_Permit
_Permit:
		exec	Permit
		rts

	public	_Supervisor
_Supervisor:
		exec	Supervisor
		rts

	public	_AllocSignal
_AllocSignal:
		move.l	4(sp),d0
		exec	AllocSignal
		rts

	public	_FreeSignal
_FreeSignal:
		move.l	4(sp),d0
		exec	FreeSignal
		rts

	public	_OpenDevice
_OpenDevice:
		move.l	4(sp),a0
		movem.l	8(sp),d0/a1
		move.l	16(sp),d1
		exec	OpenDevice
		rts

	public	_CloseDevice
_CloseDevice:
		move.l	4(sp),a1
		exec	CloseDevice
		rts

	public	_FindTask
_FindTask:
		move.l	4(sp),a1
		exec	FindTask
		rts

	public	_FindPort
_FindPort:
		move.l	4(sp),a1
		exec	FindPort
		rts

	public	_WaitPort
_WaitPort:
		move.l	4(sp),a0
		exec	WaitPort
		rts

	public	_NewList
_NewList:
		move.l	4(sp),a1
		NEWLIST	a1
		rts

	public	_Enqueue
_Enqueue:
		movem.l	4(sp),a0/a1
		exec	Enqueue
		rts

	public	_PutMsg
_PutMsg:
		movem.l	4(sp),a0/a1
		exec	PutMsg
		rts

	public	_GetMsg
_GetMsg:
		move.l	4(sp),a0
		exec	GetMsg
		rts

	public	_ReplyMsg
_ReplyMsg:
		move.l	4(sp),a1
		exec	ReplyMsg
		rts

	public	_DoIO
_DoIO:
		move.l	4(sp),a1
		exec	DoIO
		rts

	public	_DoDevIO
	public	_DevIOReq
_DoDevIO:
		lea	4(sp),a0
		lea	_DevIOReq,a1
		move.l	(a0)+,d0
		move.w	d0,IO_COMMAND(a1)
		move.l	(a0)+,IO_OFFSET(a1)
		move.l	(a0)+,IO_LENGTH(a1)
		move.l	(a0)+,IO_DATA(a1)
		exec	DoIO
		rts

	public	_SendIO
_SendIO:
		move.l	4(sp),a1
		exec	SendIO
		rts

	public	_Wait
_Wait:
		move.l	4(sp),d0
		exec	Wait
		rts

	public	_Signal
_Signal:
		move.l	4(sp),a1
		move.l	8(sp),d0
		exec	Signal
		rts

	public	_FetchA4
_FetchA4:
		move.l	a4,d0
		rts

	public	_AddDosNode
_AddDosNode:
		movem.l	4(sp),d0/d1/a0
		push	a6
		move.l	_ExpBase,a6
		jsr	_LVOAddDosNode(a6)
		pop	a6
		rts

_LVOCreateProc	equ	-138
_LVOLoadSeg	equ	-150

	public	_CreateProc
_CreateProc:
		save	d3/d4/a6
		movem.l	16(sp),d1-d4
		move.l	_DOSBase,a6
		jsr	_LVOCreateProc(a6)
		restore	d3/d4/a6
		rts

	public	_LoadSeg
_LoadSeg:
		movem.l	4(sp),d1
		push	a6
		move.l	_DOSBase,a6
		jsr	_LVOLoadSeg(a6)
		pop	a6
		rts

***********************************************************************
***********************************************************************
***********************************************************************

	public	_PutChar
_PutChar:
		move.l	4(sp),d0
rawPutChar:	movem.l	d0/a0/a6,-(sp)
		move.l	4,a6
		jsr	LVO.RawPutChar(a6)
		movem.l	(sp)+,d0/a0/a6
		rts

	public Print		a0->format  a1->data
Print:		move.l	a2,-(sp)
		lea	rawPutChar,a2
		exec	RawDoFmt
		move.l	(sp)+,a2
		rts

	public	_Debug1
_Debug1:
		moveq.l	#1,d0
		bra.s	Debug

	public	_Debug2
_Debug2:
		moveq.l	#2,d0
		bra.s	Debug

	public	_Debug3
_Debug3:
		moveq.l	#3,d0
		bra.s	Debug

	public	_Debug4
_Debug4:
		moveq.l	#4,d0

Debug:
		move.l	_DebugLevel,d1	; V34.4
		cmp.w	d1,d0		; V34.4 lower word only!
		bgt.s	dexit

	public	_Debug0
_Debug0:
		move.l	4(sp),a0	; format
		lea	8(sp),a1	; data
		bsr	Print
dexit:		rts


	XDEF	EndCode
EndCode
		dc.w	0
	END

