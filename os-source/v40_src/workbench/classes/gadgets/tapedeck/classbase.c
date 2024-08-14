#include <exec/types.h>
#include <intuition/classes.h>

#include <clib/intuition_protos.h>

#include <pragmas/intuition_pragmas.h>

/*****************************************************************************/

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************/

VOID CallCHook(VOID);

/*****************************************************************************/

BOOL CreateClass (VOID)
{
    Class *cl;

    if (cl = MakeClass ("tapedeck.gadget", "gadgetclass", NULL, sizeof (struct objectData), 0))
    {
        cl->cl_Dispatcher.h_Entry    = (VOID *) CallCHook;
        cl->cl_Dispatcher.h_SubEntry = (VOID *) ClassDispatcher;
	cl->cl_Dispatcher.h_Data     = ClassBase;
	AddClass (cl);
    }

    ClassBase->cb_Class = cl;

    return (cl != NULL);
}


/*****************************************************************************/


BOOL DestroyClass (VOID)
{
    struct ClassLib *cb = ClassBase;
    BOOL result;

    if (result = FreeClass (cb->cb_Class))
	cb->cb_Class = NULL;

    return (result);
}
