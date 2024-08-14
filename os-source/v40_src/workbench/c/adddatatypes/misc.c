/* misc.c
 *
 */

#include "adddatatypes.h"

/*****************************************************************************/

#define	DB(x)	x

/*****************************************************************************/

struct DataType *InitList (struct GlobalData * gd, STRPTR name, struct List *mlist, struct List * list, UWORD type, ULONG gid, ULONG id)
{
    struct DataTypeHeader *dth;
    struct DataType *dtn;
    ULONG msize;

    NewList (list);

    msize = DTSIZE + DTHSIZE + strlen (name) + 1;
    if (dtn = AllocVec (msize, MEMF_CLEAR))
    {
	dtn->dtn_Header        = MEMORY_FOLLOWING (dtn);
	dth                    = dtn->dtn_Header;
	dth->dth_Name          = MEMORY_FOLLOWING (dth);
	dth->dth_BaseName      = dth->dth_Name;
	dth->dth_GroupID       = gid;
	dth->dth_ID            = id;
	dtn->dtn_Node1.ln_Name = dth->dth_Name;
	dtn->dtn_Node2.ln_Name = dth->dth_Name;
	strcpy (dth->dth_Name, name);
	dth->dth_Flags         = type;

	NewList (&dtn->dtn_ToolList);
	dtn->dtn_Length = msize;

	AddTail (list, &dtn->dtn_Node1);
	EnqueueAlpha (gd, mlist, &dtn->dtn_Node2);
    }

    return (dtn);
}

/*****************************************************************************/

WORD BasicCmp (struct DataType * dtn1, struct DataType * dtn2)
{
    struct DataTypeHeader *dth1 = dtn1->dtn_Header;
    struct DataTypeHeader *dth2 = dtn2->dtn_Header;

    if ((dtn1->dtn_SysFlags & DTSF_MATCHALL) && !(dtn2->dtn_SysFlags & DTSF_MATCHALL))
	return (1);
    else if (!(dtn1->dtn_SysFlags & DTSF_MATCHALL) && (dtn2->dtn_SysFlags & DTSF_MATCHALL))
	return (-1);

    if (dth1->dth_Priority < dth2->dth_Priority)
	return (-1);
    else if (dth1->dth_Priority > dth2->dth_Priority)
	return (1);
    else
	return (0);
}

/*****************************************************************************
 * Returns -1 if mask1 is lower than mask2.
 * Returns  0 if they are equal.
 * Returns  1 if mask2 is lower than mask1.
 *
 */
WORD MaskCmp (struct DataType * dtn1, struct DataType * dtn2)
{
    struct DataTypeHeader *dth1 = dtn1->dtn_Header;
    struct DataTypeHeader *dth2 = dtn2->dtn_Header;
    WORD i, retval;

    /***********************/
    /* Both have functions */
    /***********************/
    if (dtn1->dtn_Function && dtn2->dtn_Function)
    {
	if ((dth1->dth_MaskLen < 1) && (dth2->dth_MaskLen < 1))
	    return (BasicCmp (dtn1, dtn2));

	if (dth1->dth_MaskLen < 1)
	    return (-1);

	if (dth2->dth_MaskLen < 1)
	    return (1);
    }

    /*****************************************/
    /* One has a function, the other doesn't */
    /*****************************************/
    if (dtn1->dtn_Function && (dtn2->dtn_Function == 0))
	return (-1);

    if (dtn2->dtn_Function && (dtn1->dtn_Function == 0))
	return (1);

    /***************************/
    /* No mask and no function */
    /***************************/
    if ((dth1->dth_MaskLen < 1) && (dth2->dth_MaskLen < 1) &&
	(dtn1->dtn_Function == 0) && (dtn2->dtn_Function == 0))
	return (BasicCmp (dtn1, dtn2));

    if ((dth1->dth_MaskLen < 1) && (dtn1->dtn_Function == 0))
	return (1);

    if ((dth2->dth_MaskLen < 1) && (dtn2->dtn_Function == 0))
	return (-1);

    /******************************************************************/
    /* They either both have a function or both don't have a function */
    /******************************************************************/
    if ((dth1->dth_MaskLen < 1) && (dth2->dth_MaskLen < 1))
	return (BasicCmp (dtn1, dtn2));

    if (dth1->dth_MaskLen < 1)
	return (1);

    if (dth2->dth_MaskLen < 1)
	return (-1);

    /*********************/
    /* Compare the masks */
    /*********************/
    for (i = retval = 0; ((i < dth1->dth_MaskLen) && (i < dth2->dth_MaskLen) && (retval == 0)); i++)
	retval = dth1->dth_Mask[i] - dth2->dth_Mask[i];

    if (retval == 0)
    {
	if (dth1->dth_MaskLen < dth2->dth_MaskLen)
	    return (1);
	else if (dth1->dth_MaskLen > dth2->dth_MaskLen)
	    return (-1);

	return (BasicCmp (dtn1, dtn2));
    }

    return (retval);
}

/*****************************************************************************/

VOID EnqueueAlphaMask (struct GlobalData * gd, struct List * list, struct DataType * dtn)
{
    struct Node *node;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
	if (MaskCmp ((struct DataType *) node, dtn) >= 0)
	    break;

	node = node->ln_Succ;
    }
    Insert (list, (struct Node *) dtn, node->ln_Pred);
}

/*****************************************************************************/

VOID EnqueueAlpha (struct GlobalData * gd, struct List * list, struct Node * new)
{
    struct Node *node;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
	if (Stricmp (node->ln_Name, new->ln_Name) >= 0)
	    break;

	node = node->ln_Succ;
    }
    Insert (list, new, node->ln_Pred);
}
