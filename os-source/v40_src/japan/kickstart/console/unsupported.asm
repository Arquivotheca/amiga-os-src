**
**	$Id: unsupported.asm,v 36.3 90/04/13 13:32:13 kodiak Exp $
**
**      console device unsupported functions
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"


**	Exports

	XDEF		Update

	XDEF		CBUpdate


**	Imports

	XREF		Invalid


**********************************************************************
*
*   INPUTS
*	iORequest -- the I/O Request for this command.
*
**********************************************************************
CBUpdate	EQU	-1

Update:
		bra	Invalid

	END
