	TTL    '$Header: printertag.asm,v 1.1 87/09/08 11:39:13 daveb Exp $'
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
*   $Header: printertag.asm,v 1.1 87/09/08 11:39:13 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.1  87/09/08  11:39:13  daveb
*   reserved space for PrintMode and added null ptr to char conversion function.
*   
*   Revision 1.0  87/08/20  14:03:54  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  13:25:16  daveb
*   added to rcs
*   
*   Revision 32.4  86/06/30  21:06:24  andy
*   *** empty log message ***
*   
*   Revision 32.3  86/06/30  20:52:34  andy
*   enabled 8 bit characters
*   
*   Revision 32.2  86/06/10  12:56:13  andy
*   Corrected printer name
*   
*   Revision 32.1  86/02/10  14:32:18  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:21:54  kodiak
*   added to rcs for updating
*   
*   Revision 1.1  85/10/09  23:56:58  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/09  23:56:52  kodiak
*   added to rcs for updating in version 1
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

	INCLUDE		"diablo_adv_rev.i"

	INCLUDE		"../printer/prtbase.i"


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
		DC.B	PPC_BWALPHA	; PrinterClass
		DC.B	PCC_BW		; ColorClass
		DC.B	80		; MaxColumns
		DC.B	1		; NumCharSets
		DC.W	0		; NumRows
		DC.L	0		; MaxXDots
		DC.L	0		; MaxYDots
		DC.W	0		; XDotsInch
		DC.W	0		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial
		DC.L	_Render
		DC.L	30
		DC.L	_ExtendedCharTable	; 8BitChars
		DS.L	1	; PrintMode (reserve space)
		DC.L	0	; ptr to char conversion function

printerName:
		STRING	<'Diablo_Advantage_D25'>

		END
