**********************************************************************
*
* NetBuff_LVOs.asm
*
* Copyright 1991 Raymond S. Brand, All rights reserved.
*
* 19910212
*
* $Id$
*
**********************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"


	SECTION	_LVO

	DATA

LDEF	MACRO
	LIBDEF	_LVO\1
	XDEF	_LVO\1
	ENDM


	LIBINIT

	LDEF	IntAllocSegments
	LDEF	AllocSegments
	LDEF	IntFreeSegments
	LDEF	FreeSegments
	LDEF	SplitNetBuff
	LDEF	TrimNetBuff
	LDEF	CopyToNetBuff
	LDEF	CopyFromNetBuff
	LDEF	CopyNetBuff
	LDEF	CompactNetBuff
	LDEF	ReadyNetBuff
	LDEF	IsContiguous
	LDEF	NetBuffAppend
	LDEF	PrependNetBuff
	LDEF	ReclaimSegments


	END
