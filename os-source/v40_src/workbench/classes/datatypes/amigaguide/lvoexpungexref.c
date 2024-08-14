/* ExpungeXRefLVO.c
 *
 */

#include "amigaguidebase.h"

VOID ASM LVOExpungeXRef (REG (a6) struct AGLib *ag)
{
    struct AmigaGuideToken *agt = ag->ag_Token;
    struct Node *node, *next;
    struct List *list;

    ObtainSemaphore (&agt->agt_Lock);

    /* Clear out the cross reference table */
    while (node = RemHead (&agt->agt_XRefList))
	FreeVec (node);
    NewList (&agt->agt_XRefList);

    /* Mark the entire table as unloaded */
    list = &agt->agt_XRefFileList;
    if (list && (list->lh_TailPred != (struct Node *) list))
    {
	/* start at the beginning */
	node = list->lh_Head;
	while (next = node->ln_Succ)
	{
	    /* Clear the loaded flag */
	    ((struct XRefFile *) node)->xrf_Flags &= ~XRF_LOADED;

	    /* get the next node */
	    node = next;
	}
    }

    ReleaseSemaphore (&agt->agt_Lock);
}
