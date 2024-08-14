#include <exec/nodes.h>
#include <exec/lists.h>
#include "functions.h"
#include "xfunctions.h"

void TransferList(struct List *ls,struct List *ld)
{	struct Node 	*head,*tail;

	NewList(ld);
	if ( !(head = GetHead(ls)) ) return;
	tail = GetTail(ls);
	if (tail == head) { AddHead(ld,head); return; }
	ld->lh_Head = head;
	ld->lh_TailPred = tail;
	head->ln_Pred = (struct Node *)&ld->lh_Head;
	tail->ln_Succ = (struct Node *)&ld->lh_Tail;
}
