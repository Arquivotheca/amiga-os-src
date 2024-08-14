	TTL    '$Id: write.asm,v 1.2 90/07/27 02:19:48 bryce Exp $'
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
*	printer device write command C interface
*
*   Source Control
*   --------------
*   $Id: write.asm,v 1.2 90/07/27 02:19:48 bryce Exp $
*
*   $Locker:  $
*
*   $Log:	write.asm,v $
*   Revision 1.2  90/07/27  02:19:48  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:26:03  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:29:28  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:50:16  daveb
*   added to rcs
*   
*   Revision 25.1  85/06/16  01:05:35  kodiak
*   *** empty log message ***
*   
*   Revision 25.0  85/06/13  18:53:50  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"


*------ Imported Names -----------------------------------------------

	XREF		_PCWrite

	XREF		EndCommand


*------ Exported Names -----------------------------------------------

	XDEF		PCWrite

	XDEF		PCBWrite


*------ printer.device/Write -----------------------------------------
*
*   NAME
*	Write - write ANSI text to printer
*
*   FUNCTION
*	Write the ANSI text contained in the command to the printer
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Command	CMD_WRITE
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	the number of bytes in io_Data
*	io_Data		the ANSI characters to interpret and write to the printer
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	_PCWrite(printerData, ioRequest) is called.  It returns an
*	error code to be passed to EndCommand.
*
**********************************************************************
PCBWrite	EQU	0

PCWrite:
		MOVE.L	A1,-(A7)	; save IORequest
		MOVE.L	A1,-(A7)	; pass IORequest as parm
		JSR	_PCWrite
		ADDQ.L	#4,A7
		MOVE.L	(A7)+,A1
		BSR	EndCommand
		RTS

		END
