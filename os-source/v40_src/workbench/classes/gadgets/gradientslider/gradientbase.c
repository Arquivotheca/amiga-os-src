
#include <exec/types.h>
#include <intuition/classes.h>

#include <clib/intuition_protos.h>

#include <pragmas/intuition_pragmas.h>

#include "gradientbase.h"
#include "gradient.h"


/*****************************************************************************/


VOID CallCHook(VOID);


BOOL CreateClass(VOID)
{
Class *cl;

    if (cl = MakeClass("gradientslider.gadget","gadgetclass",NULL,sizeof(struct SliderInfo),0))
    {
        cl->cl_Dispatcher.h_Entry    = (VOID *)CallCHook;
        cl->cl_Dispatcher.h_SubEntry = (VOID *)ClassDispatcher;
        cl->cl_Dispatcher.h_Data     = GradientBase;
        AddClass(cl);
    }

    GradientBase->gb_Class = cl;

    return (cl != NULL);
}


/*****************************************************************************/


BOOL DestroyClass(VOID)
{
struct GradientLib *gb = GradientBase;
BOOL                result;

    if (result = FreeClass(gb->gb_Class))
        gb->gb_Class = NULL;

    return(result);
}
