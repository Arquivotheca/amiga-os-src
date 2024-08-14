

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
    IX_VERSION,         /* required                    */
    IECLASS_TIMER,      /* Only when timer goes by...  */

    0,                  /* Code: won't care             */
    0,                  /* CodeMask: 0 means don't care */

    0,
    (IEQUALIFIER_LEFTBUTTON | IEQUALIFIER_MIDBUTTON | IEQUALIFIER_RBUTTON),
    0                   /* synonyms irrelevant          */
};


/*****************************************************************************/


/* This defines the maximum distance that the second click can be from
 * the first and still be called a double click...
 */
#define	MAX_DISTANCE 3

/* This checks to see it a,b is within the legal distance... */
#define	CHECK_MAX(a,b,c) (((a-b)>(c)) || ((b-a)>(c)))


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
CxObj *objList;

    if (objList = CxFilter(NULL))
    {
        SetFilterIX(objList,&filterIX);
        AttachCxObj(objList,CxSignal(FindTask(NULL),cxSignal));
    }
    AttachCxObj(broker,objList);

    if (CxObjError(objList) == 0)
    {
        if (LayersBase = OpenLibrary("layers.library",37))
        {
            oldPri = SetTaskPri(FindTask(NULL),19);
            return(TRUE);
        }
    }

    return(FALSE);
}


/*****************************************************************************/


VOID DisposeCustomCx(VOID)
{
    CloseLibrary(LayersBase);
    SetTaskPri(FindTask(NULL),oldPri);
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
struct Window *window=NULL;
struct Screen *screen;
struct Layer  *layer;
LONG  	       lock;

    lock = LockIBase(0);

    for (screen = IntuitionBase->FirstScreen; screen && (screen->MouseY < 0); screen=screen->NextScreen);

    if (CHECK_MAX(oldX,IntuitionBase->MouseX,MAX_DISTANCE*2)) screen=NULL;
    if (CHECK_MAX(oldY,IntuitionBase->MouseY,MAX_DISTANCE)) screen=NULL;

    if (screen)
    {
        layer = WhichLayer(&screen->LayerInfo,screen->MouseX,screen->MouseY);
        if ((layer) && (layer != screen->BarLayer))
        {
            if (window = (struct Window *)layer->Window)
            {
                if (window == IntuitionBase->ActiveWindow)
                    window = NULL;
            }
        }
    }

    oldX = IntuitionBase->MouseX;
    oldY = IntuitionBase->MouseY;

    Forbid();
    UnlockIBase(lock);

    if (window)
        ActivateWindow(window);

    Permit();
}
