/*
 * $Id: main.c,v 39.4 93/02/19 16:39:16 mks Exp $
 *
 * $Log:	main.c,v $
 * Revision 39.4  93/02/19  16:39:16  mks
 * Added code to build the physical device list
 * 
 * Revision 39.3  93/02/10  12:03:49  mks
 * Cleaned up backdrop display window and some code savings
 *
 * Revision 39.2  93/01/21  14:48:20  mks
 * Cleaned up the disk init code...  (Removed much redundant code)
 *
 * Revision 39.1  92/06/11  15:48:23  mks
 * Now use FORBID and PERMIT macros rathern than calling Forbid() and Permit()
 *
 * Revision 38.8  92/04/16  10:20:05  mks
 * Now uses the new copyright strings method
 *
 * Revision 38.7  92/04/14  13:18:45  mks
 * Removed initialization of the Wait pointer
 *
 * Revision 38.6  92/03/19  09:20:50  mks
 * Changed shutdown order...  (Closes windows first)
 *
 * Revision 38.5  92/01/28  11:19:34  mks
 * Made sure that the draw pots are correct at open time...
 *
 * Revision 38.4  92/01/07  13:56:40  mks
 * Changed the init/shutdown to deal with the new prefs features
 *
 * Revision 38.3  91/11/13  10:57:25  mks
 * Removed the initfunc() from C and made a simple assembly version.
 *
 * Revision 38.2  91/11/12  18:40:31  mks
 * Improved wbmain() and a few other little changes
 *
 * Revision 38.1  91/06/24  11:37:04  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/alerts.h"
#include "exec/tasks.h"
#include "exec/memory.h"
#include "dos/dosextens.h"
#include "dos/dostags.h"
#include "wbstart.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

extern void *AbsExecBase;

extern void *Max_Copy_Mem;

/*
 * Free a path list as passed by LoadWB!!!
 */
void FreeMyPath(ULONG tmp1)
{
ULONG *tmp;

    /*
     * Convert into real pointer from BPTR and check it for valid...
     */
    while (tmp = (ULONG *)(tmp1 << 2))
    {
        UNLOCK(tmp[1]);	/* UNLOCK checks for NULL */

        /* Free the memory and go to the next... */
        tmp1=tmp[0];
        FreeMem(tmp,8);	/* This *MUST* be a FreeMem */
    }
}

/*
 * Another tick has happened...  Check that path situation...
 */
void PathTick(void)
{
struct WorkbenchBase *wb = getWbBase();
ULONG old;

    FORBID;
    if (old=wb->wb_OldPath)
    {
        if (wb->wb_OldPathTick)
	{
	    wb->wb_OldPathTick--;
	    old=NULL;
	}
	else wb->wb_OldPath=NULL;
    }
    PERMIT;

    FreeMyPath(old);
}

/*
	Quit workbench.  Close all windows, free all resources,
	free all memory, close all libraries, etc.
*/
void Quit()
{
    MP(("Quit: calling wbCleanup\n"));
    MasterSearch(wbCleanup); /* close all windows */

    MP(("Quit: calling uninitDisks\n"));
    uninitDisks(); /* free all disks */

    MP(("Quit: calling FreeWBMenus\n"));
    FreeWBMenus(); /* get rid of menus */
    MP(("Quit: calling WBFreeWBObject\n"));
    MasterSearch(WBFreeWBObject); /* free all objects */

    MP(("Quit: calling uninitTimer\n"));
    uninitTimer(); /* get rid of timer */
    MP(("Quit: calling uninitWbGels\n"));
    uninitWbGels(); /* get rid of gels */
    MP(("Quit: calling UnInitScreenAndWindows\n"));
    UnInitScreenAndWindows();
    MP(("Quit: calling termText\n"));
    termText(); /* terminate text */
    MP(("Quit: calling FORBID\n"));
    FORBID;
    MP(("Quit: calling AlohaWorkbench\n"));
    AlohaWorkbench(NULL); /* say goodbye to Intuition */
    MP(("Quit: calling PERMIT\n"));
    PERMIT;
    MP(("Quit: calling ioloopuninit\n"));
    ioloopuninit(); /* get rid of io stuff */
#ifdef ALLOCDEBUG
    FreeAllocList();
#endif ALLOCDEBUG
}

/*
 * Note that the only way into here is via workbench and we know that
 * the library had to be open when we were called...
 */
long wbmain(void)
{
struct WorkbenchBase *wb = getWbBase();

    MP(("wbmain: workbenchbase=%lx, arg=%lx\n",wb, wb->wb_Argument));

	/*
	 * Set up the maximum amount of memory that copy can use...
	 */
    wb->wb_ByteCount=2097152;

	/* Set up all of the lists... */
    {
    struct List *list;

	for (list=&(wb->wb_MasterList);list<=&(wb->wb_NonInfoList);list++) NewList(list);
    }

    MP(("main: calling ReadWBPrefs...\n"));
    ReadWBPrefs(); /* this is the initialization call */

    MP(("main: calling InitPattern...\n"));
    if (InitPattern())
    {
        MP(("main: calling InitScreenAndWindows...\n"));
        if (InitScreenAndWindows())
        {
            MP(("main: calling ioloopinit...\n"));
            ioloopinit();

            MP(("main: calling initText...\n"));
            initText();

            MP(("main: calling CreateWBMenus...\n"));
            if (CreateWBMenus())
            {
                MP(("main: calling InitPotion...\n"));
                if (InitPotion())
                {
                    MP(("main: calling BeginDiskIO...\n"));
                    BeginDiskIO();

                    MP(("main: calling initWbGels...\n"));
                    if (initWbGels())
                    {
                        MP(("main: calling initTimer...\n"));
                        if (initTimer())
                        {
                            /* Build list of physical devices */
                            VolumeListSearch(FindDevices,NULL,NULL,NULL,NULL);

                            MP(("main: calling AlohaWorkbench...\n"));
                            AlohaWorkbench((long)&wb->wb_IntuiPort);

                            /* flag that workbench is now up and running */
                            wb->wb_WorkbenchStarted=1;

                            MP(("main: calling StartupDrawer...\n"));
                            StartupDrawer();
                            MP(("main: calling EndDiskIO...\n"));
                            EndDiskIO();

                            /*
                             * Display the Copyright message...
                             */
                            UpdateMemory();
                            MessageTitle(CopyrightFormat,wb->wb_Copyright1,wb->wb_Copyright2,wb->wb_Copyright3);

                            wb->wb_Argument &= ~WBARGF_DELAYON;
                            if (wb->wb_Argument & WBARGF_CLEANUP) Cleanup(wb->wb_RootObject);
                            else RefreshDrawer(wb->wb_RootObject);

                            MinMaxDrawer(wb->wb_RootObject);

                            MP(("main: calling ioloop...\n"));
                            ioloop();

                            MP(("main: calling BeginDiskIO...\n"));
                            BeginDiskIO();
                        }
                    }
                    MP(("main: calling EndDiskIO...\n"));
                    EndDiskIO();
                }
            }
        }
    }

    /*
     * In case we had to bail and someone is still waiting...
     */
    wb->wb_Argument &= ~WBARGF_DELAYON;

    /* Set flag that we are going away... */
    wb->wb_Quit=TRUE;

/* PATH kludge... Begin ****************************************************/
    /*
     * Blow away the possible old path...
     */
    wb->wb_OldPathTick=0;
    PathTick();

    /*
     * mks:  We are no freeing the old lock list if it is there...
     * This is major kludge, but it must be done this way...  :-(
     */
    {
    ULONG *tmp;

	tmp=&(((struct CommandLineInterface *)((((struct Process *)(wb->wb_Task))->pr_CLI)<<2))->cli_CommandDir);

        MP(("main: kludge: freeing invalid path list...\n"));
	FreeMyPath(*tmp);

        /*
         * mks:  We are going to do something nasty now...
         * We are going to blow away the lock list from the CLI
         */
        MP(("main: kludge: clearing the CLI CommandDir...\n"));
        *tmp=NULL;

    }
/* PATH kludge... End ******************************************************/


    Quit();

    /*
     * Make sure that the expunge does not blow away the "last bit of code"
     * that has to run:  Namely, a moveq.l #0,d0 and rts
     */
    FORBID;
    wb->wb_Task=NULL;	/* We be gone now! */

    CLOSELIBRARY((struct Library *)wb);

    MP(("main: calling RemLibrary...\n"));
    RemLibrary(wb); /* force an expunge */

    MP(("main: falling off the end...\n"));
    return(NULL);
}

/*
 * The stacksize given to workbench...
 */
#define STACKSIZE	6000

/*
******i workbench.library/StartWorkbench ************************************
*
*   NAME
*	StartWorkbench - start workbench from scratch.
*
*   SYNOPSIS
*	error = StartWorkbench(arg, CmdPath)
*         D0                    D0    D1
*	ULONG StartWorkbench(ULONG, ULONG);
*
*   FUNCTION
*	Attempt to start workbench from scratch.  This routine can only
*	be called once and gets workbench up and running.
*
*   INPUTS
*	arg - arguement to workbench.
*	CmdPath - command path for workbench.
*
*   RESULTS
*	error - NULL if error, non-null if something worked...
*
*   SEE ALSO
*
*   BUGS
*	The command path is not a normal command path allocated and/or
*	freed by DOS.  It is allocated in its own unique way by LoadWB
*	and is freed by Workbench on exit.  This is due to compatibility
*	reasons that have no clear solution at this moment.  Hopefully
*	something can be done about this soon.  If this command returns
*	FAIL then the path given will need to be cleaned up by the caller.
*
******************************************************************************
*/
void WbSegment(void);

ULONG StartWorkbench(arg, CmdPath)
ULONG arg;
ULONG CmdPath;
{
struct WorkbenchBase *wb = getWbBase();
ULONG result=FALSE;
LONG CreateNewProcArray[] = {
	NP_Path, 0,
	NP_Entry, (LONG)(&WbSegment),
	NP_StackSize, STACKSIZE,	/* Stack size */
	NP_Priority, 1,			/* Workbench priority */
	NP_Name, (LONG)SystemWorkbenchName,
	NP_Cli, TRUE,			/* We are a CLI */
	NP_CommandName, (LONG)SystemWorkbenchName,
	NP_ConsoleTask, NULL,		/* No console task */
	NP_WindowPtr, NULL,		/* No window pointer */
	NP_CurrentDir, NULL,		/* No current dir */
	TAG_DONE, NULL
    };

    /* don't let us start twice */
    MP(("StartWorkbench: FORBID!\n"));
    FORBID;

    wb->wb_KPrintfOK	= (arg & WBARGF_DEBUGON);

    MP(("\nStartWorkbench: enter\n"));
    MP(("StartWorkbench: arg=%lx, CmdPath=%lx, wb=%lx\n",arg, CmdPath, wb));

    if (!wb->wb_Task)
    {
	wb->wb_Argument = arg;

        /* spin off the workbench.library process via CreateNewProc */

        CreateNewProcArray[1] = CmdPath;

        if (wb->wb_Task=(struct Process *)CREATENEWPROC((struct TagItem *)CreateNewProcArray))
        {
            MP(("wb->wb_Task=%lx\n", wb->wb_Task));
            result=TRUE;

            while (wb->wb_Argument & WBARGF_DELAYON) Delay(5);
        }
    }
    else
    {
	MP(( "swb: wbtask already exists (%lx)\n", wb->wb_Task ));
	/*
	 * Check if we should install a new path...
	 */
	if (arg & WBARGF_NEWPATH)
	{
        ULONG *tmp;

	    MP(("swb: Installing new path\n"));

	    tmp=&(((struct CommandLineInterface *)((((struct Process *)(wb->wb_Task))->pr_CLI)<<2))->cli_CommandDir);

	    /*
	     * Free the old path and install the current path into the old path
	     */
	    while (wb->wb_OldPath)
	    {
		Delay(50L);	/* Don't tick down the old path too fast! */
		PathTick();
	    }
	    wb->wb_OldPathTick=2;
	    wb->wb_OldPath=*tmp;
	    *tmp=CmdPath;

	    /*
	     * Return TRUE (WBARGF_NEWPATH) such that LoadWB will not
	     * free the path again...
	     */
	    result|=WBARGF_NEWPATH;
	}
    }

    MP(("StartWorkbench: PERMIT!, exit=%ld\n",result));
    PERMIT;
    return(result);
}
