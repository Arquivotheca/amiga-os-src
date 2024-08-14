**
**	$Filename$
**	$Release: 1.4 $
**	$Revision: 36.6 $
**	$Date: 90/04/13 11:37:37 $
**
**	boot module macros and stack variables
**
**	(C) Copyright 1985,1986,1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

**	Included Files

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/io.i"

	INCLUDE	"libraries/configvars.i"
	INCLUDE	"libraries/expansionbase.i"
	INCLUDE	"libraries/filehandler.i"

**********************************************************************
**********************************************************************

ABSEXECBASE	EQU	4

XLVO	MACRO
	XREF	_LVO\1
	ENDM

CALLLVO	MACRO
	CALLLIB	_LVO\1
	ENDM
