	TTL    '$Header: wait.asm,v 1.0 87/08/21 17:29:18 daveb Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*	wait
*
*   Source Control
*   --------------
*   $Header: wait.asm,v 1.0 87/08/21 17:29:18 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	wait.asm,v $
*   Revision 1.0  87/08/21  17:29:18  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:51:47  daveb
*   added to rcs
*   
*   
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/io.i"

	INCLUDE		"devices/timer.i"

	INCLUDE		"../printer/macros.i"
	INCLUDE		"../printer/prtbase.i"


*------ Imported Names -----------------------------------------------

*------ Imported Functions -------------------------------------------

	XREF_EXE	Forbid
	XREF_EXE	Permit
	XREF_EXE	WaitIO
	XREF		_SysBase

	XREF		_PD


*------ Exported Functions -------------------------------------------

	XDEF		_PWait


*------ printer.device/PWait -----------------------------------------
*
*   NAME
*	PWait - wait for a time
*
*   SYNOPSIS
*	PWait(seconds, microseconds);
*
*   FUNCTION
*	PWait uses the timer device to wait after writes are complete
*
*---------------------------------------------------------------------
_PWait:
		MOVEM.L	A4/A6,-(A7)
		MOVE.L	_PD,A4
		MOVE.L	pd_PBothReady(A4),A0
		JSR	(A0)
		TST.L	D0
		BNE.S	error

		LEA	pd_TIOR(A4),A1
		MOVE.W	#TR_ADDREQUEST,IO_COMMAND(A1)
		MOVE.L	12(A7),IOTV_TIME+TV_SECS(A1)
		MOVE.L	16(A7),IOTV_TIME+TV_MICRO(A1)
		CLR.B	IO_FLAGS(A1)
		MOVE.L	IO_DEVICE(A1),A6
		JSR	DEV_BEGINIO(A6)
		LINKEXE	Forbid
		LEA	pd_TIOR(A4),A1
		LINKEXE	WaitIO
		LINKEXE	Permit
		MOVEQ	#0,D0
		TST.L	D0
error:
		MOVEM.L	(A7)+,A4/A6
		RTS

		END
