	TTL    '$Header: printertag.asm,v 1.1 87/09/09 06:38:47 daveb Exp $'
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
*   $Header: printertag.asm,v 1.1 87/09/09 06:38:47 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.1  87/09/09  06:38:47  daveb
*   changed render ptr from null to Render
*   
*   Revision 1.0  87/08/20  13:46:49  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  12:11:38  daveb
*   added to rcs
*   
*   Revision 1.3  87/08/03  11:32:40  daveb
*   added null ptr to char conversion function at end of table
*   
*   Revision 1.2  87/07/30  10:41:28  daveb
*   added 'DS.L 1' at end to reserve space for PrintMode
*   
*   Revision 1.1  87/07/21  11:47:26  daveb
*   added 'PPC_VERSION_2' to PrinterClass
*   
*   Revision 1.0  87/07/21  11:46:22  daveb
*   added to rcs
*   
*   Revision 32.5  86/06/30  21:12:45  andy
*   *** empty log message ***
*   
*   Revision 32.4  86/06/11  16:00:36  andy
*   corrected printer name
*   
*   Revision 32.3  86/06/10  16:44:26  andy
*   tag for printer dependent code
*   
*   Revision 32.1  86/02/10  14:33:25  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:24:28  kodiak
*   added to rcs for updating
*   
*   Revision 1.1  85/10/09  23:59:05  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/09  23:58:58  kodiak
*   added to rcs for updating in version 1
*   
*   Revision 29.1  85/07/31  18:27:25  kodiak
*   change XDotsInch from 144 to 120
*   
*   Revision 29.0  85/07/31  18:26:50  kodiak
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

	INCLUDE		"okimate20_rev.i"

	INCLUDE		"../printer/prtbase.i"


*------ Imported Names -----------------------------------------------

	XREF		_Init
	XREF		_Expunge
	XREF		_Open
	XREF		_Close
	XREF		_CommandTable
	XREF		_PrinterSegmentData
	XREF		_DoSpecial
	XREF		_ExtendedCharTable
	XREF		_Render

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
		DC.B	PCC_YMC_BW	; ColorClass
		DC.B	80		; MaxColumns
		DC.B	1		; NumCharSets
		DC.W	24		; NumRows
		DC.L	960		; MaxXDots
		DC.L	0		; MaxYDots
		DC.W	120		; XDotsInch
		DC.W	144		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial
		DC.L	_Render	; Render
		DC.L	60		; slow color graphics printer
		DC.L	_ExtendedCharTable ; 8BitChars
		DS.L	1		; PrintMode (reserve space)
		DC.L	0		; ptr to char conversion function

printerName:
		STRING	<'Okimate-20'>

		END
