
*************************************************************************
*									*
*  Copyright (C) 1985,1989 Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* asmsupp.i
*
* Source Control
* ------ -------
* 
* $Id: asmsupp.i,v 39.0 91/10/28 16:27:55 mks Exp $
*
* $Locker:  $
*
*************************************************************************

ABSEXECBASE	EQU 4

;
; sigh.  A moveq is faster than a clear instruction.
;
CLEAR	MACRO
	MOVEQ	#0,\1
	ENDM

;
; there is no "clear" instruction for A registers, but subtracting the
; register from itself is space efficient and is as fast as there is.
;
CLRA    MACRO
	SUBA.L	\1,\1
	ENDM

;
; I never can remember what bcc and bcs really mean, so I use the more
; meaningful mnemonics;
;
BHS	MACRO
	IFC	'\0',''
	BCC	\1
	ENDC
	IFNC	'\0',''
	BCC.\0	\1
	ENDC
	ENDM

BLO	MACRO
	IFC	'\0',''
	BCS	\1
	ENDC
	IFNC	'\0',''
	BCS.\0	\1
	ENDC
	ENDM

bhs	MACRO
	IFC	'\0',''
	BCC	\1
	ENDC
	IFNC	'\0',''
	BCC.\0	\1
	ENDC
	ENDM

blo	MACRO
	IFC	'\0',''
	BCS	\1
	ENDC
	IFNC	'\0',''
	BCS.\0	\1
	ENDC
	ENDM

;
; Ok, ok, so I grew up on an ancient assembler...  By the way, this
; aligns the instruction stream on an even boundary.  A non-standard
; way to do this would be cnop 0,2.  To align on a long boundard,
; try cnop 0,4.
;
EVEN	MACRO
	DS.W	0
	ENDM

;
; these next three are to hide the "_LVO" (Library Vector Offset) prefix
; on library constants from normal programming
;
XLIB	MACRO		; define a reference to a library
	XREF	_LVO\1
	ENDM

JMPSYS	MACRO		; jmp to a library whose base is already in a6
	jmp	_LVO\1(a6)
	ENDM

CALLSYS	MACRO		; call a library whose base is already in a6
	jsr	_LVO\1(a6)
	ENDM

LINKSYS	MACRO		; call a library whose base is in \2
	move.l	a6,-(sp)
	move.l	\2,a6
	CALLSYS	\1
	move.l	(sp)+,a6
	ENDM

LINKEXE	MACRO		; call exec
	move.l	a6,-(sp)
	move.l	ABSEXECBASE,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,a6
	ENDM

LINKDOS	MACRO		; call dos
	XREF	DosBaseOffset
	LINKSYS	\1,DosBaseOffset(a6)
	ENDM


XSEM	MACRO
	XREF	_LVO\1
	ENDM
CALLSEM	MACRO
	jsr	_LVO\1(a6)
	ENDM
LINKSEM	MACRO
	move.l	a6,-(sp)
	move.l	ABSEXECBASE,a6
	jsr	_LVO\1(a6)
	move.l	(sp)+,a6
	ENDM

