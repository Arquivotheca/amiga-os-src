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
* $Id: asmsupp.i,v 36.7 91/01/25 15:45:46 rsbx Exp $
*
* $Locker:  $
*
* $Log:	asmsupp.i,v $
* Revision 36.7  91/01/25  15:45:46  rsbx
* Change to V37
* 
* Revision 36.6  90/04/01  19:12:24  rsbx
* RCS version change.
* 
* Revision 36.5  89/08/09  19:24:12  rsbx
* *** empty log message ***
* 
* Revision 36.4  89/08/09  18:10:12  rsbx
* *** empty log message ***
* 
* Revision 36.2  89/08/09  14:03:37  rsbx
* revised for rewritten timer.device
* 
*
*************************************************************************

*	optimon $ffffffff

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
	LINKLIB _LVO\1,TD_SYSLIB(A6)
	ENDM

XSYS	MACRO
	XREF	_LVO\1
	ENDM

