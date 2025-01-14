
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include "cli_rev.h"


/*****************************************************************************/


char *version = VERSTAG;

LONG WB2CLI(struct WBStartup *wbmsg, ULONG defaultStack, struct Library *DOSBase);


/*****************************************************************************/


LONG main(VOID)
{
struct Library    *SysBase = (*((struct Library **) 4));
struct Library    *IconBase;
struct Library    *DOSBase;
struct Process    *process;
struct WBStartup  *WBenchMsg = NULL;
APTR               oldWP;
BPTR               oldCD;
BPTR               cd;
BPTR               lock;
STRPTR             string;
struct WBArg      *wbarg;
struct DiskObject *diskObj;
STRPTR             from;
STRPTR             window;
ULONG              stack;
char               command[270];

    process = (struct Process *)FindTask(NULL);
    if (!process->pr_CLI)
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *)GetMsg(&process->pr_MsgPort);
    }

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        stack    = 4096;
        window   = "CON:0/50//130/AmigaShell/CLOSE";
        from     = NULL;
        oldCD    = process->pr_CurrentDir;
        cd       = NULL;
        diskObj  = NULL;
        IconBase = NULL;

        if (WBenchMsg)
        {
            if (IconBase = OpenLibrary("icon.library",37))
            {
                wbarg = WBenchMsg->sm_ArgList;
                if (WBenchMsg->sm_NumArgs > 1)
                    wbarg++;

                CurrentDir(wbarg->wa_Lock);
                if (diskObj = GetDiskObject(wbarg->wa_Name))
                {
                    if (string = FindToolType(diskObj->do_ToolTypes,"STACK"))
                        StrToLong(string,&stack);

                    if (string = FindToolType(diskObj->do_ToolTypes,"WINDOW"))
                        window = string;

                    from = FindToolType(diskObj->do_ToolTypes,"FROM");
                }
            }

            if (stack < 400)
                stack = 4096;

            if (WB2CLI(WBenchMsg,stack,DOSBase))
            {
                if (cd = Lock("SYS:",SHARED_LOCK))
                {
                    CurrentDir(cd);
                    SetCurrentDirName("SYS:");
                }
            }
        }

        if (from)
        {
            if (!(lock = Lock(from,ACCESS_READ)))
                from = NULL;
        }
        else
        {
            from                  = "S:Shell-Startup";
            oldWP                 = process->pr_WindowPtr;
            process->pr_WindowPtr = (APTR)-1;

            if (!(lock = Lock(from,ACCESS_READ)))
                from = "NIL:";

            process->pr_WindowPtr = oldWP;
        }
        UnLock(lock);

        strcpy(command,"NewShell \"");
        strcat(command,window);
        strcat(command,"\"");

        if (from)
        {
            strcat(command," \"");
            strcat(command,from);
            strcat(command,"\"");
        }

        System(command,NULL);

        if (diskObj)
            FreeDiskObject(diskObj);

        CurrentDir(oldCD);
        UnLock(cd);

        CloseLibrary(IconBase);
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
        Forbid();
        ReplyMsg(WBenchMsg);
    }

    return(0);
}


/*****************************************************************************/


LONG WB2CLI(struct WBStartup *wbMsg, ULONG defaultStack, struct Library *DOSBase)
{
struct Library              *SysBase = (*((struct ExecBase **) 4));
struct Process		    *process;
struct CommandLineInterface *cli;
struct MsgPort		    *wbPort;
struct Process		    *wbProc;
struct CommandLineInterface *wbCLI;
ULONG			    *wbPath;
ULONG			    *lastPath;
ULONG			    *tmp;
STRPTR                      prompt;

    process = (struct Process *)FindTask(NULL);
    cli     = BADDR(process->pr_CLI);

    if (!cli && wbMsg && (cli = AllocDosObjectTagList(DOS_CLI,NULL)))
    {
        cli->cli_DefaultStack  = ((defaultStack+3) / 4);
        process->pr_CLI        = MKBADDR(cli);
        process->pr_Flags     |= PRF_FREECLI;

        Forbid();

        if (wbPort = wbMsg->sm_Message.mn_ReplyPort)
        {
            if ((wbPort->mp_Flags & PF_ACTION) == PA_SIGNAL)
            {
                if (wbProc = wbPort->mp_SigTask)
                {
                    if (wbProc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
                    {
                        if (wbCLI = BADDR(wbProc->pr_CLI))
                        {
			    prompt = BADDR(wbCLI->cli_Prompt);
                            if (prompt)
                                SetPrompt(&(prompt[1]));

                            wbPath   = BADDR(wbCLI->cli_CommandDir);
                            lastPath = &(cli->cli_CommandDir);
                            while (wbPath)
                            {
                                if (wbPath[1])
                                {
                                    if (tmp = AllocVec(8,MEMF_CLEAR|MEMF_PUBLIC))
                                    {
                                        if (!(tmp[1] = DupLock(wbPath[1])))
                                        {
                                            FreeVec(tmp);
                                            break;
                                        }
                                        lastPath[0] = MKBADDR(tmp);
                                        lastPath    = tmp;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }

                                wbPath = BADDR(wbPath[0]);
                            }
                        }
                    }
                }
            }
        }

        Permit();
    }

    return((LONG)cli);
}
