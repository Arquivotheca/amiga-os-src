
/* includes */
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
#include <utility/tagitem.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include "iconx_rev.h"


/*****************************************************************************/


char *version = VERSTAG;

LONG SystemStub(struct Library *DOSBase, STRPTR cmdLine, ULONG tag, ...);
LONG WB2CLI(struct WBStartup *wbmsg, ULONG defaultStack, struct Library *DOSBase);


/*****************************************************************************/


LONG main(VOID)
{
struct Library    *IconBase;
struct Library    *DOSBase;
struct Library    *SysBase = (*((struct Library **) 4));
struct Process    *process;
struct WBStartup  *WBenchMsg;
struct DiskObject *diskObj;
struct WBArg      *wbArg;
LONG               argCnt;
STRPTR             ttype;
LONG               failureLevel = RETURN_FAIL;
BPTR               oldCD;
char               ioName[256];
BPTR               ioFile;
LONG               delay;
char               args[1024];
UWORD              len0,len1;
ULONG              stackSize;
BOOL               userShell;

    process = (struct Process *)FindTask(NULL);
    if (!(process->pr_CLI))
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);

        if (DOSBase = OpenLibrary("dos.library",37))
        {
            failureLevel = RETURN_ERROR;
            delay        = 2*TICKS_PER_SECOND;
            stackSize    = 4096;
            userShell    = FALSE;
            strcpy(ioName,"CON:0/50//80/IconX/AUTO");

            wbArg        = WBenchMsg->sm_ArgList;
            argCnt       = WBenchMsg->sm_NumArgs;

            if (argCnt > 1)
            {
                wbArg++;
                oldCD = CurrentDir(wbArg->wa_Lock);

                if (IconBase = OpenLibrary("icon.library",37))
                {
                    if (diskObj = GetDiskObject(wbArg->wa_Name))
                    {
                        if (ttype = FindToolType(diskObj->do_ToolTypes,"WINDOW"))
                            strncpy(ioName,ttype,sizeof(ioName));

                        if (ttype = FindToolType(diskObj->do_ToolTypes,"STACK"))
                            StrToLong(ttype,&stackSize);

                        if (ttype = FindToolType(diskObj->do_ToolTypes,"USERSHELL"))
                            userShell = MatchToolValue(ttype,"YES");

                        if (ttype = FindToolType(diskObj->do_ToolTypes,"WAIT"))
                        {
                            StrToLong(ttype,&delay);
                            delay = delay * TICKS_PER_SECOND;
                            if (!delay)
                                strcat(ioName,"/CLOSE/WAIT");
                        }
                        else if (ttype = FindToolType(diskObj->do_ToolTypes,"DELAY"))
                        {
                            StrToLong(ttype,&delay);
                            if (!delay)
                                strcat(ioName,"/CLOSE/WAIT");
                        }
                        FreeDiskObject(diskObj);
                    }
                    CloseLibrary(IconBase);
                }

                /* Turn us into a CLI process */
                WB2CLI(WBenchMsg,stackSize,DOSBase);

                sprintf(args,"FailAt 100\nExecute \"%s\"",wbArg->wa_Name);

                while (argCnt > 2)
                {
                    wbArg++;
                    strcat(args," \"");
                    len0 = strlen(args);
                    NameFromLock(wbArg->wa_Lock,&args[len0],sizeof(args)-len0);
                    len1 = strlen(args);
                    AddPart(&args[len0],wbArg->wa_Name,sizeof(args)-len1);
                    strcat(args,"\"");
                    argCnt--;
                }

                strcat(args,"\n");

                if (ioFile = Open(ioName,MODE_OLDFILE))
                {
                    SystemStub(DOSBase,args,SYS_Output,    NULL,
                                            SYS_Input,     ioFile,
                                            SYS_UserShell, userShell,
                                            TAG_DONE);

                    if (delay)
                        Delay(delay);

                    Close(ioFile);
                }
                CurrentDir(oldCD);
            }
            CloseLibrary(DOSBase);
        }

    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}


/*****************************************************************************/


LONG SystemStub(struct Library *DOSBase, STRPTR cmdLine, ULONG tag, ...)
{
    return(System(cmdLine,(struct TagItem *)&tag));
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
