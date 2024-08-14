/* device.c */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/filehandler.h"
#include <string.h>

#include "libhdr.h"

#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "device_protos.h"
#include "klib_protos.h"
#include "blib_protos.h"

#define TOBPTR(x)	(((LONG) (x)) >> 2)
#define SAME		0


struct DosList * ASM
LockDosList (REG(d1) ULONG flags)
{
	return InternalLockDosList(flags,TRUE);
}

struct DosList * ASM
AttemptLockDosList (REG(d1) ULONG flags)
{
	return InternalLockDosList(flags,FALSE);
}

struct DosList * __regargs
lockit (struct DosList *dl, struct SignalSemaphore *sem,
	ULONG flags, ULONG synch)
{
	if (synch) {
		if (flags & LDF_WRITE)
			ObtainSemaphore(sem);
		else
			ObtainSemaphoreShared(sem);
	} else {
		/* FIX! shared? */
		if (!AttemptSemaphore(sem))
			return NULL;
	}
	return dl;
}

struct DosList * ASM
InternalLockDosList (REG(d1) ULONG flags,REG(d2) LONG synch)
{
	struct DosInfo *di;
	struct DosList *dl;
	LONG rw;

	di = (struct DosInfo *) BADDR(rootstruct()->rn_Info);
	dl = (struct DosList *) BADDR(di->di_DevInfo);

	/* did they specify one of READ or WRITE? */
	rw = flags & (LDF_READ|LDF_WRITE);
	if (!rw || rw == (LDF_READ|LDF_WRITE))
		return NULL;				/* error */

	/* check if flags are valid (unknown flags == error) */
	if (flags & ~(LDF_DELETE|LDF_ENTRY|LDF_ALL|LDF_READ|LDF_WRITE))
		return NULL; 				/* error */

	/* first lock device list if asked */
	if (flags & LDF_ALL)	/* any one of devices, volumes, assigns */
		dl = lockit(dl,&(di->di_DevLock),flags,synch);

	/* now lock entry if asked */
	if (flags & LDF_ENTRY)
		dl = lockit(dl,&(di->di_EntryLock),flags,synch);

	/* now lock entry if asked */
	if (flags & LDF_DELETE)
		dl = lockit(dl,&(di->di_DeleteLock),flags,synch);

	/* FIX! remove in 1.5!!!!!!! */
	if (dl) {
		Forbid();
		/* return INVALID ptr to indicate he hasn't examined the 1st */
		return (struct DosList *) (((LONG) dl) + 1);
	}
	return NULL; // failure
}

void ASM
UnLockDosList (REG(d1) ULONG flags)
{
	struct SignalSemaphore *s;
	struct DosInfo *di = (struct DosInfo *) BADDR(rootstruct()->rn_Info);

	if (flags & LDF_ALL)
	{
		s = &(di->di_DevLock);
		ReleaseSemaphore(s);
	}

	if (flags & LDF_ENTRY)
	{
		s = &(di->di_EntryLock);
		ReleaseSemaphore(s);
	}

	if (flags & LDF_DELETE)
	{
		s = &(di->di_DeleteLock);
		ReleaseSemaphore(s);
	}

	/* FIX! remove in 1.5!!!!!!! */
	Permit();
}

/* assumes doslist is locked exclusive! */

LONG ASM
RemDosEntry (REG(d1) struct DosList *dlist)
{
	struct DosInfo *di;
	struct DosList *dl;
	LONG success = DOSTRUE;

	/* make sure no one is mucking with a node (perhaps this node!)  */
	/* see GetDeviceProc notes... 					 */
	/* Delete requires that NO handler be starting up.  Add doesn't, */
	/* just that no one is traversing the list.			 */
	/* Handler startup MUST grab a read-lock on LDF_DELETE OR	 */
	/* a write-lock on LDF_ENTRY.					 */
	/* we grab write-locks on both.					 */
	LockDosList(LDF_DELETE|LDF_ENTRY | LDF_WRITE);

	di = (struct DosInfo *) BADDR(rootstruct()->rn_Info);
	dl = (struct DosList *) BADDR(di->di_DevInfo);

	if (dl == dlist)	/* head case */
		di->di_DevInfo = dl->dol_Next;
	else {
		/* make sure it's in the device list, and find parent */
		while (dl->dol_Next && BADDR(dl->dol_Next) != (APTR) dlist)
			dl = (struct DosList *) BADDR(dl->dol_Next);

		/* did we find it in the list? */
		if (dl->dol_Next)
			dl->dol_Next = dlist->dol_Next;
		else
			success = FALSE;
	}

	UnLockDosList(LDF_DELETE|LDF_ENTRY | LDF_WRITE);

	return success;
}

/* does NOT require the DosList to be locked! */
LONG ASM
AddDosEntry (REG(d1) struct DosList *dlist)
{
	struct DosInfo *di;
	struct DosList *dl;
	char *name;
	LONG success = DOSTRUE;

	/* first, lock the device list for writing */
	/* deal with funny returned ptr!!!         */
	/* safe to add things while handlers are starting... */

	dl = LockDosList(LDF_ALL|LDF_WRITE);
	dl = (struct DosList *) (((LONG) dl) - 1);

	/* now, search for duplicates if not DLT_VOLUME */
	name = (char *) BADDR(dlist->dol_Name);
	for (; dl; dl = (struct DosList *) BADDR(dl->dol_Next))
	{
		/* dol_Name is null-terminated BSTR */
		if (mystricmp((char *) BADDR(dl->dol_Name),name) ==
		    SAME)
		{
			/* CONFLICT! */
			if (dl->dol_Type == DLT_VOLUME)
			{
			    /* volumes conflict with volumes IF they      */
			    /* have the same dates.			  */
			    /* non-volumes conflict with existing volumes */
			    if (dlist->dol_Type != DLT_VOLUME ||
				CompareDates(
				    &(dl->dol_misc.dol_volume.dol_VolumeDate),
				 &(dlist->dol_misc.dol_volume.dol_VolumeDate))
				 == SAME)
				{
				  /* don't allow two vols of the same date! */
					goto err;
				}
			} else {
				/* this conflicts with a non-volume */
				if (dlist->dol_Type != DLT_VOLUME)
				{
				  /* if we're adding anything but a volume */
err:					success = FALSE;
					setresult2(ERROR_OBJECT_EXISTS);
					goto add_error;
				}  /* volumes can conflict with non-volumes */
			}
		}
	}

	/* now add it at head */
	di = (struct DosInfo *) BADDR(rootstruct()->rn_Info);
	dlist->dol_Next = di->di_DevInfo;
	di->di_DevInfo  = TOBPTR(dlist);

add_error:
	UnLockDosList(LDF_ALL|LDF_WRITE);	/* FIX? LDF_EXCLUSIVE? */
	return success;
}

/* array for converting dol_Type to a LDF_xxx flag for checking. */
/* array is offset by one for DLT_PRIVATE. */
/* all current values fit in a UBYTE 	   */
static UBYTE dlt_to_ldf[] = {
	0,		/* DLT_PRIVATE (-1) */
	LDF_DEVICES,	/* DLT_DEVICE       */
	LDF_ASSIGNS,	/* DLT_DIRECTORY    */
	LDF_VOLUMES,	/* DLT_VOLUME       */
	LDF_ASSIGNS,	/* DLT_LATE         */
	LDF_ASSIGNS,	/* DLT_NONBINDING   */
};

/* DOWNCODE! */
/* list must be locked shared or exclusive */

struct DosList * ASM
FindDosEntry (REG(d1) struct DosList *dlist, REG(d2) UBYTE *name,
	      REG(d3) ULONG flags)
{
	/* if he hasn't used NextDosEntry this is needed */
	if (((LONG) dlist) & 1)
		dlist = (struct DosList *) (((LONG) dlist) - 1);

	while (dlist)
	{
		/* if name == NULL, don't check it */
		if ((((ULONG) dlt_to_ldf[dlist->dol_Type + 1]) & flags) &&
		    (!name ||
		     (mystricmp(((char *) BADDR(dlist->dol_Name)) + 1,name) ==
		      SAME)))
		{
			/* found it! */
			break;
		}
		dlist = (struct DosList *) BADDR(dlist->dol_Next);
	}
	return dlist;
}

/* internal for ErrorReport - locks list for you */

struct DosList * __regargs
FindDosEntryByTask (struct MsgPort *port)
{
	struct DosList *dlist;

	dlist = LockDosList(LDF_ALL|LDF_READ);
	dlist = (struct DosList *) (((LONG) dlist) - 1);

	while (dlist)
	{
		if (dlist->dol_Type == DLT_DEVICE &&
		    dlist->dol_Task == port)
		{
			/* found it! */
			break;
		}
		dlist = (struct DosList *) BADDR(dlist->dol_Next);
	}

	/* leaves list locked for read!!!! */
	return dlist;
}

#define SAMEDEVICE
#ifdef SAMEDEVICE
/* return if two locks are on partitions of the same device */
LONG ASM
SameDevice (REG(d1) BPTR lock1, REG(d2) BPTR lock2)
{
	struct FileLock *lock;
	struct DosList *vol1, *vol2;
	struct DosList *dev1, *dev2;
	struct FileSysStartupMsg *fs1, *fs2;
	LONG rc = FALSE;

	lock = BADDR(lock1);
	vol1 = BADDR(lock->fl_Volume);
	lock = BADDR(lock2);
	vol2 = BADDR(lock->fl_Volume);

	if (vol1 == vol2 || vol1->dol_Task == vol2->dol_Task)
		return DOSTRUE;

	dev1 = FindDosEntryByTask(vol1->dol_Task);
	if (!dev1)
		goto cleanup2;

	dev2 = FindDosEntryByTask(vol2->dol_Task);
	if (!dev2)
		goto cleanup;

	/* we have two devices, check startups */
	/* FIX! use symbol for minimum valid startup! */
	if (dev1->dol_misc.dol_handler.dol_Startup < 64 ||
	    dev2->dol_misc.dol_handler.dol_Startup < 64)
		goto cleanup;				/* no device */

	fs1 = BADDR(dev1->dol_misc.dol_handler.dol_Startup);
	fs2 = BADDR(dev2->dol_misc.dol_handler.dol_Startup);

	if ((fs1->fssm_Unit == fs2->fssm_Unit) &&
	    (strcmp(BADDR(fs1->fssm_Device),BADDR(fs2->fssm_Device)) == SAME))
	{
		rc = DOSTRUE;
	}
cleanup:
	UnLockDosList(LDF_ALL|LDF_READ);
cleanup2:
	UnLockDosList(LDF_ALL|LDF_READ);

	return rc;
}
#endif

/* DOWNCODE! */
/* list must be locked shared or exclusive */

struct DosList * ASM
NextDosEntry (REG(d1) struct DosList *dlist, REG(d2) ULONG flags)
{
	/* if he hasn't used NextDosEntry this is needed */
	if (((LONG) dlist) & 1)
		dlist = (struct DosList *) (((LONG) dlist) - 1);
	else
		dlist = (struct DosList *) BADDR(dlist->dol_Next);

	return FindDosEntry(dlist,NULL,flags);
}

struct DosList * ASM
MakeDosEntry (REG(d1) UBYTE *name, REG(d2) LONG type)
{
	struct DosList *devslot;
	LONG len;
	char *devname;

	/* sizeof(struct DeviceList) MUST be an integral number of LONGS! */
	devslot = (struct DosList *)
		  AllocVecPubClear(sizeof(struct DosList));

	/* volumename MUST be null-terminated BSTR! */
	len = strlen(name);
	devname = AllocVecPub(len+2);		/* 1 for length, 1 for null */
	if (!devslot || !devname)
	{
		/* freeVec(0) is safe */
		freeVec(devslot);
		freeVec(devname);
		return NULL;
	}
	/* must produce null-terminated BSTR, null not included in count!!! */
	strcpy(devname+1,name);
	*devname = len;

	devslot->dol_Name = (BSTR) TOBPTR(devname);
	devslot->dol_Type = type;

	/* dol_Task = 0, which is good			*/
	/* dol_Lock = 0, which is good			*/
	/* the union is all 0, which is good		*/

	return devslot;
}

/* The entry MUST have been removed from the DosList! */

void ASM
FreeDosEntry (REG(d1) struct DosList *dlist)
{
	if (dlist) {
		freeVec(BADDR(dlist->dol_Name));
		freeVec(dlist);
	}
}

/* should really be called under forbid, so caller can set things up */
/* FIX! should be called under LockDosList! */

struct DosList * ASM
adddevice (REG(d1) char *name)		/* CSTR */
{
	struct DosList *devslot;

	devslot = MakeDosEntry(name,DLT_DEVICE);
	if (devslot)
		if (!AddDosEntry(devslot))
		{
			FreeDosEntry(devslot);
			return NULL;
		}
	return devslot;
}

/* DOWNCODE next 3! */
/* assign name: to lock.  Should kick off DISKINSERTED! FIX!  */
/* aka AssignLock */

LONG ASM
assign (REG(d1) char *name, REG(d2) BPTR lock)
{
	return assigncommon(name,lock,DLT_DIRECTORY);
}

/* assign name: to string, late-binding.  Should kick off DISKINSERTED! FIX!  */

LONG ASM
assignlate (REG(d1) char *name, REG(d2) UBYTE *string)
{
	char *str;

	str = AllocVecPub(strlen(string)+1);
	if (!str)
		return FALSE;

	strcpy(str,string);
	return assigncommon(name,(BPTR) str,DLT_LATE);
}

/* assign name: to string, non-binding.  Should kick off DISKINSERTED! FIX!  */

LONG ASM
assignstring (REG(d1) char *name, REG(d2) UBYTE *string)
{
	char *str;

	str = AllocVecPub(strlen(string)+1);
	if (!str)
		return FALSE;

	strcpy(str,string);
	return assigncommon(name,(BPTR) str,DLT_NONBINDING);
}

/* 'lock' may be CSTR for dlt_late */

LONG ASM
assigncommon (REG(d1) char *name, REG(d2) BPTR lock, REG(d3) LONG type)
{
	struct DosList *dl;
	LONG rc = FALSE;

	if (strlen(name) > 30)	/* FIX! make a constant! */
	{
		setresult2(ERROR_INVALID_COMPONENT_NAME);
		return FALSE;
	}

	dl = LockDosList(LDF_ASSIGNS|LDF_WRITE);
	dl = FindDosEntry(dl,name,LDF_ASSIGNS);
	if (dl)
	{
		/* it's an assign - undo previous value */
		if (dl->dol_Type != DLT_DIRECTORY)
			freeVec(dl->dol_misc.dol_assign.dol_AssignName);
		freeobj(dl->dol_Lock);
		freelocklist(dl->dol_misc.dol_assign.dol_List);
		dl->dol_misc.dol_assign.dol_List = NULL;

	} else {
		if (lock == NULL)
		{
			/* assign never existed or already gone */
			goto ass_success;
		}
		/* make new entry - locks nest! */
		/* adddevice will fail if it conflicts with a device/volume */
		dl = adddevice(name);
		if (!dl)
			goto ass_exit;
	}

	if (lock == NULL)
	{
		/* remove assign (list is locked) */
		if (RemDosEntry(dl))
		{
			FreeDosEntry(dl);
			goto ass_success;
		} else
			goto ass_exit;
	}

	/* got a device, node, now set it up */
	dl->dol_Type = type;

	if (type == DLT_DIRECTORY)
	{
		dl->dol_Lock = lock;
		/* apparently some people rely on dol_Task, try to help them */
		dl->dol_Task = ((struct FileLock *) BADDR(lock))->fl_Task;
	} else {
		dl->dol_Lock = NULL;
		dl->dol_Task = NULL;
		dl->dol_misc.dol_assign.dol_AssignName = (UBYTE *) lock;
	}

ass_success:
	rc = DOSTRUE;

ass_exit:
	UnLockDosList(LDF_ASSIGNS|LDF_WRITE);
	return rc;
}

void __regargs
freelocklist (struct AssignList *list)
{
	struct AssignList *next;

	while (list)
	{
		next = list->al_Next;
		freeobj(list->al_Lock);
		freeVec(list);
		list = next;
	}
}

/* only works for regular assigns!!!!! */

LONG ASM
addassign (REG(d1) char *name, REG(d2) BPTR lock)
{
	struct DosList *dl;
	LONG rc = FALSE;
	struct AssignList *al,*t;

	dl = LockDosList(LDF_ASSIGNS|LDF_WRITE);
	dl = FindDosEntry(dl,name,LDF_ASSIGNS);
	if (dl)
	{
		/* found assign of that name - is it a regular assign? */
		if (dl->dol_Type != DLT_DIRECTORY)
		{
			/* error - name refers to late or non-binding assign */
			goto ass_add_exit;
		}

		/* got it.  Add to list */
		al = AllocVecPubClear(sizeof(struct AssignList));
		if (!al)
			goto ass_add_exit;

		/* adding at the end makes more sense */
		if (!(t = dl->dol_misc.dol_assign.dol_List))
		{
			/* empty list */
			dl->dol_misc.dol_assign.dol_List = al;
		} else {
			for (/* t already set */ ; t->al_Next; t = t->al_Next)
				; /* NULL */

			t->al_Next = al;
		}
			
		/* al_Next is NULL already - allocated PubClear */
		al->al_Lock = lock;

		rc = DOSTRUE;
	}

ass_add_exit:
	UnLockDosList(LDF_ASSIGNS|LDF_WRITE);
	return rc;
}

/* remove lock for which SameLock() returns LOCK_SAME. */
LONG ASM
remaddassign (REG(d1) char *name, REG(d2) BPTR lock)
{
	struct DosList *dl;
	LONG rc = FALSE;
	struct AssignList *al,*nextal;

	dl = LockDosList(LDF_ASSIGNS|LDF_WRITE);
	dl = FindDosEntry(dl,name,LDF_ASSIGNS);
	if (dl)
	{
		/* found assign of that name - is it a regular assign? */
		if (dl->dol_Type != DLT_DIRECTORY)
		{
			/* error - name refers to late or non-binding assign */
			/* FIX!? */
			goto ass_rem_exit;
		}

		if (SameLock(dl->dol_Lock,lock) == LOCK_SAME)
		{
			// master entry is to be removed.  Morph first entry.
			freeobj(dl->dol_Lock);
			if ((al = dl->dol_misc.dol_assign.dol_List) == NULL)
			{
				if (RemDosEntry(dl))	// already locked
				{
					FreeDosEntry(dl);
					rc = DOSTRUE;
				}
			} else {
				// turn first entry on list into master
				dl->dol_Lock = al->al_Lock;
				dl->dol_misc.dol_assign.dol_List = al->al_Next;
				freeVec(al);
				rc = DOSTRUE;
			}
			goto ass_rem_exit;
		}

		/* got it.  search list for correct entry */
		al = dl->dol_misc.dol_assign.dol_List;
		if (!al)
			goto ass_rem_exit;

		if (SameLock(al->al_Lock,lock) == LOCK_SAME)
		{
			/* the first entry is to be removed.  bump others up */
			nextal = al;
			dl->dol_misc.dol_assign.dol_List = al->al_Next;
		} else {
			while (nextal = al->al_Next)
			{
				if (SameLock(nextal->al_Lock,lock) == LOCK_SAME)
				{
					al->al_Next = nextal->al_Next;
					goto got_al;
				}
				al = nextal;
			}

			/* didn't find it */
			goto ass_rem_exit;
		}

		/* nextal has node to remove */
got_al:		freeobj(nextal->al_Lock);
		freeVec(nextal);
		     
		rc = DOSTRUE;
	}

ass_rem_exit:
	UnLockDosList(LDF_ASSIGNS|LDF_WRITE);
	return rc;
}
