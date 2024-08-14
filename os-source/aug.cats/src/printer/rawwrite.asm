	TTL    '$Id: rawwrite.asm,v 1.4 91/02/14 15:26:13 darren Exp $'
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
*	printer device nonstandard command C interface
*
*   Source Control
*   --------------
*   $Id: rawwrite.asm,v 1.4 91/02/14 15:26:13 darren Exp $
*
*   $Locker:  $
*
*   $Log:	rawwrite.asm,v $
*   Revision 1.4  91/02/14  15:26:13  darren
*   Autodoc stuff
*   
*   Revision 1.3  90/07/27  02:19:16  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.2  90/04/06  19:25:12  daveb
*   for rcs 4.x header change
*   
*   Revision 1.1  88/08/17  10:31:03  daveb
*   made changes to autodocs
*   
*   Revision 1.0  87/08/21  17:27:53  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:50:38  daveb
*   added to rcs
*   
*   Revision 25.1  85/06/16  01:04:56  kodiak
*   *** empty log message ***
*   
*   Revision 25.0  85/06/13  18:53:25  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/io.i"


*------ Imported Names -----------------------------------------------

	XREF		PWrite
	XREF		_PBothReady
	XREF		EndCommand


*------ Exported Names -----------------------------------------------

	XDEF		PCRawWrite

	XDEF		PCBRawWrite


******* printer.device/PRD_RAWWRITE **************************************
*
*   NAME
*	PRD_RAWWRITE -- transparent write command
*
*   FUNCTION
*	This is a non standard write command that performs no
*	processing on the data passed to it.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Command	PRD_RAWWRITE
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_Length	the number of bytes in io_Data
*	io_Data		the raw bytes to write to the printer
*
**********************************************************************
PCBRawWrite	EQU	0

PCRawWrite:
		MOVE.L	A1,-(A7)	; save IORequest
		MOVE.L	IO_DATA(A1),A0
		MOVE.L	IO_LENGTH(A1),D0
		CLR.L	IO_ACTUAL(A1)
		BSR	PWrite
		TST.L	D0
		BNE.S	finishWrite
		MOVE.L	(A7),A1
		MOVE.L	IO_LENGTH(A1),IO_ACTUAL(A1)
finishWrite:
		BSR	_PBothReady
		MOVE.L	(A7)+,A1
		BSR	EndCommand
		RTS

		END
