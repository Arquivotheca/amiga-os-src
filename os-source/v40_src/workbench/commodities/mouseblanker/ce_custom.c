

#include <exec/types.h>
#include <exec/libraries.h>
#include <devices/inputevent.h>
#include <intuition/intuitionbase.h>
#include <libraries/commodities.h>
#include <graphics/gfxbase.h>
#include <graphics/sprite.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>
#include <clib/commodities_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/commodities_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "ce_custom.h"


/*****************************************************************************/


extern struct Library       *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library       *CxBase;
extern struct GfxBase       *GfxBase;
extern LONG                  cxSignal;


/*****************************************************************************/


BOOL                blanked = FALSE;
UWORD               tickCnt;
UWORD __chip        blankData[] = {0,0,0,0,0,0,0,0};
struct SimpleSprite blankMouse;


/*****************************************************************************/


struct InputXpression filterIX =
{
    IX_VERSION,
    IECLASS_RAWKEY | IECLASS_RAWMOUSE | IECLASS_POINTERPOS | IECLASS_NEWPOINTERPOS,

    0,                  /* Code: won't care             */
    0,                  /* CodeMask: 0 means don't care */

    0,
    IX_NORMALQUALS,
    0                   /* synonyms irrelevant          */
};


/*****************************************************************************/


#define BlankMouse()   {ChangeSprite(NULL,&blankMouse,(APTR)blankData); tickCnt = 0;}
#define UnblankMouse() RethinkDisplay();


/*****************************************************************************/


VOID __stdargs __saveds BlankAction(CxMsg *cxm, CxObj *co)
{
struct InputEvent *ie;
ULONG              class;
UWORD              code;
UWORD              quals;

    ie    = (struct InputEvent *) CxMsgData(cxm);
    class = ie->ie_Class;
    code  = ie->ie_Code;
    quals = ie->ie_Qualifier;

    if (!blanked)
    {
        if ((class == IECLASS_RAWKEY) && (!(code & IECODE_UP_PREFIX)))
        {
            BlankMouse();
            blanked = TRUE;
        }
    }
    else
    {
        if ((class == IECLASS_RAWMOUSE)
        ||  (class == IECLASS_POINTERPOS)
        ||  (class == IECLASS_NEWPOINTERPOS))
        {
            UnblankMouse();
            blanked = FALSE;
        }
        else if ((quals & IEQUALIFIER_RCOMMAND) || (quals & IEQUALIFIER_LCOMMAND))
        {
	    /* keyboard mouse movement */
            if ((code > 0xCB) && (code < 0xD0))
            {
                UnblankMouse();
                blanked = FALSE;
            }
        }
        else if (class == IECLASS_TIMER)
        {
            tickCnt++;
            if (tickCnt == 10)
                BlankMouse();
        }
    }
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
CxObj *objList;

    objList = CxFilter(NULL);
    SetFilterIX(objList,&filterIX);
    AttachCxObj(broker,objList);
    AttachCxObj(broker,CxCustom(BlankAction,NULL));

    return(CxObjError(objList) == 0);
}


/*****************************************************************************/


VOID DisposeCustomCx(VOID)
{
    if (blanked)
        UnblankMouse();
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
