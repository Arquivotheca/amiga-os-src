*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* debug.i
*
* Source Control
* ------ -------
* 
* $Id: debug.i,v 36.8 91/04/09 13:45:43 darren Exp $
*
* $Locker:  $
*
* $Log:	debug.i,v $
* Revision 36.8  91/04/09  13:45:43  darren
* *** empty log message ***
* 
* Revision 36.7  91/03/22  12:19:15  darren
* no changes
* 
* Revision 36.6  91/03/14  17:29:23  darren
* *** empty log message ***
* 
* Revision 36.5  91/01/25  15:45:52  rsbx
* Change to V37
* 
* Revision 36.4  90/04/01  19:12:30  rsbx
* RCS version change.
* 
* Revision 36.3  89/09/15  18:54:56  rsbx
* 
* 
* Revision 36.2  89/08/09  19:25:06  rsbx
* *** empty log message ***
* 
* Revision 36.1  89/08/09  18:10:19  rsbx
* *** empty log message ***
* 
* Revision 1.1  89/08/09  14:05:16  rsbx
* Initial revision
*
*
*************************************************************************

		XREF	KPutStr
		XREF	KPutFmt

ERRMSG:		MACRO	* msg
		
		MOVEM.L	A0/A1/D0/D1,-(SP)
		DISABLE	A0
		LEA	msg\@,A0
		JSR	KPutStr
		ENABLE	A0
		MOVEM.L (SP)+,D0/D1/A0/A1
		BRA.S	end\@

msg\@		dc.b	\1
		dc.b	10
		dc.b	0
		DS.W	0
end\@
		ENDM


DPRINTF:	MACRO	* msg
		
		MOVEM.L	A0/A1/D0/D1,-(SP)
		LEA	dmsg\@,A0
		LEA	4*4(SP),A1
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	dend\@

dmsg\@		dc.b	<\1>
		Dc.b	10
		dc.b	0
		ds.w	0
dend\@
		ENDM
