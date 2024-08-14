*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: messages.i,v 34.9 87/12/04 19:14:44 bart Exp $
*
*	$Locker:  $
*
*	$Log:	messages.i,v $
*   Revision 34.9  87/12/04  19:14:44  bart
*   checkpoint
*   
*   Revision 34.8  87/12/04  12:09:26  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.7  87/10/14  15:52:48  bart
*   10-13 rev 1
*   
*   Revision 34.6  87/10/14  14:16:42  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.5  87/07/08  14:01:35  bart
*   y
*   
*   Revision 34.4  87/06/11  15:48:58  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  10:59:57  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:36:25  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:40:02  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:40:27  bart
*   added to rcs for updating
*   
*
*******************************************************************************


*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************



		XREF	KPutFmt

PUTFMT:		MACRO
		JSR	KPutFmt
		ENDM

INFOMSG:	MACRO	* level,msg
		
		IFGE	INFO_LEVEL-\1

		PEA	hdName(PC)
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@(PC),A0
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

		PEA	hdName(PC)
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	msg\@(PC),A0
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
