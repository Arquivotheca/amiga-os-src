
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>

/* prototypes */
#include <clib/exec_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>

#include "nofastmem_rev.h"


/*****************************************************************************/


char *version = VERSTAG;


/*****************************************************************************/


#define LVOAllocMem (-198)
#define LVOAvailMem (-216)
#define TASKNAME    "« NoFastMem »"


/*****************************************************************************/


extern VOID NoFastAllocMem();
extern VOID NoFastAvailMem();

APTR oldAllocMem;
APTR oldAvailMem;


/*****************************************************************************/


LONG main(VOID)
{
struct Task      *task;
struct Process	 *process;
struct Library   *SysBase = (*((struct Library **) 4));
struct WBStartup *WBenchMsg = NULL;
STRPTR            oldName;
APTR              newAllocMem;
APTR              newAvailMem;
LONG              failureLevel;

    process = (struct Process *)FindTask(NULL);
    if (!(process->pr_CLI))
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    Forbid();

    if (task = FindTask(TASKNAME))
    {
        Signal(task,SIGBREAKF_CTRL_C);
        failureLevel = RETURN_WARN;
    }
    else
    {
	oldName = process->pr_Task.tc_Node.ln_Name;
	process->pr_Task.tc_Node.ln_Name = TASKNAME;

	oldAllocMem = SetFunction(SysBase, LVOAllocMem, (APTR)NoFastAllocMem);
	oldAvailMem = SetFunction(SysBase, LVOAvailMem, (APTR)NoFastAvailMem);

	while (TRUE)
        {
	    Wait(SIGBREAKF_CTRL_C);

	    newAllocMem = SetFunction(SysBase, LVOAllocMem, oldAllocMem);
	    newAvailMem = SetFunction(SysBase, LVOAvailMem, oldAvailMem);

	    if ((newAllocMem == (APTR)NoFastAllocMem)
            &&  (newAvailMem == (APTR)NoFastAvailMem))
	        break;

	    SetFunction(SysBase, LVOAllocMem, newAllocMem);
	    SetFunction(SysBase, LVOAvailMem, newAvailMem);
	}

	process->pr_Task.tc_Node.ln_Name = oldName;
	failureLevel = RETURN_OK;
    }

    Permit();

    if (WBenchMsg)
    {
    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}
