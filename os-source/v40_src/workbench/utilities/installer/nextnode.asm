*************************************************
*		Parsec Soft Systems xlib functions		*
*************************************************

;----------------------------------------------------------
;	NextNode(node) - get next node in list

		section	nextnode.asm,code

		include 'macros.i'
		include 'exec/nodes.i'

		DECLAREL	NextNode

		move.l	4(sp),a0				; get node
		move.l	LN_SUCC(a0),a1			; get next node
		move.l	a1,d0					; keep as result
		tst.l	LN_SUCC(a1)				; is next node's succ NULL?
		bne.s	1$						;	no, so node valid
		moveq	#0,d0					; was last node
1$		rts

		end

