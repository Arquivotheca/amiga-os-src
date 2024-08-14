
**********************************************************************
*
* messages.i
*
* (C) Copyright 1985,1990 Commodore-Amiga Inc., All Rights Reserved
*
* $Id: messages.i,v 39.0 91/10/28 16:27:58 mks Exp $
*
* $Locker:  $
*
**********************************************************************

		XREF	KPutFmt

		IFD	CAPE
		OPTIMON	$fd000000	;disable MOVEM and branch length ops
		ENDC

INFOLEVEL		EQU	0	;How much debugging to enable
OnePointFourDOS		EQU 	1	;True for 2.0 dos library
OnePointFourExec	EQU	1	;True for 2.0 Kickstart


DOCODE  MACRO
	SECTION	expansion
        ENDM

DODATA  MACRO
        section section,data
        ENDM



INFOMSG:	MACRO	* level,msg
		
		IFGE	INFOLEVEL-\1

		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@,A0
		LEA	4*4(SP),A1
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1

		DODATA

msg\@		DC.B	'expansion:',\2,10,0
		DS.W	0

		DOCODE

		ENDC
		ENDM
