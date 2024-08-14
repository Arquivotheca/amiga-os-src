**********************************************************************
*
* messages.i
*
* Copyright (C) 1985, Commodore-Amiga Inc., All Rights Reserved
*
* $Id: messages.i,v 36.11 90/05/06 00:37:16 bryce Exp $
*
* $Locker:  $
*
**********************************************************************

		XREF	KPutFmt

ERRMSG:		MACRO	* level,msg
		
		IFGE	INFOLEVEL-\1

		PEA	subsysName(PC)
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@,A0
		LEA	4*4(SP),A1
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	end\@

msg\@		STRINGR	<\2>
		DS.W	0
end\@
		ENDC
		ENDM
		

PUTMSG:		MACRO	* level,msg
		
		IFGE	INFOLEVEL-\1

		PEA	subsysName(PC)
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@,A0
		LEA	4*4(SP),A1
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	end\@

msg\@		STRINGR	<\2>
		DS.W	0
end\@
		ENDC
		ENDM
