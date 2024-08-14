
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* mess1.i
*
* Source Control
* ------ -------
* 
* $Header: messages.i,v 27.1 85/06/24 13:37:23 neil Exp $
*
* $Locker:  $
*
* $Log:	messages.i,v $
* 
*************************************************************************

		XREF	__doprnt
		XREF	_stdout

PUTFMT:		MACRO	* fmt_string, arg_offset_from_sp

		PEA	\2(SP)
		PEA	\1
		MOVE.L	_stdout,-(SP)
		JSR	__doprnt
		LEA	12(SP),SP
		ENDM

INFOMSG:	MACRO	* level,msg
		
		IFGE	INFO_LEVEL-\1

		PEA	tdName
		MOVEM.L	A0/A1/D0/D1,-(SP)
		PUTFMT	msg\@,4*4
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	end\@

msg\@		DC.B	\2,10,13,0
		DS.W	0
end\@
		ENDC
		ENDM
		

PUTMSG:		MACRO	* level,msg
		
		INFOMSG	\1,<\2>

		ENDM

*****AMSG:		MACRO	* num
*****		PEA	tdName
*****		MOVEM.L	A0/A1/D0/D1,-(SP)
*****		LEA	4*4(SP),A1
*****		ALERT	\1,A1,A0
*****		MOVEM.L	(SP)+,D0/D1/A0/A1
*****		ADDQ.L	#4,SP
*****		ENDM
