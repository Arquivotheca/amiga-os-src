**********************************************************************
*
* Transfer routine for Seiko_5300
*
* David Berezowski - Jun/88
*
**********************************************************************

	INCLUDE "exec/types.i"

	INCLUDE	"intuition/intuition.i"
	INCLUDE "../printer/printer.i"
	INCLUDE "../printer/prtbase.i"
	INCLUDE "../printer/prtgfx.i"

	XREF	_PD

	XDEF	_Transfer

_Transfer:
; Transfer(PInfo, y, ptr, pcm)
; struct PrtInfo *PInfo		4-7
; UWORD y;			8-11
; UBYTE *ptr;			12-15
; UWORD pcm;			16-19
;

	movem.l	d2-d7/a2-a3,-(sp)	;save regs used

	movea.l	36(sp),a0		;a0 = PInfo
	move.l	40(sp),d0		;d0 = y
	movea.l	44(sp),a1		;a1 = ptr
	move.l	48(sp),d7		;d7 = pcm

	move.w	pi_width(a0),d1		;d1 = width
	subq.w	#1,d1			;adjust for dbra

	move.w	pi_threshold(a0),d3	;d3 = threshold, thresholding?
	beq.s	grey_scale		;no, grey-scale or color

threshold:
; a0 - PInfo
; a1 - ptr
; d0 - y
; d1 - width
; d3 - threshold
; d7 - pcm (NOT USED)

	eori.b	#15,d3			;d3 = dvalue
	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)

; a0 - sxptr
; a1 - ptr
; a2 - ColorInt ptr
; a3 - dmatrix ptr (NOT USED)
; d0 - byte to set (x >> 3)
; d1 - width
; d2 - x
; d3 - dvalue
; d4 - Black
; d5 - sx
; d6 - bit to set
; d7 - pcm (NOT USED)

twidth_loop:
	move.b	PCMBLACK(a2),d4		;d4 = Black
	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,d5		;d5 = # of times to use this pixel (sx)

	cmp.b	d3,d4			;render this pixel?
	ble.s	tsx_end			;no, skip to next pixel.
	subq.w	#1,d5			;adjust for dbra

tsx_render:				;yes, render this pixel sx times
	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to set
	move.w	d2,d6
	bset.b	d6,0(a1,d0.w)		;*(ptr + x >> 3) |= 2 ^ x

	addq.w	#1,d2			;x++
	dbra	d5,tsx_render		;sx--
	dbra	d1,twidth_loop		;width--
	bra.s	exit			;all done
	
tsx_end:
	add.w	d5,d2			;x += sx
	dbra	d1,twidth_loop		;width--
	bra.s	exit

grey_scale:
; a0 - PInfo
; a1 - ptr
; d0 - y
; d1 - width
; d7 - pcm


	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	moveq.l	#3,d2
	and.w	d0,d2			;d2 = y & 3
	lsl.w	#2,d2			;d2 = (y & 3) << 2
	movea.l	pi_dmatrix(a0),a3	;a3 = dmatrix
	adda.l	d2,a3			;a3 = dmatrix + ((y & 3) << 2)
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)

; a0 - sxptr
; a1 - ptr
; a2 - ColorInt ptr
; a3 - dmatrix ptr
; d0 - byte to set (x >> 3)
; d1 - width
; d2 - x
; d3 - dvalue (dmatrix[x & 3])
; d4 - Color
; d5 - sx
; d6 - bit to set
; d7 - pcm

gwidth_loop:
	move.b	0(a2,d7.w),d4		;d4 = color
	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,d5		;d5 = # of times to use this pixel (sx)
	subq.w	#1,d5			;adjust for dbra

gsx_loop:
	moveq.l	#3,d3
	and.w	d2,d3			;d3 = x & 3	
	move.b	0(a3,d3.w),d3		;d3 = dmatrix[x & 3]

	cmp.b	d3,d4			;render this pixel?
	ble.s	gsx_end			;no, skip to next pixel.

	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to set
	move.w	d2,d6
	bset.b	d6,0(a1,d0.w)		;*(ptr + x >> 3) |= 2 ^ x

gsx_end
	addq.w	#1,d2			;x++
	dbra	d5,gsx_loop		;sx--
	dbra	d1,gwidth_loop		;width--

exit:
	movem.l	(sp)+,d2-d7/a2-a3	;restore regs used
	moveq.l	#0,d0			;flag all ok
	rts				;goodbye

	END
