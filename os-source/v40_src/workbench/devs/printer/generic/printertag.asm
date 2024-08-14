	TTL    '$Header: /usr.MC68010/ghostwheel/darren/printer/generic/RCS/printertag.asm,v 1.1 90/04/06 20:05:51 daveb Exp Locker: darren $'
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
*   $Header: /usr.MC68010/ghostwheel/darren/printer/generic/RCS/printertag.asm,v 1.1 90/04/06 20:05:51 daveb Exp Locker: darren $
*
*   $Locker: darren $
*
*   $Log:	printertag.asm,v $
*   Revision 1.1  90/04/06  20:05:51  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/20  14:11:42  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  13:28:13  daveb
*   added to rcs
*   
*   Revision 32.1  86/02/10  14:33:00  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:23:28  kodiak
*   added to rcs for updating
*   
*   Revision 1.1  85/10/09  23:58:01  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/09  23:57:55  kodiak
*   added to rcs for updating in version 1
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"generic_rev.i"

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
		DC.L	0		; 8BitChars
		DS.L	1	; PrintMode (reserve space)
		DC.L	0	; ptr to char conversion function

printerName:
		STRING	<'generic'>

		END
