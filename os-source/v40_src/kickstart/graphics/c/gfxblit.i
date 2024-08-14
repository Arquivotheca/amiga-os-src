*******************************************************************************
*
*	$Id: gfxblit.i,v 39.0 91/08/21 17:19:00 chrisg Exp $
*
*******************************************************************************

*
	STRUCTURE	GFXBLTND,0
	WORD	gn_bltsize
	WORD	gn_fillbytes
	WORD	gn_fwmask
	WORD	gn_lwmask
	WORD	gn_bltcon1
	WORD	gn_bltmdb
	WORD	gn_bltmda
	WORD	gn_bltmdc
	APTR	gn_bltpta
	APTR	gn_bltptb
	WORD	gn_jstop
	BYTE	gn_filln
	BYTE	gn_j
	WORD	gn_bltcon0
	WORD	gn_lminy
	WORD	gn_lmaxy
	WORD	gn_bpw
	LONG	gn_bltptr
	WORD	gn_fudge
	WORD	gn_rx
	WORD	gn_blitrows
	WORD	gn_blitwords
	LABEL	gn_SIZEOF
