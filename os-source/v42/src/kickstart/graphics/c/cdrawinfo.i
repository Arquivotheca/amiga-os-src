*******************************************************************************
*
*	$Id: cdrawinfo.i,v 42.0 93/06/16 11:17:31 chrisg Exp $
*
*******************************************************************************

	STRUCTURE	cdrawinfo,0
		WORD	cd_dx
		WORD	cd_dy
		WORD	cd_absdx
		WORD	cd_absdy
		WORD	cd_con1
		BYTE	cd_code1
		BYTE	cd_code2
		BYTE	cd_xmajor
		BYTE	cd_pad
	LABEL	cd_sizeof