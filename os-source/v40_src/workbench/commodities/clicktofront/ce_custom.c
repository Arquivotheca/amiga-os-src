

#include <exec/types.h>
#include <exec/libraries.h>
#include <devices/inputevent.h>
#include <intuition/intuitionbase.h>
#include <libraries/commodities.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>
#include <clib/commodities_protos.h>
#include <clib/icon_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/commodities_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include "ce_custom.h"


/*****************************************************************************/


#define	Q_MASK	(IEQUALIFIER_MIDBUTTON | IEQUALIFIER_RBUTTON |\
                 IEQUALIFIER_LSHIFT    | IEQUALIFIER_RSHIFT  |\
                 IEQUALIFIER_RALT      | IEQUALIFIER_LALT    |\
                 IEQUALIFIER_LCOMMAND  | IEQUALIFIER_RCOMMAND|\
                 IEQUALIFIER_CONTROL)

UWORD far umask = 0;


/*****************************************************************************/


extern struct Library       *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library       *CxBase;
extern struct Library       *IconBase;
extern LONG                  cxSignal;

struct Library *LayersBase;
LONG            oldPri;
WORD            oldX = -1;
WORD            oldY = -1;
ULONG           micros = 0;
ULONG           seconds = 0;
struct Screen  *oldScreen=NULL;
struct Window  *oldWindow=NULL;
struct Task    *parentTask;


/*****************************************************************************/


/* This defines the maximum distance that the second click can be from
 * the first and still be called a double click...
 */
#define	MAX_DISTANCE	4

/* This macro checks to see it a,b is within the legal distance... */
#define	CHECK_MAX(a,b)	(!(((a-b)>MAX_DISTANCE) || ((b-a)>MAX_DISTANCE)))


/*****************************************************************************/


VOID __stdargs __saveds ClickAction(CxMsg *cxm, CxObj *co)
{
struct Window	  *newWindow = NULL;
struct InputEvent *ie;
struct Screen	  *sc;
struct Layer	  *layer;
LONG		   lock;

    ie = (struct InputEvent *)CxMsgData(cxm);

    lock = LockIBase(0);

    for (sc=IntuitionBase->FirstScreen;sc;sc=sc->NextScreen)
        if (sc->MouseY >= 0)
            break;

    if (sc)
    {
        layer = WhichLayer(&(sc->LayerInfo),(LONG)(sc->MouseX),(LONG)(sc->MouseY));
        if (layer && (layer != sc->BarLayer))
        {
            if ((!(layer->Flags & LAYERBACKDROP)) && (newWindow=(struct Window *)layer->Window))
            {
                if (newWindow == oldWindow)
                {
                    if (newWindow != sc->LayerInfo.top_layer->Window)
                    {
                        if (CHECK_MAX(oldX,sc->MouseX) && CHECK_MAX(oldY,sc->MouseY))
                        {
                            if (DoubleClick(seconds,micros,ie->ie_TimeStamp.tv_secs,ie->ie_TimeStamp.tv_micro))
                            {
                                oldScreen=sc;
                                Signal(parentTask,(1L << cxSignal));
                            }
                        }
                    }
                }
            }
        }

        oldX = sc->MouseX;
        oldY = sc->MouseY;
    }

    UnlockIBase(lock);

    oldWindow = newWindow;
    seconds   = ie->ie_TimeStamp.tv_secs;
    micros    = ie->ie_TimeStamp.tv_micro;
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
CxObj                 *objList;
struct InputXpression ix;

    parentTask = FindTask(NULL);

    ix.ix_Version   = IX_VERSION;
    ix.ix_Class     = IECLASS_RAWMOUSE;
    ix.ix_Code      = IECODE_LBUTTON;
    ix.ix_CodeMask  = ~0;   /* must match IECODE_LBUTTON exactly */
    ix.ix_Qualifier = umask;
    ix.ix_QualMask  = Q_MASK;
    ix.ix_QualSame  = 0;

    if (objList = CxFilter(NULL))
    {
        SetFilterIX(objList,&ix);
        AttachCxObj(objList,CxCustom(ClickAction,NULL));
    }
    AttachCxObj(broker,objList);

    ix.ix_Class     = IECLASS_POINTERPOS;

    if (objList = CxFilter(NULL))
    {
        SetFilterIX(objList,&ix);
        AttachCxObj(objList,CxCustom(ClickAction,NULL));
    }
    AttachCxObj(broker,objList);

    ix.ix_Class = IECLASS_NEWPOINTERPOS;

    if (objList = CxFilter(NULL))
    {
        SetFilterIX(objList,&ix);
        AttachCxObj(objList,CxCustom(ClickAction,NULL));
    }
    AttachCxObj(broker,objList);

    if (CxObjError(objList) == 0)
    {
        if (LayersBase = OpenLibrary("layers.library",37))
        {
            oldPri = SetTaskPri(FindTask(NULL),21);
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


BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObject *diskObj,
                       ULONG *cliOpts)
{
STRPTR arg,qualName;

    qualName = "";
    if (wbMsg)
    {
        if (arg = FindToolType(diskObj->do_ToolTypes,"QUALIFIER"))
            qualName = arg;
    }
    else if (cliOpts[OPT_QUALIFIER])
    {
        qualName = (STRPTR)cliOpts[OPT_QUALIFIER];
    }

    if (!(stricmp(qualName,"LALT")))      umask |= IEQUALIFIER_LALT;
    if (!(stricmp(qualName,"LEFT_ALT")))  umask |= IEQUALIFIER_LALT;
    if (!(stricmp(qualName,"RALT")))      umask |= IEQUALIFIER_RALT;
    if (!(stricmp(qualName,"RIGHT_ALT"))) umask |= IEQUALIFIER_RALT;
    if (!(stricmp(qualName,"CONTROL")))   umask |= IEQUALIFIER_CONTROL;
    if (!(stricmp(qualName,"CTRL")))      umask |= IEQUALIFIER_CONTROL;

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
LONG	       lock;
struct Window *w      = oldWindow;
struct Window *window = NULL;
struct Screen *s      = oldScreen;
struct Screen *screen;

    lock = LockIBase(0);

    if (w && s)
    {
        for (screen=IntuitionBase->FirstScreen; screen; screen=screen->NextScreen)
            if (screen==s)
                break;

        if (screen)
        {
            for (window=screen->FirstWindow; window; window=window->NextWindow)
                if (window==w)
                    break;
        }
    }

    Forbid();
    UnlockIBase(lock);

    if (window)
        WindowToFront(window);

    if (w == oldWindow)
        oldWindow=NULL;

    Permit();
}
