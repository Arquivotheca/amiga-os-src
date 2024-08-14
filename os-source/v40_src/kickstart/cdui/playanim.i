*******************************************************************************
*
* (c) Copyright 1993 Commodore-Amiga, Inc.  All rights reserved.
*
* This software is provided as-is and is subject to change; no warranties
* are made.  All use is at your own risk.  No liability or responsibility
* is assumed.
*
* playanim.i - header file for the anim code. 
*
*******************************************************************************

	include	'exec/types.i'

    STRUCTURE BitMapHeader,0
	UWORD	bmhd_w
	UWORD	bmhd_h
	WORD	bmhd_x
	WORD	bmhd_y
	UBYTE	bmhd_nplanes
	UBYTE	bmhd_Masking
	UBYTE	bmhd_Compression
	UBYTE	bmhd_pad1
	UWORD	bmhd_TransparentColor
	UBYTE	bmhd_XAspect
	UBYTE	bmhd_YAspect
	WORD	bmhd_PageWidth
	WORD	bmhd_PageHeight
    LABEL bmhd_SIZEOF

