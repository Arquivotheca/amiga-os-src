*******************************************************************************
*
*	$Id: ffill.i,v 39.0 91/08/21 17:17:59 chrisg Exp $
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

