**
**	$Id: pixel_al.asm,v 8.0 91/03/24 12:19:10 kodiak Exp $
**
**      pixel_align()
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
**
	SECTION	bullet

**	Exported Names

	XDEF	_pixel_align

**	Imported Names
 
	XREF	_pixel_aligned

**	Structures

; pixel_aligned structure
value		EQU	0
grid_line	EQU	2

; COORD_DATA structure
p_pixel		EQU	2
bin_places	EQU	6
round		EQU	16



; - - -	pixel_align - - - - - - - - - - - - - - - - - - - - - - - - -
;	pixel_align(WORD val, COORD_DATA *xy, WORD round_i)
_pixel_align:
		movem.l	d2-d3,-(a7)
		move.l	12(a7),d2		; val
		move.l	d2,d0			; value
    		;-- value = ABS(val);
		bpl.s	paValOK
		neg.l	d0
paValOK:
		;-- grid = (WORD)((((LONG)value <<xy->bin_places) +
		;	xy->round[round_i]) / xy->p_pixel);
		move.l	16(a7),a0		; xy
		move.w	bin_places(a0),d3	; xy->bin_places
		lsl.l	d3,d0			; value << xy->bin_places
		move.w	22(a7),d1		; round_i
		lsl.w	#2,d1			; (long index)
		add.l	round(a0,d1.w),d0	; + xy->round[round_i]
		move.w	p_pixel(a0),d1
		divs	d1,d0
		;-- if (val < 0)        /* 11-29-90 tbh */
		;--     grid = - grid;  /* 11-29-90 tbh */
		tst.l	d2
		bpl.s	paGridOK
		neg.w	d0
paGridOK:
		;-- pixel_aligned.grid_line = grid;
		move.w	d0,_pixel_aligned+grid_line(a4)
		;-- pixel_aligned.value = (WORD)
		;	((LONG)grid * (LONG)xy->p_pixel) >> xy->bin_places);
		muls	d1,d0			; * p_pixel
		asr.l	d3,d0
		move.w	d0,_pixel_aligned+value(a4)
		movem.l	(a7)+,d2-d3
		rts

	END
