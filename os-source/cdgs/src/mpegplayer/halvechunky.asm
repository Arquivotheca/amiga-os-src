
	OPTIMON

;---------------------------------------------------------------------------

	XDEF	_HalveChunky

;---------------------------------------------------------------------------

;void ASM HalveChunky(REG(a0) STRPTR src,
;                     REG(a1) STRPTR dst,
;                     REG(d0) LONG width,
;                     REG(d1) LONG height)

_HalveChunky:
	movem.l	d2-d7/a2,-(a7)	; save these...

	moveq.l	#0,d3		; clear the upper bytes
	moveq.l	#0,d2
	moveq.l	#0,d6
	moveq.l	#0,d7

	asr.l	#1,d1		; height / 2
	move.l	d0,d5
	asr.l	#1,d5		; width / 2

	move.l	a0,a2
	add.l	d0,a2

	bra.s	EndRowLoop	; enter row loop
RowLoop:
	move.l	d5,d4		; d4 has width / 2
	bra.s	EndColLoop	; enter column loop

ColLoop:
	move.b	(a0)+,d3	; get the first pixel of the first line
	move.b	(a0)+,d2	; get the second pixel of the first line
	move.b	(a2)+,d6	; get the first pixel of the second line
	move.b	(a2)+,d7	; get the second pixel of the second line
	add.w	d2,d3		; combine the four pixels...
	add.w	d6,d3
	add.w	d7,d3
	asr.w	#4,d3		; average and scale the color
	add.b	#64,d3		; offset it
	move.b	d3,(a1)+	; save the pixel

EndColLoop:
	dbra	d4,ColLoop	; next column

	add.l	d0,a0		; skip one row...
	add.l	d0,a2		; skip one row...

EndRowLoop:
	dbra	d1,RowLoop     	; next row

	movem.l	(a7)+,d2-d7/a2	; get back what we saved...
	rts

;---------------------------------------------------------------------------

	XDEF	_WCP
	XREF	_LVOWriteChunkyPixels

; patch to WriteChunkyPixel(), that compensates for a ROM bug with not
; preserving all the registers correctly
;
_WCP:	movem.l	d2-d7/a2-a6,-(a7)
	jsr	_LVOWriteChunkyPixels(a6)
	movem.l	(a7)+,d2-d7/a2-a6
	rts

;---------------------------------------------------------------------------

	END
