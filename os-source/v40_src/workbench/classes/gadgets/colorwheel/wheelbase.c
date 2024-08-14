
#include <exec/types.h>
#include <intuition/classes.h>

#include <clib/intuition_protos.h>

#include <pragmas/intuition_pragmas.h>

#include "wheelbase.h"
#include "wheel.h"


/*****************************************************************************/


VOID CallCHook(VOID);


BOOL CreateClass(VOID)
{
Class *cl;

    if (cl = MakeClass("colorwheel.gadget","gadgetclass",NULL,sizeof(struct Wheel),0))
    {
        cl->cl_Dispatcher.h_Entry    = (VOID *)CallCHook;
        cl->cl_Dispatcher.h_SubEntry = (VOID *)ClassDispatcher;
        cl->cl_Dispatcher.h_Data     = WheelBase;
        AddClass(cl);
    }

    WheelBase->wb_Class = cl;

    return (cl != NULL);
}


/*****************************************************************************/


BOOL DestroyClass(VOID)
{
struct WheelLib *wb = WheelBase;
BOOL             result;

    if (result = FreeClass(wb->wb_Class))
        wb->wb_Class = NULL;

    return(result);
}
