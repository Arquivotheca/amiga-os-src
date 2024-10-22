	xdef	_arccopyin
	xdef	_arccopyout

;
; copy flat buffer to arcnet buffer
;
_arccopyout:			; arccopyout(src, dst, cnt)
	move.l	4(sp),a0	; source
	move.l	8(sp),a1	; destination
	move.l	12(sp),d0	; get cnt
; 
; Check that src pointer is aligned on word boundary so movem.l doesn't bomb
;
	move.l	a0,d1		; check src for pointer alignment
	lsr.l	#1,d1		; set carry to be lsb of pointer
	bcc.s	.6		; bad aligment - copy one byte manually
;
; copy out one byte to fix alignment for fast copy loop
;
	move.b	(a0)+,(a1)	; copy a byte to fix alignment
	addq.l	#2,a1		; advance dst pointer
	subq.l	#1,d0		; dec cnt by one
;
; Set up movem/movep copy routine.  d0 is cnt/16, regs d2-d5 are the
; copy buffer and must be saved.
;
.6	movem.l	d0/d2-d5,-(sp) 	; need some regs for copy buffer
	lsr.l	#4,d0		; uns divide cnt by 16
	moveq.l	#16,d1		; for compare op, must be power of two
	bra.s	.2

.1	add.l	d1,a1		; offset dst*2
	add.l	d1,a1		;
	movem.l	(a0)+,d2-d5 	; pick up data
	movep.l	d2,-32(a1)	; output 4 bytes to arcnet
	movep.l	d3,-24(a1)	; output 4 bytes to arcnet
	movep.l	d4,-16(a1)	; output 4 bytes to arcnet
	movep.l	d5,-8(a1)	; output 4 bytes to arcnet
.2	dbra	d0,.1		; iterate

	movem.l	(sp)+,d0/d2-d5 	; restore copy regs
	subq.l	#1,d1		; make mask
	and.l	d1,d0		; mask remaining copy bytes	
;
; per byte copy routine - used only for < 16 last bytes
;
	moveq.l	#2,d1		; setup increment
	bra.s	.4

.3	move.b	(a0)+,(a1)	; get byte
	add.l	d1,a1		; inc dest pointer by 2
.4	dbra	d0,.3		; loop for all
	rts

;
; copy arcnet buffer to flat buffer 
;
_arccopyin:			; arccopyin(src, dst, cnt)
	move.l	4(sp),a0	; source
	move.l	8(sp),a1	; destination
	move.l	12(sp),d0	; get cnt
; 
; Check that dst pointer is aligned on word boundary so movem.l doesn't bomb
;
	move.l	a1,d1		; check for pointer alignment
	lsr.l	#1,d1		; set carry to be lsb of pointer
	bcc.s	.16		; bad aligment - use per byte copier
;
; fix alignment for fast copier
;
	move.b	(a0),(a1)+	; copy in one byte
	addq.l	#2,a0		; advance src pointer
	subq.l	#1,d0		; dec cntr
;
; Set up movem/movep copy routine.  d0 is cnt/16, regs d2-d5 are the
; copy buffer and must be saved.
;
.16	movem.l	d0/d2-d5,-(sp) 	; need some regs for copy buffer
	lsr.l	#4,d0		; uns divide cnt by 16
	moveq.l	#16,d1		; for compare op, must be power of two
	bra.s	.12

.11	add.l	d1,a0		; offset dst*2
	add.l	d1,a0		;
	movep.l	-32(a0),d2	; get 4 bytes from arcnet
	movep.l	-24(a0),d3	; get 4 bytes from arcnet
	movep.l	-16(a0),d4	; get 4 bytes from arcnet
	movep.l	-8(a0),d5	; get 4 bytes from arcnet
	movem.l	d2-d5,(a1) 	; pick up data
	add.l	d1,a1		; move a1 forward
.12	dbra	d0,.11		; iterate

	movem.l	(sp)+,d0/d2-d5 	; restore copy regs
	subq.l	#1,d1		; make mask
	and.l	d1,d0		; mask remaining copy bytes	
;
; per byte copy routine - used only for last < 16 bytes
;
	moveq.l	#2,d1		; setup increment
	bra.s	.14

.13	move.b	(a0),(a1)+	; get byte
	add.l	d1,a0		; inc src pointer by 2
.14	dbra	d0,.13		; loop for all
	rts

