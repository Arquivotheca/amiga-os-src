	TTL    '$Header: printertag.asm,v 1.0 88/05/06 01:15:17 daveb Exp $'
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
*   $Header: printertag.asm,v 1.0 88/05/06 01:15:17 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.0  88/05/06  01:15:17  daveb
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

	INCLUDE		"tektronix_4696_rev.i"

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
	DC.B    PPC_COLORGFX    ; PrinterClass
	DC.B    PCC_YMCB        ; ColorClass
	DC.B    85              ; MaxColumns
	DC.B    1               ; NumCharSets
	DC.W    4               ; NumRows
	DC.L    1024            ; MaxXDots
	DC.L    0               ; MaxYDots
	DC.W    121             ; XDotsInch
	DC.W    120             ; YDotsInch
	DC.L    _CommandTable   ; Commands
	DC.L    _DoSpecial
	DC.L    _Render
	DC.L    45              ; Timeout (slow alpha printer)
	DC.L    _ExtendedCharTable ; 8BitChars
	DS.L	1		; PrintMode (reserve space)
	DC.L	0		; ptr to char conversion function

printerName:
	STRING	<'Tektronix_4696'>

	END
