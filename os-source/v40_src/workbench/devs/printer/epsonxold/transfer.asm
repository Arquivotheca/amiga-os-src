**********************************************************************
*
* Transfer routine for EpsonXOld
*
* David Berezowski - June/88
*
**********************************************************************

	INCLUDE "exec/types.i"

	INCLUDE	"intuition/intuition.i"
	INCLUDE "devices/printer.i"
	INCLUDE "devices/prtbase.i"
	INCLUDE "devices/prtgfx.i"

	XDEF	_Transfer

_Transfer:
; Transfer(PInfo, y, ptr)
; struct PrtInfo *PInfo		4-7
; UWORD y;			8-11
; UBYTE *ptr;			12-15

	movem.l	d2-d6/a2-a3,-(sp)	;save regs used

	movea.l	32(sp),a0		;a0 = PInfo
	move.l	36(sp),d0		;d0 = y
	movea.l	40(sp),a1		;a1 = ptr

	move.l	d0,d3			;save y
	moveq.l	#3,d2
	and.w	d0,d2			;d2 = y & 3
	lsl.w	#2,d2			;d2 = (y & 3) << 2
	movea.l	pi_dmatrix(a0),a3	;a3 = dmatrix
	adda.l	d2,a3			;a3 = dmatrix + ((y & 3) << 2)

	move.w	d0,d6
	not.b	d6			;d6 = bit to set

; a0 - PInfo
; a1 - ptr
; a3 - dmatrix ptr
; d0 - y
; d6 - bit to set
	
	move.w	pi_width(a0),d1		;d1 = width
	subq.w	#1,d1			;adjust for dbra

	move.w	pi_threshold(a0),d3	;d3 = threshold, thresholding?
	beq.s	grey_scale		;no, grey-scaling

threshold:
; a0 - PInfo
; a1 - ptr
; a3 - dmatrix ptr
; d1 - width-1
; d3 - threshold
; d6 - bit to set

	eori.b	#15,d3			;d3 = dvalue
	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)
	adda.w	d2,a1			;ptr += x

; a0 - sxptr
; a1 - ptr
; a2 - ColorInt ptr
; a3 - dmatrix ptr (NOT USED)
; d1 - width
; d3 - dvalue
; d4 - Black
; d5 - sx
; d6 - bit to set

twidth_loop:
	move.b	PCMBLACK(a2),d4		;d4 = Black
	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,d5		;d5 = # of times to use this pixel

	cmp.b	d3,d4			;render this pixel?
	ble.s	tsx_end			;no, skip to next pixel.
	subq.w	#1,d5			;adjust for dbra

tsx_render:				;yes, render this pixel sx times
	bset.b	d6,(a1)			;*(ptr) |= bit;

	adda.w	#1,a1			;ptr++
	dbra	d5,tsx_render		;sx--
	dbra	d1,twidth_loop		;width--
	bra.s	exit			;all done
	
tsx_end:
	adda.w	d5,a1			;ptr += sx
	dbra	d1,twidth_loop		;width--
	bra.s	exit

grey_scale:
; a0 - PInfo
; a1 - ptr
; a3 - dmatrix ptr
; d0 - y
; d1 - width-1
; d6 - bit to set

	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)

; a0 - sxptr
; a1 - ptr
; a2 - ColorInt ptr
; a3 - dmatrix ptr
; d1 - width
; d2 - x
; d3 - dvalue (dmatrix[x & 3])
; d4 - Black
; d5 - sx
; d6 - bit to set

gwidth_loop:
	move.b	PCMBLACK(a2),d4		;d4 = Black
	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,d5		;d5 = # of times to use this pixel
	subq.w	#1,d5			;adjust for dbra

gsx_loop:
	moveq.l	#3,d3
	and.w	d2,d3			;d3 = x & 3	
	move.b	0(a3,d3.w),d3		;d3 = dmatrix[x & 3]

	cmp.b	d3,d4			;render this pixel?
	ble.s	gsx_end			;no, skip to next pixel.

	bset.b	d6,0(a1,d2.w)		;*(ptr + x) |= bit

gsx_end
	addq.w	#1,d2			;x++
	dbra	d5,gsx_loop		;sx--
	dbra	d1,gwidth_loop		;width--

exit:
	movem.l	(sp)+,d2-d6/a2-a3	;restore regs used
	moveq.l	#0,d0			;flag all ok
	rts				;goodbye

	END
