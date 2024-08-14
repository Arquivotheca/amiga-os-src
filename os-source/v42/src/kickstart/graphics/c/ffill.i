*******************************************************************************
*
*	$Id: ffill.i,v 42.0 93/06/16 11:17:33 chrisg Exp $
*
*******************************************************************************

	structure	flood_info,0
	APTR	ffi_freelist
	APTR	ffi_top
	APTR	ffi_blocklist
	WORD	ffi_MinX
	WORD	ffi_MaxX
	WORD	ffi_MinY
	WORD	ffi_MaxY
	APTR	ffi_rp
	APTR	ffi_bm
	label	ffi_SIZEOF

