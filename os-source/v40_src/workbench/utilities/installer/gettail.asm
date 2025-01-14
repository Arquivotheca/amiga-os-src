*************************************************
*		Parsec Soft Systems xlib functions		*
*************************************************

;----------------------------------------------------------
;	GetTail(list) - get (but doesn't remove) tail from list

		section	code

		include 'macros.i'
		include 'exec/lists.i'

		DECLAREA	GetTail

		move.l	4(sp),a0				; get list
GetTail
		move.l	LH_TAILPRED(a0),d0		; get last node on list
		cmpa.l	d0,a0					; if not pointing to list,
		bne.s	1$						;	list not empty so exit
		moveq	#0,d0					; list empty, set to NULL
1$		rts

		end

void *GetTail(list) struct List *list;
{	struct Node *node;
	node = list->lh_TailPred;
	if (node != (struct Node *)list) return node;
	return NULL;
}

