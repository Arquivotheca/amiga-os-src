
**********************************************************************
*
* asmsupp.i
*
* Copyright (C) 1985, Commodore-Amiga Inc., All Rights Reserved
*
* $Id: asmsupp.i,v 36.11 90/05/06 00:36:58 bryce Exp $
*
* $Locker:  $
*
**********************************************************************


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
