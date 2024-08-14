

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


/*****************************************************************************/


struct InputXpression filterIX =
{
    IX_VERSION,             /* required                                 */
    IECLASS_RAWKEY,

    0,                      /* Code: won't care                         */
    0,                      /* CodeMask: 0 means don't care about Code  */

    IEQUALIFIER_CAPSLOCK,   /* qualifier I am interested in             */
    IEQUALIFIER_CAPSLOCK,   /* and it's the only qualifier of interest  */
    0                       /* synonyms irrelevant                      */
};

struct InputXpression filterRest =
{
    IX_VERSION,             /* required                                 */
    IECLASS_RAWKEY,

    0,                      /* Code: won't care                         */
    0xff,                   /* CodeMask: all bits much match            */

    IEQUALIFIER_NUMERICPAD, /* qualifier I am interested in             */
    IEQUALIFIER_NUMERICPAD, /* and it's the only qualifier of interest  */
    0                       /* synonyms irrelevant                      */
};


/*****************************************************************************/


UBYTE far codeMap[] =
{
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x5a,  /* ( */
    0x5b,  /* ) */
    0x5c,  /* / */
    0x5d,  /* * */
    0x00,
    0x00,
    0x00,
    0x0F,

    /* 0x10 */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x3d,  /* 7 */
    0x3e,  /* 8 */
    0x3f,  /* 9 */
    0x4a,  /* - */
    0x00,
    0x00,
    0x1d,
    0x1e,
    0x1f,

    /* 0x20 */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x2d,  /* 4 */
    0x2e,  /* 5 */
    0x2f,  /* 6 */
    0x5e,  /* + */
    0x00,
    0x00,
    0x2d,
    0x2e,
    0x2f,

    /* 0x30 */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x1d,   /* 1 */
    0x1e,   /* 2 */
    0x1f,   /* 3 */
    0x00,
    0x00,
    0x3d,
    0x3e,
    0x3f,

    /* 0x40 */
    0x0f,   /* 0 */
    0x41,
    0x42,
    0x43,
    0x43,   /* ENTER */
    0x45,
    0x46,
    0x00,
    0x00,
    0x00,
    0x4a,
    0x4b,
    0x4c,
    0x4d,
    0x4e,
    0x4f,

    /* 0x50 */
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5a,
    0x5b,
    0x5c,
    0x5d,
    0x5e,
    0x5f,

    /* 0x60 */
    0x60,
    0x61,
    0x62,
    0x63,
    0x64,
    0x65,
    0x66,
    0x67,
    0x68,
    0x69,
    0x6a,
    0x6b,
    0x6c,
    0x6d,
    0x6e,
    0x6f,

    /* 0x70 */
    0x70,
    0x71,
    0x72,
    0x73,
    0x74,
    0x75,
    0x76,
    0x77,
    0x78,
    0x79,
    0x7a,
    0x7b,
    0x7c,
    0x7d,
    0x7e,
    0x7f,
};

VOID __stdargs __saveds NoCapsAction(CxMsg *cxm, CxObj *co)
{
struct InputEvent *ie;
UWORD              high;
UWORD              code;
UWORD              map;

    ie = (struct InputEvent *) CxMsgData(cxm);
    ie->ie_Qualifier &= ~IEQUALIFIER_CAPSLOCK;

    code = ie->ie_Code;
    high = code & 0x80;
    map  = codeMap[code & 0x7f] | high;

    if (code != map)
    {
        ie->ie_Qualifier |= IEQUALIFIER_NUMERICPAD;
        ie->ie_Code       = map;
    }
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
CxObj *filter;
CxObj *filter2;

    if (filter = CxFilter(NULL))
    {
        SetFilterIX(filter,&filterIX);
        AttachCxObj(filter,CxCustom(NoCapsAction,0));
        if (CxObjError(filter) == 0)
        {
            AttachCxObj(broker,filter);

            if (filter2 = CxFilter(NULL))
            {
                SetFilterIX(filter2,&filterRest);
                AttachCxObj(filter2,CxTranslate(NULL));
                AttachCxObj(filter,filter2);

                if (CxObjError(filter) == 0)
                    return(TRUE);
            }
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
