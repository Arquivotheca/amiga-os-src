        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"graphics/gfx.i"
	INCLUDE	"hardware/custom.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	_ColumnCopyBM

;---------------------------------------------------------------------------

	OPT	p=68020

;
;
; colcopy.asm - copy a list of columns from a source bitmap to a destination
; bitmap.

; void __asm ColumnCopyBM(register __a0 struct BitMap *srcbm,
;			  register __a1 struct BitMap *dstbm,
;			  register __a2 WORD *coltable,
;			  register __d0 WORD DstStartCol,
;			  register __d1 WORD SrcHeight);
;
;
; source and destination must both be interleaved, and must be
; the same depth.
;
; format of coltable:
;	srcx, dsty
;	srcx, dsty... -1

_ColumnCopyBM:
	movem.l	a2-a6/d2-d7,-(a7)	; save those regs for the stupid
					; c compiler.
	moveq	#0,d2
	move.b	bm_Depth(a0),d2
	mulu	d2,d1			; *depth for interleaved
	subq	#1,d1			; adjust for dbra

	movem.l	bm_Planes(a0),d2/d3	; src modulo
	sub.l	d2,d3			; d3=src modulo
	move.l	d2,a3			; a3=src adr

	movem.l	bm_Planes(a1),d2/d4
	sub.l	d2,d4			; d4=destination modulo
	move.l	d2,a4			; a4=dest adr
	move.w	bm_BytesPerRow(a1),d2	; d2=destbpr
	move	d3,a6

; let's go!

nextcol:
	move.w	(a2)+,d3		; d3=srcx
	bmi.s	done_colcopy
	move.w	(a2)+,d7		; d4=desty
	mulu	d2,d7			; convert to adr
	lea	0(a4,d7.l),a5		; output ptr
	move.w	d1,d5			; dbra counter
	move.l	a3,a0
pixcpylp:
	bfextu	(a0){d3:1},d6		; get source pixel
	bfins	d6,(a5){d0:1}		; and store
	add.w	d4,a5			; destptr+=destmod
	add.l	a6,a0			; srcptr+=srcmod
	dbra	d5,pixcpylp
	addq	#1,d0			; update output ptr
	bra.s	nextcol
done_colcopy:
	movem.l	(a7)+,a2-a6/d2-d7
	rts

;---------------------------------------------------------------------------

	END
