*************************************************************************
*
* $Id: messages.i,v 38.1 91/06/24 19:02:03 mks Exp $
*
* $Log:	messages.i,v $
* Revision 38.1  91/06/24  19:02:03  mks
* Changed to V38 source tree - Trimmed Log
* 
*************************************************************************

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

msg\@		DC.B	\2,10,0
		DS.W	0
end\@
		ENDC
		ENDM


INFOMSG:	MACRO	* level,msg

		IFGE	INFOLEVEL-\1

		PEA	subsysName(PC)
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@,A0
		LEA	4*4(SP),A1
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	end\@

msg\@		DC.B	\2,10,0
		DS.W	0
end\@
		ENDC
		ENDM
