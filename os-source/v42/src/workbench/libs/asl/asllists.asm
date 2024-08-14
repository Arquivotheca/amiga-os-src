        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/memory.i"
        INCLUDE "aslbase.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	_NewList
	XDEF	_EnqueueAlpha
	XDEF	_FindNameNC
	XDEF	_FindNum
	XDEF	_FindNodeNum
	XDEF	_FindClosest
	XDEF	_FreeList
	XDEF	_AllocNode
	XDEF	_AllocNamedNode

	XDEF	_AllocNamedNode2

;---------------------------------------------------------------------------

	XREF	_LVOAllocVec
	XREF	_LVOFreeVec
	XREF	_LVOStricmp

;-----------------------------------------------------------------------

_FreeList:
; A0 - list pointer
	movem.l	d2/a0/a6,-(sp)		; preserve these
	move.l	ASL_SysBase(a6),a6	; load this for FreeVec
	move.l	a0,a1			; a1 - list pointer
	move.l	LN_SUCC(a1),a1		; address of first node

1$:     move.l	LN_SUCC(a1),d2		; load pointer to next node
        beq.s	2$			; if next node is NULL, we're all done

	CALL	FreeVec			; free node

	move.l	d2,a1			; reload pointer to next node
	bra.s	1$

2$:	movem.l	(sp)+,d2/a0/a6		; restore these

;!!! FALLS THROUGH TO NEWLIST!

;---------------------------------------------------------------------------

;!!! FREELIST FALLS THROUGH HERE!

_NewList:
; A0 - list pointer
	clr.l	LH_TAIL(a0)
	move.l	a0,LH_TAILPRED(a0)
	addq.l	#LH_TAIL,a0
	move.l	a0,-(a0)
	rts

;---------------------------------------------------------------------------

_AllocNamedNode:
; D0 - node size
; A0 - node name, or NULL
; RETURNS:
; D0 - node, or NULL
	move.l	a0,d1				; set the CC
	beq.s	_AllocNode			; no name string

	movem.l	d2/a2/a6,-(sp)			; save these

	move.l	a0,a1				; a1 - address of name
	move.l	a0,a2				; a2 - address of name
	move.l	d0,d2				; d2 - original node size

1$:	tst.b	(a1)+
	bne.s	1$   				; looking for a NUL
	sub.l	a0,a1				; a1 - length of string plus one
	add.l	a1,d0				; nodeSize = nodeSize+nameSize

2$:     move.l	ASL_SysBase(a6),a6		; load SysBase
	move.l	#MEMF_ANY!MEMF_CLEAR,d1		; load allocation attributes
	CALL	AllocVec			; allocate the memory
	tst.l	d0				; did we get the memory?
	beq.s	4$				; no memory? so exit

	move.l	d0,a1				; address of node
	add.l	d0,d2				; address where name goes
	move.l	d2,LN_NAME(a1)			; store name pointer in node
	move.l	d2,a1				; name pointer

3$:	move.b	(a2)+,(a1)+			; copy everything until a NUL
	bne.s	3$

4$:	movem.l	(sp)+,d2/a2/a6
	rts					; bye

;---------------------------------------------------------------------------

_AllocNode:
; D0 - node size
; RETURNS:
; D0 - node, or NULL
	move.l	a6,-(sp)			; save AslBase
	move.l	ASL_SysBase(a6),a6		; load SysBase
	move.l	#MEMF_ANY!MEMF_CLEAR,d1		; load allocation attributes
	CALL	AllocVec			; allocate the memory
	move.l	(sp)+,a6			; restore AslBase
	rts					; bye

;---------------------------------------------------------------------------

_AllocNamedNode2:
; D0 - node size
; D1 - leading byte for name
; A0 - node name, or NULL
; RETURNS:
; D0 - node, or NULL
	sub.l	a1,a1				; clear this guy
	cmp.l	a1,a0				; set the CC
	beq.s	_AllocNode			; no name string

	movem.l	d2/d3/a2/a6,-(sp)		; save these

	move.l	a0,a1				; a1 - address of name
	move.l	a0,a2				; a2 - address of name
	move.l	d0,d2				; d2 - original node size
	move.b	d1,d3				; d3 - leading byte of name

1$:	tst.b	(a1)+
	bne.s	1$   				; looking for a NUL
	sub.l	a0,a1				; a1 - length of string plus one
	add.l	a1,d0				; nodeSize = nodeSize+nameSize
	addq.l	#1,d0				; count leading byte

2$:     move.l	ASL_SysBase(a6),a6		; load SysBase
	move.l	#MEMF_ANY!MEMF_CLEAR,d1		; load allocation attributes
	CALL	AllocVec			; allocate the memory
	tst.l	d0				; did we get the memory?
	beq.s	4$				; no memory? so exit

	move.l	d0,a1				; address of node
	add.l	d0,d2				; address where name goes
	move.l	d2,LN_NAME(a1)			; store name pointer in node
	add.l	#1,LN_NAME(a1)
	move.l	d2,a1				; name pointer

	move.b  d3,(a1)+			; move leading byte into place
3$:	move.b	(a2)+,(a1)+			; copy everything until a NUL
	bne.s	3$

4$:	movem.l	(sp)+,d2/d3/a2/a6
	rts					; bye

;---------------------------------------------------------------------------

_EnqueueAlpha:
; A0 - list pointer
; A1 - pointer to node to insert

	movem.l	a2/a3/a4/a6,-(sp)      	; save these guys
	move.l	ASL_UtilityBase(a6),a6	; load UtilityBase in A6
	move.l	a0,a2			; a2 - address of list
	move.l	a1,a3			; a3 - address of new node
	move.l	LN_NAME(a3),a4		; a4 - address of name of new node

1$:	move.l	LN_SUCC(a2),a2		; load address of first/next node
	tst.l	LN_SUCC(a2)		; test if at end of list
	beq.s	2$			; if at end of list, then exit

	move.l	LN_NAME(a2),a0		; name of current node
	move.l	a4,a1			; name of new node
	CALL	Stricmp			; compare them
	tst.l	d0			; test the result
	ble.s	1$			; not here, try try again...

2$:	; when we get here, a2 has node before which to insert, and a3 has
	; the node to insert

	move.l	LN_PRED(a2),LN_PRED(a3)
	move.l	a2,LN_SUCC(a3)
	move.l	a3,LN_PRED(a2)
	move.l	LN_PRED(a3),a2
	move.l	a3,LN_SUCC(a2)

	movem.l	(sp)+,a2/a3/a4/a6	; restore these
	rts

;---------------------------------------------------------------------------

_FindNum:
; A0 - list pointer
; D0 - number of node to find (UWORD)
; RETURNS:
; D0 - pointer to node

1$:	movea.l	LN_SUCC(a0),a0		; load first/next node
	dbra	d0,1$			; loop back

	move.l	a0,d0
	rts

;-----------------------------------------------------------------------

_FindNodeNum:
; A0 - list pointer
; A1 - node pointer
; RETURNS:
; D0 - number of the node in the list

	moveq.l	#-1,d0		; init counter

1$:	addq.l	#1,d0		; bump counter
	move.l	LN_SUCC(a0),a0	; load first/next node
	cmp.l	a0,a1		; is this the node?
	bne.s	1$		; nope, so look at the next one

	rts

;-----------------------------------------------------------------------

_FindNameNC
; A0 - list pointer
; A1 - name pointer
; RETURNS:
; D0 - node pointer, or NULL
	movem.l	d2/a2/a3/a6,-(sp)	; save these guys
	move.l	ASL_UtilityBase(a6),a6	; load UtilityBase in A6
	move.l	a1,a3			; a3 - name of node
	move.l	LN_SUCC(a0),d2		; d2 - address of first node

1$:	move.l	d2,a2			; load address of next node
	move.l	LN_SUCC(a2),d2		; load pointer to next node
	beq.s	3$			; if at end of list, then exit

	move.l	a3,a0			; name being searched for
	move.l	LN_NAME(a2),a1		; name of current node
	CALL	Stricmp			; compare them
	tst.l	d0			; test the result
	bne.s	1$			; if not equal, then we didn't find it

	move.l	a2,d0			; found it, return the pointer
	movem.l	(sp)+,d2/a2/a3/a6	; restore these
	rts

3$:	moveq.l	#0,d0			; didn't find it
	movem.l	(sp)+,d2/a2/a3/a6	; restore these
	rts

;-----------------------------------------------------------------------

_FindClosest:
; A0 - list pointer
; A1 - name pointer
; RETURNS:
; D0 - node pointer

	movem.l	d2/a0/a2/a3/a6,-(sp)   	; save these guys
	move.l	a0,a2
	move.l	a1,a3
	bsr.s	_FindNameNC
	tst.l	d0
	beq.s	0$
	movem.l	(sp)+,d2/a0/a2/a3/a6
	rts

0$:	move.l	a2,a0
	move.l	a3,a1

	move.l	ASL_UtilityBase(a6),a6	; load UtilityBase in A6
	move.l	a1,a3			; a3 - name of node
	move.l	LN_SUCC(a0),d2		; d2 - address of first node

1$:	move.l	d2,a2			; load node pointer
	move.l	LN_SUCC(a2),d2		; load pointer to next node
	beq.s	3$			; if at end of list, then exit

	move.l	a3,a0			; name being searched for
	move.l	LN_NAME(a2),a1		; name of current node
	CALL	Stricmp			; compare them
	tst.l	d0			; test the result
	bgt.s	1$			; if (name < current) then loop

	move.l	a2,d0			; found it, return the pointer
	movem.l	(sp)+,d2/a0/a2/a3/a6	; restore these
	rts

3$:     moveq.l	#0,d0			; didn't find it
	movem.l	(sp)+,d2/a0/a2/a3/a6	; restore these
	move.l  LH_TAILPRED(a0),a0      ; test if list is empty
	beq.s	4$			; if it is empty, just return NULL
	move.l	LH_TAILPRED(a0),d0	; if not empty, return last entry in list
4$:
	rts

;-----------------------------------------------------------------------

        END
