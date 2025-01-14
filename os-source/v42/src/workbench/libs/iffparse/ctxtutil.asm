        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "iffparsebase.i"
        INCLUDE "iffprivate.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	_FindPropContextLVO
	XDEF	_CurrentChunkLVO
	XDEF	_ParentChunkLVO
	XDEF	_FindLocalItemLVO
	XDEF	_IDtoStrLVO

;---------------------------------------------------------------------------

_CurrentChunkLVO:
	movea.l	iffp_Stack+MLH_HEAD(a0),a0	; Ptr to head of stk.
	tst.l	cn_ID(a0)
	beq.s	1$
	move.l	a0,d0
	rts

1$:	moveq	#0,d0
	rts

;---------------------------------------------------------------------------

_ParentChunkLVO:
	movea.l	(a0),a0		; cn = NEXT (cn);
	tst.l	(a0)		; ASSERT (TEST (cn));
	beq.s	1$
	tst.l	cn_ID(a0)
	beq.s	1$
	move.l	a0,d0
	rts

1$:	moveq	#0,d0
	rts

;---------------------------------------------------------------------------

_IDtoStrLVO:
	move.l	d0,-(sp)	; push id on stack
	movea.l	sp,a1		; load stack top address
	move.l	a0,d0		; move address of buffer in d0 for later
	move.b	(a1)+,(a0)+	; copy the bytes
	move.b	(a1)+,(a0)+
	move.b	(a1)+,(a0)+
	move.b	(a1)+,(a0)+
	clr.b	(a0)		; put a null byte
	addq.w	#4,sp		; remove id from stack
	rts

;---------------------------------------------------------------------------

_FindLocalItemLVO:
	movem.l	d2-d4,-(sp)	; +12
	move.l	iffp_Stack+MLH_HEAD(a0),d3	; Ptr to head ctxt.

nextcn:
	movea.l	d3,a0		; This node
	move.l	(a0),d3		; Pointer to next node
	beq.s	endofctxt	; NULL for next node means EOL.

	move.l	cnp_LocalItems+MLH_HEAD(a0),d4	; Ptr to head lci.

nextlci:
	movea.l	d4,a1
	move.l	(a1),d4
	beq.s	nextcn

	cmp.l	lci_Ident(a1),d2
	bne.s	nextlci
	cmp.l	lci_ID(a1),d1
	bne.s	nextlci
	cmp.l	lci_Type(a1),d0
	bne.s	nextlci

	move.l	a1,d0
	movem.l	(sp)+,d2-d4
	rts

endofctxt:
	moveq	#0,d0
	movem.l	(sp)+,d2-d4
	rts

;---------------------------------------------------------------------------

_FindPropContextLVO:
	move.l	#ID_FORM,d1  			; cache this value
	move.l	#ID_LIST,a1			; cache this value

	movea.l	iffp_Stack+MLH_HEAD(a0),a0	; CurrentChunk()
	move.l	(a0),d0				; ParentChunk (CurrentChunk())

nextcontext:
	movea.l	d0,a0		; This node
	move.l	MLN_SUCC(a0),d0	; D0 = Next node
	beq.s	xit_fpc		; Next == NULL means end of list

	cmp.l	cn_ID(a0),d1	; FORM?
	beq.s	foundone
	cmp.l	cn_ID(a0),a1	; LIST?
	bne.s	nextcontext

foundone:
	move.l	a0,d0

xit_fpc:
	rts

;---------------------------------------------------------------------------

	END
