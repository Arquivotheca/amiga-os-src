/*
 * $Id: matrix.c,v 38.3 92/06/11 09:25:55 mks Exp $
 *
 * $Log:	matrix.c,v $
 * Revision 38.3  92/06/11  09:25:55  mks
 * Cleaned up some unneeded code
 * 
 * Revision 38.2  92/04/27  14:22:38  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 *
 * Revision 38.1  91/06/24  11:37:13  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "intuition/intuition.h"
#include "libraries/dosextens.h"
#include "libraries/filehandler.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "startup.h"
#include "macros.h"
#include "quotes.h"

struct DragData {
    SHORT		dd_DeltaX;	/* within window delta */
    SHORT		dd_DeltaY;
    LONG		dd_WinDeltaX;	/* window relative delta */
    LONG		dd_WinDeltaY;
    struct WBObject	*dd_HitObj;
    struct WBObject	*dd_NewDrawer;
    struct Window 	*dd_NewWin;
};

void Matrix(msg)
struct IntuiMessage *msg;
/* ...
 * Then we take appropriate action based on that collision
 */
{
    struct WorkbenchBase *wb = getWbBase();
    struct Window *oldwin, *newwin;
    struct WBObject *obj, *newdrawer, *hitobj;
    struct NewDD *dd;
    struct WorkbenchAppInfo *aw;
    struct WorkbenchAppInfo *ai;
    LONG x, y;
    SHORT newx, newy, deltax, deltay;
    struct DragData dragdata;

    newx = msg->MouseX;
    newy = msg->MouseY;
    oldwin = msg->IDCMPWindow;

    deltax = dragdata.dd_DeltaX = newx - wb->wb_XOffset;
    deltay = dragdata.dd_DeltaY = newy - wb->wb_YOffset;


    if (wb->wb_InputTrashed) {
	wb->wb_InputTrashed = 0;
	return;
    }

    if (!(obj = LastSelected())) return;

    if (wb->wb_DoubleClick) return; /* we NEVER move on a double click */

    /* see if we moved enough to matter */
    if (abs(deltax) >= 10 || abs(deltay) >= 5) goto dragging;

    SubTime((struct timeval *)&msg->Seconds, &wb->wb_Tick);
    /* he has not held it down long enough... */
    if (msg->Seconds == 0 && msg->Micros <= 300000) return;

    if (!deltax && !deltay) return; /* it did not move */

dragging:
    wb->wb_Tick.tv_secs = 0; /* prevent double clicks */

    /* find which Window we're hitting */
    newx += oldwin->LeftEdge;
    newy += oldwin->TopEdge;

    newwin = WhichWindow(newx, newy);

    dragdata.dd_NewWin = newwin;

    /* see if the new window is one of ours... */
    if ((newdrawer = WindowToObj(newwin)) == NULL) {
	MP(("Matrix: newwin=%lx (%s)\n", newwin, newwin->Title));
	/* see if the new window is a drop window */
	if ((aw = (struct WorkbenchAppInfo *)SiblingSuccSearch(wb->wb_AppWindowList.lh_Head,
	    WindowToAppWindow, newwin)) == NULL) {
	    ErrorTitle(Quote(Q_ICON_CANT_B_IN_WIN));
	    return;
	}
	MP(("matrix: calling SendAppWindowMsg, aw=%06lx, msg=%06lx\n",
		aw, msg));
	SendAppWindowMsg(aw, msg);
	return;
    }

    dragdata.dd_NewDrawer = newdrawer;
    dd = newdrawer->wo_DrawerData;

    /* temporary variables */
    x = dd->dd_CurrentX - newwin->BorderLeft - newwin->LeftEdge;
    y = dd->dd_CurrentY - newwin->BorderTop - newwin->TopEdge;

    /* new window coordinates of where selected gadget was be dropped */
    dragdata.dd_WinDeltaX = deltax + x;
    dragdata.dd_WinDeltaY = deltay + y;

    /* figure out if the user droped the icon on something... */

    /* we are not interested in a hit of this object, so run succ until
     * we either find no one or someone else
     */
    msg->MouseX = newx - newwin->LeftEdge;
    msg->MouseY = newy - newwin->TopEdge;
    msg->IDCMPWindow = newwin;
    if (hitobj = TranslateSelect(msg, 1))
    {
	MP(("martix: (maybe) hitobj=$%lx (%s)\n", hitobj, hitobj->wo_Name));
	/* lets see if it means something */
	if (CheckForNotType(hitobj,ISDISKLIKE|ISDRAWER))
	{
	    if (ai = (struct WorkbenchAppInfo *)SiblingSuccSearch(wb->wb_AppIconList.lh_Head,IconToAppIcon, hitobj))
	    {
		MP(("matrix: hitobj was an AppIcon, sending message\n"));
		/* 0 means app icon got an icon dropped on it */
		SendAppIconMsg(ai, NULL, 0);
		goto end;
	    }
	    else
	    {
		MP(("matrix: hitobj was not disk/drawer type, hitobj = NULL!\n"));
		/* only drawer-like or disklike objects can get hit upon */
		hitobj = NULL;
	    }
	}
    }
#ifdef DEBUGGING
    else {
	MP(("matrix: hitobj NULL\n"));
    }
#endif DEBUGGING
    dragdata.dd_HitObj = hitobj;
    SiblingPredSearch(wb->wb_SelectList.lh_TailPred, DragObject, &dragdata);
end:
    return;
}

#define	ID_BUSY_DISK	(((long)'B'<<24) | ((long)'U'<<16) | ('S'<<8) | ('Y'))	/* BUSY */

void WBFormat(struct WBObject *disk)
{
struct ActiveDisk *ad;
struct InfoData *id;
struct WBArg *arglist, *argsave;
ULONG type;
int numargs = 2;
BPTR lock;

    BeginDiskIO();
    lock=DUPLOCK(GetParentsLock(disk));

    MP(("WBFormat: enter, disk=%lx (%s)\n",disk, disk->wo_Name, disk->wo_Name));

    ad = ObjectToActive(disk);
    MP(("WBFormat: ad=$%lx\n", ad));
    if (ad) {
	id = &ad->ad_Info;
	type = (ULONG)id->id_DiskType;
	MP(("WBFormat: type=$%lx, BUSY=$%lx\n", type, ID_BUSY_DISK));
	if (type == ID_BUSY_DISK) {
	    ErrorTitle(Quote(Q_CANNOT_FORMAT));
	    goto end;
	}
    }

    arglist = argsave = (struct WBArg *)
	ALLOCVEC(numargs * sizeof(struct WBArg), MEMF_CLEAR|MEMF_PUBLIC);
    if (!arglist) {
	MP(("WBFormat: could not allocate %ld bytes for arglist\n",
	    numargs * sizeof(struct WBArg)));
	NoMem();
	goto end;
    }
    if (PrepareArg(disk, &arglist, NULL, SystemFormatName) ||
	PrepareArg(disk, &arglist, (struct WBObject *)-1, NULL)) {
	MP(("WBFormat: unable to PrepareArgs!\n"));
	ExitArgs(numargs, argsave);
	goto end;
    }
    CreateTool(disk, SystemFormatName, numargs, argsave);
end:
    UNLOCK(lock);
    EndDiskIO();
    MP(("WBFormat: exit\n"));
}

#define ID_DOSx_DISK		(ID_DOS_DISK & 0xffffff00)

void DiskCopy(struct WBObject *fromdisk, struct WBObject *todisk)
{
char *name;
struct WBArg *arglist, *argsave;
struct ActiveDisk *ad;
struct InfoData *id;
ULONG type;
int numargs=2;
BPTR lock1;
BPTR lock2=NULL;

    BeginDiskIO();

    lock1=DUPLOCK(GetParentsLock(fromdisk));
    if (todisk)
    {
	lock2=DUPLOCK(GetParentsLock(todisk));
	numargs++;
    }

    name = fromdisk->wo_DefaultTool;
    if (!name || !(*name)) name = SystemDiskCopyName;

    MP(("DiskCopy: enter, fromdisk=%lx (%s), todisk=%lx (%s), name='%s'\n",fromdisk, fromdisk->wo_Name, todisk, todisk->wo_Name, name));

    if (ad = ObjectToActive(fromdisk))
    {
        id = &ad->ad_Info;
        type = (ULONG)id->id_DiskType;

        if (((type & 0xffffff00)!=ID_DOSx_DISK) && (type !=ID_KICKSTART_DISK))
        {
            ErrorTitle(Quote(Q_CANT_DISKCOPY_SRC));
            goto end;
        }

        MP(("DiskCopy: numargs=%ld, name=$%lx (%s), *name=$%lx\n",numargs, name, name, *name));

        arglist = argsave = (struct WBArg *)ALLOCVEC(numargs * sizeof(struct WBArg), MEMF_CLEAR|MEMF_PUBLIC);

        if (!arglist)
        {
            NoMem();
            goto end;
        }
        if (PrepareArg(fromdisk, &arglist, NULL, name) ||
            PrepareArg(fromdisk, &arglist, (struct WBObject *)-1, NULL)) {
exitargs:
            /* something went wrong.  Don't even bother */
            ExitArgs(numargs, argsave);
            goto end;
        }
        if (todisk && PrepareArg(todisk, &arglist, (struct WBObject *)-1, NULL)) {
            goto exitargs;
        }

        MP(("DiskCopy: fromdisk=(%s) name=(%s) numargs=%ld\n",fromdisk->wo_Name,name,numargs));
        CreateTool(fromdisk, name, numargs, argsave);
    }

end:
    UNLOCK(lock1);
    UNLOCK(lock2);
    EndDiskIO();
    MP(("DiskCopy: exit\n"));
}

long BackgroundDir(struct WBObject *obj, BPTR fl)
{
long result=FALSE;
struct NewDD *dd;

    if (dd=obj->wo_DrawerData)
    {
	if (SAMELOCK(dd->dd_Lock,fl)==LOCK_SAME) result=TRUE;
    }
    return(result);
}

void ObjMove(obj, olddrawer, newdrawer, curx, cury)
struct WBObject *obj, *olddrawer, *newdrawer;
LONG curx, cury;
{
    struct WorkbenchBase *wb = getWbBase();
    struct NewDD *newdd = newdrawer->wo_DrawerData;
    int freelock = 0;
    int outofbackground = 0;
    int result;
    struct WBObject *newobj;

    MP(("ObjMove: obj=%lx (%s), olddrawer=%lx (%s), newdrawer=%lx (%s)\n",
	obj, obj->wo_Name, olddrawer, olddrawer->wo_Name, newdrawer,
	newdrawer->wo_Name));

    BeginDiskIO();

    if (newdrawer != wb->wb_RootObject && !newdd->dd_Lock) {
	if (!LockDrawer(newdrawer)) {
	    MP(("ObjMove: error, could not LockDrawer on %lx (%s)\n",
		newdrawer, newdrawer->wo_Name));
	    goto err;
	}
	freelock = 1;
    }

    /* check for movement from the backdrop */
    if (obj->wo_Background)
    {
	MP(("ObjMove: obj %lx (%s) [%lx] is in background\n", obj, obj->wo_Name, obj->wo_Lock));
	if (newdrawer == (struct WBObject *) MasterSearch(BackgroundDir, obj->wo_Lock))
	{
	    /* we are moving out of the backdrop */
	    MP(("ObjMove: restoring from background\n"));
	    outofbackground = 1;
	}
    }

    /* tell the file system to move the file */
    MP(("ObjMove: calling WBMove\n"));
    result = WBMove(obj, olddrawer, newdrawer, curx, cury);
    MP(("ObjMove: ok, result=$%lx\n", result));
    if (!result) goto err;

    if (result > 0) {
	/* this was really a move */
postmove:
	/* remove the object from the old window */
	REMOVE(&obj->wo_Siblings);

	MP(("ObjMove: decrementing %s's UseCount to %ld.\n",obj->wo_Name, obj->wo_UseCount - 1));

	obj->wo_UseCount--;
	olddrawer->wo_UseCount--;
	RefreshDrawer(olddrawer);

	MP(("ObjMove: removed object %s from old drawer\n", obj->wo_Name));

	/* set the current virtual position.  This is figured by the
	 * new position in the physical window plus the virtual window
	 * offset, with a fudge factor (save - oldgadget origin).  This
	 * fudge factor takes into account where on the gadget the
	 * user originally selected, so we keep the same relative
	 * position in relation to the pointer.
	 */
	MP(("ObjMove: obj %s moved to (%ld,%ld)\n",obj->wo_Name, curx, cury));

	PostObjMove(newdrawer, obj, curx, cury, outofbackground);
    }
    else {
	/* we had really done a copy */
	MP(("ObjMove: really a copy\n"));
	CURRENTDIR(newdd->dd_Lock);
	if (newobj = WBGetWBObject(obj->wo_Name)) {
	    MP(("ObjMove: newobj = %lx (%s)\n", newobj, newobj->wo_Name));
	    PostObjMove(newdrawer, newobj, curx, cury, outofbackground);
	}
    }

    /*
     * Make sure object still exists!
     */
    if (MasterSearch(ObjOnList,obj)) if (outofbackground && obj->wo_Parent)
    {
	MP(("ObjMove: about to free object's lock %lx (%lx)\n",obj->wo_Lock, BADDR(obj->wo_Lock)));
	UNLOCK(obj->wo_Lock);
	MP(("ObjMove: after freeing object's lock %lx (%lx)\n",obj->wo_Lock, BADDR(obj->wo_Lock)));
	obj->wo_Lock = 0;
	obj->wo_Background = 0;
    }
err:
    if (freelock)
    {
	UNLOCK(newdd->dd_Lock);
	newdd->dd_Lock = 0;
    }
    MP(("ObjMove: calling EndDiskIO\n"));
    EndDiskIO();
    MP(("ObjMove: exit\n"));
}


void PostObjMove(drawer, obj, newx, newy, existing)
struct WBObject *drawer, *obj;
LONG newx, newy;
int existing;
{
struct NewDD *dd = drawer->wo_DrawerData;

    existing=(!(obj->wo_Background) && !(existing));

    if (dd->dd_DrawerWin || drawer->wo_DrawerSilent)
    {
        obj->wo_CurrentX = newx;
        obj->wo_CurrentY = newy;
        AddIcon(obj, drawer);
        if (dd->dd_DrawerWin)
        {
            MinMaxDrawer(drawer);
            if (dd->dd_ViewModes > DDVM_BYICON) RefreshDrawer(drawer);
	}

        if (existing) PutObject(obj);

	/*
	 * Make sure that we do not put a fake icon into a window that is
	 * not supposed to have it.
	 *
	 * However, if the icon is on the backdrop, we still let it happen
	 * even though the backdrop does not handle fake icons normally
	 */
        if ((!(dd->dd_Flags & DDFLAGS_SHOWALL)) && (obj->wo_FakeIcon) && (!(obj->wo_Background))) FullRemoveObject(obj);
    }
    else
    {
        /* turn this guy into an orphan */
        obj->wo_CurrentX = NO_ICON_POSITION;
        obj->wo_CurrentY = NO_ICON_POSITION;
        UNLOCK(obj->wo_Lock);
        obj->wo_Lock = DUPLOCK(dd->dd_Lock);
        obj->wo_Parent = NULL; /* object has no parent */
        obj->wo_IconWin = NULL; /* object has no window */
        if (obj->wo_Selected)
        {
            obj->wo_Selected = 0;
            REMOVE(&obj->wo_SelectNode);
        }

        if (existing) PutObject(obj);

        if (!(dd=obj->wo_DrawerData) || !(dd->dd_DrawerWin || obj->wo_DrawerSilent)) FullRemoveObject(obj);
    }
}

BOOL SameVolume(struct WBObject *obj1, struct WBObject *obj2)
{
long result;

    MP(("SV: obj1=%lx (%s), obj2=%lx (%s)\n",obj1, obj1->wo_Name, obj2, obj2->wo_Name));

    result=SAMELOCK(GetParentsLock(obj1),GetParentsLock(obj2));
    return((result==LOCK_SAME) || (result==LOCK_SAME_HANDLER));
}

DragObject(obj, dd)
struct WBObject *obj;
struct DragData *dd;
{
    struct Window *oldwin;
    struct WBObject *olddrawer;
    struct WBObject *hitobj = dd->dd_HitObj;
    struct NewDD *newdd;

    olddrawer = obj->wo_Parent;
    oldwin = obj->wo_IconWin;

    MP(("DragObject: enter\n"));
    MP(("\tobj %lx (%s)\n", obj, obj->wo_Name));
    MP(("\toldwin %lx (%s)\n", oldwin, oldwin->Title));
    MP(("\tolddrawer %lx (%s)\n", olddrawer, olddrawer->wo_Name));
    MP(("\thitobj %lx (%s)\n",dd->dd_HitObj, dd->dd_HitObj ? dd->dd_HitObj->wo_Name : "NULL"));
    MP(("\tNewWin %lx (%s)\n",dd->dd_NewWin, dd->dd_NewWin->Title));
    MP(("\tNewDrawer %lx (%s)\n", dd->dd_NewDrawer, dd->dd_NewDrawer->wo_Name));
#ifdef DEBUGGING
    if (!olddrawer || !oldwin) {
	/* hmmm -- the old window is not ours??? */
	MP(("DragObject:  oldwin %lx (%s) has no object\n",
	    oldwin, oldwin->Title));
	Debug(0L);
	return(0);
    }
#endif DEBUGGING

    /* see if we switched windows */
    if (oldwin != dd->dd_NewWin)
    {
	/* if not allowed to switch windows */
	if (CheckForNotType(obj,WINDOWCHANGE))
	{
	    /* if not a disk, you are out of luck buddy */
	    if (obj->wo_Type != WBDISK) {
		ErrorTitle(Quote(Q_CANT_MOV_OUT_OF_WIN),obj->wo_Name );
		return(0);
	    }
	    /* cannot copy a disk to itself, only to another volume */
	    else if (SameVolume(obj, dd->dd_NewDrawer)) {
		/* this object type not allowed to switch windows */
		ErrorTitle(Quote(Q_CANT_COPY_TO_ITSELF), obj->wo_Name);
		return(0);
	    }

	    /* copying a disk to a dir at this point */
	    MP(("DragObject: copying a disk to a dir, calling ObjMove\n"));
	    goto movetonewwindow;
	}
    }

    if ((hitobj) && (obj->wo_Type!=WBGARBAGE))
    {
	MP(("DragObject: %lx (%s) hit object %lx (%s)\n",obj, obj->wo_Name, hitobj, hitobj->wo_Name));

	if (obj->wo_Type == WBAPPICON)
	{
	    ErrorTitle(Quote(Q_CANT_COPY),obj->wo_Name);
	}
	else if (CheckForType(obj,ISDISKLIKE)) {
	    MP(("DragObject: from obj is a disk\n"));
	    if (CheckForNotType(hitobj,ISDISKLIKE)) goto moveobject;
	    /* Let DiskCopy check the disks for me... */
	    DiskCopy(obj,hitobj);
	}
	else if ((olddrawer != hitobj) && (hitobj->wo_DrawerData))
	{
	    MP(("DragObject: calling ObjMove\n"));
	    /* NEED TO CALL A ROUTINE TO CALL ObjMove ASYNCHRONOUSLY */
	    /* we move/copy this object into the specified drawer */
	    ObjMove(obj, olddrawer, hitobj, NO_ICON_POSITION,NO_ICON_POSITION);
	}
	return(0);
    }
moveobject:
    if (oldwin != dd->dd_NewWin) {
	/* MOVED OBJECT FROM ONE WINDOW TO ANOTHER */
	MP(("DragObject: switching windows, oldwin %lx (%s), newwin %lx (%s)\n",oldwin, oldwin->Title, dd->dd_NewWin, dd->dd_NewWin->Title));

	/* pass in the new position of the icon.  Be sure to correct
	 * (x,y) for the difference between the gadget position and where
	 * on the gadget the user actually hit (so the mouse stays
	 * over the same spot on the icon)
	 */
movetonewwindow:
	ObjMove(obj, olddrawer, dd->dd_NewDrawer,
	    oldwin->LeftEdge + obj->wo_Gadget.LeftEdge + dd->dd_WinDeltaX,
	    oldwin->TopEdge + obj->wo_Gadget.TopEdge + dd->dd_WinDeltaY);
    }
    else {
	/* MOVED OBJECT INSIDE ITS OWN WINDOW */
	newdd = dd->dd_NewDrawer->wo_DrawerData;
	if (newdd->dd_ViewModes <= DDVM_BYICON) {
	    /* lets figure out how much we moved by */
	    MP(("DragObject: %lx (%s) moved by (%ld,%ld)\n",
		obj, obj->wo_Name, dd->dd_DeltaX, dd->dd_DeltaY));
	    obj->wo_CurrentX += dd->dd_DeltaX;
	    obj->wo_CurrentY += dd->dd_DeltaY;
	    obj->wo_Gadget.LeftEdge += dd->dd_DeltaX;
	    obj->wo_Gadget.LeftEdge += dd->dd_DeltaY;

	    /* move the object to the top */
	    REMOVE(&obj->wo_Siblings);
	    ADDHEAD(&newdd->dd_Children, &obj->wo_Siblings);

	    /* redraw the drawer */
	    RefreshDrawer(olddrawer);
	}
    }
    return(0);
}
