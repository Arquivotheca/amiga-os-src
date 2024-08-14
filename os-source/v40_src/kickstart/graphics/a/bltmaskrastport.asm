*******************************************************************************
*
*	$Id: BltMaskRastPort.asm,v 39.0 91/08/21 17:24:15 chrisg Exp $
*
*******************************************************************************

	section	graphics
	xdef	_BltMaskRastPort
	xref	_bltmaskrastport
*********************
*
* c interface for bltmaskrastport
*
*********************
_BltMaskRastPort:
	move.l	a2,-(sp)	* bltmask
	movem.l	d6/d5/d4/d3/d2,-(sp)
*	move.l	d6,-(sp)	* minterm
*	move.l	d5,-(sp)	* sizeY
*	move.l	d4,-(sp)	* sizeX
*	move.l	d3,-(sp)	* destY
*	move.l	d2,-(sp)	* destX
	move.l	a1,-(sp)	* destrp
	move.l	d1,-(sp)	* srcY
	move.l	d0,-(sp)	* srcX
	move.l	a0,-(sp)	* srcbm
	jsr		_bltmaskrastport
	lea		10*4(sp),sp
	rts

	end
