

#include <exec/types.h>
#include <exec/libraries.h>
#include <devices/inputevent.h>
#include <intuition/intuitionbase.h>
#include <libraries/commodities.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>
#include <clib/commodities_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/commodities_pragmas.h>

#include "ce_custom.h"


/*****************************************************************************/


extern struct Library       *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library       *CxBase;
extern LONG                  cxSignal;

struct Library *LayersBase;
LONG            oldPri;
WORD            oldX;
WORD            oldY;


/*****************************************************************************/


struct InputXpression filterIX =
{
    IX_VERSION,             /* required                           */
    IECLASS_RAWKEY,

    0,                      /* Code: won't care                     */
    0,                      /* CodeMask: 0 means don't care about Code   */

    IEQUALIFIER_CAPSLOCK,   /* qualifier I am interested in            */
    IEQUALIFIER_CAPSLOCK,   /* and it's the only qualifier of interest   */
    0                       /* synonyms irrelevant                  */
};


/*****************************************************************************/


VOID __stdargs __saveds NoCapsAction(CxMsg *cxm, CxObj *co)
{
struct InputEvent *ie;

   ie = (struct InputEvent *) CxMsgData(cxm);
   ie->ie_Qualifier &= ~IEQUALIFIER_CAPSLOCK;
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
CxObj *filter;

    if (filter = CxFilter(NULL))
    {
        SetFilterIX(filter,&filterIX);
        AttachCxObj(filter,CxCustom(NoCapsAction,0));
        if (CxObjError(filter) == 0)
        {
            AttachCxObj(broker,filter);
            return(TRUE);
        }
        DeleteCxObjAll(filter);
    }
    return(FALSE);
}


/*****************************************************************************/


VOID DisposeCustomCx(VOID)
{
}


/*****************************************************************************/


BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObj *diskObj,
                       ULONG *cliOpts)
{
    return(TRUE);
}


/*****************************************************************************/


VOID ProcessCustomCxMsg(ULONG cmd)
{
}


/*****************************************************************************/


VOID ProcessCustomCxCmd(ULONG cmd)
{
}


/*****************************************************************************/


VOID ProcessCustomCxSig(VOID)
{
}
