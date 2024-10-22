
#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dostags.h>
#include <dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "loadresource.h"


/*****************************************************************************/


extern struct ExecBase *SysBase;
extern struct Library  *DOSBase;
extern struct Library  *LocaleBase;
extern struct Library  *UtilityBase;
extern struct Library  *GfxBase;
extern BPTR             mainHunk;
extern struct Task     *parentTask;


/*****************************************************************************/


VOID StartLoadResource(VOID);


/*****************************************************************************/


LONG main(VOID)
{
BPTR               *startupHunk;
struct CmdLineArgs  args;
struct Process     *process;
struct RDArgs      *rdargs;
struct Process     *child;
LONG                failureCode;
LONG                failureLevel;
struct MsgPort     *port;

    geta4();

    SysBase       = (*((struct ExecBase **) 4));
    parentTask    = SysBase->ThisTask;
    process       = (struct Process *)parentTask;

    failureCode   = ERROR_INVALID_RESIDENT_LIBRARY;
    failureLevel  = RETURN_FAIL;

    LocaleBase    = OpenLibrary("locale.library",38);
    DOSBase       = OpenLibrary("dos.library",39);
    UtilityBase   = OpenLibrary("utility.library",39);
    GfxBase       = OpenLibrary("graphics.library",39);

    if (DOSBase && UtilityBase && GfxBase)  /* we don't care about LocaleBase */
    {
        memset(&args,0,sizeof(args));
        if (rdargs = ReadArgs(TEMPLATE,args.cla_Arguments,NULL))
        {
            startupHunk = BADDR(Cli()->cli_Module);
            mainHunk    = *startupHunk;

            Forbid();

            if (!(port = FindPort(ProgramName())))
            {
                if (child = CreateNewProcTags(NP_Entry,      StartLoadResource,
                                              NP_Input,      NULL,
                                              NP_Output,     NULL,
                                              NP_CurrentDir, NULL,
                                              NP_StackSize,  3000,
                                              NP_Name,       ProgramName(),
                                              NP_Priority,   0,
                                              NP_WindowPtr,  NULL,
                                              NP_HomeDir,    NULL,
                                              NP_CopyVars,   FALSE,
                                              TAG_DONE))
                {
                    /* the following causes the first hunk of the load file to
                     * remain attached to the shell (so it gets freed), while
                     * all other hunks are detached and so stay in memory after
                     * this process terminate
                     */
                    *startupHunk = NULL;

                    port = &child->pr_MsgPort;
                }
            }

            if (port)
            {
                args.cla_Message.mn_Length    = sizeof(struct CmdLineArgs);
                args.cla_Message.mn_ReplyPort = &process->pr_MsgPort;
                args.cla_Version              = CLA_VERSION;
                args.cla_CurrentDir           = process->pr_CurrentDir;
                args.cla_Input                = Input();
                args.cla_Output               = Output();

                PutMsg(port,&args);
                WaitPort(&process->pr_MsgPort);
                GetMsg(&process->pr_MsgPort);

                failureLevel = args.cla_Result1;
                failureCode  = args.cla_Result2;
            }
            else
            {
                failureCode = IoErr();
                PrintFault(failureCode,NULL);
            }

            Permit();

            FreeArgs(rdargs);
        }
        else
        {
            failureCode = IoErr();
            PrintFault(failureCode,NULL);
        }
    }

    if (DOSBase)
    {
        SetIoErr(failureCode);

        CloseLibrary(DOSBase);
        CloseLibrary(UtilityBase);
        CloseLibrary(LocaleBase);
        CloseLibrary(GfxBase);
    }

    return(failureLevel);
}


/*****************************************************************************/


VOID StartLoadResource(VOID)
{
struct CmdLineArgs *args;
struct MsgPort      port;
struct MsgPort     *otherInstance;
struct Process     *process;

    geta4();

    process = (struct Process *)SysBase->ThisTask;
    WaitPort(&process->pr_MsgPort);
    args = (struct CmdLineArgs *)GetMsg(&process->pr_MsgPort);

    Forbid();

    if (otherInstance = FindPort(ProgramName()))
    {
        PutMsg(otherInstance,args);
        UnLoadSeg(mainHunk);
    }
    else
    {
        port.mp_SigTask      = SysBase->ThisTask;
        port.mp_SigBit       = AllocSignal(-1);  /* this always works... */
        port.mp_Flags        = PA_SIGNAL;
        port.mp_Node.ln_Type = NT_MSGPORT;
        port.mp_Node.ln_Name = ProgramName();
        port.mp_Node.ln_Pri  = -1;
        NewList(&port.mp_MsgList);
        AddPort(&port);
        PutMsg(&port,args);

        Permit();

        LoadResource(&port);
    }
}
