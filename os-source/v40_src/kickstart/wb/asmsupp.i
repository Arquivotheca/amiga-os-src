**********************************************************************
*
* $Id: asmsupp.i,v 38.1 91/06/24 11:33:46 mks Exp $
*
* $Log:	asmsupp.i,v $
* Revision 38.1  91/06/24  11:33:46  mks
* Initial V38 tree checkin - Log file stripped
* 
**********************************************************************


CLEAR	MACRO
	MOVEQ	#0,\1
	ENDM

BHS	MACRO
	IFNC	'\0',''
	BCC.\0	\1
	ENDC
	IFC	'\0',''
	BCC	\1
	ENDC
	ENDM

BLO	MACRO
	IFNC	'\0',''
	BCS.\0	\1
	ENDC
	IFC	'\0',''
	BCS	\1
	ENDC
	ENDM

EVEN	MACRO
	DS.W	0
	ENDM

CLRA    MACRO
	SUBA.\0 \1,\1
	ENDM

XLIB	MACRO
	XREF	_LVO\1
	ENDM

CALLSYS	MACRO
	CALLLIB	_LVO\1
	ENDM

LINKSYS	MACRO
	LINKLIB	_LVO\1,_AbsExecBase
	ENDM
