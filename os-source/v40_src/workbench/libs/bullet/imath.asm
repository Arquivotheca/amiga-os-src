**
**	$Id: imath.asm,v 8.0 91/03/24 12:19:03 kodiak Exp $
**
**      intellifont math support
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
**
	SECTION	bullet

**	Exported Names

	XDEF	_tx
	XDEF	_ty
	XDEF	_nx
	XDEF	_ny
	XDEF	_multiply_i_i
	XDEF	_scale_iii
	XDEF	_scale_rem

**	Imported Names

	XREF	_d
	XREF	_x_tran
	XREF	_y_tran

**	Structures

; d
x_p_pixel	EQU	2
x_p_half_pix	EQU	4
y_p_pixel	EQU	68
y_p_half_pix	EQU	70

; x_tran, y_tran
num		EQU	0
den		EQU	2
old0		EQU	4
new0		EQU	6
half_den	EQU	8


; - - -	tx  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;-- tranform from the design space to the working power of two space.
_tx:
		;-- val.w.hi = v;
		move.w	6(a7),d0
		swap	d0
		;-- val.w.lo = 0
		clr.w	d0
		;-- val.l = val.l >> 4
		asr.l	#4,d0

		;-- return (WORD)((val.l + d.x.p_half_pix) / d.x.p_pixel);
		move.w	_d+x_p_half_pix(a4),d1
		ext.l	d1
		add.l	d1,d0
		divs	_d+x_p_pixel(a4),d0
		ext.l	d0
		rts

; - - -	ty  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;-- tranform from the design space to the working power of two space.
_ty:
		;-- val.w.hi = v;
		move.w	6(a7),d0
		swap	d0
		;-- val.w.lo = 0
		clr.w	d0
		;-- val.l = val.l >> 4
		asr.l	#4,d0

		;-- return (WORD)((val.l + d.x.p_half_pix) / d.x.p_pixel);
		move.w	_d+y_p_half_pix(a4),d1
		ext.l	d1
		add.l	d1,d0
		divs	_d+y_p_pixel(a4),d0
		ext.l	d0
		rts


; - - -	nx  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_nx:
		;-- return (WORD)((((LONG)(v-x_tran.old0)*
		;   (LONG)x_tran.num+x_tran.half_den)/x_tran.den)+x_tran.new0);
		move.w	6(a7),d0
		sub.w	_x_tran+old0(a4),d0
		muls	_x_tran+num(a4),d0
		move.w	_x_tran+half_den(a4),d1
		ext.l	d1
		add.l	d1,d0
		divs	_x_tran+den(a4),d0
		add.w	_x_tran+new0(a4),d0
		ext.l	d0
		rts


; - - -	ny  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_ny:
		;-- return (WORD)((((LONG)(v-y_tran.old0)*
		;   (LONG)y_tran.num+y_tran.half_den)/y_tran.den)+y_tran.new0);
		move.w	6(a7),d0
		sub.w	_y_tran+old0(a4),d0
		muls	_y_tran+num(a4),d0
		move.w	_y_tran+half_den(a4),d1
		ext.l	d1
		add.l	d1,d0
		divs	_y_tran+den(a4),d0
		add.w	_y_tran+new0(a4),d0
		ext.l	d0
		rts


; - - -	multiply_i_i  - - - - - - - - - - - - - - - - - - - - - - - -
_multiply_i_i:
		;-- return ((LONG)x * (LONG)y);
		move.w	6(a7),d0
		muls	10(a7),d0
		rts


; - - -	scale_iii - - - - - - - - - - - - - - - - - - - - - - - - - -
_scale_iii:
		;-- return((WORD)(((LONG)x*(LONG)y)/(LONG)z));
		move.w	6(a7),d0		; x
		muls	10(a7),d0		; *y
		divs	14(a7),d0		; /z
		rts


; - - -	scale_rem - - - - - - - - - - - - - - - - - - - - - - - - - -
;	WORD scale_rem(WORD a, WORD p, WORD q, WORD *ptr_rem, WORD *ptr_deltax1)
;	WORD x, rem, deltax1;
_scale_rem:
		;-- x = scale_iii(a, p, q);
		movem.l	d2-d3,-(a7)
		movem.l	12(a7),d0/d1/d2/a0/a1	; a p q ptr_rem ptr_deltax1
		muls	d1,d0			; x = a*p
		move.l	d0,d3			; a*p
		divs	d2,d0			; x = a*p/q

		;-- rem = (WORD)((LONG)a*(LONG)p-(LONG)x*(LONG)q);
		muls	d0,d2			; x*q
		sub.l	d2,d3			; rem = a*p-x*q
		;-- rem <<= 1;
		add.w	d3,d3

		;-- if (rem < 0) rem = - rem;
		bpl.s	srPosRem
		neg.w	d3
srPosRem:
		;-- *ptr_rem = rem;
		move.w	d3,(a0)

		;-- if (x > 0)
		move.w	d0,d2
		ext.l	d0
		ble.s	srLSX
		;--	*ptr_deltax1 = x + 1;
srPX:
		addq.w	#1,d2
		move.w	d2,(a1)
		bra.s	srDone

srLSX:
		;-- else if (x < 0)
		beq.s	srSX
		;--	*ptr_deltax1 = x - 1;
srLX:
		subq.w	#1,d2
		move.w	d2,(a1)
srDone:
		movem.l	(a7)+,d2/d3
		rts

srSX:
		;-- else /* x == 0 */
		;--	if (p > 0) *ptr_deltax1 = x + 1;
		tst.w	d1
		bgt.s	srPX
		;--	else *ptr_deltax1 = x -1;
		bra.s	srLX

	END
