	TTL    '$Header: printertag.asm,v 1.0 87/08/20 14:05:36 daveb Exp $'
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
*   $Header: printertag.asm,v 1.0 87/08/20 14:05:36 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.0  87/08/20  14:05:36  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  13:25:54  daveb
*   added to rcs
*   
*   Revision 1.4  87/08/03  10:58:07  daveb
*   added ptr to char conversion function at end of table
*   
*   Revision 1.3  87/07/30  10:27:11  daveb
*   added 'DS.L 1' at end to reserve space for PrintMode
*   
*   Revision 1.2  87/07/21  11:32:59  daveb
*   added 'PPC_VERSION_2' to PrinterClass
*   
*   Revision 1.1  87/04/30  11:56:12  daveb
*   *	changed MaxColumns from 80 to 85 (Operator's Guide, pg 10-4)
*   
*   Revision 1.0  87/04/30  11:53:10  daveb
*   added to rcs
*   
*   Revision 32.5  86/06/30  21:06:46  andy
*   *** empty log message ***
*   
*   Revision 32.4  86/06/11  15:57:04  andy
*   corrected printer name
*   
*   Revision 32.3  86/06/10  16:43:37  andy
*   printer dependent file
*   
*   Revision 32.1  86/02/10  14:32:33  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:22:17  kodiak
*   added to rcs for updating
*   
*   Revision 1.2  85/10/09  23:57:10  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.1  85/09/25  18:45:12  kodiak
*   double timeout: alpha is too slow to print 400 chars in 30 sec.
*   
*   Revision 1.0  85/09/25  18:32:57  kodiak
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

	INCLUDE		"diablo_c_rev.i"

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

	MOVEQ	#0,D0	; show error for OpenLibrary()
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
	DC.B	PCC_YMCB		; ColorClass
	DC.B	85				; MaxColumns
	DC.B	1				; NumCharSets
	DC.W	4				; NumRows
	DC.L	1024			; MaxXDots
	DC.L	0				; MaxYDots
	DC.W	120				; XDotsInch
	DC.W	120				; YDotsInch
	DC.L	_CommandTable	; Commands
	DC.L	_DoSpecial
	DC.L	_Render
	DC.L	60				; twice normal: slow alpha
	DC.L	_ExtendedCharTable ; 8BitChars
	DS.L	1				; PrintMode (reserve space)
	DC.L	0				; ptr to char conversion function

printerName:
	STRING	<'Diablo_C-150'>

	END
