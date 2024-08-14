/*
 * $Id: startup.c,v 38.5 93/03/25 11:40:45 mks Exp $
 *
 * $Log:	startup.c,v $
 * Revision 38.5  93/03/25  11:40:45  mks
 * Fixed problem with a tool doing APP calls while Workbench is
 * in the Tool-not-done requester from WBStartup...
 * 
 * Revision 38.4  92/06/29  18:55:10  mks
 * No real changes...  Just some silly rewording...
 *
 * Revision 38.3  92/05/30  16:55:46  mks
 * Now uses the GETMSG/PUTMSG stubs
 *
 * Revision 38.2  92/04/27  14:20:41  mks
 * No longer tries to run the wrong types of icons in WBStartUp
 * WBGetWB...() no longer needs the NULL parameter.
 *
 * Revision 38.1  91/06/24  11:38:26  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/* Routines added for Startup Drawer support (Feb/89) */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/alerts.h"
#include "graphics/text.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "intuition/intuition.h"
#include "macros.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "errorindex.h"
#include "support.h"
#include "quotes.h"

#define STARTDRAWER	"SYS:WBStartUp"		/* where to find dir */
#define STARTDELAY	300			/* in jiffies (5 seconds) */

void StartupDrawerWait(obj)
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();
    struct MsgPort *wbp = &wb->wb_WorkbenchPort;
    struct Message *msg;
    struct Window *win;
    char *s;
    int timeout, delay;
    BOOL doitagain = FALSE, keep_going;

    struct EasyStruct es = {
	sizeof(struct EasyStruct),
	NULL,
	SystemWorkbenchName,
	NULL,	/* filled in */
	NULL,	/* filled in */
    };

    MP(("SDW: enter, obj=$%lx (%s)\n", obj, obj->wo_Name));

    delay = STARTDELAY; /* default: wait for STARTDELAY jiffies */

    if (s = FindToolType(obj->wo_ToolTypes, "WAIT"))
    {
	delay = (atoi(s) * 60) + 1; /* wait for 's' seconds */
    }

    if (FindToolType(obj->wo_ToolTypes, "DONOTWAIT"))
    {
	delay = -1; /* fake that the program exited */
    }

    do
    {
	timeout = delay;
	while (timeout>0)
	{   /* wait for timeout or toolexit message */
	    /* look for toolexit message */
	    while (msg = GETMSG(wbp))
	    {
		switch(((SHORT *)msg)[-1])
		{
		case MTYPE_TOOLEXIT:
		    MP(("SDW: got a MTYPE_TOOLEXIT message\n"));
		    ExitTool((struct WBStartup *)msg);
		    timeout = 0; /* flag that tool exited */
		    break;
		case MTYPE_TIMER:
		    MP(("SDW: got a MTYPE_TIMER message\n"));
		    TimeTick((struct timerequest *)msg);
		    break;
		case MTYPE_APP_FUNC:	/* We got an Add/Del AppXXX */
		    DoAppWork(msg);
		    break;
		default:
		    Alert(AN_Workbench|AN_WBBadStartupMsg1);
		    break;
		}
	    }
	    WaitTOF();
	    timeout--;
	}

	if (timeout==0) /* if no toolexit msg */
	{
	ULONG sigbits=(1L << wbp->mp_SigBit);

	    /* put up requester */
	    es.es_TextFormat = Quote(Q_STARTUP_WAIT);
	    es.es_GadgetFormat = Quote(Q_OK_CANCEL_TEXT);
	    win = BuildEasyRequestArgs(NULL, &es, NULL, &obj->wo_Name);
	    if (win) if (((ULONG)win)!=1L) sigbits|=(1L << win->UserPort->mp_SigBit);
	    Signal(wb->wb_Task,sigbits);	/* Make sure we get aleast 1 */
	    keep_going = TRUE;
	    do
	    { /* wait for user responce or toolexit msg */
		Wait(sigbits);
		while (msg = GETMSG(wbp))
		{
		    switch(((SHORT *)msg)[-1]) {
		    case MTYPE_TOOLEXIT:
			MP(("SDW: got a MTYPE_TOOLEXIT message\n"));
			ExitTool((struct WBStartup *)msg);
			keep_going = FALSE;
			doitagain = FALSE;
			break;
		    case MTYPE_TIMER:
			MP(("SDW: got a MTYPE_TIMER message\n"));
			TimeTick((struct timerequest *)msg);
			break;
		    case MTYPE_APP_FUNC:	/* We got an Add/Del AppXXX */
			DoAppWork(msg);
			break;
		    default:
			Alert(AN_Workbench|AN_WBBadStartupMsg2);
			break;
		    }
		}
		if (keep_going)
		{
			if (((ULONG)win)==1L)
			{	/* Continue from Alert... */
				keep_going=FALSE;
				doitagain=TRUE;
			}
			else if (!win)
			{	/* Cancel from Alert... */
				keep_going=FALSE;
				doitagain=FALSE;
			}
			else do
			{
				doitagain=SysReqHandler(win,NULL,FALSE);
				if (doitagain>=0) keep_going=FALSE;
			} while ((win->UserPort->mp_MsgList.lh_Head->ln_Succ) && (keep_going));
		}
	    } while (keep_going);
	    FreeSysRequest(win); /* remove requester (or NOP if 0 or 1) */
	}
    } while (doitagain);
    MP(("SDW: exit\n"));
}

/*
	Scan the startup drawer.  For each icon found in the drawer,
	priority insert it into a StartupList (the priority is
	specified via a ToolType of STARTPRI; 0 is the default).
	For each icon found in the StartupList,	put it on the
	SelectList and call PotionRunTool which will synchronously
	launch any icon on the select list (there will only be one).
*/
StartupDrawer()
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBObject *obj;
    struct FileInfoBlock *fib;
    struct Node *node;
    char *s;
    char *p;
    BPTR dirlock;
    int priority;
    struct List StartupList;

    /* get mem for fib */
    if (!(fib = ALLOCVEC(sizeof(struct FileInfoBlock), MEMF_PUBLIC))) return(NULL);

    /* get a lock on the startup dir */
    dirlock = LOCK(STARTDRAWER, ACCESS_READ);
    if (dirlock)
    {
	/* set the current dir to the startup dir */
	CURRENTDIR(dirlock);
	/* if we can examine the directory */
	if (EXAMINE(dirlock, fib))
	{
	    NewList(&StartupList);
	    /* while more entries in the directory */
	    while (EXNEXT(dirlock, fib))
	    {
                /* ignore non .info file files */
                if (s = suffix(fib->fib_FileName, InfoSuf))
                {
                    *s = '\0'; /* replace '.' with NULL */
                    if (obj = WBGetWBObject(fib->fib_FileName))
                    {
                        if (p = FindToolType(obj->wo_ToolTypes, "STARTPRI"))
                        {
                            priority=atoi(p);
                            if (priority>127) priority = 127;
                            else if (priority<-128) priority = -128;
                            obj->wo_UtilityNode.ln_Pri = priority;
                        }
                        Enqueue(&StartupList, &obj->wo_UtilityNode);
                    }
                    *s = '.';
                }
	    }

	    while (node = RemHead(&StartupList))
	    { /* while not empty */
		/* calc and assign ptr to object */
		obj = (struct WBObject *)((ULONG)(node) - node->ln_Type);

		obj->wo_Parent = NULL;
		obj->wo_Lock = (long)dirlock;
		obj->wo_Startup = 1;

		/*
		 * Only Projects and Tools...
		 */
		if (CheckForType(obj,WBF_TOOL | WBF_PROJECT))
		{
                    /* put on select list */
                    ADDTAIL(&wb->wb_SelectList, &obj->wo_SelectNode);
                    /* launch select list */
                    PotionRunTool();
                    StartupDrawerWait(obj);
                    /* clear select list */
                    NewList(&wb->wb_SelectList);
                    /* restore current dir */
                    CURRENTDIR(dirlock);
		}
		WBFreeWBObject(obj);
	    }
	}
        UNLOCK(dirlock);
    }

    FREEVEC(fib);

    return(NULL);
}
