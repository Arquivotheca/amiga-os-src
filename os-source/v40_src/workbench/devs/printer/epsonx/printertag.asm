	TTL    '$Id: printertag.asm,v 1.5 91/07/09 14:28:12 darren Exp $'
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
*   $Id: printertag.asm,v 1.5 91/07/09 14:28:12 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.5  91/07/09  14:28:12  darren
*   Obtain include files from include directory.
*   
*   Revision 1.4  90/08/29  15:11:34  darren
*   Changed header
*   
*   Revision 1.3  90/04/06  20:04:39  daveb
*   for rcs 4.x header change
*   
*   Revision 1.2  88/04/14  12:03:35  daveb
*   V1.3 Gamma 11 release
*   
*   Revision 1.1  87/10/27  15:30:30  daveb
*   V1.3 gamma 1 check-in
*   
*   Revision 1.0  87/08/20  14:10:02  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  13:27:35  daveb
*   added to rcs
*   
*   Revision 1.3  87/08/03  11:05:54  daveb
*   added null ptr to char conversion function at end of table
*   
*   Revision 1.2  87/07/30  10:35:12  daveb
*   added 'DS.L 1' at end to reserve space for PrintMode
*   
*   Revision 1.1  87/07/21  11:37:42  daveb
*   added 'PPC_VERSION_2' to PrinterClass
*   
*   Revision 1.0  87/07/21  11:37:10  daveb
*   added to rcs
*   
*   Revision 32.6  86/06/30  21:07:52  andy
*   *** empty log message ***
*   
*   Revision 32.5  86/06/30  20:54:48  andy
*   enabled 8 bit characters
*   
*   Revision 32.4  86/06/11  16:16:44  andy
*   *** empty log message ***
*   
*   Revision 32.3  86/06/10  12:57:11  andy
*   Corrected printer name
*   
*   Revision 32.2  86/02/12  18:16:13  kodiak
*   YDotsInch -> 72
*   
*   Revision 32.1  86/02/10  14:32:51  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:22:56  kodiak
*   added to rcs for updating
*   
*   Revision 1.1  85/10/09  23:57:45  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/09  23:57:39  kodiak
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

	INCLUDE		"epsonX_rev.i"

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
		DC.B	PPC_COLORGFX	;PrinterClass
		DC.B	PCC_YMCB	; ColorClass
		DC.B	136		; MaxColumns
		DC.B	10		; NumCharSets
		DC.W	8		; NumRows
		DC.L	1632		; MaxXDots
		DC.L	0		; MaxYDots
		DC.W	120		; XDotsInch
		DC.W	72		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial
		DC.L	_Render
		DC.L	30		; Timeout
		DC.L	_ExtendedCharTable	; 8BitChars
		DS.L	1		; PrintMode (reserve space)
		DC.L	0		; ptr to char conversion function

printerName:
		STRING	<'EpsonX'>

		END
