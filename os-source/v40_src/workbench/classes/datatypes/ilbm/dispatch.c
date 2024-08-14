/* dispatch.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

ULONG setdtattrs (struct ClassBase *cb, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG getdtattrs (struct ClassBase *cb, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

typedef unsigned long (*HOOKFUNC)();

Class *initClass (struct ClassBase *cb)
{
    Class *cl;

    if (cl = MakeClass (cb->cb_Lib.lib_Node.ln_Name, PICTUREDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = (HOOKFUNC)Dispatch;
	cl->cl_UserData           = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}

/*****************************************************************************/

ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DoSuperMethodA (cl, o, msg))
	    {
		BOOL success = FALSE;
		BPTR handle;

		if (GetAttr (DTA_Handle, (Object *) retval, &handle))
		{
		    if (GetILBM (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList))
			success = TRUE;
		}

		if (!success)
		{
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    return NULL;
		}
	    }
	    break;

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}
