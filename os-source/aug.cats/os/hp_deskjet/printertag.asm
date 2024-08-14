	TTL    '$Id: printertag.asm,v 1.5 91/07/01 13:05:12 darren Exp $'
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
*   $Id: printertag.asm,v 1.5 91/07/01 13:05:12 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.5  91/07/01  13:05:12  darren
*   Use includes from includes directory.
*
*   Revision 1.4  90/08/29  13:56:24  darren
*   *** empty log message ***
*
*   Revision 1.3  90/08/29  13:32:53  darren
*   Change header
*
*   Revision 1.2  90/04/09  09:24:10  daveb
*   for rcs 4.x header change
*
*   Revision 1.1  88/06/05  17:48:08  daveb
*   V1.3 Gamma 15
*
*   Revision 1.0  87/10/27  15:21:01  daveb
*   added to rcs
*
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"HP_Deskjet_rev.i"

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
		DC.B	PCC_YMC_BW	; ColorClass
		DC.B	0		; MaxColumns
		DC.B	0		; NumCharSets
		DC.W	1		; NumRows
		DC.L	2400		; MaxXDots
		DC.L	3000		; MaxYDots
		DC.W	300		; XDotsInch
		DC.W	300		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial
		DC.L	_Render
		DC.L	30
		DC.L	_ExtendedCharTable	; 8BitChars
		DS.L	1	; PrintMode (reserve space)
		DC.L	_ConvFunc	; ptr to char conversion function

printerName:
	DC.B	"HP_DeskJet"
	DC.B	0

		END
