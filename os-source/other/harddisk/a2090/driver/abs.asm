	LLEN	120
	PLEN	60
	LIST

*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* Source Control
* ------ -------
* 
* $Header: abs.asm,v 27.1 85/06/24 13:36:12 neil Exp $
*
* $Locker:  $
*
* $Log:	abs.asm,v $
* 
*************************************************************************

******* Included Files ***********************************************

	SECTION section

	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE	"exec/interrupts.i"
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'
	INCLUDE 'exec/alerts.i'
	INCLUDE 'exec/resident.i'

	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE	'hddisk_rev.i'
	INCLUDE	'libraries/expansion.i'
	LIST
	INCLUDE	'internal.i'

******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	EXTERN_LIB	OpenDevice
	EXTERN_LIB	CloseDevice
	EXTERN_LIB	RemDevice

	XREF	Init
	XREF	EndCode

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	Remove

*------ Data ---------------------------------------------------------

	XDEF	hdName
	XDEF	hdIDString
	XDEF	VERNUM
	XDEF	REVNUM
	XDEF	ExLibName
	XDEF	IntuitLibName

******* Local Definitions ********************************************


*------	Jump Table of vectors:

		MOVEQ	#0,D0
		RTS

initDDescrip:
				;STRUCTURE RT,0
	DC.W	RTC_MATCHWORD	; UWORD RT_MATCHWORD
	DC.L	initDDescrip	; APTR	RT_MATCHTAG
	DC.L	EndCode		; APTR	RT_ENDSKIP
	DC.B	RTW_COLDSTART	; UBYTE RT_FLAGS
	DC.B	VERSION		; UBYTE RT_VERSION
	DC.B	NT_DEVICE	; UBYTE RT_TYPE
	DC.B	20		; BYTE	RT_PRI
	DC.L	hdName		; APTR  RT_NAME
	DC.L	hdIDString	; APTR	RT_IDSTRING
	DC.L	Init		; APTR	RT_INIT
				; LABEL RT_SIZE


		CNOP	0,4	; LONG WORD ALIGN
ExLibName	EXPANSIONNAME	; Expansion Library Name

		CNOP	0,4	; STRINGS MUST BE LONG WORD ALIGNED!
IntuitLibName	
		DC.B	'intuition.library',0
		DS.W	0

		CNOP	0,4	; STRINGS MUST BE LONG WORD ALIGNED!
hdName:
		HD_NAME

*		;------ our name identification string
		CNOP	0,4	; STRINGS MUST BE LONG WORD ALIGNED!
hdIDString:	VSTRING

Remove:
*		;----- Open
		LEA	-IOSTD_SIZE(SP),SP
		MOVE.L	SP,A1
		LEA	hdName,A0
		MOVE.L	A0,LN_NAME(A1)

		CLEAR	D0
		CLEAR	D1

		CALLSYS OpenDevice
		TST.L	D0
		BEQ.S	Remove_End		; no device by that name

*		;----- Close
		MOVE.L	SP,A1
		MOVE.L	IO_DEVICE(A1),-(SP)
		CALLSYS CloseDevice


*		;----- Remove
		MOVE.L	(SP)+,A1
		CALLSYS RemDevice

Remove_End:
		LEA	IOSTD_SIZE(SP),SP
		RTS

VERNUM:		EQU	VERSION

REVNUM		EQU	REVISION

	END
