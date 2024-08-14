**
**	$Filename$
**	$Release: 1.4 $
**	$Revision: 1.1 $
**	$Date: 92/08/20 13:46:05 $
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
