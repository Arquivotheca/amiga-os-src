
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dostags.h>
#include <dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "conclip.h"
#include "eventloop.h"
#include "conclip_rev.h"


/*****************************************************************************/


extern struct ExecBase *SysBase;
extern struct Library  *DOSBase;
extern struct Library  *IntuitionBase;
extern struct Library  *LocaleBase;

struct Task *parentTask;


/*****************************************************************************/


#define TEMPLATE  "CLIPUNIT=UNIT/N,OFF/S" VERSTAG
#define OPT_UNIT  0
#define OPT_OFF   1
#define OPT_COUNT 2

#define CONCLIPNAME "« ConClip »"


/*****************************************************************************/


VOID ProcessLoop(VOID);


/*****************************************************************************/


LONG main(VOID)
{
BPTR           *nextHunk;
struct CmdMsg   cm;
LONG            opts[OPT_COUNT];
struct Process *process;
struct RDArgs  *rdargs;
struct Task    *conClipTask;
LONG            failureCode;
LONG            failureLevel;

    geta4();

    SysBase       = (*((struct ExecBase **) 4));
    parentTask    = SysBase->ThisTask;
    process       = (struct Process *)parentTask;

    failureCode   = ERROR_INVALID_RESIDENT_LIBRARY;
    failureLevel  = RETURN_FAIL;

    DOSBase       = OpenLibrary("dos.library",39);
    IntuitionBase = OpenLibrary("intuition.library",39);
    LocaleBase    = OpenLibrary("locale.library",38);

    if (DOSBase && IntuitionBase)  /* we don't care about LocaleBase */
    {
        memset(opts,0,sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
        {
            Forbid();

            if (!(conClipTask = FindTask(CONCLIPNAME)))
            {
                if (conClipTask = (struct Task *)CreateNewProcTags(
                                                 NP_Entry,      ProcessLoop,
                                                 NP_Input,      NULL,
                                                 NP_Output,     NULL,
                                                 NP_CurrentDir, NULL,
                                                 NP_StackSize,  3000,
                                                 NP_Name,       CONCLIPNAME,
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
                }
            }

            if (conClipTask && conClipTask->tc_UserData)
            {
                /* send the options packet to the subtask */

                cm.cm_Msg.mn_Length    = sizeof(struct CmdMsg);
                cm.cm_Msg.mn_ReplyPort = &process->pr_MsgPort;
                cm.cm_Type             = TYPE_ARGS;
                cm.cm_Unit             = 0;
                cm.cm_Shutdown         = (BOOL)opts[OPT_OFF];

                if (opts[OPT_UNIT])
                    cm.cm_Unit = *(ULONG *)opts[OPT_UNIT];

                PutMsg((struct MsgPort *)conClipTask->tc_UserData, &cm);
                WaitPort(&process->pr_MsgPort);
                GetMsg(&process->pr_MsgPort);

                failureLevel = RETURN_OK;
                failureCode  = 0;
            }
            else
            {
                failureCode = ERROR_NO_FREE_STORE;
            }

            Permit();
        }
        else
        {
            failureCode = IoErr();
        }
    }

    if (DOSBase)
    {
        if (failureCode)
            PrintFault(failureCode,NULL);

        CloseLibrary(DOSBase);
        CloseLibrary(IntuitionBase);
        CloseLibrary(LocaleBase);
    }

    return(failureLevel);
}


/*****************************************************************************/


VOID ProcessLoop(VOID)
{
struct Task    *task;
struct MsgPort *port;

    geta4();

    task              = SysBase->ThisTask;
    port              = CreateMsgPort();
    task->tc_UserData = port;

    if (port)
    {
        Signal(parentTask,SIGF_CHILD);
        EventLoop(port);

        /* The above never returns */
    }

    /* life ends... */
    Forbid();
    Signal(parentTask,SIGF_CHILD);
}
