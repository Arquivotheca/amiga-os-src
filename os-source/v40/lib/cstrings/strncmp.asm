


	section	strings.lib

;
; returns:
;	-1 if a < b
;	 0 if a = b
;	 1 if a > b
; all comparisons use unsigned arithmetic
;
	xdef	_strncmp
	xdef	strncmp

_strncmp:
	movem.l	4(sp),a0/a1	; string_a, string_b
	move.l	12(sp),d0	; count

strncmp:

	subq.l	#1,d0
	blt.s	strncmp_equal

	move.b	(a1)+,d1
	cmp.b	(a0)+,d1
	bne.s	strncmp_notequal

	tst.b	d1
	bne.s	strncmp

strncmp_equal:
	moveq.l	#0,d0		; set up the return value

strncmp_end:
	rts

strncmp_notequal:
	moveq.l	#1,d0
	bgt.s	strncmp_end

	moveq.l	#-1,d0
	rts

	end
