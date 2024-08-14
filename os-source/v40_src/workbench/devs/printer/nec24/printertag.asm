	TTL    '$Header: /usr.MC68010/ghostwheel/darren/V38/printer/nec24/RCS/printertag.asm,v 1.2 91/07/03 14:50:15 darren Exp $'
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
*   $Header: /usr.MC68010/ghostwheel/darren/V38/printer/nec24/RCS/printertag.asm,v 1.2 91/07/03 14:50:15 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.2  91/07/03  14:50:15  darren
*   Obtain includes from standard includes directory instead of
*   relative to printer directory.
*   
*   Revision 1.1  90/04/06  20:10:57  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  88/03/11  13:32:27  daveb
*   added to rcs
*   
*   
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"nec24_rev.i"

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
	DC.W	90		; XDotsInch
	DC.W	180		; YDotsInch
	DC.L	_CommandTable	; Commands
	DC.L	_DoSpecial
	DC.L	_Render
	DC.L	30
	DC.L	_ExtendedCharTable	; 8BitChars
	DS.L    1		; PrintMode (reserve space)
	DC.L	0		; ptr to char conversion function

printerName:
	STRING	<'Nec_Pinwriter'>

	END
