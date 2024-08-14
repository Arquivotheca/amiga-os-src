	TTL    '$Id: dumprport.asm,v 1.1 90/07/27 02:19:01 bryce Exp $'
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
*   $Id: dumprport.asm,v 1.1 90/07/27 02:19:01 bryce Exp $
*
*   $Locker:  $
*
*   $Log:	dumprport.asm,v $
*   Revision 1.1  90/07/27  02:19:01  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.0  87/08/21  17:26:39  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:51:24  daveb
*   added to rcs
*   
*   Revision 25.1  85/06/16  01:04:45  kodiak
*   *** empty log message ***
*   
*   Revision 25.0  85/06/14  04:08:43  kodiak
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

	XREF		_PCDumpRPort

	XREF		EndCommand


*------ Exported Names -----------------------------------------------

	XDEF		PCDumpRPort

	XDEF		PCBDumpRPort


*------ printer.device/PCDumpRPort -----------------------------------
*
*   NAME
*	PCDumpRPort - dump a RastPort to the printer
*
*   FUNCTION
*	This command will issue a text attribute change command to
*	the printer, according to the printer command and parameters
*	in the request.  This command is translated to the appropriate
*	device dependent printer control code and output.
*
*   IO REQUEST
*	This command uses the non-standard IO request IODRPReq.
*
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	as initialized by the call to OpenDevice
*	io_Unit		as initialized by the call to OpenDevice
*	io_Command	PRD_DUMPRPORT
*	io_Flags	IOB_QUICK set if quick I/O is possible
*	io_RastPort	the RastPort to print
*	io_ColorMap	the ColorMap associated with the RastPort
*	io_XSize	the x size of the print
*	io_YSize	the y size of the print
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	_PCDumpRPort(printerData, ioRequest) is called.  It returns
*	an error code to be passed to EndCommand.
*
**********************************************************************
PCBDumpRPort	EQU	0

PCDumpRPort:
		MOVE.L	A1,-(A7)	; save IORequest
		MOVE.L	A1,-(A7)	; pass IORequest as parm
		JSR	_PCDumpRPort
		ADDQ.L	#4,A7
		MOVE.L	(A7)+,A1
		BSR	EndCommand
		RTS

		END
