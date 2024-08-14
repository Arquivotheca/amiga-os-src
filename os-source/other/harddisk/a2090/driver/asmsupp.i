
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* asmsupp.i
*
* Source Control
* ------ -------
* 
* $Header: asmsupp.i,v 27.1 85/06/24 13:36:17 neil Exp $
*
* $Locker:  $
*
* $Log:	asmsupp.i,v $
* Revision 27.1  85/06/24  13:36:17  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:52  neil
* *** empty log message ***
* 
* 
*************************************************************************

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

LINKSYS MACRO
	LINKLIB _LVO\1,HD_SYSLIB(A6)
	ENDM

CALLSYS	MACRO
	CALLLIB	_LVO\1
	ENDM
