	TTL    '$Id: printertag.asm,v 1.6 91/07/01 10:50:22 darren Exp $'
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
*   $Id: printertag.asm,v 1.6 91/07/01 10:50:22 darren Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.6  91/07/01  10:50:22  darren
*   Refer to include directory instead of ../printer
*   
*   Revision 1.5  90/08/29  12:53:50  darren
*   Change header
*   
*   Revision 1.4  90/04/06  20:08:16  daveb
*   for rcs 4.x header change
*   
*   Revision 1.3  88/06/05  18:15:28  daveb
*   V1.3 Gamma 15        
*   
*   Revision 1.2  88/04/15  16:58:35  daveb
*   fixed docs for devcon
*   V1.3 Gamma 13
*   
*   Revision 1.1  87/10/27  15:33:12  daveb
*   V1.3 gamma 1 check-in
*   
*   Revision 1.0  87/08/20  14:12:28  daveb
*   added to rcs
*   
*   Revision 1.0  87/08/20  13:28:44  daveb
*   added to rcs
*   
*   Revision 1.3  87/08/03  11:09:33  daveb
*   added null ptr to char conversion function at end of table
*   
*   Revision 1.2  87/07/30  10:37:11  daveb
*   added 'DS.L 1' at end to reserve space for PrintMode
*   
*   Revision 1.1  87/07/21  11:42:04  daveb
*   added 'PPC_VERSION_2' to PrinterClass
*   
*   Revision 1.0  87/07/21  11:41:36  daveb
*   added to rcs
*   
*   Revision 32.4  86/06/30  21:09:33  andy
*   *** empty log message ***
*   
*   Revision 32.3  86/06/30  20:55:47  andy
*   enabled 8 bit characters
*   
*   Revision 32.2  86/06/10  12:58:00  andy
*   Corrected printer name
*   
*   Revision 32.1  86/02/10  14:33:17  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:23:56  kodiak
*   added to rcs for updating
*   
*   Revision 1.2  85/10/09  23:58:23  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.1  85/10/09  16:11:31  kodiak
*   daveb density changes
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

	INCLUDE		"hp_rev.i"

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
		DC.B	PPC_BWGFX	; PrinterClass
		DC.B	PCC_BW		; ColorClass
		DC.B	0		; MaxColumns
		DC.B	0		; NumCharSets
		DC.W	1		; NumRows
		DC.L	600		; MaxXDots
		DC.L	795		; MaxYDots
		DC.W	75		; XDotsInch
		DC.W	75		; YDotsInch
		DC.L	_CommandTable	; Commands
		DC.L	_DoSpecial
		DC.L	_Render
		DC.L	30		; Timeout
		DC.L	_ExtendedCharTable	; 8BitChars
		DS.L	1		; PrintMode (reserve space)
		DC.L	_ConvFunc	; ptr to char conversion function

printerName:
		STRING	<'HP_LaserJet'>

		END
