/*
 * $Id: objectio.c,v 38.4 92/05/30 16:55:09 mks Exp $
 *
 * $Log:	objectio.c,v $
 * Revision 38.4  92/05/30  16:55:09  mks
 * Now uses the GETMSG/PUTMSG stubs
 * 
 * Revision 38.3  92/04/27  14:23:32  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 *
 * Revision 38.2  92/04/14  13:20:45  mks
 * Really made this thing smaller using the new intuition stuff
 *
 * Revision 38.1  91/06/24  11:37:35  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/ports.h"
#include "intuition/intuition.h"
#include "libraries/dosextens.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

PutObject(struct WBObject *obj)
{
char *name;
LONG lock, savex, savey;
struct Gadget SaveGadget;
int result = NULL, saveflag = NULL;
BYTE dio=WbBASE->wb_DiskIONestCnt;

    DP(("PutObject: enter, obj=$%lx (%s), parent=$%lx (%s)\n",
	obj, obj->wo_Name, obj->wo_Parent,
	obj->wo_Parent ? obj->wo_Parent->wo_Name : "NULL"));

    if (!dio) BeginDiskIO();

    if( lock = GetParentsLock( obj ) )
    {
	if( obj->wo_Type == WBDISK ) name = DiskIconName;
	else name = obj->wo_Name;

	CURRENTDIR( lock );

	DP(("PutObject: GadgetRender=$%lx, IOGadgetRender=$%lx, NameImage=$%lx\n",
		obj->wo_Gadget.GadgetRender, obj->wo_IOGadget.GadgetRender,
		&obj->wo_NameImage));
	/* if object is in viewbytext mode, temp. switch to viewbyicon mode */
	if (obj->wo_Gadget.GadgetRender == (APTR)&obj->wo_NameImage) {
		DP(("PutObject: saving and switching\n"));
		/* save text posn */
		savex = obj->wo_CurrentX;
		savey = obj->wo_CurrentY;
		/* switch to icon posn */
		obj->wo_CurrentX = obj->wo_SaveX;
		obj->wo_CurrentY = obj->wo_SaveY;
		SaveGadget = obj->wo_Gadget; /* save text imagery */
		obj->wo_Gadget = obj->wo_IOGadget; /* install graphic imagery */
		saveflag = 1;
	}

	DP(("PO: calling PWBO, name=%s, obj=$%lx (%s)\n",name, obj, obj->wo_Name));
	result = WBPutWBObject( name, obj );

	if (saveflag) {
		DP(("PutObject: restoring\n"));
		/* restore text posn */
		obj->wo_CurrentX = savex;
		obj->wo_CurrentY = savey;
		obj->wo_Gadget = SaveGadget; /* restore text imagery */
	}

    }
    if (!dio) EndDiskIO();
    DP(("PutObject: exit, result = %ld, IOERR()=%ld\n", result, IOERR()));
    return( result );
}

GetParentsLock( obj )
struct WBObject *obj;
{
#ifdef DEBUGGING
    struct WorkbenchBase *wb = getWbBase(); /* Added for calls */
    struct Process *pr = (struct Process *)(wb->wb_Task);
#endif DEBUGGING
    struct WBObject *parent;

    MP(("GetParentsLock: enter, obj=$%lx (%s), Type=$%lx\n",
	obj, obj ? obj->wo_Name : "NULL", obj->wo_Type));
    MP(("\twb_Task=$%lx, FindTask(0)=$%lx, pr_WindowPtr=$%lx\n",
	wb->wb_Task, FindTask(0), pr->pr_WindowPtr));

    if (!obj) {
	goto nolock;
    }
    if (obj->wo_Type == WBKICK) {
	goto nolock;
    }
    if (obj->wo_Type == WBAPPICON) {
	goto nolock;
    }

    parent = obj->wo_Parent;

    MP(("GetLock: obj->wo_Lock=$%lx, parent=$%lx (%s), parent->wo_Lock=$%lx\n",
	obj->wo_Lock, parent, parent ? parent->wo_Name : "NULL",
	parent ? parent->wo_Lock : 0));

    if (obj->wo_Lock) {
#ifdef DEBUGGING
	if(!obj->wo_Background && parent) {
	    MP(("GetLock: Error? object has a lock but does not have its background flag set\n"));
	    MP(("         and has a non null parent ptr\n"));
	    MP(("obj=%lx (%s), lock=%lx, parent=%lx (%s), lock=%lx, root=%lx\n",
		obj, obj->wo_Name, obj->wo_Lock, parent, parent->wo_Name, parent->wo_Lock, wb->wb_RootObject));
	    MasterSearch(PrintObj);
	    Debug(0L);
	}
	MP(("GetLock: exit, using obj->wo_Lock for '%s' (BACKGROUND?)\n", obj->wo_Name));
#endif DEBUGGING
	return(obj->wo_Lock);
    }

    if (obj->wo_Type == WBDISK) {
#ifdef DEBUGGING
	struct NewDD *dd = obj->wo_DrawerData;
	if (dd == NULL) {
	    MP(("GetLock: obj '%s' is a disk but has a null DrawerData\n",
		obj->wo_Name));
	    MasterSearch(PrintObj);
	    Debug(0L);
	}
#endif DEBUGGING
	MP(("GetLock: returning MakeDiskLock(obj)\n"));
	return(MakeDiskLock(obj));
    }

    if (parent) if (parent->wo_DrawerData) {
#ifdef DEBUGGING
	if (parent->wo_DrawerData == NULL ||
	    parent->wo_DrawerData->dd_Lock == NULL) {
	    MP(("GetLock: no obj wo_Lock and null parent DD or null parent DD lock\n"));
	    MP(("\tparent='%s', parent DD=%lx, parent DDlock=%lx, root=%lx\n",
		parent->wo_Name, parent->wo_DrawerData, parent->wo_DrawerData->dd_Lock));
	    MasterSearch(PrintObj);
	    Debug(0L);
	}
#endif DEBUGGING
	MP(("GetLock: DrawerData=$%lx (@$%lx)\n",
	    parent->wo_DrawerData, &parent->wo_DrawerData));
	MP(("GetLock: DrawerData->dd_Lock=$%lx (@$%lx)\n",
	    parent->wo_DrawerData->dd_Lock, &parent->wo_DrawerData->dd_Lock));
	return(parent->wo_DrawerData->dd_Lock);
    }

    /* no lock for this baby... */
nolock:
    MP(("GetLock: exit, returning NULL\n"));
    return(NULL);
}


SetWbPointer(struct WBObject *obj, struct TagItem *tags)
{
struct NewDD *dd;
struct Window *win;

    if( dd = obj->wo_DrawerData )
    {
	if( win = dd->dd_DrawerWin )
	{
		SetWindowPointerA(win,(struct TagItem *)tags);
	}
    }
    return( NULL );
}

void SelectBracketDiskIO(func, arg1)
int (*func)();
ULONG arg1;
{
    BeginDiskIO();
    SelectSearch(func, arg1);
    EndDiskIO();
}

/* we're about to do a long operation: don't listen to the user */
void BeginDiskIO()
{
struct WorkbenchBase *wb = getWbBase();
ULONG tags[]={WA_BusyPointer,TRUE,WA_PointerDelay,TRUE,NULL};

    MP(("BeginDiskIO: enter, count=%ld\n", wb->wb_DiskIONestCnt));
    if (wb->wb_DiskIONestCnt++ == 0) {
	MasterSearch(SetWbPointer,tags);
	MasterSearch(WindowOperate, ClearMenuStrip);
	wb->wb_LastCloseWindow=NULL;	/* Clear the LastCloseWindow pointer */
    }
    CheckDiskIO();
    MP(("BeginDiskIO: exit, count=%ld\n", wb->wb_DiskIONestCnt));
}


/* we're in the middle of a long operation: process refresh events, and
** user aborts, discard "illegal" messages, and postpone everything
** else...
*/
void CheckDiskIO()
{
    struct WorkbenchBase *wb = getWbBase();
    struct IntuiMessage *msg;

    while (msg = (struct IntuiMessage *)GETMSG(&wb->wb_IntuiPort)) {
	StripMessage(msg);
    }
}


/* we're done with long operations: give the workbench back */
void EndDiskIO()
{
    struct WorkbenchBase *wb = getWbBase();

    MP(("EndDiskIO: enter, count=%ld\n", wb->wb_DiskIONestCnt));

#ifdef	DEBUGGING
    ASSERT(wb->wb_DiskIONestCnt > 0);
#endif	/* DEBUGGING */

    CheckDiskIO();
    if (--wb->wb_DiskIONestCnt == 0) {
	MasterSearch(WindowOperate, SetMenuStrip, wb->wb_MenuStrip);
	MasterSearch(SetWbPointer,NULL);
	RethinkMenus();
    }
    MP(("EndDiskIO: exit, count=%ld\n", wb->wb_DiskIONestCnt));
}


void
StripMessage(msg)
struct IntuiMessage *msg;
{
    struct WorkbenchBase *wb = getWbBase();
    struct List *lh = &wb->wb_SeenPort.mp_MsgList;

    switch(msg->Class) {
    case MOUSEBUTTONS:
	if (msg->Code != SELECTUP) goto trashed;
    case NEWSIZE:
    case REFRESHWINDOW:
    case GADGETUP:
    case GADGETDOWN:
    case INTUITICKS:
#ifdef DEBUGGING
	MP(("StripMessage: processing class %lx\n", msg->Class));
	PrintIDCMPFlags(msg->Class);
#endif DEBUGGING
	IncomingEvent(msg);
	break;

    case DISKINSERTED:
    case DISKREMOVED:
    case NEWPREFS:
    case WBENCHMESSAGE:
    case ACTIVEWINDOW:
#ifdef DEBUGGING
	MP(("StripMessage: preserving class %lx\n", msg->Class));
	PrintIDCMPFlags(msg->Class);
#endif DEBUGGING
	ADDTAIL(lh, (struct Node *)msg);
	break;

    case CLOSEWINDOW:
	wb->wb_LastCloseWindow=msg->IDCMPWindow;
	/* Continue into the default case... */

trashed:
    default:
/*
	Note: this handles...
	MOUSEBUTTONS (that are not SELECTUP)
	SIZEVERIFY
	MOUSEMOVE
	REQSET
	MENUPICK
	RAWKEY
	REQVERIFY
	REQCLEAR
	MENUVERIFY
	INACTIVEWINDOW
	DELTAMOVE
	VANILLAKEY
*/
	wb->wb_InputTrashed = 1;
#ifdef DEBUGGING
	MP(("StripMessage: trashing class %lx\n", msg->Class));
	PrintIDCMPFlags(msg->Class);
#endif DEBUGGING
	REPLYMSG((struct Message *)msg);
	break;
    }
}


MakeDiskLock( disk )
struct WBObject *disk;
{
#ifdef LATTICE_ML_WORKAROUND
    struct WorkbenchBase *wb = getWbBase(); /* Added for calls */
#endif LATTICE_ML_WORKAROUND
    struct ActiveDisk *ad;
    struct NewDD *dd;
    LONG lock;

#ifdef DEBUGGING
    if (disk->wo_Type != WBDISK) {
	MP(("MakeDiskLock: non-disk object 0x%lx\n", disk));
	Debug(0L);
    }
#endif DEBUGGING

    dd = disk->wo_DrawerData;

    if( lock = dd->dd_Lock ) return( lock );

    /* find the active disk structure */
    ad = ObjectToActive( disk );

    MP(("MakeDiskLock: calling ManufactureDiskLock\n"));
    lock = ManufactureDiskLock( ad );

    dd->dd_Lock = lock;

    return( lock );
}
