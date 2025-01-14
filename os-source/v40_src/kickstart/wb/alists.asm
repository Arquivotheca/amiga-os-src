*
* $Id: alists.asm,v 38.1 91/06/24 11:32:14 mks Exp $
*
* $Log:	alists.asm,v $
* Revision 38.1  91/06/24  11:32:14  mks
* Initial V38 tree checkin - Log file stripped
* 

	SECTION section

	NOLIST
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"workbench.i"
	INCLUDE	"workbenchbase.i"
	LIST

	XDEF	MasterSearch
	XDEF	SelectSearch
	XDEF	UtilitySearch
	XDEF	UtilityRevSearch
	XDEF	SiblingSuccSearch
	XDEF	SiblingPredSearch
	XDEF	NodeToObj
	XDEF	ActiveSearch
	XDEF	SearchList
	XDEF	PeekTail
	XDEF	SizeList
	XDEF	LastSelected

	XDEF	_MasterSearch
	XDEF	_SelectSearch
	XDEF	_UtilitySearch
	XDEF	_UtilityRevSearch
	XDEF	_SiblingSuccSearch
	XDEF	_SiblingPredSearch
	XDEF	_NodeToObj
	XDEF	_ActiveSearch
	XDEF	_SearchList
	XDEF	_PeekTail
	XDEF	_SizeList
	XDEF	_LastSelected

	TASK_ABLES

	XREF	_AbsExecBase

clear	MACRO
	moveq.l	#0,\1
	ENDM

FWDNODE	MACRO
	move.l	\1,\2
	move.l	(\2),\1
	beq.s	\3
	ENDM

REVNODE	MACRO
	move.l	\1,\2
	move.l	LN_PRED(\2),\1
	beq.s	\3
	ENDM

_MasterSearch:
	lea	4(sp),a1

MasterSearch:
	move.l	wb_MasterList+LH_HEAD(WBBASE),a0
	bra.s	SiblingSuccSearch

_SelectSearch:
	lea	4(sp),a1

SelectSearch:
	move.l	wb_SelectList+LH_HEAD(WBBASE),a0
	bra.s	SiblingSuccSearch


_UtilitySearch:
	lea	4(sp),a1

UtilitySearch:
	move.l	wb_UtilityList+LH_HEAD(WBBASE),a0
	bra.s	SiblingSuccSearch


_UtilityRevSearch:
	lea	4(sp),a1

UtilityRevSearch:
	move.l	wb_UtilityList+LH_TAILPRED(WBBASE),a0
	bra.s	SiblingPredSearch

_SiblingSuccSearch:
	move.l	4(sp),a0
	lea	8(sp),a1

SiblingSuccSearch:
	movem.l	a2/a3/a4/d2,-(sp)

	move.l	a0,d2		; node pointer
	move.l	a1,a2		; arguments

ss_loop:
	FWDNODE d2,a0,ss_fail

	; convert list node to object node
	bsr.s	NodeToObj
	move.l	d0,a3

	movem.l	(a2),d0/d1/a0/a1/a4	; push passed args
	movem.l	d1/a0/a1/a4,-(sp)
	move.l	a3,-(sp)		; push object node

	move.l	d0,a0		; call function
	jsr	(a0)
	lea	5*4(sp),sp

	tst.l	d0
	beq.s	ss_loop

	move.l	a3,d0
	bra.s	ss_end

ss_fail:
	clear	d0

ss_end:
	movem.l	(sp)+,a2/a3/a4/d2
	rts

_SiblingPredSearch:
	move.l	4(sp),a0
	lea	8(sp),a1

SiblingPredSearch:
	movem.l	a2/a3/a4/d2,-(sp)

	move.l	a0,d2		; node pointer
	move.l	a1,a2		; arguments

sp_loop:
	REVNODE d2,a0,sp_fail

	; convert list node to object node
	bsr.s	NodeToObj
	move.l	d0,a3

	movem.l	(a2),d0/d1/a0/a1/a4	; push passed args
	movem.l	d1/a0/a1/a4,-(sp)
	move.l	a3,-(sp)		; push object node

	move.l	d0,a0		; call function
	jsr	(a0)
	lea	5*4(sp),sp

	tst.l	d0
	beq.s	sp_loop

	move.l	a3,d0
	bra.s	sp_end

sp_fail:
	clear	d0

sp_end:
	movem.l	(sp)+,a2/a3/a4/d2
	rts


_NodeToObj:
	move.l	4(sp),a0

NodeToObj:
	move.l	a0,d0
	beq.s	nto_end

	clear	d0
	move.b	LN_TYPE(a0),d0
	ext.w	d0
	suba.w	d0,a0		; implicit extend to long
	move.l	a0,d0
nto_end:
	rts

_SizeList:
	move.l	4(sp),a0

SizeList:
	move.l	(a0),d1
	clear	d0

sizel_loop:
	FWDNODE	d1,a0,sizel_end
	addq.l	#1,d0
	bra.s	sizel_loop

sizel_end:
	rts


_ActiveSearch:
	lea	4(sp),a1

ActiveSearch:
	lea.l	wb_ActiveDisks(WBBASE),a0
	bra.s	SearchList

_SearchList:
	lea	8(sp),a1
	move.l	4(sp),a0

SearchList:	; ( list:a0, arglistpointer:a1 )
	;------ save registers
	movem.l	d2/a2/a3/a4,-(sp)

	;------ get list head
	move.l	(a0),d2

	;------ save argument pointer
	move.l	a1,a2

searchl_loop:
	FWDNODE	d2,a3,searchl_fail

	;------ push arguments (note that routine addr is in d0)
	movem.l	(a2),d0/d1/a0/a1/a4
	movem.l	d1/a0/a1/a4,-(sp)

	;------ push current node
	move.l	a3,-(sp)

	;------ call routine
	move.l	d0,a0
	jsr	(a0)
	lea	5*4(sp),sp

	;------ check routine return value.  If non-zero, break out of loop
	tst.l	d0
	beq.s	searchl_loop

	;------ return node address
	move.l	a3,d0
	bra.s	searchl_end

searchl_fail:
	;------ fell off end of list
	clear	d0

searchl_end:
	;------ restore saved registers
	movem.l	(sp)+,d2/a2/a3/a4
	rts


_PeekTail:
	move.l	4(sp),a0

PeekTail:
	move.l	LH_TAILPRED(a0),d0
	cmp.l	a0,d0
	bne.s	pt_end

	clear	d0
pt_end:
	rts

_LastSelected:
LastSelected:
	lea	wb_SelectList(WBBASE),a0
	bsr.s	PeekTail
	move.l	d0,a0
	bsr	NodeToObj
	rts

	END
