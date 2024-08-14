/* removedatatype.c
 *
 */

#include "adddatatypes.h"

/*****************************************************************************/

LONG ASM RemoveDataType (REG (a6) struct GlobalData *gd, REG (a0) struct DataType *dtn)
{
    if (dtn)
    {
	if (dtn->dtn_Node1.ln_Succ && dtn->dtn_Node1.ln_Pred)
	{
	    Remove (&dtn->dtn_Node1);
	    Remove (&dtn->dtn_Node2);
	    dtn->dtn_Node1.ln_Succ = dtn->dtn_Node1.ln_Pred = NULL;
	    dtn->dtn_Node2.ln_Succ = dtn->dtn_Node2.ln_Pred = NULL;
	}
	FreeVec (dtn);
    }
    return (0L);
}
