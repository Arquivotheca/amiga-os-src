**
** $Source: HOG:Other/cd32mpeg/RCS/mpeg_device.asm,v $
** $State: Exp $
** $Revision: 40.3 $
** $Date: 94/01/26 11:57:56 $
** $Author: kcd $
**
** Amiga MPEG Device Driver Assembly Glue
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
	include	"dos/dos.i"
	include	"dos/dosextens.i"
	include	"dos/dostags.i"
	include	"mpeg_device.i"
        include "cd32mpeg_rev.i"

	LIST

ABSEXECBASE	EQU	4	;Absolute location of the pointer to exec.library base
**
** First executable location
**

FirstAddress:
	moveq	#-1,d0
	rts

MPEGPRI	EQU   5

	XREF	EndCode

initDDescrip:
					; STRUCTURE RT,0
	DC.W	RTC_MATCHWORD		; UWORD	RT_MATCHWORD (Magic cookie)
	DC.L	initDDescrip		; APTR	RT_MATCHTAG  (Back pointer)
	DC.L	EndCode			; APTR	RT_ENDSKIP   (To end of this hunk)
	DC.B	RTF_AUTOINIT		; UBYTE	RT_FLAGS     (magic-see "Init:")
	DC.B	105 ;VERSION			; UBYTE	RT_VERSION
	DC.B	NT_DEVICE		; UBYTE	RT_TYPE	     (must be correct)
	DC.B	MPEGPRI			; BYTE	RT_PRI
	DC.L	MPEGName		; APTR	RT_NAME	     (exec name)
	DC.L	idString		; APTR	RT_IDSTRING  (text string)
	DC.L	Init			; APTR	RT_INIT
					; LABEL	RT_SIZE

MPEGName:	MPEGDEVNAME
;MPEGName:	dc.b	'cd32mpeg2.device',0

** This is an identifier tag to help in supporting the device
** format is 'name version.revision (dd MON yyyy)',<cr>,<lf>,<null>

idString:	VSTRING

** Force word alignment

	DS.W   0

Init:
	DC.L	MPEGDev_Sizeof	; data space size
	DC.L	funcTable	; pointer to function initializers
	DC.L	Null
	DC.L	initRoutine	; routine to run

**
** Standard System Routines
**
	XREF	_DevInit
	XREF	_DevOpen
	XREF	_DevClose
	XREF	_DevExpunge
	XREF	_DevBeginIO
	XREF	_DevAbortIO
	XREF	_DevSetSCR
	XREF	_DevGetSCR
	XREF	_DevGetSector
	XDEF	_EndianFixLong
	XDEF	_EndianFixWord

LONG_VECTORS	EQU	1

	IFNE	LONG_VECTORS
V_DEF	MACRO
	DC.L	\1
	ENDM

V_START	MACRO
	ENDM

V_END	MACRO
	DC.L	-1
	ENDM

	ELSE

V_DEF	MACRO
	DC.W	\1+(*-funcTable)
	ENDM

V_START	MACRO
	DC.W	-1
	ENDM

V_END	MACRO
	DC.W	-1
	ENDM

	ENDC
funcTable:
	V_START
	V_DEF	_DevOpen
	V_DEF	_DevClose
	V_DEF	_DevExpunge
	V_DEF	Null
	V_DEF	_DevBeginIO
	V_DEF	_DevAbortIO
	V_DEF	_DevSetSCR
	V_DEF	_DevGetSCR
	V_DEF	_DevGetSector
	V_END


dataTable:
	INITBYTE	LN_TYPE,NT_DEVICE	;Must be LN_TYPE!
	INITLONG	LN_NAME,MPEGName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERSION
	INITWORD	LIB_REVISION,REVISION
	INITLONG	LIB_IDSTRING,idString
	DC.W   0	;terminate list

	CNOP	0,4

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
	move.l	a6,md_SysBase(a5)
	move.l	a0,md_SegList(a5)
	move.l	#REVISION,LIB_REVISION(a5)
	lea.l	md_Lock(a5),a0
	jsrlib	InitSemaphore
	bsr	_DevInit
	move.l	a5,d0
Init_Exit:
	movem.l	(sp)+,d1-d7/a0-a5
	rts

Null:
	moveq.l	#0,d0
	rts

_EndianFixLong:
	ror.w	#8,d0
	swap	d0
	ror.w	#8,d0
	rts

_EndianFixWord:
	ror.w	#8,d0
	rts

	CNOP	0,4

	END
