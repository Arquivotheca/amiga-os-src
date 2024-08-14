
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
* $Id: asmsupp.i,v 32.2 90/06/01 23:14:32 jesup Exp $
*
* $Locker:  $
*
* $Log:	asmsupp.i,v $
* Revision 32.2  90/06/01  23:14:32  jesup
* Conform to include standard du jour
* 
* Revision 32.1  85/12/22  14:16:48  neil
* added XLIB macro
* 
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
	LINKLIB _LVO\1,TD_SYSLIB(A6)
	ENDM

CALLSYS	MACRO
	CALLLIB	_LVO\1
	ENDM

XLIB	MACRO
	XREF	_LVO\1
	ENDM
