	TTL    '$Header: /usr.MC68010/ghostwheel/darren/printer/generic/RCS/init.asm,v 1.1 90/04/06 20:05:46 daveb Exp $'
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
*   $Header: /usr.MC68010/ghostwheel/darren/printer/generic/RCS/init.asm,v 1.1 90/04/06 20:05:46 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	init.asm,v $
*   Revision 1.1  90/04/06  20:05:46  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/20  14:11:30  daveb
*   added to rcs
*   
*   Revision 1.1  85/10/09  19:27:27  kodiak
*   remove _stdout variable
*   
*   Revision 1.0  85/10/09  19:23:33  kodiak
*   added to rcs for updating in version 1
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

	INCLUDE		"../printer/macros.i"

*------ Imported Functions -------------------------------------------

	XREF_EXE	CloseLibrary
	XREF_EXE	OpenLibrary
	XREF		_AbsExecBase


	XREF		_PEDData


*------ Exported Globals ---------------------------------------------

	XDEF		_Init
	XDEF		_Expunge
	XDEF		_Open
	XDEF		_Close


**********************************************************************
	SECTION		printer,CODE
_Init:
_Expunge:
_Open:
_Close:
		MOVEQ	#0,D0
		RTS

		END
