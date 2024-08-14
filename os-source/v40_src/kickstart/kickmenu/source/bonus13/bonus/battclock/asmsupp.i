*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* asmsupp.i
*
* Source Control
* ------ -------
* 
* $Header: asmsupp.i,v 36.1 89/10/06 13:58:18 rsbx Exp $
*
* $Locker: rsbx $
*
* 
*
*************************************************************************

;	optimon $ffffffff

CLEAR	MACRO
	MOVEQ	#0,\1
	ENDM

BHS	MACRO
	BCC.\0	\1
	ENDM

BLO	MACRO
	BCS.\0	\1
	ENDM

EVEN	MACRO
	DS.W	0
	ENDM

CLRA    MACRO
	SUBA.\0 \1,\1
	ENDM

CALLSYS	MACRO
	JSR	_LVO\1(A6)
	ENDM

LINKSYS	MACRO	* &func
	LINKLIB _LVO\1,BTC_Exec(A6)
	ENDM

XSYS	MACRO
	XREF	_LVO\1
	ENDM

