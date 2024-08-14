**
**      $Id: pearl_e_vec.asm,v 1.1 93/02/01 12:20:50 darren Exp $
**
**      diskfont.library -- font extension support code
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/resident.i"

	INCLUDE "/diskfont/vecfont.i"

** Return if executed by hand; must find DiskFont Vector ID to init this properly

	moveq	#00,d0
	rts

	SECTION	DATA
fe_tag:
	dc.w	DFV_ID
	dc.l	fe_tag		; back pointer
	dc.w	DiskFontVector_SIZEOF	; size of structure
	dc.l	fe_fontname	; must match font name
	dc.l	fe_libname	; pointer to name of vector library
	dc.l	0		; version of vector library to open
	dc.l	fe_custom	; pointer to custom data
	dc.l	0		; codeset used by this font

fe_custom:
	dc.w	1		; flag WHITE SPACE ON

fe_opename:
	dc.b	'pearl.font',0

fe_fontname:
	dc.b	'pearl_e.font',0

fe_libname:
	dc.b	'eucdriver.library',0

