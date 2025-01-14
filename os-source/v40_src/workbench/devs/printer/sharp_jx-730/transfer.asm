**********************************************************************
*
* Transfer routine for Sharp_JX-730
*
* David Berezowski - Dec/89
*
**********************************************************************

	INCLUDE "exec/types.i"

	INCLUDE	"intuition/intuition.i"
	INCLUDE "../printer/printer.i"
	INCLUDE "../printer/prtbase.i"
	INCLUDE "../printer/prtgfx.i"

	XREF	_PD
	XREF	_LVODebug
	XREF	_AbsExecBase

	XDEF	_Transfer

_Transfer:
; Transfer(PInfo, y, ptr, colors)
; struct PrtInfo *PInfo		4-7
; UWORD y;			8-11
; UBYTE *ptr;			12-15
; UWORD *colors;		16-19
;

	movem.l	d2-d6/a2-a3,-(sp)	;save regs used

	movea.l	32(sp),a0		;a0 = PInfo
	move.l	36(sp),d0		;d0 = y
	movea.l	40(sp),a1		;a1 = ptr
	movea.l	44(sp),a2		;a2 = colors

	moveq.l	#3,d2
	and.w	d0,d2			;d2 = y & 3
	adda.w	d2,a2
	adda.w	d2,a2			;a2 = colors[y3];
	movea.l	_PD,a3			;a3 = ptr to PD
	cmpi.w	#SHADE_COLOR,pd_Preferences+pf_PrintShade(a3)	;color dump?
	bne	not_color		;no

color:
; a0 - PInfo
; a1 - ptr
; a2 - colors[y3]
; d0 - y
; d2 - y & 3

	movem.l	d7/a4-a6,-(sp)		;save regs used

	movea.l	a1,a4			;a4 = ptr
	adda.w	24(a2),a4		;a4 = ptr + colors[12 + y3] (cptr)
	movea.l	a1,a5			;a5 = ptr
	adda.w	16(a2),a5		;a5 = ptr + colors[8 + y3] (mptr)
	movea.l	a1,a6			;a6 = ptr
	adda.w	8(a2),a6		;a6 = ptr + colors[4 + y3] (yptr)
	adda.w	(a2),a1			;a1 = ptr + colors[0 + y3] (bptr)

	movea.l	pi_ColorInt(a0),a2	;a2 = ColorInt ptr
	move.w	pi_width(a0),width	;# of pixels to do
	moveq.l	#3,d2
	and.w	d0,d2			;d2 = y & 3
	lsl.w	#2,d2			;d2 = (y & 3) << 2
	movea.l	pi_dmatrix(a0),a3	;a3 = dmatrix
	adda.l	d2,a3			;a3 = dmatrix + ((y & 3) << 2)
	move.w	pi_xpos(a0),d2		;d2 = x
	movea.l	pi_ScaleX(a0),a0	;a0 = ScaleX (sxptr)

; a0 - sxptr
; a1 - bptr
; a2 - ColorInt ptr
; a3 - dmatrix ptr
; a4 - cptr
; a5 - mptr
; a6 - yptr
; d0 - byte to set (x >> 3)
; d1 - Black
; d2 - x
; d3 - dvalue (dmatrix[x & 3])
; d4 - Cyan
; d5 - Magenta
; d6 - Yellow
; d7 - bit to clear

cwidth_loop:
	move.b	PCMBLACK(a2),d1		;d1 = Black
	move.b	PCMCYAN(a2),d4		;d4 = Cyan
	move.b	PCMMAGENTA(a2),d5	;d5 = Magenta
	move.b	PCMYELLOW(a2),d6	;d6 = Yellow
	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,sx		;# of times to use this pixel

csx_loop:
	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to clear
	move.b	d2,d7
	not.b	d7			;compute bit to clear

	moveq.l	#3,d3
	and.w	d2,d3			;d3 = x & 3	
	move.b	0(a3,d3.w),d3		;d3 = dmatrix[x & 3]

black:
	cmp.b	d3,d1			;render black?
	ble.s	cyan			;no, try cmy
	bset.b	d7,0(a1,d0.w)		;*(ptr + x >> 3) |= 2 ^ (7 - (x & 7))
	bra.s	csx_end

cyan:
	cmp.b	d3,d4			;render cyan pixel?
	ble.s	magenta			;no.
	bset.b	d7,0(a4,d0.w)		;*(ptr + x >> 3) |= 2 ^ (7 - (x & 7))

magenta:
	cmp.b	d3,d5			;render magenta pixel?
	ble.s	yellow			;no.
	bset.b	d7,0(a5,d0.w)		;*(ptr + x >> 3) |= 2 ^ (7 - (x & 7))

yellow:
	cmp.b	d3,d6			;render yellow pixel?
	ble.s	csx_end			;no, skip to next pixel.
	bset.b	d7,0(a6,d0.w)		;*(ptr + x >> 3) |= 2 ^ (7 - (x & 7))

csx_end
	addq.w	#1,d2			;x++
	subq.w	#1,sx			;sx--
	bne.s	csx_loop
	subq.w	#1,width		;width--
	bne.s	cwidth_loop

	movem.l	(sp)+,d7/a4-a6		;restore regs used
	bra	exit

not_color:
; a0 - PInfo
; a1 - ptr
; a2 - colors[y3]
; d0 - y
; d2 - y & 3

	adda.w	(a2),a1			;a1 = ptr + colors[y3]
	move.w	pi_width(a0),d1		;d1 = width
	subq.w	#1,d1			;adjust for dbra

	move.w	pi_threshold(a0),d3	;d3 = threshold, thresholding?
	beq.s	grey_scale		;no, grey-scaling

threshold:
; a0 - PInfo
; a1 - ptr (final)
; d1 - width-1
; d3 - threshold

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

twidth_loop:
	move.b	PCMBLACK(a2),d4		;d4 = Black
	addq.l	#ce_SIZEOF,a2		;advance to next entry

	move.w	(a0)+,d5		;d5 = # of times to use this pixel

	cmp.b	d3,d4			;render this pixel?
	ble.s	tsx_end			;no, skip to next pixel.
	subq.w	#1,d5			;adjust for dbra

tsx_render:				;yes, render this pixel sx times
	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to set
	move.b	d2,d6
	not.b	d6			;compute bit to set
	bset.b	d6,0(a1,d0.w)		;*(ptr + x >> 3) |= 2 ^ (7 - (x & 7))

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
; a1 - ptr (final)
; d0 - y
; d1 - width-1

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

	move.w	d2,d0
	lsr.w	#3,d0			;compute byte to set
	move.b	d2,d6
	not.b	d6			;compute bit to set
	bset.b	d6,0(a1,d0.w)		;*(ptr + x >> 3) |= 2 ^ (7 - (x & 7))

gsx_end
	addq.w	#1,d2			;x++
	dbra	d5,gsx_loop		;sx--
	dbra	d1,gwidth_loop		;width--

exit:
	movem.l	(sp)+,d2-d6/a2-a3	;restore regs used
	moveq.l	#0,d0			;flag all ok
	rts				;goodbye

sx	dc.w	0
width	dc.w	0

	END
