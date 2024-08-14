
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* messages.i
*
* Source Control
* ------ -------
* 
* $Header: messages.i,v 27.1 85/06/24 13:37:23 neil Exp $
*
* $Locker:  $
*
* $Log:	messages.i,v $
* Revision 27.1  85/06/24  13:37:23  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:56  neil
* *** empty log message ***
* 
* 
*************************************************************************

		XREF	KPutFmt

PUTFMT:		MACRO
		JSR	KPutFmt
		ENDM

INFOMSG:	MACRO	* level,msg
		
		IFGE	INFO_LEVEL-\1

		PEA	hdName
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@,A0
		LEA	4*4(SP),A1
		PUTFMT
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	end\@

msg\@		DC.B	\2,10,13,0
		DS.W	0
end\@
		ENDC
		ENDM
		

PUTMSG:		MACRO	* level,msg
		
		IFGE	INFO_LEVEL-\1

		PEA	hdName
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@,A0
		LEA	4*4(SP),A1
		PUTFMT
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	end\@

msg\@		DC.B	\2,10,13,0
		DS.W	0
end\@
		ENDC
		ENDM
