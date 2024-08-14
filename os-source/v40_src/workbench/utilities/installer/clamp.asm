;----------------------------------------------------------------
;	clamp(min,val,max) - force a longword to be between two other longword values

			section code

			xdef	_clamp
_clamp		movem.l	4(sp),d0/d1		; min/value

; find the higher of d0 and d1
			cmp.l	d0,d1			; d1 - d0  : if d1 > d0 then d0 = d1
			blt.s	1$				; 
			move.l	d1,d0
; find the lower of max (d1) and d0
1$			move.l	12(sp),d1		; max
			cmp.l	d0,d1			; d1 - d0
			bgt.s	2$
			move.l	d1,d0

2$			rts


			end
