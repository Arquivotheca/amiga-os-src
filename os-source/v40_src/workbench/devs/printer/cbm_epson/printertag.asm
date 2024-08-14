	TTL    '$Id: printertag.asm,v 1.3 91/07/08 14:20:40 darren Exp $'
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
*   $Header: V38:src/workbench/devs/printer/cbm_epson/RCS/printertag.asm,v 1.3 91/07/08 14:20:40 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.3  91/07/08  14:20:40  darren
*   Header change, and reference include files from standard includes
*   directory.
*   
*   Revision 1.2  90/04/06  19:30:26  daveb
*   for rcs 4.x header change
*   
*   Revision 1.1  87/09/09  06:41:30  daveb
*   changed incorrect 'DS.L' to 'DC.L' for CharConversionFunc
*   
*   Revision 1.0  87/08/20  13:55:55  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  13:22:11  daveb
*   added to rcs
*   
*   Revision 1.4  87/08/03  11:38:42  daveb
*   comment cleanup
*   
*   Revision 1.3  87/08/03  10:44:58  daveb
*   added null ptr to conversion function at end of table
*   
*   Revision 1.2  87/07/30  10:29:01  daveb
*   added 'DS.L 1' at end to reserve space for PrintMode
*   
*   Revision 1.1  87/07/21  11:38:58  daveb
*   added 'PPC_VERSION_2' to PrinterClass
*   
*   Revision 1.0  87/07/21  11:38:50  daveb
*   added to rcs
*   
*   Revision 32.6  86/06/30  21:05:28  andy
*   *** empty log message ***
*   
*   Revision 32.5  86/06/11  15:56:39  andy
*   corrected printer name
*   
*   Revision 32.4  86/06/10  16:42:35  andy
*   CBM_MPS1000 printer driver
*   
*   Revision 32.2  86/02/12  18:15:38  kodiak
*   YDotsInch -> 72
*   
*   Revision 32.1  86/02/10  14:31:57  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:21:21  kodiak
*   added to rcs for updating
*   
*   Revision 1.1  85/10/09  23:56:25  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/09  23:56:20  kodiak
*   added to rcs for updating in version 1
*   
*   Revision 29.1  85/08/19  06:40:56  kodiak
*   change CBM Epson to CBM MPS1000 in name.
*   
*   Revision 29.0  85/08/19  06:40:34  kodiak
*   added to rcs for updating in version 29
*   
*   Revision 25.1  85/06/16  01:02:15  kodiak
*   *** empty log message ***
*   
*   Revision 25.0  85/06/15  06:40:00  kodiak
*   added to rcs
*   
*   Revision 25.0  85/06/13  18:53:36  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"cbm_epson_rev.i"

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
		DC.L	printerName	; ptr tp printer name string
		DC.L	_Init		; ptr to Init function
		DC.L	_Expunge	; ptr to Expunge function
		DC.L	_Open		; ptr to Open function
		DC.L	_Close		; ptr to Close function
		DC.B	PPC_BWGFX	; PrinterClass
		DC.B	PCC_BW	; ColorClass
		DC.B	80		; MaxColumns
		DC.B	10		; NumCharSets
		DC.W	8		; NumRows
		DC.L	960		; MaxXDots
		DC.L	0		; MaxYDots
		DC.W	120		; XDotsInch
		DC.W	72		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial	; ptr to special cmd handler
		DC.L	_Render		; ptr to render function
		DC.L	30		; delay (in secs) for printer trouble requestor
		DC.L	_ExtendedCharTable ; 8BitChars
		DS.L    1		; PrintMode (reserve space)
		DC.L	0		; ptr to char conversion function

printerName:
		STRING	<'CBM_MPS1000'>

		END
