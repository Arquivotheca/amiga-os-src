	TTL    '$Id: printertag.asm,v 1.3 90/08/29 12:42:31 darren Exp $'
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
*   $Id: printertag.asm,v 1.3 90/08/29 12:42:31 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.3  90/08/29  12:42:31  darren
*   Change $Header to $Id
*   
*   Revision 1.2  90/04/06  20:06:58  daveb
*   for rcs 4.x header change
*   
*   Revision 1.1  88/06/05  18:13:09  daveb
*   V1.3 Gamma 15        
*   
*   Revision 1.0  88/04/05  04:55:47  daveb
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

	INCLUDE		"howtek_rev.i"

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
	XREF		_ConvFunc

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
		DC.B	80		; MaxColumns
		DC.B	1		; NumCharSets
		DC.W	1		; NumRows
		DC.L	640		; MaxXDots
		DC.L	800		; MaxYDots
		DC.W	80		; XDotsInch
		DC.W	80		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial
		DC.L	_Render
		DC.L	60
		DC.L	_ExtendedCharTable	; 8BitChars
		DS.L	1		; PrintMode (reserve space)
		DC.L	_ConvFunc	; ptr to char conversion function

printerName:
		STRING	<'Howtek_Pixelmaster'>

		END
