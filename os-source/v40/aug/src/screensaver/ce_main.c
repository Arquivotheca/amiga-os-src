
#include <exec/types.h>
#include <dos/dos.h>
#include <workbench/startup.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/commodities_protos.h>
#include <clib/icon_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/commodities_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "ce_custom.h"
#include "ce_window.h"
#include "ce_strings.h"


/*****************************************************************************/


BOOL CreateCx(struct MsgPort *port);
VOID DisposeCx(VOID);
BOOL ProcessCxMsg(CxMsg *msg);


/*****************************************************************************/


extern struct WBStartup *WBenchMsg;
extern struct Library   *SysBase;
struct IntuitionBase    *IntuitionBase;
struct Library          *CxBase;
struct GfxBase          *GfxBase;
struct Library	        *DOSBase;
struct Library          *GadToolsBase;
struct Library          *IconBase;
struct Library          *UtilityBase;

struct MsgPort	   	*cxPort;
CxObj                   *cxBroker;
LONG                     cxSignal;
LONG                     cxPri;
BOOL                     cxPopUp;
STRPTR                   cxPopKey;

#if CE_WINDOW
extern struct MsgPort   *intuiPort;
extern ULONG             intuiMask;
#else
       ULONG             intuiMask = 0;
#endif


/*****************************************************************************/


LONG main(VOID)
{
ULONG              sigs;
APTR               msg;
ULONG              cxSigMask;
ULONG              cxMask;
BOOL               ok;
LONG               opts[OPT_COUNT];
BPTR               oldCD;
STRPTR             arg;
struct WBArg      *wbArg;
struct RDArgs     *rdargs;
struct DiskObject *diskObj;

    CxBase        = OpenLibrary("commodities.library",37);
    DOSBase       = OpenLibrary("dos.library",37);
    IntuitionBase = OpenLibrary("intuition.library",37);
    GfxBase       = OpenLibrary("graphics.library",37);
    GadToolsBase  = OpenLibrary("gadtools.library",37);
    UtilityBase   = OpenLibrary("utility.library",37);
    IconBase      = OpenLibrary("icon.library",37);
    cxPort        = CreateMsgPort();
    cxSignal      = AllocSignal(-1);
    cxBroker      = NULL;
    cxPri         = CE_PRIORITY;
    cxPopUp       = CE_POPUP;
    cxPopKey      = CE_POPKEY;
    rdargs        = NULL;
    diskObj       = NULL;
    ok            = FALSE;

    if (IntuitionBase && CxBase && GfxBase && DOSBase && GadToolsBase && UtilityBase && IconBase
        && cxPort && (cxSignal >= 0))
    {
        OpenCat();

        cxMask    = (1L << cxPort->mp_SigBit);
        cxSigMask = (1L << cxSignal);
        CurrentDir(oldCD = CurrentDir(NULL));

        if (WBenchMsg)
        {
            wbArg = WBenchMsg->sm_ArgList;
            CurrentDir(wbArg->wa_Lock);

            if (diskObj = GetDiskObject(wbArg->wa_Name))
            {
                if (arg = FindToolType(diskObj->do_ToolTypes,"CX_PRIORITY"))
                    StrToLong(arg,&cxPri);

#if CE_WINDOW
                if (arg = FindToolType(diskObj->do_ToolTypes,"CX_POPUP"))
                    cxPopUp = !MatchToolValue(arg,"NO");

                if (arg = FindToolType(diskObj->do_ToolTypes,"CX_POPKEY"))
                    cxPopKey = arg;
#endif
            }

            ok = TRUE;
        }
        else
        {
            memset(opts,0,sizeof(opts));
            if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
            {
                if (opts[OPT_PRIORITY])
                    cxPri = *((LONG *)opts[OPT_PRIORITY]);

#if CE_WINDOW
                if (opts[OPT_POPUP])
                    cxPopUp = (Stricmp((STRPTR)opts[OPT_POPUP],"NO") != 0);

                if (opts[OPT_POPKEY])
                    cxPopKey = (STRPTR)opts[OPT_POPKEY];
#endif

                ok = TRUE;
            }
            else
            {
                PrintFault(IoErr(),GetString(CE_NAME));
            }
        }

        if (ok && ProcessCustomArgs(WBenchMsg,diskObj,(ULONG *)&opts))
        {
            if (CreateCx(cxPort))
            {
#if CE_WINDOW
                if (cxPopUp)
                    CreateWindow();
#endif

                while (ok)
                {
                    sigs = Wait(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F | intuiMask | cxMask | cxSigMask);

                    if ((sigs & SIGBREAKF_CTRL_E) || (sigs & SIGBREAKF_CTRL_C))
                        break;

#if CE_WINDOW
                    if (sigs & SIGBREAKF_CTRL_F)
                        CreateWindow();
#endif

                    if (sigs & cxSigMask)
                        ProcessCustomCxSig();

                    while (ok && (msg = GetMsg(cxPort)))
                        ok = ProcessCxMsg(msg);

#if CE_WINDOW
                    while (ok && intuiPort && (msg = GT_GetIMsg(intuiPort)))
                        ok = ProcessIntuiMsg(msg);
#endif
                }
                DisposeCx();
#if CE_WINDOW
                DisposeWindow();
#endif
            }
        }

        if (diskObj)
            FreeDiskObject(diskObj);
        CurrentDir(oldCD);
        if (rdargs)
            FreeArgs(rdargs);

        CloseCat();
    }

    if (cxSignal >= 0)
        FreeSignal(cxSignal);

    if (cxPort)
    {
        while (msg = GetMsg(cxPort))
            ReplyMsg(msg);

        DeleteMsgPort(cxPort);
    }

    CloseLibrary(IconBase);
    CloseLibrary(CxBase);
    CloseLibrary((struct Library *)IntuitionBase);
    CloseLibrary((struct Library *)GfxBase);
    CloseLibrary(DOSBase);
    CloseLibrary(GadToolsBase);
    CloseLibrary(UtilityBase);

    return(0);
}


/*****************************************************************************/


#if CE_WINDOW
BOOL ProcessCxMsg(CxMsg *msg)
{
ULONG msgID;
ULONG msgType;

    msgID   = CxMsgID(msg);
    msgType = CxMsgType(msg);
    ReplyMsg((struct Message *)msg);

    switch (msgType)
    {
        case CXM_IEVENT  : switch(msgID)
                           {
                               case CE_POPKEYID: CreateWindow();
                                                 break;

                               default         : ProcessCustomCxMsg(msgID);
                                                 break;
                           }
                           break;

        case CXM_COMMAND : switch(msgID)
                           {
                               case CXCMD_DISABLE  : ActivateCxObj(cxBroker,0);
                                                     break;

                               case CXCMD_ENABLE   : ActivateCxObj(cxBroker,1);
                                                     break;

                               case CXCMD_APPEAR   :
                               case CXCMD_UNIQUE   : CreateWindow();
                                                     break;

                               case CXCMD_DISAPPEAR: DisposeWindow();
                                                     break;

                               case CXCMD_KILL     : return(FALSE);

                               default             : ProcessCustomCxCmd(msgID);
                                                     break;
                           }

    }
    return(TRUE);
}

#else  /* NOT CE_WINDOW */

BOOL ProcessCxMsg(CxMsg *msg)
{
ULONG msgID;
ULONG msgType;

    msgID   = CxMsgID(msg);
    msgType = CxMsgType(msg);
    ReplyMsg((struct Message *)msg);

    switch (msgType)
    {
        case CXM_IEVENT  : ProcessCustomCxMsg(msgID);
                           break;

        case CXM_COMMAND : switch(msgID)
                           {
                               case CXCMD_DISABLE  : ActivateCxObj(cxBroker,0);
                                                     break;

                               case CXCMD_ENABLE   : ActivateCxObj(cxBroker,1);
                                                     break;

                               case CXCMD_KILL     :
                               case CXCMD_APPEAR   :
                               case CXCMD_UNIQUE   : return(FALSE);

                               case CXCMD_DISAPPEAR: break;

                               default             : ProcessCustomCxCmd(msgID);
                                                     break;
                           }

    }
    return(TRUE);
}
#endif /* CE_WINDOW */


/*****************************************************************************/


BOOL CreateCx(struct MsgPort *port)
{
struct NewBroker nb;
#if CE_WINDOW
CxObj           *filter;
#endif

    DisposeCx();

    nb.nb_Version         = NB_VERSION;
    nb.nb_Name            = GetString(CE_NAME);
    nb.nb_Title           = GetString(CE_TITLE);
    nb.nb_Descr           = GetString(CE_DESCRIPTION);
    nb.nb_Unique          = NBU_NOTIFY | NBU_UNIQUE;
    nb.nb_Pri             = cxPri;
    nb.nb_Port            = port;
    nb.nb_ReservedChannel = NULL;

#if CE_WINDOW
    nb.nb_Flags           = COF_SHOW_HIDE;
#else
    nb.nb_Flags           = 0;
#endif

    if (cxBroker = CxBroker(&nb,NULL))
    {

#if CE_WINDOW
        /* install a hotkey for popping up window   */
        filter = CxFilter(cxPopKey);
        AttachCxObj(filter,CxSender(port,CE_POPKEYID));
        AttachCxObj(filter,CxTranslate(NULL));
        AttachCxObj(cxBroker,filter);
#endif

        if (CreateCustomCx(cxBroker))
        {
            if (CxObjError(cxBroker) == 0)
            {
                ActivateCxObj(cxBroker,1);
                return(TRUE);
            }
            DisposeCustomCx();
        }
        DeleteCxObjAll(cxBroker);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID DisposeCx(VOID)
{
    if (cxBroker)
    {
        DisposeCustomCx();
        DeleteCxObjAll(cxBroker);
        cxBroker = NULL;
    }
}
