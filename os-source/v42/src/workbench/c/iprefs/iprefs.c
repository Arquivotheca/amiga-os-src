
/* includes */
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/tasks.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/notify.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <libraries/locale.h>
#include <prefs/sound.h>
#include <prefs/icontrol.h>
#include <internal/iprefs.h>
#include <string.h>
#include <dos.h>

/* prototypes */
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/locale_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>

#define EXEC_PRIVATE_PRAGMAS

/* direct ROM interface */
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

/* application includes */
#include "iprefs_rev.h"
#include "iprefs.h"
#include "sound.h"
#include "eventloop.h"


/*****************************************************************************/


extern struct ExecBase *SysBase;
extern struct Library  *IntuitionBase;
extern struct Library  *GfxBase;
extern struct Library  *DOSBase;
extern struct Library  *UtilityBase;
extern struct Library  *LayersBase;
extern struct Library  *LocaleBase;


/*****************************************************************************/


#ifdef BETA_VERSION
VOID BetaAlert(VOID);
STRPTR TaggedOpenLibrary(LONG);
#endif

static VOID ProcessLoop(VOID);

VOID __far DisplayBeepPatch(VOID);
VOID __far OpenScreenPatch(VOID);
VOID __far CloseWorkBenchPatch(VOID);
VOID __far BackdropRender(VOID);


/*****************************************************************************/


#define NUM_PREFS_FILES 13


/*****************************************************************************/


/* from eventloop.c */
extern struct MsgPort  audioPort;
extern BOOL	       doResetWB;
extern struct Task    *parentTask;
extern struct Task    *iprefsTask;

/* from sound.c */
extern BOOL              doFlash;
extern struct SoundPrefs sp;
extern APTR              oldDisplayBeep;

/* from backdrop.c */
extern struct Hook            render;
extern struct SignalSemaphore bl;
extern APTR                   oldOpenScreen;
extern APTR                   oldCloseWorkBench;


extern APTR  _BSSBAS;
extern ULONG _BSSLEN;


/*****************************************************************************/


LONG main(VOID)
{
BPTR *nextHunk;

    geta4();

    /* kludge for SAS/C 6.3, to prevent it from generating data hunks that
     * are too large. Referencing _BSSBAS and _BSSLEN prevents the linker from
     * expanding the data hunk of the executable to include all BSS data. as
     * well. This reduces the on-disk size of the program by the size of the
     * BSS hunk.
     */
    memset(_BSSBAS, 0, _BSSLEN);

    SysBase       = (*((struct ExecBase **) 4));
    parentTask    = SysBase->ThisTask;

    DOSBase       = OpenLibrary("dos.library" VERSTAG,39);
    IntuitionBase = OpenLibrary("intuition.library",39);
    GfxBase       = OpenLibrary("graphics.library",39);
    UtilityBase   = OpenLibrary("utility.library",39);
    LayersBase    = OpenLibrary("layers.library",39);

    if (DOSBase && IntuitionBase && GfxBase && UtilityBase && LayersBase)
    {
        Forbid();

        if (!FindTask(IPREFSNAME))
        {
            if (iprefsTask = (struct Task *)CreateNewProcTags(NP_Entry,      ProcessLoop,
                                                              NP_CurrentDir, NULL,
                                                              NP_StackSize,  3500,
                                                              NP_Name,       IPREFSNAME,
                                                              NP_Priority,   0,
                                                              NP_WindowPtr,  NULL,
                                                              NP_HomeDir,    NULL,
                                                              NP_CopyVars,   FALSE,
                                                              TAG_DONE))
            {
                SetSignal(0,SIGF_CHILD);
                Wait(SIGF_CHILD);

                /* the following causes the first hunk of the load file to
                 * remain attached to the shell (so it gets freed), while
                 * all other hunks are detached and so stay in memory after
                 * this process terminate
                 */
                nextHunk  = BADDR(Cli()->cli_Module);
                *nextHunk = NULL;

                parentTask = NULL;

                Permit();

                return(RETURN_OK);
            }
        }

        Permit();
    }

    if (DOSBase)
    {
        CloseLibrary(DOSBase);
        CloseLibrary(GfxBase);
        CloseLibrary(IntuitionBase);
        CloseLibrary(LayersBase);
        CloseLibrary(UtilityBase);
    }

    return(RETURN_FAIL);
}


/*****************************************************************************/


static VOID CheckLocale(VOID)
{
BPTR lock;

    if (LocaleBase = OpenLibrary("locale.library",38))
    {
        if (lock = Lock("ENV:Sys/locale.prefs",ACCESS_READ))
        {
            UnLock(lock);
        }
        else
        {
            /* guaranteed to work */
            CloseLocale(SetDefaultLocale(OpenLocale(NULL)));
        }
    }
}


/*****************************************************************************/


static VOID MakeWBInterleaved(VOID)
{
struct IScreenModePrefs ismp;
struct Preferences      prefs;
ULONG                   mode;

    GetPrefs(&prefs,sizeof(prefs));

    mode = HIRES_KEY;
    if (prefs.LaceWB)
        mode = HIRESLACE_KEY;

    ismp.ism_DisplayID = mode;
    ismp.ism_Width     = (UWORD)-1;
    ismp.ism_Height    = (UWORD)-1;
    ismp.ism_Depth     = 2;
    ismp.ism_Control   = WBINTERLEAVED;

    SetIPrefs(&ismp, (LONG)sizeof ismp, (LONG)IP_SCREENMODE);
}


/*****************************************************************************/


static VOID TurnOnPromotion(VOID)
{
struct IIControlPrefs icp;
BPTR                  lock;

    if (lock = Lock("ENV:Sys/icontrol.prefs",ACCESS_READ))
    {
        UnLock(lock);
    }
    else
    {
        icp.iic_TimeOut     = 50;
        icp.iic_MetaDrag    = IEQUALIFIER_LCOMMAND;
        icp.iic_ICFlags     = ICF_COERCE_LACE | ICF_STRGAD_FILTER | ICF_MENUSNAP | ICF_MODEPROMOTE;
        icp.iic_WBtoFront   = 'N';
        icp.iic_FrontToBack = 'M';
        icp.iic_ReqTrue     = 'V';		/* CKey: Requester TRUE		*/
        icp.iic_ReqFalse    = 'B';

        SetIPrefs(&icp, (LONG)sizeof icp, (LONG)IP_ICONTROL);
    }
}


/*****************************************************************************/


VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)list;
}


/*****************************************************************************/


static VOID ProcessLoop(VOID)
{
struct NotifyRequest   nr[NUM_PREFS_FILES];
struct MsgPort         notifyPort;
UWORD                  i;
char                   name[30];
struct SignalSemaphore iprefsLock;

    geta4();

    memset(nr,0,sizeof(nr));

    notifyPort.mp_SigTask      = iprefsTask;
    notifyPort.mp_SigBit       = SIGBREAKB_CTRL_C;
    notifyPort.mp_Flags        = PA_SIGNAL;
    notifyPort.mp_Node.ln_Type = NT_MSGPORT;
    NewList(&notifyPort.mp_MsgList);

    audioPort.mp_SigTask      = iprefsTask;
    audioPort.mp_SigBit       = SIGBREAKB_CTRL_D;
    audioPort.mp_Flags        = PA_SIGNAL;
    audioPort.mp_Node.ln_Type = NT_MSGPORT;
    NewList(&audioPort.mp_MsgList);

    CheckLocale();
    MakeWBInterleaved();
    TurnOnPromotion();

    Forbid();

    /* install the patch to DisplayBeep() */
    doFlash             = TRUE;
    sp.sop_DisplayQueue = TRUE;
    sp.sop_AudioQueue   = FALSE;
    oldDisplayBeep = (APTR)SetFunction(IntuitionBase,-96,(VOID *)DisplayBeepPatch);

    /* install the patches to OpenScreenTagList() and CloseWorkBench() */
    render.h_Entry = (HOOKFUNC)BackdropRender;
    InitSemaphore(&bl);
    oldOpenScreen     = (APTR)SetFunction(IntuitionBase,-612,(VOID *)OpenScreenPatch);
    oldCloseWorkBench = (APTR)SetFunction(IntuitionBase,-78,(VOID *)CloseWorkBenchPatch);

    Permit();

#ifdef BETA_VERSION
    if (Strnicmp("Beta",TaggedOpenLibrary(-5),4) != 0)
    {
        BetaAlert();
    }
#endif

    i = 0;
    while (i < NUM_PREFS_FILES)
    {
        ExtractPrefsName(i,name);

        nr[i].nr_Name                 = name;
        nr[i].nr_UserData             = i;
        nr[i].nr_stuff.nr_Msg.nr_Port = &notifyPort;
        nr[i].nr_Flags                = NRF_SEND_MESSAGE|NRF_NOTIFY_INITIAL|NRF_WAIT_REPLY;

        if (!StartNotify(&nr[i]))
            break;

        i++;
    }

    InitSemaphore(&iprefsLock);
    iprefsLock.ss_Link.ln_Name = iprefsTask->tc_Node.ln_Name;
    AddSemaphore(&iprefsLock);

    doResetWB = FALSE;

    EventLoop(&notifyPort,&iprefsLock);
}
