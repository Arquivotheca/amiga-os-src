*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* $Id: asmsupp.asm,v 39.1 92/04/20 09:37:00 mks Exp $
*
* $Log:	asmsupp.asm,v $
* Revision 39.1  92/04/20  09:37:00  mks
* Removed graphics.library string
* 
* Revision 39.0  92/01/20  13:10:12  mks
* Timer device now calls battclock to read the base system time.
* Also made some branches short...
*
*************************************************************************

	SECTION	timer

	NOLIST
	INCLUDE	'exec/types.i'
	INCLUDE	'exec/resident.i'
	INCLUDE	'resources/cia.i'
	INCLUDE 'graphics/gfxbase.i'
	INCLUDE	"resources/battclock.i"
	LIST

	INCLUDE	'timer.i'
	INCLUDE	'timer_rev.i'

*
* assembly support for timer driver
*
	XREF	Init
	XREF	EndCode

	XDEF	timerName
	XDEF	ciaName
	XDEF	BattClockName

	XDEF	VERNUM
	XDEF	REVNUM
	XDEF	VERSTRING

initDDescrip:		     	;STRUCTURE RT,0
	DC.W	RTC_MATCHWORD	; UWORD RT_MATCHWORD
	DC.L	initDDescrip	; APTR  RT_MATCHTAG
	DC.L	EndCode		; APTR  RT_ENDSKIP
	DC.B	RTF_COLDSTART 	; UBYTE RT_FLAGS
	DC.B	VERSION		; UBYTE RT_VERSION
	DC.B	NT_DEVICE	; UBYTE RT_TYPE
	DC.B	50		; BYTE  RT_PRI
	DC.L	timerName	; APTR  RT_NAME
	DC.L	VERSTRING 	; APTR  RT_IDSTRING
	DC.L	Init		; APTR  RT_INIT
				; LABEL RT_SIZE


ciaName:
		CIAANAME

timerName:
		TIMERNAME

BattClockName:
		BATTCLOCKNAME

VERSTRING:
		VSTRING

VERNUM:		EQU	VERSION

REVNUM:		EQU	REVISION

		END
