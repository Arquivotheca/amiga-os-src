/*
 * $Id: diskchange.c,v 38.4 93/01/21 14:48:46 mks Exp $
 *
 * $Log:	diskchange.c,v $
 * Revision 38.4  93/01/21  14:48:46  mks
 * Cleaned up the disk init code...  (Removed much redundant code)
 * 
 * Revision 38.3  92/05/14  17:15:06  mks
 * Changed to use new stccpy() routine (which is now part of the WB
 * source)  It also now uses a simple optimization for "BAD" disks.
 *
 * Revision 38.2  91/11/12  14:29:18  mks
 * Fixed problem with free memory access during shutdown
 *
 * Revision 38.1  91/06/24  11:34:39  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/execbase.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "libraries/filehandler.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "diskchange.h"
#include "global.h"
#include "support.h"

#define TOBPTR(x)	(BPTR)(((ULONG)(x)) >> 2)

BPTR FindVolume(struct DeviceList *dl,struct ActiveDisk *ad)
{
BPTR result=0;

    if (!CompareDates(&dl->dl_VolumeDate, &ad->ad_CreateTime))
    {
	if (!stricmp(1+(UBYTE *)BADDR(dl->dl_Name), ad->ad_Name))
	{
	    result = ad->ad_Volume = TOBPTR(dl);
	}
    }
    return(result);
}

FindDevices(dl)
struct DeviceList *dl;
{
    if (dl->dl_Type == DLT_DEVICE)
    {
	/* TestPhysical is safe here - the list is locked */
	if (TestPhysical((struct DeviceNode *)dl))
	{
	    /* this a physical unit.  make an ad structure */
	    NewActive(dl, VOL_DEVICE);
	}
    }
    return(0);
}

ClearActive(ad)
struct ActiveDisk *ad;
{
    if (ad->ad_Active != VOL_DEVICE) ad->ad_Active = VOL_GONE;
    return(0);
}

CheckForLeftOutChildren(obj)
struct WBObject *obj;
{
struct FileInfoBlock *fib;

    MP(("CFLOC: enter, obj=$%lx (%s), LeftOut=%ld\n",obj, obj->wo_Name, obj->wo_LeftOut));
    if (obj->wo_LeftOut)
    { /* if this object has been left out.. */
	if (fib = ALLOCVEC(sizeof(struct FileInfoBlock), MEMF_PUBLIC))
	{
	    /* if cannot lock object, then its volume must not be available */
	    if (!EXAMINE(obj->wo_Lock, fib))
	    {
		obj->wo_LeftOut = 0; /* no longer left out */
		obj->wo_PutAway = 1; /* needs to be put away */
	    }

	    FREEVEC(fib);
	}
    }
    MP(("CFLOC: exit\n"));
    return(NULL);
}

PutAwayLeftOutIcons(obj)
struct WBObject *obj;
{
	if (obj->wo_PutAway) PutAway(obj,PUTAWAY_QUICK);
	return(NULL);
}

PutAwayAllChildren(struct WBObject *obj)
{
	if (obj->wo_LeftOut) PutAway(obj,PUTAWAY_QUICK);
	return(NULL);
}

UpdateVolumes(dl)
struct DeviceList *dl;
{
    struct WorkbenchBase *wb = getWbBase();
    struct ActiveDisk *ad;

    if (dl->dl_Type == DLT_VOLUME)
    {
	ad = (struct ActiveDisk *)ActiveSearch(VolumeToActive, TOBPTR(dl));
	if (ad == NULL)
	{
	    DP(("UpdateVolumes: VOL_NEW, calling NewActive\n"));
	    /* aha! a new volume */
	    NewActive(dl, VOL_NEW);
	}
	else
	{
	    if (dl->dl_Task)
	    {
		ad->ad_Active = VOL_PRESENT;
		MP(("UpdateVolumes: ad=%lx (%s), state=%ld (VOL_PRESENT)\n",ad, ad->ad_Name, ad->ad_Active));

		ad->ad_Handler = dl->dl_Task;
		BtoC(wb->wb_Buf0, (UBYTE *)(BADDR(dl->dl_Name)));
		if (stricmp(ad->ad_Name, wb->wb_Buf0))
		{ /* name changed */
		    ad->ad_Active = VOL_RELABEL; /* flag change */
		    FREEVEC(ad->ad_Name); /* free mem for old name */
		    /* alloc mem for and copy new name */
		    ad->ad_Node.ln_Name = ad->ad_Name = scopy(wb->wb_Buf0);
		    MP(("UpdateVolumes: ad=%lx (%s), state=%ld (VOL_RELABEL)\n",ad, ad->ad_Name, ad->ad_Active));
		}
	    }
	    else
	    {
		ad->ad_Active = VOL_NOTPRESENT;

		MP(("UpdateVolumes: ad=%lx (%s), state=%ld (VOL_NOTPRESENT)\n",ad, ad->ad_Name, ad->ad_Active));
		/* David Berezowski - April/90 */
		SiblingPredSearch(wb->wb_RootObject->wo_DrawerData->dd_Children.lh_TailPred, CheckForLeftOutChildren);
	    }
	}
    }
    return(NULL); /* tell list processor to continue */
}

void PhantomLockCleanup(obj)
struct WBObject *obj;
{
    struct NewDD *dd = obj->wo_DrawerData;

    if (obj->wo_Type == WBDISK && obj->wo_UseCount == 1)
    {
	MP(("PhantomLockCleanup: obj=$%lx (%s)  CLEANING UP\n",	obj, obj ? obj->wo_Name : "NULL"));
        UNLOCK(dd->dd_Lock);	/* UNLOCK checks for NULL */
        dd->dd_Lock = 0;
    }
}

RemoveActiveDisk(ad)
struct ActiveDisk *ad;
{
    struct WBObject *obj = ad->ad_Object;

    MP(("RemoveActiveDisk: ad=$%lx (%s)\n", ad, ad->ad_Name));
    REMOVE(&ad->ad_Node);
    FREEVEC(ad->ad_Name);
    if (obj) FullRemoveObject(obj);
    return(0);
}

RemoveOldDisks( ad )
struct ActiveDisk *ad;
{
    if (ad->ad_Active == VOL_GONE) {
	MP(("RemoveOldDisks: removing ad=%lx (%s)\n", ad, ad->ad_Name));
	RemoveActiveDisk(ad);
    }
    return(0);
}

VolumeListSearch(fptr, arg1, arg2, arg3, arg4) /* from alists.asm */
int (*fptr)();
LONG arg1, arg2, arg3, arg4;
{
    struct DosList *p;
    int result = 0;

    MP(("VLS: enter, calling LockDosList...\n"));
    p = LockDosList(LDF_ALL|LDF_READ);
    MP(("\tok, calling NextDosEntry...\n"));
    /* Must call NextDosEntry before using p! */
    while (p = NextDosEntry(p, LDF_ALL|LDF_READ)) {
	MP(("\tok, calling function $%lx...\n", fptr));
	if (result = (*fptr)(p, arg1, arg2, arg3, arg4)) break;
	MP(("\tok, calling NextDosEntry...\n"));
    }
    MP(("\tok, calling UnLockDosList...\n"));
    UnLockDosList(LDF_ALL|LDF_READ);
    MP(("\tok, resurning %ld\n", result));
    return(result);
}

ProcessDisks( ad )
struct ActiveDisk *ad;
{
struct WBObject *obj;
int result;

    obj = ad->ad_Object;
    MP(("ProcessDisks: enter, ad=%lx, (%s)", ad, ad->ad_Name));
    switch( ad->ad_Active ) {
    case VOL_NOTPRESENT:
	MP(("\tVOL_NOTPRESENT\n"));
	/* drive exists, but is not present */

	/* clean up phantom disk locks */
	PhantomLockCleanup( obj );

	/* this volume will need FillBackdrop called next time it goes to VOL_PRESENT */
	ad->ad_Flags &= ~ADF_FILLBACKDROP;

	if( ! VolumeListSearch( FindVolume, (long)ad, NULL, NULL, NULL ) ) RemoveActiveDisk( ad );
	break;

    case VOL_NEW:
	MP(("\tVOL_NEW\n"));

	result = DiskInserted(ad); /* try to get lock on disk */

	/* FilllBackdrop has been called (via DiskInserted) for this volume */
	ad->ad_Flags |= ADF_FILLBACKDROP;

	if (!result) {
	    MP(("ProcessDisks: calling Remove\n"));
	    REMOVE( &ad->ad_Node );
	    MP(("ProcessDisks: calling sfree\n"));
	    FREEVEC( ad->ad_Name );
	    MP(("ProcessDisks: calling FREEMEM\n"));
	    FREEMEM( ad, sizeof(struct ActiveDisk) );
	    break;
	}
	MP(("ProcessDisks: ok\n"));

	/* the object didn't exist until after DiskInserted... */
	obj = ad->ad_Object;
	MP(("ProcessDisks: calling AddFreeList\n"));
	AddFreeList( &obj->wo_FreeList, (APTR) ad, sizeof( struct ActiveDisk ) );
	ad->ad_Active = VOL_PRESENT;
	MP(("ProcessDisks: ad=%lx (%s), state=%ld (VOL_PRESENT)\n",ad, ad->ad_Name, ad->ad_Active));

	MP(("ProcessDisks: new disk '%s'\n", obj->wo_Name));
	/* fall through */

    case VOL_PRESENT:
vol_present:
	MP(("\tVOL_PRESENT\n"));
/*	DiskGauge( obj, ad, 1 );
*/	PhantomLockCleanup( obj );

	/*
	    if this volume has not yet had its backdrop file read
	    (actually in this case it needs to be re-read).
	*/
	if (!(	ad->ad_Flags & ADF_FILLBACKDROP)) {
	    FillBackdrop(GetParentsLock(obj));
	    ad->ad_Flags |= ADF_FILLBACKDROP; /* FillBackdrop has been called */
	}

	break;

    case VOL_DEVICE:
	MP(("\tVOL_DEVICE\n"));
	break;

    case VOL_RELABEL:
	MP(("\tVOL_RELABEL\n"));
	ad->ad_Active = VOL_PRESENT;
	RenameObject(ad->ad_Object, ad->ad_Name);
	goto vol_present;
	break;

#ifdef DEBUGGING
    default:
	MP(("ProcessDisks: fell out of switch (%ld)\n", ad->ad_Active ));
	Debug(0L);
	break;
#endif DEBUGGING

    }
    MP(("ProcessDisks: exit\n\n"));
    return(0);
}

NewDiskInfo(ad)
struct ActiveDisk *ad;
{
    ULONG handler;
    int result = 0;

    MP(("NewDiskInfo: ad=$%lx (%s)\n", ad, ad->ad_Name));
    if (handler = (ULONG)ad->ad_Handler) {
	result = DoPkt1((struct MsgPort *)handler, ACTION_DISK_INFO, ((ULONG)(&ad->ad_Info)) >> 2);
    }
    return(result);
}

#define ID_DOSx_DISK		(ID_DOS_DISK & 0xffffff00)
/*
    This routine checks the disk type which is stored as the first LONG on the disk.
    The known disk types as of 11/16/89 are...

    ID_NO_DISK_PRESENT	(-1)							FFFF
    ID_UNREADABLE_DISK	(((long)'B'<<24) | ((long)'A'<<16) | ('D'<<8))		BAD?
    ID_DOS_DISK		(((long)'D'<<24) | ((long)'O'<<16) | ('S'<<8))		DOSx
    ID_NOT_REALLY_DOS	(((long)'N'<<24) | ((long)'D'<<16) | ('O'<<8) | ('S'))	NDOS
    ID_KICKSTART_DISK	(((long)'K'<<24) | ((long)'I'<<16) | ('C'<<8) | ('K'))	KICK
*/
FloppyCheck(ad)
struct ActiveDisk *ad;
{
    struct WorkbenchBase *wb = getWbBase();
    struct InfoData *id;
    struct WBObject *obj;
    char *string;
    ULONG type;
    BOOL newicon = FALSE; /* assume we are not creating a new icon */

    MP(("FloppyCheck: enter, ad=%lx (%s), Active=%lx\n",
	ad, ad->ad_Name, ad->ad_Active));
    MP(("\tad->ad_Object=$%lx (%s)\n",
	ad->ad_Object, ad->ad_Object ? ad->ad_Object->wo_Name : "NULL"));

    if (ad->ad_Active == VOL_DEVICE) {

	if (!NewDiskInfo(ad))
	{
	    MP(("FloppyCheck: error %ld on NewDiskInfo\n", IOERR()))
	    goto end;
	}

	id = &ad->ad_Info;
	/* daveb - aug 18/88 */
	/* allow DOSx instead of just DOS0 */
	type = (ULONG)id->id_DiskType;
	/*                            D O S */
	if ((type & 0xffffff00) == 0x444F5300) { /* if this is a DOS disk */
	    MP(("\trecognized this disk as a DOSx disk\n"));
	    type  &= 0xffffff00;
	}
	MP(("FloppyCheck: unit=%ld, type=%lx, DiskType=%lx\n",
		id->id_UnitNumber, type, id->id_DiskType));
	MP(("\tID_DOSxDISK=%lx, ID_NO_DISK_PRESENT=%lx\n",
		ID_DOSx_DISK, ID_NO_DISK_PRESENT));

	/* now the hard part.  If it is type DOS, or no disk in drive,
	 * then go away
	 */
	if (type == ID_DOSx_DISK || type == ID_NO_DISK_PRESENT) {
remobject:
	    MP(("FloppyCheck: at remobject: check\n"));
	    if (obj = ad->ad_Object) {
		MP(("FloppyCheck: getting rid of an old disk, type=%ld, name='%s'\n",
		    type, obj->wo_Name));
		RemoveObject(obj);
		ad->ad_Object = NULL;
	    }
	}
	else {
	    /*
		at this point the disk MUST be of type...
		ID_UNREADABLE_DISK	(((long)'B'<<24) | ((long)'A'<<16) | ('D'<<8))			BAD?
		ID_NOT_REALLY_DOS	(((long)'N'<<24) | ((long)'D'<<16) | ('O'<<8) | ('S'))		NDOS
		ID_KICKSTART_DISK	(((long)'K'<<24) | ((long)'I'<<16) | ('C'<<8) | ('K'))		KICK
	    */
	    string = wb->wb_Buf1;
	    strcpy(string, ad->ad_Name);
	    strcat(string, ":");

		/* If BAD, make into ???? */
	    if (type == ID_UNREADABLE_DISK) type=0x3F3F3F3F;

	    stccpy(string + strlen(string), (char *)&type, 5);

	    if (!(obj = ad->ad_Object)) {
		MP(("FloppyCheck: calling WBAllocWBObject('%s')\n", string));
		/* lets make an icon */
		if (!(obj = WBAllocWBObject(string))) {
		    MP(("FloppyCheck: ERROR, could not AlloWBObject\n"));
		    NoMem();
		    goto end;
		}
		newicon = TRUE; /* we created a new icon */
		ad->ad_Object = obj;
		/* WARNING: this call could fail.  See the backout code in
		    DiskInserted (diskin.c) for an example of how to punt. */
		obj->wo_Type = WBKICK;
		initgadget2(obj);
	    }

	    MP(("FloppyCheck: comparing %s and %s\n", string, obj->wo_Name));
	    if ((stricmp(string, obj->wo_Name) != 0) || newicon) {
		/* hmmm -- the names are not the same or new icon */
		if (obj->wo_Parent) {
		    /* we must have missed the removal... */
		    MP(("FloppyCheck: remobject on (%s)\n", obj->wo_Name));
		    goto remobject;
		}
		MP(("FloppyCheck: calling AddDiskIcon (%s)\n", obj->wo_Name));
		AddDiskIcon(ad, obj);
	    }
    	}
    }
#ifdef DEBUGGING
    else {
	MP(("FloppyCheck: not a DEVICE\n"));
    }
#endif DEBUGGING
end:
    MP(("FloppyCheck: exit\n"));
    return(0);
}

RemoveAllDisks(struct ActiveDisk *ad)
{
UBYTE	status=ad->ad_Active;

	RemoveActiveDisk(ad);
	if (status == VOL_DEVICE) FREEMEM(ad,sizeof(struct ActiveDisk));
	return(0);
}

void uninitDisks()
{
struct WorkbenchBase *wb = getWbBase();

    if (wb->wb_RootObject)
    {
        SiblingPredSearch(wb->wb_RootObject->wo_DrawerData->dd_Children.lh_TailPred, PutAwayAllChildren);
    }
    ActiveSearch(RemoveAllDisks); /* free all disks */
}

/*
 * This routine is called every time find-disks is run.
 * It updates the disk information in the window (if open)
 */
int FixInfo(struct WBObject *obj)
{
struct	NewDD	*dd;

	if ((obj->wo_Type==WBDISK) && (dd=obj->wo_DrawerData))
	{
		if (dd->dd_DrawerWin) RedrawGauge(obj);
	}
	return(0);
}

void FindDisks()
{
struct WorkbenchBase *wb = getWbBase();
APTR oldwin;

    oldwin=((struct Process *)(wb->wb_Task))->pr_WindowPtr;
    ((struct Process *)(wb->wb_Task))->pr_WindowPtr=(APTR)(-1);     /* No requesters */

    MP(("\nFindDisks: enter, calling ActiveSearch(ClearActive)\n"));
    /* clear out the old status */
    ActiveSearch( ClearActive );
    MP(("FindDisks: calling VolumeListSearch(UpdateVolumes)\n"));
    /* compute the new status */
    VolumeListSearch( UpdateVolumes, NULL, NULL, NULL, NULL );

    /* David Berezowski - Apr/90 */
    SiblingPredSearch(wb->wb_RootObject->wo_DrawerData->dd_Children.lh_TailPred, PutAwayLeftOutIcons);

    /* and act on the status that we just found */
    MP(("FindDisks: calling ActiveSearch(RemoveOldDisks)\n"));
    ActiveSearch( RemoveOldDisks );
    MP(("FindDisks: calling ActiveSearch(FloppyCheck)\n"));
    ActiveSearch( FloppyCheck );
    MP(("FindDisks: calling ActiveSearch(ProcessDisks)\n"));
    ActiveSearch( ProcessDisks );

    MP(("FindDisks: calling MasterSearch(FixInfo)\n"));
    MasterSearch(FixInfo);

    ((struct Process *)(wb->wb_Task))->pr_WindowPtr=oldwin;

    MP(("FindDisks: exit\n\n"));
}

VolumeToActive(ad, vol)
struct ActiveDisk *ad;
BPTR vol;
{
    struct DeviceList *dl;

    MP(("VolumeToActive: ad=%lx (%s), ad->ad_Volume=%lx, vol=%lx\n",
	ad, ad->ad_Name, ad->ad_Volume, vol));
    if (ad->ad_Volume == vol) {
	dl = (struct DeviceList *)BADDR(vol);
	/* CompareDates of zero means the times are the same */
	if (CompareDates(&dl->dl_VolumeDate, &ad->ad_CreateTime) == 0) {
	   return(1);
	}
#ifdef DEBUGGING
	else {
	    DP(("VolumeToActive: ad=%lx (%s), ad->ad_Volume=%lx, vol=%lx\n",
		ad, ad->ad_Name, ad->ad_Volume, vol));
	    DP(("VTA: same volume, different date\n"));
	}
#endif DEBUGGING
    }
    return(0);
}

FindActive(ad, obj)
struct ActiveDisk *ad;
struct WBObject *obj;
{
    MP(("FindActive: ad=$%lx (%s), obj=$%lx (%s)\n",ad, ad->ad_Name, obj, obj ? obj->wo_Name : "NULL"));
    if (ad->ad_Object == obj) return(1);
    return(0);
}

VOID RedrawGauge(struct WBObject *obj)
{
    struct NewDD *dd = obj->wo_DrawerData;
    UBYTE *ptr;

    MP(("RedrawGauge:\tobj=$%lx (%s)\n", obj, obj->wo_Name));
    MP(("\tdd=$%lx, win=$%lx, title=$%lx (%s)\n",dd, dd->dd_DrawerWin, dd->dd_DrawerWin->Title,dd->dd_DrawerWin->Title ? dd->dd_DrawerWin->Title : "NULL"));

    FormatWindowName(obj, &ptr, dd->dd_DrawerWin);
}

struct ActiveDisk *
NewActive( dl, state )
struct DeviceList *dl;
int state;
{
    struct WorkbenchBase *wb = getWbBase();
    struct ActiveDisk *ad;

    ad = (struct ActiveDisk *)
	ALLOCMEM( sizeof( struct ActiveDisk ), MEMF_CLEAR|MEMF_PUBLIC );

    if( ad == NULL ) {
	NoMem();
	return( NULL );
    }

    ad->ad_Volume = (BPTR) dl >> 2;
    BtoC( wb->wb_Buf0, (UBYTE *) (BADDR( dl->dl_Name )) );
    ad->ad_Node.ln_Name = ad->ad_Name = scopy( wb->wb_Buf0 );
    ad->ad_Handler = dl->dl_Task;
    ad->ad_CreateTime = dl->dl_VolumeDate;
    ad->ad_Active = state;
    MP(("NewActive: ad=%lx (%s), state=%ld\n",ad, ad->ad_Name, ad->ad_Active));

    ADDTAIL( &wb->wb_ActiveDisks, (struct Node *) ad );

    return( ad );
}

MatchObjectToActive( ad, obj )
struct ActiveDisk *ad;
struct WBObject *obj;
{
    if( ad->ad_Object == obj ) return( 1 );
    return( 0 );
}


struct ActiveDisk *
ObjectToActive( obj )
struct WBObject *obj;
{
    struct ActiveDisk *ad;

    ad = (struct ActiveDisk *) ActiveSearch( MatchObjectToActive, obj );

#ifdef DEBUGGING
    if (ad == NULL) {
	MP(("ObjectToActive: null active disk! obj 0x%lx\n", obj));
	Debug(0L);
    }
#endif DEBUGGING

    return( ad );
}

/* this is evil - we are implementing an IS_FILESYSTEM packet which
   should do this in one call - and safer too. REJ */

/* MUST be called with the device list lock!!!! Danger!!!! */

/* see if this device node refers to a physical device */
TestPhysical(dn)
struct DeviceNode *dn;
{
    struct WorkbenchBase *wb = getWbBase();
    ULONG val;
    struct FileSysStartupMsg *fssm;
    char buf[32];

    /* if process does not yet exist, give it up (!!! -- questionable rule) */
    if (!dn->dn_Task) {
	MP(("TestPhysical: process doesn't exist, EXIT\n"));
	return(0);
    }

    strcpy(buf, ((char *)BADDR(dn->dn_Name)) + 1);
    strcat(buf, ":");

    /* all "real" disks can be inhibited */
    if (!Inhibit(buf, 0))
    {
        if (IOERR()==ERROR_ACTION_NOT_KNOWN)
        {
	    MP(("TestPhysical: could not ACTION_INHIBIT, EXIT\n"));
	    return(0);
	}
    }

    /* see if the startup message is valid */
    if (!(val = (ULONG)dn->dn_Startup)) {
	MP(("TestPhysical: dn_Startup=%lx, EXIT\n", val));
	return(0);
    }

    fssm = (struct FileSysStartupMsg *)BADDR(val);

    MP(( " fssm is 0x%lx\n", fssm ));

    /* make sure the device name is valid */
    if (!(val = fssm->fssm_Device)) {
	MP(("TestPhysical: fssm_Device=%lx, EXIT\n", val));
	return(0);
    }

    MP(( " device is 0x%lx/%s\n", BADDR( val ), ((char *)BADDR(val)) + 1 ));

    /* make sure the device name is in exec's name list. note: val is a BSTR */
    if (!FindName(&((struct ExecBase *)wb->wb_SysBase)->DeviceList,
	((char *)(BADDR(val)) + 1))) {
	MP(("TestPhysical: device name not in exec's name list, EXIT\n"));
	return(0);
    }

    MP(("TestPhysical: SUCCESS!\n"));

    return(1); /* well, we passed all the tests */
}
