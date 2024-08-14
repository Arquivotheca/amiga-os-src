*******************************************************************************
*
*	$Id: ffill2asm.asm,v 39.0 91/08/21 17:18:13 chrisg Exp $
*
*******************************************************************************


	include 'exec/types.i'
	include 'graphics/gfx.i'

	ifne	bm_BytesPerRow
		fail
	endc

	xdef	_address
_address:
	move.l	4(sp),a0	* get bm pointer
	move.w	14(sp),d0	* get y
	muls	(a0),d0		* BytesPerRow * y
	move.l	bm_Planes(a0),a0
	add.l	d0,a0
	move.w	10(sp),d0	* get x
	asr.w	#3,d0
	and.w	#$FFFE,d0
	adda.w	d0,a0		* sign extended d0 first
	move.l	a0,d0
	rts

*	xdef	address
**	input = a0 = bm pointer
**	        d2 = x
**           d3 = y
**   output = a0
**   uses d0,d1
*address:
*	move.w	d3,d0		* d0 <- y
*	muls	(a0),d0		* y*BytesPerRow
*	move.l	bm_Planes(a0),a0
*	add.l	d0,a0
*	move.w	d2,d0		* d0 <- x
*	asr.w	#3,d0
*	and.w	#$FFFE,d0
*	adda.w	d0,a0		* d0 is sign extended first
*	rts

*bit_table:
*	dc.w $8000
*	dc.w $4000
*	dc.w $2000
*	dc.w $1000
*	dc.w $800
*	dc.w $400
*	dc.w $200
*	dc.w $100
*	dc.w $80
*	dc.w $40
*	dc.w $20
*	dc.w $10
*	dc.w $8
*	dc.w $4
*	dc.w $2
*	dc.w $1
*
*	xdef	_EXTREME
*_EXTREME:
*	movem.l	a2/a3/a4/d2/d3,-(sp)
*	move.l	20+4(sp),a2	* get fi
*	movem.l	20+8(sp),d2/d3		* get x,y
*	movem.l	20+16(sp),a2/a3		* get &right,&left
*	move.l	ffi_bm(a2),a0
*	bsr		address		* returns p in a0
*	move.w	d2,d0
*	and.w	#15,d0		* extract bit number
*	add.w	d0,d0
*	move.w	bit_table(d0.w),d1	* mask
*	
*	movem.l	(sp)+,a2/a3/a4/d2/d3
*	rts

	end
