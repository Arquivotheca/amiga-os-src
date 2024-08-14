	TTL    '$Header: /usr.MC68010/ghostwheel/darren/V38/printer/epsonQ/RCS/printertag.asm,v 1.6 91/07/03 11:47:11 darren Exp $'
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
*	printer device dependent code tag
*
*   Source Control
*   --------------
*   $Header: /usr.MC68010/ghostwheel/darren/V38/printer/epsonQ/RCS/printertag.asm,v 1.6 91/07/03 11:47:11 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.6  91/07/03  11:47:11  darren
*   Obtain include files from standard include directory
*   
*   Revision 1.5  88/04/19  17:12:34  daveb
*   V1.3 Gamma 13
*   
*   Revision 1.4  88/04/15  16:42:12  daveb
*   fixed docs for devcon
*   V1.3 Gamma 13
*   
*   Revision 1.3  87/10/27  15:26:31  daveb
*   V1.3 gamma 1 check-in.
*   
*   Revision 1.2  87/10/01  09:17:56  daveb
*   changed NumRows to 16 to support 'weak' 24-pin printers
*   V1.3 beta 4 check-in
*   
*   Revision 1.1  87/08/21  10:01:13  daveb
*   set XDotsInch and MaxXDots to default (60 dpi) values
*   
*   Revision 1.0  87/08/20  14:08:42  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  13:27:02  daveb
*   added to rcs
*   
*   Revision 1.3  87/08/03  11:03:52  daveb
*   added null ptr to char conversion function at end of table
*   
*   Revision 1.2  87/07/30  10:34:12  daveb
*   added 'DS.L 1' at end to reserve space for PrintMode
*   
*   Revision 1.1  87/07/21  11:36:30  daveb
*   added 'PPC_VERSION_2' to PrinterClass
*   
*   Revision 1.0  87/07/21  11:35:43  daveb
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"epsonQ_rev.i"

	INCLUDE		"devices/prtbase.i"


*------ Imported Names -----------------------------------------------

	XREF		_Init
	XREF		_Expunge
	XREF		_Open
	XREF		_Close
	XREF		_CommandTable
	XREF		_PrinterSegmentData
	XREF		_DoSpecial
	XREF		_Render
	XREF		_ExtendedCharTable

*------ Exported Names -----------------------------------------------

	XDEF		_PEDData


**********************************************************************

		MOVEQ	#0,D0		; show error for OpenLibrary()
		RTS
		DC.W	VERSION
		DC.W	REVISION
_PEDData:
		DC.L	printerName
		DC.L	_Init
		DC.L	_Expunge
		DC.L	_Open
		DC.L	_Close
		DC.B	PPC_COLORGFX	; PrinterClass
		DC.B	PCC_YMCB	; ColorClass
		DC.B	136		; MaxColumns
		DC.B	10		; NumCharSets
		DC.W	24		; NumRows
		DC.L	1088		; MaxXDots
		DC.L	0		; MaxYDots
		DC.W	80		; XDotsInch
		DC.W	180		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial
		DC.L	_Render
		DC.L	30		; Timeout
		DC.L	_ExtendedCharTable	; 8BitChars
		DS.L    1		; PrintMode (reserve space)
		DC.L	0		; ptr to char conversion function

printerName:
		STRING	<'EpsonQ'>

		END
