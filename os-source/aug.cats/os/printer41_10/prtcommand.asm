	TTL    '$Id: prtcommand.asm,v 1.2 90/07/27 02:19:09 bryce Exp $'
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
*   $Id: prtcommand.asm,v 1.2 90/07/27 02:19:09 bryce Exp $
*
*   $Locker:  $
*
*   $Log:	prtcommand.asm,v $
*   Revision 1.2  90/07/27  02:19:09  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:24:43  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:27:25  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:50:57  daveb
*   added to rcs
*   
*   Revision 25.1  85/06/16  01:05:21  kodiak
*   *** empty log message ***
*   
*   Revision 25.0  85/06/14  04:08:58  kodiak
*   added to rcs
*   
*   Revision 25.0  85/06/13  18:53:25  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"


*------ Imported Names -----------------------------------------------

	XREF		_PCPrtCommand

	XREF		EndCommand


*------ Exported Names -----------------------------------------------

	XDEF		PCPrtCommand

	XDEF		PCBPrtCommand


*------ printer.device/PCPrtCommand ----------------------------------
*
*   NAME
*	PCPrtCommand - perform a printer command
*
*   FUNCTION
*	This command will issue a text attribute change command to
*	the printer, according to the printer command and parameters
*	in the request.  This command is translated to the appropriate
*	device dependent printer control code and output.
*
*   IO REQUEST
*	This command uses the non-standard IO request IOPrtCmdReq.
*
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	as initialized by the call to OpenDevice
*	io_Unit		as initialized by the call to OpenDevice
*	io_Command	PRD_PRTCOMMAND
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_PrtCommand	The printer command
*	io_Parm0	The first (optional) parameter
*	io_Parm1	The second (optional) parameter
*	io_Parm2	The third (optional) parameter
*	io_Parm3	The fourth (optional) parameter
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	_PCPrtCommand(printerData, ioRequest) is called.  It returns
*	an error code to be passed to EndCommand.
*
**********************************************************************
PCBPrtCommand	EQU	0

PCPrtCommand:
		MOVE.L	A1,-(A7)	; save IORequest
		MOVE.L	A1,-(A7)	; pass IORequest as parm
		JSR	_PCPrtCommand
		ADDQ.L	#4,A7
		MOVE.L	(A7)+,A1
		BSR	EndCommand
		RTS

		END
