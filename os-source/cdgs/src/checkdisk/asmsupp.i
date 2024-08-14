

************************************************************************
*
* $Id: asmsupp.i,v 1.1 92/03/11 23:51:05 jesup Exp $
*
* $Locker:  $
*
************************************************************************

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
