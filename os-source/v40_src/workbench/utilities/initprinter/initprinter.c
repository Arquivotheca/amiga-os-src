
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "initprinter_rev.h"


/*****************************************************************************/


char *version = VERSTAG;


/*****************************************************************************/


LONG main(VOID)
{
struct Library    *DOSBase;
struct Library    *SysBase = (*((struct Library **) 4));
struct Process    *process;
struct WBStartup  *WBenchMsg = NULL;
LONG               failureLevel = RETURN_FAIL;
BPTR               file;

    process = (struct Process *)FindTask(NULL);
    if (!(process->pr_CLI))
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    if (DOSBase = OpenLibrary("dos.library",0))
    {
        if (file = Open("PRT:",MODE_OLDFILE))
        {
            if (Write(file,"\x1b#1\n",4) == 4)
                failureLevel = RETURN_OK;

            Close(file);
        }

        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
        Forbid();
        ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}
