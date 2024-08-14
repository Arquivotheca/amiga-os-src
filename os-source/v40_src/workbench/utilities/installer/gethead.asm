*************************************************
*		Parsec Soft Systems xlib functions		*
*************************************************

;----------------------------------------------------------
;	GetHead(list) - get (but doesn't remove) head from list

		section	gethead.asm,code

		include 'macros.i'
		include 'exec/lists.i'

		DECLAREL	GetHead

		move.l	4(sp),a0
GetHead							; assembly entry point -- a0 = list
		move.l	LH_HEAD(a0),a0
		tst.l	LN_SUCC(a0)
		beq.s	1$
		move.l	a0,d0
		rts

1$		moveq	#0,d0
		rts

		end

void *GetHead(list) struct List *list;
{	struct Node *node;
	node = list->lh_Head;
	if (node->ln_Succ) return node;
	return NULL;
}