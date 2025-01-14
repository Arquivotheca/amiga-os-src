**
** $Source: HOG:Other/networking/sana2/src/slip/RCS/slip_device.asm,v $
** $State: Exp $
** $Revision: 38.1 $
** $Date: 94/02/17 14:17:31 $
** $Author: kcd $
**
** Amiga SANA2 SLIP Device Driver
**
** (C) Copyright 1992 Commodore-Amiga, Inc.
**


	SECTION	firstsection

	NOLIST

	include	"exec/types.i"
	include	"exec/devices.i"
	include	"exec/initializers.i"
	include	"exec/memory.i"
	include	"exec/resident.i"
	include	"exec/io.i"
	include	"exec/ables.i"
	include	"exec/errors.i"
	include	"exec/tasks.i"
	include	"utility/tagitem.i"
	include "utility/hooks.i"
	include	"dos/dos.i"
	include	"dos/dosextens.i"
	include	"dos/dostags.i"
	include	"devices/serial.i"
	include	"devices/sana2.i"
	include	"slip_device.i"
        include "slip_rev.i"

	LIST

ABSEXECBASE	EQU	4	;Absolute location of the pointer to exec.library base
**
** First executable location
**

FirstAddress:
	moveq	#-1,d0
	rts

SLIPPRI   EQU   5

	XREF	EndCode

initDDescrip:
					; STRUCTURE RT,0
	DC.W	RTC_MATCHWORD		; UWORD	RT_MATCHWORD (Magic cookie)
	DC.L	initDDescrip		; APTR	RT_MATCHTAG  (Back pointer)
	DC.L	EndCode			; APTR	RT_ENDSKIP   (To end of this hunk)
	DC.B	RTF_AUTOINIT		; UBYTE	RT_FLAGS     (magic-see "Init:")
	DC.B	VERSION			; UBYTE	RT_VERSION
	DC.B	NT_DEVICE		; UBYTE	RT_TYPE	     (must be correct)
	DC.B	SLIPPRI			; BYTE	RT_PRI
	DC.L	SLIPName		; APTR	RT_NAME	     (exec name)
	DC.L	idString		; APTR	RT_IDSTRING  (text string)
	DC.L	Init			; APTR	RT_INIT
					; LABEL	RT_SIZE

SLIPName:	SLIPDEVNAME

** This is an identifier tag to help in supporting the device
** format is 'name version.revision (dd MON yyyy)',<cr>,<lf>,<null>

idString:	VSTRING

** Force word alignment

	ds.w   0

Init:
	dc.l	SLIPDev_Sizeof	; data space size
	dc.l	funcTable	; pointer to function initializers
	dc.l	dataTable	; pointer to data initializers
	dc.l	initRoutine	; routine to run

**
** Standard System Routines
**

	XREF	_DevOpen
	XREF	_DevClose
	XREF	_DevExpunge
	XREF	_DevBeginIO
	XREF	_DevAbortIO
	XDEF	_ExtDeviceBase
	XDEF	_DevProcEntry
	XDEF	@IPToNum
	XDEF	_SANA2BuffCall
	XREF	_DevProcCEntry

funcTable:
	dc.l	_DevOpen
	dc.l	_DevClose
	dc.l	_DevExpunge
	dc.l	Null
	dc.l	_DevBeginIO
	dc.l	_DevAbortIO
	dc.l	-1

dataTable:
	INITBYTE	LN_TYPE,NT_DEVICE	;Must be LN_TYPE!
	INITLONG	LN_NAME,SLIPName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERSION
	INITWORD	LIB_REVISION,REVISION
	INITLONG	LIB_IDSTRING,idString
	DC.W   0	;terminate list

_ExtDeviceBase	dc.l	0

**
** initRoutine
**
** Called after device has been allocated.
** This routine is single threaded
**
** Register Usage
**
** a3 - Pointer to temporary RAM
** a4 - Pointer to expansion.library base
** a5 - Pointer to device struct
** a6 - Pointer to Exec Base

initRoutine:
	movem.l	d1-d7/a0-a5,-(sp)
	movea.l	d0,a5
	move.l	a6,sd_SysLib(a5)
	move.l	a0,sd_SegList(a5)
	lea.l	FakePFHookEntry(pc),a0
	move.l	a0,sd_DummyPFHook+h_Entry(a5)
	lea.l	sd_Lock(a5),a0
	jsrlib	InitSemaphore
	move.l	a5,d0
Init_Exit:
	movem.l	(sp)+,d1-d7/a0-a5
	rts

Null:
	moveq.l	#0,d0
	rts

@IPToNum:
	movem.l	d2-d7/a2-a6,-(sp)
	bsr	StrToNum
	lsl.w	#8,d0
	move.w	d0,d4
	bsr	StrToNum
	move.b	d0,d4
	swap	d4
	bsr	StrToNum
	lsl.w	#8,d0
	move.w	d0,d4
	bsr	StrToNum
	move.b	d0,d4
	move.l	d4,d0
	movem.l	(sp)+,d2-d7/a2-a6
	rts

StrToNum:
	moveq	#0,d0
	moveq	#0,d1
1$	move.b	(a0)+,d1
	cmp.b	#'0',d1
	blo	2$
	cmp.b	#'9',d1
	bhi	2$
	sub.b	#'0',d1
	mulu	#10,d0
	add.w	d1,d0
	bra	1$
2$	rts

_DevProcEntry:
	movea.l	_ExtDeviceBase(pc),a6
	jmp	_DevProcCEntry

_SANA2BuffCall:
	jmp	(a2)

FakePFHookEntry:
	moveq.l	#1,d0
	rts

	end
