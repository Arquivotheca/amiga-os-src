/*
 * $Id: fileops.c,v 38.2 92/06/11 07:37:54 mks Exp $
 *
 * $Log:	fileops.c,v $
 * Revision 38.2  92/06/11  07:37:54  mks
 * Removed code that was old and dead...
 * 
 * Revision 38.1  91/06/24  11:35:27  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/memory.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "errorindex.h"
#include "quotes.h"

/*
 * WBMove returns
 *	 1 if move was successfull
 *	 0 if an error occurred
 *	-1 if move was really a copy
 */

WBMove(obj, fromdrawer, todrawer, curx, cury)
struct WBObject *obj, *fromdrawer, *todrawer;
LONG curx, cury;
{
struct WorkbenchBase *wb = getWbBase();
char *name;
LONG fromlock, tolock;
int result = 0;

    DP(("WBMove: enter, obj=%lx (%s), fd=%lx (%s), td=%lx (%s)\n",
	obj, obj->wo_Name, fromdrawer, fromdrawer->wo_Name,
	todrawer, todrawer->wo_Name));

    if (obj->wo_Type == WBDISK) { /* if object is a disk... */
	fromlock = GetParentsLock(obj); /* use lock on disk */
    }
    else if (obj->wo_Background) { /* if object is in the background... */
	fromlock = obj->wo_Lock; /* lock is contained in the object */
	/* we are finally moving out of the backdrop (YAHOO!) */
	DP(("WBMove: obj was in the background, using obj->wo_Lock (%lx)\n",
	    fromlock));
    }
    else { /* object is either a file/drawer and not in the background */
	fromlock = fromdrawer->wo_DrawerData->dd_Lock;
    }

    if (!fromlock) {
	DP(("WBMove: error, fromlock=%lx\n", fromlock));
	goto end;
    }

    /* handle moves TO the background */
    if (todrawer == wb->wb_RootObject) {
	/* we are moving to the backdrop window.  This is not a real move:
	 * only the icon gets transferred */
	obj->wo_Background = 1;
	obj->wo_Lock = DUPLOCK(fromlock);
	DP(("WBMove: moved to the backdrop, returning 1\n"));
	return(1);
    }

    tolock = todrawer->wo_DrawerData->dd_Lock;

    result=SAMELOCK(fromlock,tolock);
    if ((result!=LOCK_SAME) && (result!=LOCK_SAME_HANDLER))
    {
	result = DosCopyLaunch(obj->wo_Name, obj->wo_Name, fromdrawer, todrawer,curx, cury, fromlock, tolock, obj->wo_Type);
    }
    else
    {
	if (result==LOCK_SAME) result=1;
	else
	{
	    result = DosMove(fromlock, obj->wo_Name, tolock, obj->wo_Name, 1);

	    if (result)
	    {
	        name = wb->wb_Buf0;
                strcpy(name, obj->wo_Name);
                strcat(name, InfoSuf);

                result = DosMove(fromlock, name, tolock, name, 1);
	    }
	}

	if (result)
        {
            if (obj->wo_LeftOut) PutAway(obj,PUTAWAY_NODEL);
            if (obj->wo_Background)
            {
                UNLOCK(obj->wo_Lock);
                obj->wo_Lock = 0;
                obj->wo_Background = 0;
            }
        }
    }
end:
    DP(("WBMove: result=%ld, exit\n", result));
    return(result);
}

WBRename( obj, oldname )
struct WBObject *obj;
char *oldname;
{
    struct WorkbenchBase *wb = getWbBase();
    LONG lock;
    int result;
    char *new, *old;

    lock = GetParentsLock(obj);

    new = wb->wb_Buf0;
    old = wb->wb_Buf1;

    /* the move of the object itself */
    strcpy( old, oldname );
    strcpy( new, obj->wo_Name );
    if (result = DosMove( lock, old, lock, new, 1))
    {
        strcat( new, InfoSuf );
        strcat( old, InfoSuf );
        result = DosMove( lock, old, lock, new, 1 );
    }

    return( result );
}

WBRenameDisk(obj, oldname)
struct WBObject *obj;
char *oldname;
{
    char *name = obj->wo_Name;
    struct ActiveDisk *ad = NULL;
    BPTR lock;
    int result = 0;

    obj->wo_Name = oldname;
    if (lock = GetParentsLock(obj)) {
	ad = ObjectToActive(obj);
	CURRENTDIR(lock);
	result = Relabel(":", name);
        DP(("WBRenameDisk: calling Relabel with name = %s\n"));
	DP(("\tresult = %ld, IoErr = %ld\n", result, IOERR()));
    }
    if (!result) {
	ErrorTitle(Quote(Q_CANT_RENAME_DISK));
    }
    else {
	FREEVEC(ad->ad_Name);
	ad->ad_Name = scopy(name);
	obj->wo_Name = name;
    }
    return(result);
}

#define	RENAME_ASSIGN	" ^WB^"

/* DosMove:
 *
 * This routine used to be used to move a file by either
 * renaming it or copying it.  Now it is just used to
 * rename a file.
 *
 * return value:
 *	 1 - move successfull
 *	 0 - move failed
 */
DosMove(fromlock, fromCfile, tolock, toCfile, notfoundok)
BPTR fromlock;
BPTR tolock;
char *fromCfile;
char *toCfile;
int notfoundok;
{
BPTR tmplock;
long err;
int result = FALSE; /* assume failure */
char buffer[132];

    DP(("DosMove: enter, fl=%lx, fCf=%s, tl=%lx, tCf=%s, nfok=%ld\n",fromlock, fromCfile, tolock, toCfile, notfoundok));

    CURRENTDIR(tolock);

    if (tmplock=DUPLOCK(fromlock))
    {
        if (AssignLock(RENAME_ASSIGN,tmplock))
        {
            strcpy(buffer,RENAME_ASSIGN);
            strcat(buffer,":");
            strcat(buffer,fromCfile);
            result = RENAME(buffer,toCfile);
            err=IOERR();
            AssignLock(RENAME_ASSIGN,NULL);
        }
        else
        {
            err=IOERR();
	    UNLOCK(tmplock);
	}
    }
    else err=IOERR();

    /* if we don't care if the source file wasn't found */
    if ((!result) && (notfoundok) && (err == ERROR_OBJECT_NOT_FOUND)) result = TRUE;

    if (!result)
    {
        SetResult2(err);
        ErrorDos(ERR_MOVE, fromCfile);
    }

    DP(("DosMove: result=%ld, notfoundok=%ld, IoErr=%ld, exit\n",result, notfoundok, err));
    return(result != 0);
}

/* convert a BCPL string into a C style string */
void BtoC( cbuf, bstr )
UBYTE *cbuf;
UBYTE *bstr;
{
    int len;

    len = *bstr++;
    while( len-- ) {
	*cbuf++ = *bstr++;
    }
    *cbuf = '\0';
}


WriteProtected( fl )
struct FileLock *fl;
{
    struct ActiveDisk *ad;

    /* if there is no lock, we had better not write it! */
    if( fl == NULL ) return( 1 );

    ad = (struct ActiveDisk *) ActiveSearch( VolumeToActive, fl->fl_Volume );

    MP(( "WriteProtected: type %ld\n", ad->ad_Info.id_DiskState ));

    return( ad->ad_Info.id_DiskState == ID_WRITE_PROTECTED );
}

AssignDOtoWBO(obj, dobj)
struct WBObject *obj;
struct DiskObject *dobj;
{
    memcpy(&(obj->wo_Gadget),&(dobj->do_Gadget),sizeof(struct Gadget));
    memcpy(&(obj->wo_IOGadget),&(dobj->do_Gadget),sizeof(struct Gadget));

    obj->wo_Type = dobj->do_Type;
    obj->wo_DefaultTool = dobj->do_DefaultTool;
    obj->wo_ToolTypes = dobj->do_ToolTypes;
    obj->wo_CurrentX = dobj->do_CurrentX;
    obj->wo_CurrentY = dobj->do_CurrentY;
    obj->wo_SaveX = obj->wo_CurrentX; /* init SaveX */
    DP(("AssignDOtoWBO: setting SaveX to %ld ($%lx) for %lx (%s)\n",
	obj->wo_SaveX, obj->wo_SaveX, obj, obj->wo_Name));
    obj->wo_SaveY = obj->wo_CurrentY; /* init SaveY */
    obj->wo_DrawerData = (struct NewDD *) dobj->do_DrawerData;
    obj->wo_ToolWindow = dobj->do_ToolWindow;
    obj->wo_StackSize = dobj->do_StackSize;

    MP(("AssignDO2WBO:\n"));
    MP(("\tdo_CurrentX = %ld ($%lx), do_CurrentY=%ld ($%lx)\n",
	dobj->do_CurrentX, dobj->do_CurrentX,
	dobj->do_CurrentY, dobj->do_CurrentY));
    MP(("\two_CurrentX = %ld ($%lx), wo_CurrentY=%ld ($%lx)\n",
	obj->wo_CurrentX, obj->wo_CurrentX,
	obj->wo_CurrentY, obj->wo_CurrentY));
    MP(("\two_SaveX = %ld ($%lx), wo_SaveY=%ld ($%lx)\n",
	obj->wo_SaveX, obj->wo_SaveX,
	obj->wo_SaveY, obj->wo_SaveY));

    if( obj->wo_DrawerData )
    {
    struct NewDD *dd = obj->wo_DrawerData = ObjAllocNorm( obj, sizeof( struct NewDD ) );

	if( !dd ) return(NULL);
	memcpy(dd,dobj->do_DrawerData,DRAWERDATAFILESIZE);
	MP(("AssignDO2WBO: dd_XY = %ld,%ld\n",obj->wo_DrawerData->dd_CurrentX,obj->wo_DrawerData->dd_CurrentY));
	dd->dd_Object = obj;
	NewList( &dd->dd_Children );

	dd->dd_NewWindow.DetailPen = -1;
	dd->dd_NewWindow.BlockPen = -1;
	dd->dd_NewWindow.Type = WBENCHSCREEN;

	/*
	 * Some icons had, somehow, matched our check value and
	 * had junk in these two fields...
	 */
	dd->dd_Flags &= 3;
	if (dd->dd_ViewModes > DDVM_BYSIZE) dd->dd_ViewModes=DDVM_BYDEFAULT;
    }
    return(1);
}

struct WBObject *GetWBIcon(char *name)
{
struct WBObject *obj;
struct DiskObject dobj;

    DP(("GetWBIcon: enter, calling WBAllocWBObject('%s')\n", name));
    if (obj = WBAllocWBObject(name))
    {
	DP(("GetWBIcon: obj=%lx, calling GetIcon()\n", obj));
	DP(("GetWBIcon: name='%s', &dobj=%lx, &obj->wo_FreeList=%lx\n",name, &dobj, &obj->wo_FreeList));

	if (GetIcon(name, &dobj, &obj->wo_FreeList))
	{
	    DP(("GetWBIcon: after GetIcon '%s', dobj X,Y = %ld,%ld\n",name, dobj.do_CurrentX, dobj.do_CurrentY));
	    DP(("\tdobj Gadget L,T = %ld,%ld\n",dobj.do_Gadget.LeftEdge, dobj.do_Gadget.TopEdge));
	    if (AssignDOtoWBO(obj, &dobj)) goto ok;
	}

	DP(("GetWBIcon: calling FreeWBObject\n"));
	WBFreeWBObject(obj);
	obj = NULL;
    }
ok:
    DP(("GetWBIcon: obj=%lx, exit\n", obj));
    return(obj);
}
