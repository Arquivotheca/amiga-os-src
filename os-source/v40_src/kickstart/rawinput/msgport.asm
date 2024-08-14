	TTL	'$Id: msgport.asm,v 35.4 90/04/13 12:46:01 kodiak Exp $
**********************************************************************
*
*			---------------
*   msgport.asm		RAWINPUT MODULE		message port support
*			---------------
*
*   Copyright 1985, 1987 Commodore-Amiga Inc.
*
*   Source Control	$Locker:  $
*
*   $Log:	msgport.asm,v $
*   Revision 35.4  90/04/13  12:46:01  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 35.3  90/04/02  16:30:23  kodiak
*   back to using dd_ExecBase, not SYSBASE (4)
*   
*   Revision 35.2  90/04/02  13:01:04  kodiak
*   for rcs 4.x header change
*   
*   Revision 35.1  87/11/12  13:35:31  kodiak
*   use CALLEXE instead of LINKEXE
*   
*   
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/execbase.i"
	INCLUDE		"macros.i"
	INCLUDE		"stddevice.i"


*------ Imported Names -----------------------------------------------

*------ Imported Functions -------------------------------------------

	XREF_EXE	AllocSignal
	XREF_EXE	FreeSignal


*------ Exported Functions -------------------------------------------

	XDEF		InitMsgPort


*------ InitMsgPort --------------------------------------------------
*
*   NAME
*	InitMsgPort - initialize a message port
*
*   SYNOPSYS
*	signal = InitMsgPort(msgPort), SysBase
*	D0		     A0        A6
*
*   FUNCTION
*	Initialize a port, usually for use by an I/O request.
*
*---------------------------------------------------------------------
InitMsgPort:
		MOVEM.L	A2,-(A7)
		MOVE.L	A0,A2
		MOVE.B	#NT_MSGPORT,LN_TYPE(A2)
		CLR.B	MP_FLAGS(A2)
		MOVE.L	ThisTask(a6),MP_SIGTASK(A2)
		LEA	MP_MSGLIST(A2),A0
		NEWLIST	A0
		MOVEQ	#-1,D0
		CALLEXE	AllocSignal
		MOVE.B	D0,MP_SIGBIT(A2)
		MOVEM.L	(A7)+,A2
		RTS

	END
