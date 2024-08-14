/*
 * $Id: iconlib.c,v 38.3 92/07/29 12:04:29 mks Exp $
 *
 * $Log:	iconlib.c,v $
 * Revision 38.3  92/07/29  12:04:29  mks
 * WBFreeWBObject can now also free an object that has not been
 * added to the list yet.
 * 
 * Revision 38.2  92/04/27  14:22:02  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 *
 * Revision 38.1  91/06/24  11:35:49  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

/*
 *
 *   NAME
 *       WBAllocWBObject - allocate a Workbench object
 *
 *   SYNOPSIS
 *       object = WBAllocWBObject( name )
 *       D0                      A0
 *
 *   FUNCTION
 *       This routine allocates a Workbench object, and initializes
 *       its free list.  A subsequent call to WBFreeWBObject will
 *       free all of its memory.
 *
 *       If memory cannot be obtained, a NULL is returned.
 *
 *       This routine is intended only for internal users that can
 *       track changes to the Workbench.
 *
 *   INPUTS
 *
 *   RESULTS
 *       object - the WBObject (if memory is available)
 *
 *   EXCEPTIONS
 *
 *   SEE ALSO
 *       AllocEntry, FreeEntry, WBFreeWBObject
 *
 *   BUGS
 *
 */
struct WBObject *
WBAllocWBObject( name )
char *name;
{
    struct WorkbenchBase *wb = getWbBase();
    struct WBObject *obj;
    int len;

    DP(("WBAllocWBObject: enter, name=$%lx (%s)\n",
	name, name));

    if (!(obj = (struct WBObject *)ALLOCVEC(sizeof(struct WBObject), MEMF_CLEAR|MEMF_PUBLIC))) goto end;
    NewList(&obj->wo_FreeList.fl_MemList);

    /* MUST ESTABLISH ROOT ASAP SO OTHER ROUTINES CAN COMPARE AGAINST IT */
    if (wb->wb_RootObject == NULL) {
	wb->wb_RootObject = obj;
	DP(("WBAllocWBObject: Root was null, Root now %lx\n",
	    wb->wb_RootObject));
    }

    len = WONAME_SIZE;
    if (name) len += strlen(name) + 1;
    if (!(obj->wo_NameBuffer = WBFreeAlloc(&obj->wo_FreeList, len, MEMF_CLEAR))) goto err;

    if (name) { /* the name MAY be right after the NameBuffer */
	obj->wo_Name = &obj->wo_NameBuffer[WONAME_SIZE];
	strcpy(obj->wo_Name, name);

	SetIconNames(obj);

    }

DP(("********** ADDING '%s' TO THE MASTER LIST **********\n", obj->wo_Name));
    ADDTAIL( &wb->wb_MasterList, &obj->wo_MasterNode );
end:
#ifdef DEBUGGING
    if (obj) {
	DP(("[WBAllocWBObject]: obj=%lx (%s), x,y = %ld,%ld\n",
	    obj, obj->wo_Name, obj->wo_CurrentX, obj->wo_CurrentY));
	DP(("WBAllocWBObject: obj=%lx, NameBuffer=%lx, Name=%lx (%s), len=%lx\n",
	    obj, obj->wo_NameBuffer, obj->wo_Name, obj->wo_Name, len));
    }
#endif DEBUGGING
    DP(("WBAllocWBObject: exit, returning $%lx\n", obj));
    return( obj );

err:
    DP(("WBAllocWBObject: err: freeing %ld bytes @$%lx for WBObject\n",
	sizeof(struct WBObject), obj));
    FREEVEC(obj);
    return( NULL );
}

/*
 *
 *   NAME
 *       WBFreeWBObject - free all memory in a Workbench object
 *
 *   SYNOPSIS
 *       WBFreeWBObject( obj )
 *                     A0
 *
 *   FUNCTION
 *       This routine frees all memory in a Workbench object, and the
 *       object itself.  It is implemented via WBFreeFreeList().
 *
 *       WBAllocWBObject() takes care of all the initialization required
 *       to set up the objects free list.
 *
 *       This routine is intended only for internal users that can
 *       track changes to the Workbench.
 *
 *   INPUTS
 *       free -- a pointer to a FreeList structure
 *
 *   RESULTS
 *
 *   EXCEPTIONS
 *
 *   SEE ALSO
 *       AllocEntry, FreeEntry, WBAllocWBObject, WBFreeFreeList
 *
 *   BUGS
 *
 */
void WBFreeWBObject( obj )
struct WBObject *obj;
{
    if (MasterSearch(ObjOnList, obj)) REMOVE( &obj->wo_MasterNode );

    if (obj)
    {
	FreeFreeList(&obj->wo_FreeList);

	if (obj->wo_DiskObject) FreeDiskObject(obj->wo_DiskObject);

	FREEVEC(obj);
    }
}

char *PlusInfo(char *name)
{
char	*c;

	if (c=ALLOCVEC(strlen(name)+6,MEMF_PUBLIC))
	{
		strcpy(c,name);
		strcat(c,InfoSuf);
	}
	return(c);
}

/*
 *
 *   NAME
 *       WBGetWBObject - read in a Workbench object
 *
 *   SYNOPSIS
 *       object = WBGetWBObject( name )
 *       D0                    A0
 *
 *   FUNCTION
 *       This routine reads in a Workbench object in from disk.  The
 *       name parameter will have a ".info" postpended to it, and the
 *       info file of that name will be read.  If the call fails,
 *       it will return zero.  The reason for the failure may be obtained
 *       via IoErr().
 *
 *       This routine is intended only for internal users that can
 *       track changes to the Workbench.
 *
 *   INPUTS
 *       name -- name of the object
 *
 *   RESULTS
 *       object -- the Workbench object in question
 *
 *   EXCEPTIONS
 *
 *   SEE ALSO
 *
 *   BUGS
 *
 */
struct WBObject *FibWBGetWBObject(char *name,struct FileInfoBlock *fib)
{
struct WBObject *obj;

    BeginDiskIO();

    if (obj = GetWBIcon(name))
    {
	DP(("NewWBGetWBObject: calling PrepareIcon\n"));
	PrepareIcon(obj);

        obj->wo_FileSize = fib->fib_Size;
        obj->wo_Protection = fib->fib_Protection;
        obj->wo_DateStamp = fib->fib_Date;
    }

    EndDiskIO();
    return(obj);
}


/*
 * This routine calls The new WBGetWBObject routine which has a possible
 * fib on entry
 */
struct WBObject *WBGetWBObject(char *name)
{
struct WBObject *obj=NULL;
struct FileInfoBlock *fib;
BPTR lock;
int NoFile;

    BeginDiskIO();

    if (!(lock=LOCK(name,ACCESS_READ)))
    {
    char *tmp;

        NoFile=TRUE;
        if (tmp=PlusInfo(name))
        {
            lock=LOCK(tmp,ACCESS_READ);
            FREEVEC(tmp);
        }
    }
    if (lock)
    {
        DP(("WBGetWBObject: calling ALLOCVEC\n"));
        if (fib = (struct FileInfoBlock *)ALLOCVEC(sizeof(struct FileInfoBlock), MEMF_PUBLIC))
        {
            DP(("WBGetWBObject: (fib=$%lx) calling Examine\n", fib));
            if (EXAMINE(lock, fib))
            {
                if (!NoFile) fib->fib_Size=0;
                obj=FibWBGetWBObject(name,fib);
            }
            DP(("WBGetWBObject: calling FREEMEM\n"));
            FREEVEC(fib);
        }
        DP(("WBGetWBObject: calling UNLOCK\n"));
        UNLOCK(lock);
    }

    EndDiskIO();
    return(obj);
}

struct WBObject *NewWBGetWBObject(char *name,struct FileInfoBlock *fib)
{
struct WBObject *obj;

	if (fib) obj=FibWBGetWBObject(name,fib);
	else obj=WBGetWBObject(name);

	return(obj);
}

/*
 *
 *   NAME
 *       WBPutWBObject - write out a Workbench object
 *
 *   SYNOPSIS
 *       status = WBPutWBObject( name, object )
 *       D0                    A0    A1
 *
 *   FUNCTION
 *       This routine writes a Workbench object out to disk.  The
 *       name parameter will have a ".info" postpended to it, and
 *       that file name will have the disk-resident information
 *       written into it.  If the call fails, it will return a zero.
 *       The reason for the failure may be obtained via IoErr().
 *
 *       This routine is intended only for internal users that can
 *       track changes to the Workbench.
 *
 *   INPUTS
 *       name -- name of the object
 *       object -- the Workbench object to be written out
 *
 *   RESULTS
 *       status -- non-zero if the call succeeded.
 *
 *   EXCEPTIONS
 *
 *   SEE ALSO
 *
 *   BUGS
 *
 */
WBPutWBObject( name, obj )
char *name;
struct WBObject *obj;
{
char *tmp;
struct DiskObject dobj;
int result;

    if (!obj->wo_FakeIcon) {
#ifdef DEBUGGING
	PrintWBObject(obj);
#endif DEBUGGING
	dobj.do_Magic = WB_DISKMAGIC;
	dobj.do_Version = WB_DISKVERSION;
	dobj.do_Gadget = obj->wo_Gadget;	/* structure assignment */
	result = ((int)dobj.do_Gadget.UserData) & ~WB_DISKREVISIONMASK;
	result |= WB_DISKREVISION;
	DP(("revision=%ld\n", result));
	dobj.do_Gadget.UserData = (APTR)result;
	dobj.do_ToolTypes = obj->wo_ToolTypes;
	dobj.do_Type = obj->wo_Type;
	dobj.do_CurrentX = obj->wo_CurrentX;
	dobj.do_CurrentY = obj->wo_CurrentY;
	dobj.do_DrawerData = (struct DrawerData *)obj->wo_DrawerData;
	dobj.do_DefaultTool = obj->wo_DefaultTool;
	dobj.do_ToolWindow = obj->wo_ToolWindow;
	dobj.do_StackSize = obj->wo_StackSize;

	if (tmp=PlusInfo(name))
	{
	    SETPROTECTION(tmp,0);	/* Let us write the icon... */
	}

	if (result = PutIcon(name,&dobj)) if (tmp)
	{
	    /*
	     * So, we let in the delete and write protection bit and
	     * make sure that the file not an executable and is readable...
	     *
	     * That is, only DELETE and WRITE are settable on an icon.
	     */
	    SETPROTECTION(tmp,(obj->wo_Protection & (FIBF_DELETE|FIBF_WRITE)) | FIBF_EXECUTE);
	}

	FREEVEC(tmp);
    }
    else
    {
	DP(("WBPutWBObject: bypassing Fake icon '%s'\n", name));
	result = 1; /* all ok */
    }
    DP(("WBPutWBObject: '%s', result=%ld\n", name, result));
    return( result );
}

void *WBFreeAlloc(free, size, type)
struct FreeList *free;
long size, type;
{
void *mem;

	MP(("WBFreeAlloc: free=%lx, size=%ld, type=%lx\n", free, size, type));
	if (mem=(void *)size) if (mem = (void *)ALLOCMEM(size, type))
	{
		MP(("calling WBAddFreeList..."));
		if (!AddFreeList(free, mem, size))
		{
			MP(("it failed,  "));
			FREEMEM(mem,size);
			mem = NULL;
		}
		MP(("ok, "));
	}
	MP(("mem=%lx, exiting\n", mem));
	return(mem);
}
