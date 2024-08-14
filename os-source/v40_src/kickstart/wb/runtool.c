/*
 * $Id: runtool.c,v 38.3 92/05/30 18:02:59 mks Exp $
 *
 * $Log:	runtool.c,v $
 * Revision 38.3  92/05/30  18:02:59  mks
 * Now calls ThisTask() rather than FindTask(NULL)
 * 
 * Revision 38.2  92/05/30  16:55:29  mks
 * Now uses the GETMSG/PUTMSG stubs
 *
 * Revision 38.1  91/06/24  11:38:19  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/ports.h"
#include "exec/memory.h"
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/dostags.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

#include "startup.h"
#include "errorindex.h"
#include "quotes.h"

struct typedmsg {
    WORD mtype;				/* message type */
    struct WBStartup msg;		/* std wbstartuo message */
};

/* this structure must be an integral number of longwords in size */
struct createrequest {
    WORD mtype;				/* message type */
    struct CreateToolMsg msg;		/* CreateTool struct */
    WORD pad;				/* pad for intergral # of longwords */
};

/*
 * we play a funny game here: we pass the ADDRESS of the arglist pointer,
 * and then bump it in the routine.  That way we do not need a global
 * to track it...
 * We return one on a failure.  That way the list search will stop.
 */
PrepareArg( obj, listp, tool, name )
struct WBObject *obj, *tool;
struct WBArg **listp;
char *name;
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBArg *lp = *listp;
    long lock;
    char *diskname;
    struct ActiveDisk *ad;
    struct NewDD *dd;
    int freelock = 0;
    char *pathend;

    MP(("PrepareArg: (enter) obj=0x%lx, *listp=0x%lx, tool=0x%lx, name='%s', listp=0x%lx\n", obj, *listp, tool, name, listp ));

    /* don't do the tool again */
    if( obj == tool )
    {
	MP(("PrepareArg: returning NULL since obj == tool\n"));
	return( NULL );
    }

    /* if tool is null, the this IS the tool.
     * Assume default tool in progress
     */
    if (tool)
    {
	name = obj->wo_Name;
	MP(("PrepareArg: tool is non-null, name='%s'\n", name));
    }
    else
    {
	/* if tool is null, then this IS the tool, and use the given name */
	MP(("PrepareArg: tool is null, this IS the tool, 'goto forcearg'\n"));
	goto forcearg;
    }

    if (obj->wo_Type == WBAPPICON)
    {
	if (!(lp->wa_Name=scopy(obj->wo_Name))) return(1);
    }
    else if( obj->wo_Type == WBKICK )
    {
	MP(("obj->wo_Type == WBKICK\n"));
	ad = ObjectToActive( obj );
	diskname = ad->ad_Name;
	lp->wa_Name = (char *) ALLOCVEC( strlen( diskname ) + 2, MEMF_PUBLIC );
	if( !lp->wa_Name )
	{
	    MP(("PrepareArg: returning 1 since !lp->wa_Name\n"));
	    return( 1 );
	}
	strcpy( lp->wa_Name, diskname );
	strcat( lp->wa_Name, ":" );
    }
    else if (Drawer_P(obj))
    {
	MP(("PrepareArg: obj '%s' is a drawer\n", obj->wo_Name));
	/* we pass these as a lock on the directory itself with a null name */
	dd = obj->wo_DrawerData;

	if (!dd->dd_Lock)
	{
	    if (!LockDrawer(obj))
	    {
		MP(("PrepareArg: returning 1 since !LockDrawer( obj )\n"));
		return(1);
	    }
	    freelock = 1;
	}

	MP(( "PrepareArg: lock is 0x%lx\n", dd->dd_Lock ));

	lp->wa_Lock=DUPLOCK(dd->dd_Lock);
	MP(("PA:lp->wa_Lock = $%lx ($%lx) (duplock of $%lx)\n",lp->wa_Lock, lp->wa_Lock << 2, dd->dd_Lock));

	if (freelock)
	{
	    MP(("PrepareArg: calling UNLOCK on $%lx ($%lx)\n",dd->dd_Lock, dd->dd_Lock << 2));
	    UNLOCK(dd->dd_Lock);
	    dd->dd_Lock = 0;
	}

	if (!(lp->wa_Name = scopy("")))
	{
	    return( 1 );
	}
    }
    else
    {

forcearg:
	MP(("at forcearg\n"));
	CURRENTDIR(lock=GetParentsLock(obj));
	lp->wa_Lock=NULL;
	lp->wa_Name=NULL;

	strcpy(wb->wb_Buf0,name);
	pathend=PATHPART(wb->wb_Buf0);
	*pathend='\0';

	/* Added the check for ":" to keep AmigaBasic happy... */
	if (pathend!=wb->wb_Buf0)
	{
            pathend=wb->wb_Buf0;
            while (*pathend)
            {
                pathend++;
                if (*pathend==':')
                {
                    if (lp->wa_Lock=LOCK(wb->wb_Buf0,ACCESS_READ))
                    {
                        lock=NULL;
                        name=FILEPART(name);
                    }
                    *pathend='\0';
		}
	    }
	}

	if (lock) lp->wa_Lock=DUPLOCK(lock);

	if (lp->wa_Lock) lp->wa_Name = scopy(name);

	if (!(lp->wa_Name))
	{
	    MP(("PrepareArg: returning 1 since ! lp->wa_Name\n"));
	    return(1);
	}

	MP(("PrepareArg: lock=0x%lx, name='%s'\n", lp->wa_Lock, lp->wa_Name ));
    }

    *listp = lp +1;

    MP(("PrepareArg: lock=0x%lx, name='%s', exit\n",lp->wa_Lock, lp->wa_Name ));
    return(NULL);
}

FindTool(obj)
struct WBObject *obj;
{
    return( (int)(obj->wo_Type == WBTOOL) );
}

FindDefaultTool(obj)
struct WBObject *obj;
{
char *toolname=NULL;

    if (obj->wo_Type == WBPROJECT)
    {
	toolname=obj->wo_DefaultTool;
	if (toolname) if (!(*toolname)) toolname=NULL;
    }
    return((int)toolname);
}

FindStartupIcon(obj)
struct WBObject *obj;
{
    return((int)obj->wo_Startup);
}

void
PotionRunTool()
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBObject *obj, *toolobj;
    struct WBArg *arglist, *argorig;
    struct WorkbenchAppInfo *ai;
    char *name;
    LONG lock;
    int numargs, defaulttool = 0;

    if( obj = SelectSearch( FindTool ) )
    {
	name = obj->wo_Name;
	MP(("PotionRunTool: found tool '%s'\n", name));
	toolobj = obj;
    }
    else
    {
	MP(("PotionRunTool: looking for default tools\n"));
	/* go looking for default tools */
	obj = SelectSearch( FindDefaultTool );
	if (!obj)
	{
	    MP(("PotionRunTool: looking for an app icon\n"));
	    if (ai = (struct WorkbenchAppInfo *)SiblingSuccSearch(wb->wb_AppIconList.lh_Head,IconToAppIcon,SelectSearch(FindAppIcon)))
	    {
		SendAppIconMsg(ai, NULL, 1); /* 1 means app icon got doubleclicked on */
	    }
	    else ErrorTitle(Quote(Q_ICON_HAS_NO_DEF_TOOL));

	    goto err; /* all done */
	}
	name = obj->wo_DefaultTool;
	MP(("PotionRunTool: found default tool '%s'\n", name));
	defaulttool = 1;
	toolobj = (struct WBObject *) -1;
    }

    lock=GetParentsLock(obj);
    if (obj->wo_FakeIcon || FindToolType(obj->wo_ToolTypes, "CLI"))
    {
	MP(("PotionRunTool: obj=$%lx (%s), FAKE_ICON\n",obj, obj ? obj->wo_Name : "NULL"));
	if (lock)
	{
	    DoExecute(obj->wo_Name, lock);
	}
	goto err; /* all done */
    }

    numargs = SizeList( &wb->wb_SelectList ) + defaulttool;

    arglist = argorig = ALLOCVEC( numargs * sizeof( struct WBArg ),  MEMF_CLEAR|MEMF_PUBLIC );
    if( ! arglist ) goto err; /* all done */

    /* process the tool (arg 0) */
    if( PrepareArg( obj, &arglist, NULL, name ) )
    {
exitargs:
	ExitArgs( numargs, argorig );
	goto err; /* all done */
    }

    /* process the other arguments */
    if( SelectSearch( PrepareArg, &arglist, toolobj, NULL ) ) {
	goto exitargs;
    }
    MP(("PotionRunTool: calling CreateTool, name='%s'\n", name));
    CreateTool( obj, name, numargs, argorig);

err:
    MP(("PotionRunTool: exit\n"));
    return;
}

/****************************************************************************/

int StartTool(struct CreateToolMsg *ctm)
{
    struct WorkbenchBase *wb = getWbBase();
    struct MsgPort *port;
    struct typedmsg *tm;
    struct WBStartup *wbmsg;
    struct Process *proc;
    struct TagItem CreateNewProcArray[] =
    {
	NP_Seglist, 0,		/* filled in */
	NP_FreeSeglist, FALSE,	/* do not free seglist when process ends */
	NP_StackSize, 0,	/* filled in */
	NP_Priority, 0,		/* filled in */
	NP_Name, 0,		/* filled in */
	NP_HomeDir, 0,		/* filled in */
	NP_CurrentDir, 0,	/* prevent extra lock on currentdir */
	NP_Input, 0,		/* manx programs play with pr_CIS */
	NP_Output, 0,		/* manx programs play with pr_COS */
	NP_CloseInput, FALSE,	/* do not close input file handle */
	NP_CloseOutput, FALSE,	/* do not close output file handle */
	TAG_DONE, NULL
    };
    int status = 0; /* assume error */

    tm = ctm->ctm_StartupMsg;
    wbmsg = &tm->msg;

    MP(("StartTool: name='%s', segment=%lx, wbmsg=%lx\n",ctm->ctm_Name, wbmsg->sm_Segment, wbmsg ));
    MP(("\tNumArgs=%ld, ArgList=%lx\n",wbmsg->sm_NumArgs, wbmsg->sm_ArgList));

    if (wbmsg->sm_Segment)
    {
	CreateNewProcArray[0].ti_Data = wbmsg->sm_Segment;		/* NP_Seglist */
	CreateNewProcArray[2].ti_Data = ctm->ctm_StackSize;		/* NP_StackSize */
	CreateNewProcArray[3].ti_Data = ctm->ctm_Priority;		/* NP_Priority */
	CreateNewProcArray[4].ti_Data = (long)ctm->ctm_Name;		/* NP_Name */

	CreateNewProcArray[5].ti_Data = ctm->ctm_LoadLock;		/* NP_HomeDir */

	if (proc = CREATENEWPROC(CreateNewProcArray))
	{
	    port = &proc->pr_MsgPort;
	    proc->pr_WindowPtr = NULL;

	    /* add him to the process structure so we can track
	    ** and unload him
	    */
	    wbmsg->sm_Process = port;
	    wbmsg->sm_Segment = wbmsg->sm_Segment;
	    wbmsg->sm_Message.mn_ReplyPort = &wb->wb_WorkbenchPort;

	    PUTMSG(port, (struct Message *)wbmsg);
	    wbmsg = NULL;

	    ctm->ctm_LoadLock=NULL;	/* We started, so this is gone */

	    status = 1; /* no error */
	}
	else
	{
	    ErrorTitle(Quote(Q_NO_MEM_TO_RUN), ctm->ctm_Name );
	    MP(("StartTool: UnloadSeg %lx\n", wbmsg->sm_Segment));

	    UnLoadSeg(wbmsg->sm_Segment);
	}
    }

    /* now free up the ctm structure */
    MP(("StartTool: calling UNLOCK on $%lx ($%lx)\n", ctm->ctm_Lock, ctm->ctm_Lock << 2));
    UNLOCK(ctm->ctm_Lock);	/* UNLOCK checks for NULL */
    UNLOCK(ctm->ctm_LoadLock);

    if (wbmsg)
    {
	ExitArgs(wbmsg->sm_NumArgs, wbmsg->sm_ArgList);
	MP(("StartTool: freeing %ld bytes @$%lx for tm\n",sizeof(struct typedmsg), tm));
	FREEVEC(tm);
    }
    if (ctm) FREEVEC(ctm->ctm_Name);
    if (!status) wb->wb_ToolExitCnt--;
    MP(("StartTool: freeing %ld bytes @$%lx for ctm\n",sizeof(struct createrequest), ((ULONG)ctm) - sizeof(WORD)));
    FREEVEC((void *)(((ULONG)ctm) - sizeof(WORD)));
    MP(("StartTool: exit, returning %ld\n", status));
    return(status);
}

void CreateTool(struct WBObject *obj,char *name,int numargs,struct WBArg *argbase)
{
    struct WorkbenchBase *wb = getWbBase();
    struct typedmsg *msg = NULL;
    struct CreateToolMsg *ctm = NULL;
    struct createrequest *cr = NULL;
    ULONG lock;
    struct Process *proc;
    struct MsgPort *port;
    char *string;

    BeginDiskIO(); /* put up 'zzz' cloud */

    /* get all the things that we need synchronously */
    /*
	This unfortunately is a huge kludge.  A un-initialized disk
	has a lock of 0.  The V1.3 code did not check the lock it just
	used it.  This here I only check the return value from DUPLOCK
	if the original lock was not 0.
    */
    if (lock = GetParentsLock(obj)) {
	lock = DUPLOCK(lock);
	MP(("CreateTool: lock = $%lx ($%lx)\n", lock, lock << 2));
	if (!lock) goto err_outofmem;
    }

    /* get the memory for our messages */
    cr = ALLOCVEC(sizeof(struct createrequest),MEMF_CLEAR|MEMF_PUBLIC);
    MP(("CreateTool: allocated %ld bytes @$%lx for cr\n",sizeof(struct createrequest), cr));
    msg = ALLOCVEC(sizeof(struct typedmsg), MEMF_CLEAR|MEMF_PUBLIC);
    MP(("CreateTool: allocated %ld bytes @$%lx for msg\n",sizeof(struct typedmsg), msg));
    if (!cr || !msg) goto err_outofmem;

    cr->mtype = MTYPE_IOPROC;
    ctm = &cr->msg;
    ctm->ctm_Name = NULL;
    ctm->ctm_Message.mn_ReplyPort = &wb->wb_WorkbenchPort;
    ctm->ctm_Handler = (VOID *)StartTool;
    ctm->ctm_StartupMsg = msg;
    ctm->ctm_Lock = lock;
    ctm->ctm_Cli = Cli();

    if (!(ctm->ctm_Name = scopy(name))) goto err_outofmem;

    if (!(ctm->ctm_StackSize = obj->wo_StackSize))
    {
	ctm->ctm_StackSize = DEFAULT_STACKSIZE;
    }

    ctm->ctm_Priority = DEFAULT_TOOLPRI;
    if ((string = (char *)FindToolType(obj->wo_ToolTypes, "TOOLPRI")))
    {
	ctm->ctm_Priority = atoi(string);
    }

    msg->mtype = MTYPE_TOOLEXIT;
    msg->msg.sm_ArgList = argbase;
    msg->msg.sm_NumArgs = numargs;

    if (!obj->wo_Startup)
    {
	MP(("CreateTool: calling CreateProc\n"));

	port = CREATEPROC("WBL", 5L, MKBADDR(&LoadToolSegment),DEFAULT_STACKSIZE);

	if (!port) goto err_outofmem;
	proc = (struct Process *)((ULONG)port - (ULONG)&((struct Process *)0)->pr_MsgPort);
	proc->pr_WindowPtr = NULL;
	MP(("CreateTool: calling PutMsg\n"));
	wb->wb_ToolExitCnt++;
	PUTMSG(port, &ctm->ctm_Message);
    }
    else
    {
	MP(("CreateTool: calling LoadToolCommon\n"));
	LoadToolCommon(ctm);
	MP(("CreateTool: calling StartTool\n"));
	if (StartTool(ctm))
	{
	    wb->wb_ToolExitCnt++;
	}
    }
    EndDiskIO(); /* remove 'zzz' cloud */
    MP(("CreateTool: exiting\n"));
    return;

err_outofmem:
    NoMem();

    if (ctm) FREEVEC(ctm->ctm_Name);
    FREEVEC(cr);
    FREEVEC(msg);

    UNLOCK(lock);	/* UNLOCK checks for NULL */

    ExitArgs(numargs, argbase);
    EndDiskIO(); /* remove 'zzz' cloud */
    return;
}

void LoadTool()
{
struct Process *proc;
struct CreateToolMsg *ctm;

    MP(("LoadTool: enter, finding ourselves via FindTask(0L)\n"));
    /* get our startup message */
    proc = (struct Process *)ThisTask();
    MP(("LoadTool: waiting on the port\n"));
    WAITPORT(&proc->pr_MsgPort);
    MP(("LoadTool: getting the startup message\n"));
    ctm = (struct CreateToolMsg *)GETMSG(&proc->pr_MsgPort);
    MP(("LoadTool: calling LoadToolCommon\n"));
    LoadToolCommon(ctm); /* throw away return code */
    MP(("LoadTool: back from LTC, calling ReplyMsg\n"));
    /* ask wb to start him (only if he loaded, StartTool DOES check this) */
    REPLYMSG((struct Message *)ctm);
    MP(("LoadTool: exit\n"));
}

/*
    Try to find a program to be able to loadseg.
    Attempt this for each node on the directory.
*/
ULONG PathLoadSeg(struct CreateToolMsg *ctm)
{
struct Process *pr = (struct Process *)ThisTask();
ULONG *path = (ULONG *)BADDR(ctm->ctm_Cli->cli_CommandDir);
APTR oldwindowptr;
BPTR oldlock;
ULONG segment;

    MP(("PathLoadSeg: enter\n"));

    oldlock=CURRENTDIR(ctm->ctm_Lock);

    /* the path is a linked list of BPTR's, where the second element
    ** is a lock on the directory.  Make sure we don't ask for each
    ** disk on the path...
    */
    segment=LoadSeg(ctm->ctm_Name);
    MP(("segment=$%lx, disabling DOS requesters\n", segment));

    oldwindowptr = pr->pr_WindowPtr;
    pr->pr_WindowPtr = (APTR)-1;

    while (!segment && path)
    {
	/* if out of memory, give up... */
	if (IOERR() == ERROR_NO_FREE_STORE) break;
	MP(("PLS: cd'ing to $%lx\n", path[1]));
	CURRENTDIR(path[1]);
	MP(("PLS: calling LoadSeg(%s)\n", ctm->ctm_Name));
	segment = LoadSeg(ctm->ctm_Name);
	path = (ULONG *)BADDR(path[0]);
	MP(("PLS: segment=$%lx, path=$%lx\n", segment, path));
    }

    MP(("enabling DOS requesters\n"));
    pr->pr_WindowPtr = oldwindowptr;

    /*
     * Get the lock that will be the home directory
     */
    if (segment)
    {
    BPTR tmplock=NULL;
    BPTR tmp;

        if (tmp=LOCK(ctm->ctm_Name,ACCESS_READ))
        {
            tmplock=PARENTDIR(tmp);
        }
        UNLOCK(tmp);

	/* Pick up the lock for the program... */
        ctm->ctm_LoadLock=tmplock;
    }

    CURRENTDIR(oldlock);

    MP(("PathLoadSeg: exit, segment=$%lx\n", segment));
    return(segment);
}

int LoadToolCommon(struct CreateToolMsg *ctm)
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBStartup *wbmsg;
    ULONG IDCMPFlags = DISKINSERTED;
    struct EasyStruct es =
    {
	sizeof(struct EasyStruct),
	NULL,
	NULL,
	NULL,	/* filled in */
	NULL,	/* filled in */
    };
    int status = 0; /* assume failure */

    MessageTitle(Quote(Q_LOADING), ctm->ctm_Name); /* inform user */

    wbmsg = &ctm->ctm_StartupMsg->msg;
    MP(("LTC: enter, ctm=$%lx, wbmsg=$%lx\n", ctm, wbmsg));
    for (;;)
    {
	MP(("LTC: calling PathLoadSeg for '%s'\n", ctm->ctm_Name));
	wbmsg->sm_Segment = PathLoadSeg(ctm);

	/* if we got the segment then we're done */
	if (wbmsg->sm_Segment)
	{
	    MP(("LTC: segment = $%lx, exiting\n", wbmsg->sm_Segment));
	    status = 1; /* success */
	    break;
	}

	es.es_GadgetFormat = Quote(Q_RETRY_CANCEL_TEXT);
	if (IOERR() == ERROR_NO_FREE_STORE)
	{
	    es.es_TextFormat = Quote(Q_NO_MEM_TO_RUN);
	}
	else
	{
	    /* ZZZ: we should give him a chance to change it */
	    es.es_TextFormat = Quote(Q_CANT_OPEN_TOOL);
	}
	if (!EasyRequestArgs(wb->wb_BackWindow, &es, &IDCMPFlags, &ctm->ctm_Name))
	{
	    MP(("LTC: user hit CANCEL, segment = $%lx, exiting\n", wbmsg->sm_Segment));
	    break;
	}
    }
    MessageTitle(wb->wb_ScreenTitle); /* clear title */
    MP(("LTC: exit, returning %ld\n", status));
    return(status);
}

void ExitTool(msg)
struct WBStartup *msg;
{
    struct WorkbenchBase *wb = getWbBase();
    struct typedmsg *tm;

    MP(( "ExitTool: process 0x%lx has segment 0x%lx msg 0x%lx. %ld free\n",
	msg->sm_Process, msg->sm_Segment, msg, AvailMem( 0 ) ));
    MP(("ExitTool: UnloadSeg %lx\n", msg->sm_Segment));

    wb->wb_ToolExitCnt--;

    UnLoadSeg(msg->sm_Segment);

    ExitArgs(msg->sm_NumArgs, msg->sm_ArgList);

    tm = (struct typedmsg *)(((WORD *)msg)-1);

    MP(("ExitTool: freeing %ld bytes @$%lx for tm\n",
	sizeof(struct typedmsg), tm));
    FREEVEC(tm);
    MP(("ExitTool: exit\n"));
}

void ExitArgs(wc, waorig)
int wc;
struct WBArg *waorig;
{
    int i;
    struct WBArg *wa = waorig;

    MP(("ExitArgs: enter, wc=%ld, waorig=%lx\n", wc, waorig));
    if (wa) {
	for (i = 0; i < wc; i++, wa++)
	{
	    MP(("EA: calling UNLOCK on $%lx ($%lx)\n", wa->wa_Lock, wa->wa_Lock << 2));
	    UNLOCK(wa->wa_Lock);	/* UNLOCK checks for NULL */

	    MP(("EA: calling sfree on $%lx (%s)\n", wa->wa_Name, wa->wa_Name));
	    FREEVEC(wa->wa_Name);
	}
	MP(("EA: freeing %ld bytes @$%lx\n", wc * sizeof(struct WBArg), waorig));
	FREEVEC(waorig);
    }
    MP(("ExitArgs: exit\n"));
}
