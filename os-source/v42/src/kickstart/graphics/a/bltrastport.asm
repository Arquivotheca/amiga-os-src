*******************************************************************************
*
*	$Id: BltRastPort.asm,v 42.0 93/06/16 11:13:52 chrisg Exp $
*
*******************************************************************************

	section	graphics
	xdef	_BltRastPort
	xref	_external_bltrastport
*********************
*
* c interface for clipblit
*
*********************
_BltRastPort:
	movem.l	d6/d5/d4/d3/d2,-(sp)
*	move.l	d6,-(sp)	* minterm
*	move.l	d5,-(sp)	* sizeY
*	move.l	d4,-(sp)	* sizeX
*	move.l	d3,-(sp)	* destY
*	move.l	d2,-(sp)	* destX
	move.l	a1,-(sp)	* destrp
	move.l	d1,-(sp)	* srcY
	move.l	d0,-(sp)	* srcX
	move.l	a0,-(sp)	* srcrp
	jsr		_external_bltrastport
	lea		9*4(sp),sp
	rts

	end
