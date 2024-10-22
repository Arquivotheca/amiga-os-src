	TTL    '$Header: V38:src/workbench/devs/printer/xerox_4020/RCS/printertag.asm,v 1.3 90/04/09 09:14:53 daveb Exp $'
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
*   $Header: V38:src/workbench/devs/printer/xerox_4020/RCS/printertag.asm,v 1.3 90/04/09 09:14:53 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.3  90/04/09  09:14:53  daveb
*   for rcs 4.x header change
*   
*   Revision 1.2  88/04/15  17:25:53  daveb
*   fixed docs for devcon 
*   V1.3 Gamma 13
*   
*   Revision 1.1  88/01/15  15:52:05  daveb
*   changed XDotsInch from 120 to 121.
*   changed MaxXDots from 1088 to 1080
*   V1.3 Gamma 6 release
*   
*   Revision 1.0  87/08/20  13:53:15  daveb
*   added to rcs
*   
*	Revision 1.4  87/07/30  10:43:13  daveb
*	added 'DS.L 1' at end to reserve space for PrintMode
*
*	Revision 1.3  87/07/21  11:48:04  daveb
*	added 'PPC_VERSION_2' to PrinterClass
*
*	Revision 1.2  87/04/30  11:27:14  daveb
*	changed MaxColumns from 80 to 90 (Programmer's Guide, pg 1-2)
*	changed MaxXDots from 1024 to 1088 (same)
*	changed YDotsInch from 240 to 120 (Programmer's Guide, pg 1-6)
*
*	Revision 1.1  87/04/29  18:14:32  andy
*	Initial revision
*
*   Revision 32.4  86/06/30  21:05:59  andy
*   *** empty log message ***
*   
*   Revision 32.3  86/06/30  20:51:12  andy
*   enabled 8 bit char support
*   
*   Revision 32.2  86/06/10  12:55:43  andy
*   Corrected printer name
*   
*   Revision 32.1  86/02/10  14:32:06  kodiak
*   add null 8BitChars field
*   
*   Revision 32.0  86/02/10  14:21:40  kodiak
*   added to rcs for updating
*   
*   Revision 1.1  85/10/09  23:56:42  kodiak
*   replace reference to pdata w/ prtbase
*   
*   Revision 1.0  85/10/09  23:56:36  kodiak
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

	INCLUDE		"xerox_4020_rev.i"

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
	DC.B    90              ; MaxColumns
	DC.B    1               ; NumCharSets
	DC.W    4               ; NumRows
	DC.L    1080            ; MaxXDots
	DC.L    0               ; MaxYDots
	DC.W    121             ; XDotsInch
	DC.W    120             ; YDotsInch
	DC.L    _CommandTable   ; Commands
	DC.L    _DoSpecial
	DC.L    _Render
	DC.L    30              ; Timeout
	DC.L    _ExtendedCharTable ; 8BitChars
	DS.L	1		; PrintMode (reserve space)
	DC.L	0		; ptr to char conversion function

printerName:
	STRING	<'Xerox_4020'>

	END
