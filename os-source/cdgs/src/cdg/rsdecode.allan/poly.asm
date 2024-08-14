; poly.asm
;  $ID:
;
; Created:	1/28/92		By Allan Havemose
;
;
; NOTES:
; Arithmetic for syndrome polynomials over GF64.
; deg S <= 3, so Syndrome fits in an ULONG.
;
;
;
;

	CSECT	text

	XDEF	_SynPoly_Clear
	XDEF	_SynPoly_Clear2
	XDEF	_SynPoly_Copy
	XDEF	_SynPoly_Add
	XDEF	_SynPoly_MulX


; ===============================================================================
;
; --------------------------------------------------------------------------------
; void	__asm SynPoly_Clear(register __a0 SynPoly *a)
_SynPoly_Clear:
	move.l	#0,(a0)
	rts

; --------------------------------------------------------------------------------
; void	__asm SynPoly_Clear2(register __a0 SynPoly *a)
; Clears a polynomial of degree 2*LOCLEN;
_SynPoly_Clear2:
	movea.l	a0,a1		; do not fool with a0
	move.l	#0,(a1)+
	move.l	#0,(a1)
	rts

; --------------------------------------------------------------------------------
; void	__asm SynPoly_Copy(register __a0 *SP1, register __a1 *SP2)
; Operation:	SP1 = SP2;
;
_SynPoly_Copy:
	move.l	(a1),(a0)
	rts
; --------------------------------------------------------------------------------
; void	__asm SynPoly_Add(register __a0 *SP1, register __a1 *SP2)
; Operation: SP1 += SP2;
;
_SynPoly_Add:
	move.l	(a1),d0
	eor.l	d0,(a0)
	rts

; --------------------------------------------------------------------------------
; void	__asm SynPoly_MulX(register __a0 *SP1)
; Operation: SP1 = AP1<<1 with sp1[0] = 0
; Corresponds to multiply by 'x'
;
_SynPoly_MulX:
	move.l	(a0),d0
	lsr.l	#8,d0		; shifts 0 in from right automatically
	move.l	d0,(a0)
	rts


	END