	TTL    '$Id: init.asm,v 1.2 90/08/29 13:29:33 darren Exp $'
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*	printer device functions
*
*   Source Control
*   --------------
*   $Id: init.asm,v 1.2 90/08/29 13:29:33 darren Exp $
*
*   $Locker:  $
*
*   $Log:	init.asm,v $
*   Revision 1.2  90/08/29  13:29:33  darren
*   Change header to $Id
*
*   Revision 1.1  90/04/06  19:59:38  daveb
*   for rcs 4.x header change
*
*   Revision 1.0  87/10/27  15:19:00  daveb
*   added to rcs
*
*   Revision 1.0  87/08/20  14:12:39  daveb
*   added to rcs
*
*   Revision 1.0  87/08/20  13:30:11  daveb
*   added to rcs
*
*   Revision 1.1  85/10/09  19:27:38  kodiak
*   remove _stdout variable
*
*   Revision 1.0  85/10/09  19:23:53  kodiak
*   added to rcs for updating in version 1
*
*   Revision 29.1  85/08/02  16:58:43  kodiak
*   remove dummy _Close routine -- it's used to finish print of last page.
*
*   Revision 29.0  85/08/02  16:58:17  kodiak
*   added to rcs for updating in version 29
*
*   Revision 25.0  85/06/16  01:01:22  kodiak
*   added to rcs
*
*
**********************************************************************

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"

	INCLUDE		"macros.i"

*------ Imported Functions -------------------------------------------

	XREF_EXE	CloseLibrary
	XREF_EXE	OpenLibrary
	XREF		_AbsExecBase


	XREF		_PEDData


*------ Exported Globals ---------------------------------------------

	XDEF		_Init
	XDEF		_Expunge
	XDEF		_Open
	XDEF		_PD
	XDEF		_PED
	XDEF		_SysBase
	XDEF		_DOSBase
	XDEF		_GfxBase
	XDEF		_IntuitionBase


**********************************************************************
	SECTION		printer,DATA
_PD		DC.L	0
_PED		DC.L	0
_SysBase	DC.L	0
_DOSBase	DC.L	0
_GfxBase	DC.L	0
_IntuitionBase	DC.L	0


**********************************************************************
	SECTION		printer,CODE
_Init:
		MOVE.L	4(A7),_PD
		LEA	_PEDData(PC),A0
		MOVE.L	A0,_PED
		MOVE.L	A6,-(A7)
		MOVE.L	_AbsExecBase,A6
		MOVE.L	A6,_SysBase

*	    ;------ open the dos library
		LEA	DLName(PC),A1
		MOVEQ	#0,D0
		CALLEXE OpenLibrary
		MOVE.L	D0,_DOSBase
		BEQ	initDLErr


*	    ;------ open the graphics library
		LEA	GLName(PC),A1
		MOVEQ	#0,D0
		CALLEXE OpenLibrary
		MOVE.L	D0,_GfxBase
		BEQ	initGLErr

*	    ;------ open the intuition library
		LEA	ILName(PC),A1
		MOVEQ	#0,D0
		CALLEXE OpenLibrary
		MOVE.L	D0,_IntuitionBase
		BEQ	initILErr

		MOVEQ	#0,D0
pdiRts:
		MOVE.L	(A7)+,A6
		RTS

initPAErr:
		MOVE.L	_IntuitionBase,A1
		LINKEXE	CloseLibrary

initILErr:
		MOVE.L	_GfxBase,A1
		LINKEXE	CloseLibrary

initGLErr:
		MOVE.L	_DOSBase,A1
		LINKEXE	CloseLibrary

initDLErr:
		MOVEQ	#-1,D0
		BRA.S	pdiRts

ILName:
		DC.B	'intuition.library'
		DC.B	0
DLName:
		DC.B	'dos.library'
		DC.B	0
GLName:
		DC.B	'graphics.library'
		DC.B	0
		DS.W	0


*---------------------------------------------------------------------
_Expunge:
		MOVE.L	_IntuitionBase,A1
		LINKEXE	CloseLibrary

		MOVE.L	_GfxBase,A1
		LINKEXE	CloseLibrary

		MOVE.L	_DOSBase,A1
		LINKEXE	CloseLibrary


*---------------------------------------------------------------------
_Open:
		MOVEQ	#0,D0
		RTS

		END
