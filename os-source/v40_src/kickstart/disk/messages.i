
********************************************************************
*
* $Id: messages.i,v 27.2 90/07/13 14:59:10 jesup Exp $
*
* $Locker:  $
*
********************************************************************

		XREF	KPutFmt

ERRMSG:		MACRO	* level,msg
		
		IFGE	INFOLEVEL-\1

		PEA	diskName(PC)
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

		PEA	diskName(PC)
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
