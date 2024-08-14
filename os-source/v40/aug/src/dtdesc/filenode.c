/* filenode.c
 *
 */

#include "dtdesc.h"

/*****************************************************************************/

struct Node *FindSelected (struct AppInfo *ai, struct Node *snode, WORD num)
{
    struct List *list = &ai->ai_Files;
    struct Node *node;
    WORD cnt;

    for (node = list->lh_Head, cnt = 0; node->ln_Succ; node = node->ln_Succ, cnt++)
    {
	if ((cnt == num) || (node == snode))
	{
	    ai->ai_CurFile = (struct FileNode *) node;
	    ai->ai_CurNum = cnt;
	    return (node);
	}
    }

    ai->ai_CurFile = NULL;
    ai->ai_CurNum = 0;

    return (NULL);
}

/*****************************************************************************/

struct Node *FindFileNodeLock (struct AppInfo *ai, BPTR lock)
{
    struct List *list = &ai->ai_Files;
    struct Node *node;

    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
    {
	if (SameLock (lock, ((struct FileNode *)node)->fn_Lock) == LOCK_SAME)
	{
	    return (node);
	}
    }

    return (NULL);
}

/*****************************************************************************/

VOID removefilenode (struct FileNode *fn)
{
    /* Remove the node from the list */
    Remove ((struct Node *)fn);

    if (fn->fn_FH)
    {
        Close (fn->fn_FH);
    }

    if (fn->fn_Lock)
    {
	UnLock (fn->fn_Lock);
    }

    /* Free the node itself */
    FreeVec (fn);
}

/*****************************************************************************/

VOID removefunc (struct AppInfo *ai)
{
    if (ai->ai_CurFile)
    {
	removefilenode (ai->ai_CurFile);
	ai->ai_NumFiles--;
    }

    if (!ai->ai_NumFiles)
    {
	ai->ai_Flags &= ~AIF_CHANGED;
    }
}

/*****************************************************************************/

VOID removeallfunc (struct AppInfo *ai)
{
    struct Node *node, *nxtnode;
    WORD i;

    if (ai->ai_Files.lh_TailPred != (struct Node *) &ai->ai_Files)
    {
        node = ai->ai_Files.lh_Head;
        while (nxtnode = node->ln_Succ)
        {
	    removefilenode ((struct FileNode *)node);
            node = nxtnode;
        }
    }

    for (i = 0; i < ai->ai_BufSize; i++)
    {
	ai->ai_SBuffer[i] = ai->ai_DBuffer[i] = (-1);
    }

    ai->ai_NumFiles = ai->ai_CurNum = 0;
    ai->ai_CurFile = NULL;
    ai->ai_Flags &= ~AIF_CHANGED;
}
