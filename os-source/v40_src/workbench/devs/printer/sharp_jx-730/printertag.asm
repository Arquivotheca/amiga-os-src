	TTL    '$Header: /usr.MC68010/ghostwheel/darren/V38/printer/sharp_jx-730/RCS/printertag.asm,v 1.3 91/04/08 11:05:10 darren Exp $'
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
*   $Header: /usr.MC68010/ghostwheel/darren/V38/printer/sharp_jx-730/RCS/printertag.asm,v 1.3 91/04/08 11:05:10 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.3  91/04/08  11:05:10  darren
*   Header change
*   
*   Revision 1.2  90/04/09  09:06:28  daveb
*   for rcs 4.x header change
*   
*   Revision 1.1  90/03/06  15:18:58  daveb
*   changed X and Y density to 180
*   
*   Revision 1.0  90/03/03  16:58:54  daveb
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

	INCLUDE		"sharp_jx-730_rev.i"

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
	DC.W	35
	DC.W	REVISION
_PEDData:
	DC.L	printerName
	DC.L	_Init
	DC.L	_Expunge
	DC.L	_Open
	DC.L	_Close
	DC.B    PPC_COLORGFX    ; PrinterClass
	DC.B    PCC_YMCB        ; ColorClass
	DC.B	0		; MaxColumns (filled in)
	DC.B    1               ; NumCharSets
	DC.W    4               ; NumRows
	DC.L	0		; MaxXDots (filled in)
	DC.L	0               ; MaxYDots
	DC.W	180		; XDotsInch
	DC.W	180		; YDotsInch
	DC.L    _CommandTable   ; Commands
	DC.L    _DoSpecial
	DC.L    _Render
	DC.L    30		; Timeout
	DC.L    _ExtendedCharTable ; 8BitChars
	DS.L	1		; PrintMode (reserve space)
	DC.L	0		; ptr to char conversion function

printerName:
	STRING	<'Sharp_JX-730'>

	END
