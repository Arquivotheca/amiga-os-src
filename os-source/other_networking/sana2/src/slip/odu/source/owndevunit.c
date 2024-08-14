/* OwnDevUnit.c

   Copyright 1991 by Christopher A. Wichura (caw@miroc.chi.il.us)
   All Rights Reserved.

   This implements a fairly simple blocking mechanism for a device/unit
   specification.

   Note that this probably won't compile unless using SAS 6.02.
   It also assumes KickStart 3.0 include files.

   This code is freely redistributable.  You may not charge anything
   for it, except for media and/or postage.  In using this code, you
   assume all responsibility for any damage, loss of productivity/money,
   or other disaster that occurs while this code is in use.  C. Wichura
   can not, and will not, be held responsible.
*/

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <devices/timer.h>
#include <rexx/errors.h>

#include <clib/exec_protos.h>
extern struct ExecBase *SysBase;
#include <pragmas/exec_lib.h>

#include <clib/rexxsyslib_protos.h>
extern struct RxsLib *RexxSysBase;
#include <pragmas/rexxsyslib_lib.h>

#include <string.h>
#include <stdlib.h>
#include <clib/alib_protos.h>

#include <clib/pools_protos.h>

LONG __stdargs kprintf(STRPTR fmt, ...);

/* we keep a list of all device/unit pairs we are currently tracking.
   a semaphore is used to prevent collisions while accessing this
   list. */
struct SignalSemaphore MasterListSema;
struct MinList MasterList;

typedef struct {
	struct MinNode	wn_Node;
	struct Task	*wn_Task;
} WaitListNode_t;

typedef struct {
	struct MinNode		mn_Node;
	ULONG			mn_SizeOf;
	struct Task		*mn_OwnerTask;
	ULONG			mn_Unit;
	UBYTE			*mn_OwnerName;
	struct MinList		mn_WaitList;
	UBYTE			mn_NotifyBit;
	UBYTE			mn_Device[1];	/* dynamically allocated */
} MasterListNode_t;

/* execbase and pool header for when we are running under v39 or higher */
struct ExecBase *SysBase;
APTR pool;

/* as a failsafe for programs that don't check their notifybit properly,
   we will re-signal them every RE_SIGNAL times through the loop that
   waits for the port to become free. */
#define RE_SIGNAL 10

/* strings that can be returned by LockDevUnit() and AttempDevUnit() to
   indicate an internal error.  Note that these all start with a
   special character. */

#define ODUERR_LEADCHAR "\x07"

#define ODUERR_NOMEM	ODUERR_LEADCHAR "Out of memory"
#define ODUERR_NOTIMER	ODUERR_LEADCHAR "Unable to open timer.device"
#define ODUERR_BADNAME	ODUERR_LEADCHAR "Bogus device name supplied"
#define ODUERR_BADBIT	ODUERR_LEADCHAR "Bogus notify bit supplied"
#define ODUERR_UNKNOWN	ODUERR_LEADCHAR "Unknown"
				/* returned if owner's name is NULL */

/* we keep track of our seglist for LibExpunge() */
ULONG LibrarySeg;

/* these are some error return values for the ARexx dispatcher */
#define WRONG_NUM_OF_ARGS ERR10_017
#define NO_MEM_FOR_ARGSTRING ERR10_003
#define NO_REXX_LIB ERR10_014

/***************************************************************************/
/***************************************************************************/
/**           Here we prototype the functions found in this code          **/
/***************************************************************************/
/***************************************************************************/

struct Library * __saveds __asm LibInit(register __d0 struct Library *LibBase,
                                        register __a0 ULONG LibSegment);
struct Library * __saveds __asm LibOpen(register __a6 struct Library *LibraryBase);
ULONG __saveds __asm LibClose(register __a6 struct Library *LibraryBase);
ULONG __saveds __asm LibExpunge(register __a6 struct Library *LibraryBase);

UBYTE * __saveds __asm LockDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit, register __a1 UBYTE *OwnerName,
	register __d1 UBYTE NotifyBit);
UBYTE * __saveds __asm AttemptDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit, register __a1 UBYTE *OwnerName,
	register __d1 UBYTE NotifyBit);
void __saveds __asm FreeDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit);
void __saveds __asm NameDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit, register __a1 UBYTE *OwnerName);
BOOL __saveds __asm AvailDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit);
UBYTE *__saveds __asm OwnerDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit);
ULONG __saveds __asm RexxQuery(register __a0 struct RexxMsg *RMsg,
			      register __a1 UBYTE **ReturnArgString);

UBYTE *DoDevUnit(UBYTE *Device, ULONG Unit, UBYTE *OwnerName,
	UBYTE NotifyBit, ULONG AttemptMaximum, ULONG AttemptDelay);

MasterListNode_t *FindMasterNode(UBYTE *Device, ULONG Unit);
MasterListNode_t *CreateMasterNode(UBYTE *Device, ULONG Unit);
void __inline SetMasterNode(MasterListNode_t *Node, UBYTE *OwnerName, UBYTE NotifyBit);
UBYTE * __inline BaseName(UBYTE *Path);

BOOL OpenTimer(struct timerequest *TimeRequest, struct MsgPort *ReplyPort);
void CloseTimer(struct timerequest *TimeRequest, struct MsgPort *ReplyPort);
void __inline Sleep(struct timerequest *TimeRequest, struct MsgPort *ReplyPort, ULONG Seconds);

/***************************************************************************/
/***************************************************************************/
/**     These are the library base routines used to open/close us, etc    **/
/***************************************************************************/
/***************************************************************************/

struct Library * __saveds __asm LibInit(register __d0 struct Library *LibBase,
                                        register __a0 ULONG LibSegment)
{
#ifdef DEBUG
	kprintf("LibInit called Base = %08lx\tSegment = %08lx\n", LibBase, LibSegment);
#endif
	SysBase = *(struct ExecBase **)4L;
	LibrarySeg = LibSegment;

	NewList((struct List *)&MasterList);
	InitSemaphore(&MasterListSema);

		if (!(pool = CreatePool(MEMF_PUBLIC | MEMF_CLEAR, 4096, 2048))) {
#ifdef DEBUG
	kprintf("Couldn't create memory pool\n");
#endif
			 return 0L;
		}

	return LibBase;
}

struct Library * __saveds __asm LibOpen(register __a6 struct Library *LibraryBase)
{
#ifdef DEBUG
	kprintf("LibOpen called");
#endif
	LibraryBase->lib_OpenCnt++;
	LibraryBase->lib_Flags &= ~LIBF_DELEXP;

#ifdef DEBUG
	kprintf(" OpenCount = %ld\n", LibraryBase->lib_OpenCnt);
#endif
	return LibraryBase;
}

ULONG __saveds __asm LibClose(register __a6 struct Library *LibraryBase)
{
#ifdef DEBUG
	kprintf("LibClose called");
#endif
	if (--LibraryBase->lib_OpenCnt == 0)
		if (LibraryBase->lib_Flags & LIBF_DELEXP) {
#ifdef DEBUG
			kprintf("\n");
#endif
			return LibExpunge(LibraryBase);
		}

#ifdef DEBUG
	kprintf(" OpenCount = %ld\n", LibraryBase->lib_OpenCnt);
#endif
	return (ULONG) NULL;
}

ULONG __saveds __asm LibExpunge(register __a6 struct Library *LibraryBase)
{
	BOOL	KillLib;
	LONG	LibSize;

#ifdef DEBUG
	kprintf("LibExpunge called\n");
#endif

	KillLib = FALSE;

	/* we will refuse to expunge if we are currently tracking devices,
	   regardless of our OpenCnt.  We AttemptSemaphore() the MasterList
	   before checking it.  If someone else owns it then obviously we
	   can't try to shut down. */

	if (LibraryBase->lib_OpenCnt == 0 && AttemptSemaphore(&MasterListSema)) {
#ifdef DEBUG
		kprintf("LIBEXP: Checking if MasterList is empty\n");
#endif
		if (MasterList.mlh_TailPred == (struct MinNode *)&MasterList.mlh_Head) {
			Remove((struct Node *)LibraryBase);
			KillLib = TRUE;
		}

		ReleaseSemaphore(&MasterListSema);
	}

	if (KillLib) {
#ifdef DEBUG
		kprintf("LIBEXP: Removing this library!\n");
#endif
		DeletePool(pool);

		LibSize = LibraryBase->lib_NegSize + LibraryBase->lib_PosSize;
		FreeMem((char *)LibraryBase - LibraryBase->lib_NegSize, LibSize);

		return LibrarySeg;
	}

#ifdef DEBUG
	kprintf("LIBEXP: Can't remove library, setting LIBF_DELEXP\n");
#endif
	LibraryBase->lib_Flags |= LIBF_DELEXP;
	return (ULONG) NULL;
}

/***************************************************************************/
/***************************************************************************/
/**       Now we have the routines that do smart memory allocation        **/
/***************************************************************************/
/***************************************************************************/

 /*
  * these routines will use pooled memory under V39 or higher or standard
  * memory routines when running under earlier versions of the OS. We lock
  * MasterListSema as a means to prevent multiple people from stomping on the
  * pool handle concurrently.
  */

static void *__inline MyAllocVec(ULONG size)
{
	UBYTE *memory;

	size += 4;

	ObtainSemaphore(&MasterListSema);
	memory = AllocPooled(pool, size);
	ReleaseSemaphore(&MasterListSema);

	if (!memory)
		return NULL;

	*((ULONG *) memory) = size;

	return (void *)(memory + 4);
}

static void __inline MyFreeVec(void *memory)
{
	void *realMemory;
	ULONG size;

	if (!memory)
		return;

	realMemory = (UBYTE *) memory - 4;
	size = *((ULONG *) realMemory);

	ObtainSemaphore(&MasterListSema);
	FreePooled(pool, realMemory, size);
	ReleaseSemaphore(&MasterListSema);
}

/***************************************************************************/
/***************************************************************************/
/**       Now we have the routines that acutally make up the library      **/
/***************************************************************************/
/***************************************************************************/

/* LockDevUnit() will block until it gets hold of the requested
   device and unit
*/

UBYTE * __saveds __asm LockDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit, register __a1 UBYTE *OwnerName,
	register __d1 UBYTE NotifyBit)
{
	return DoDevUnit(Device, Unit, OwnerName, NotifyBit, -1L, 3L);
}

/* AttemptDevUnit() will try to grab the requested device and unit.
   We allow up to five seconds for an owner of the lock to let it
   go before we return a failure.
*/

UBYTE *__saveds __asm AttemptDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit, register __a1 UBYTE *OwnerName,
	register __d1 UBYTE NotifyBit)
{
	return DoDevUnit(Device, Unit, OwnerName, NotifyBit, 5L, 1L);
}

/* FreeDevUnit() releases a lock on a device/unit that was previously
   attained by a call to LockDevUnit() or AttempDevUnit()

   if there is stuff hanging off the node's waitlist then we just
   null the owner task field so that LockDevUnit() and AttemptDevUnit()
   know that the port is free.  if there isn't anything on the waitlist
   then no one else wants this node so remove the node from the master
   list and free the memory used by it.
*/

void __saveds __asm FreeDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit)
{
	MasterListNode_t *Master;

#ifdef DEBUG
	kprintf("FreeDevUnit called \"%s\" %ld\n", Device, Unit);
#endif

	if (!Device)
		return;

	ObtainSemaphore(&MasterListSema);

	if (Master = FindMasterNode(Device, Unit)) {
#ifdef DEBUG
		kprintf("FDU: Master = %08lx\n", Master);
#endif
		if (Master->mn_OwnerTask == FindTask(0L)) {
#ifdef DEBUG
			kprintf("FDU: Master->mn_OwnerTask matches\n");
#endif
			if (Master->mn_WaitList.mlh_TailPred == (struct MinNode *)
			   &Master->mn_WaitList.mlh_Head) {
#ifdef DEBUG
				kprintf("Removing this Master from MasterList\n");
#endif
				Remove((struct Node *)Master);
				MyFreeVec((char *)Master);
			} else {
#ifdef DEBUG
				kprintf("Setting Master->mn_OwnerTask to NULL\n");
#endif
				Master->mn_OwnerTask = (struct Task *)NULL;
				Master->mn_OwnerName = (UBYTE *)NULL;
			}
		}
	}

	ReleaseSemaphore(&MasterListSema);
	return;
}

/* NameDevUnit() will allow the owner of a node to change the owner
   name that is returned when an AttemptDevUnit() times out and fails.
   Useful for programs that sit on the modem and start others when
   someone calls in (i.e., "Getty" becomes "Getty started <program name>").
*/

void __saveds __asm NameDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit, register __a1 UBYTE *OwnerName)
{
	MasterListNode_t *Master;

#ifdef DEBUG
	kprintf("NameDevUnit called \"%s\" %ld \"%s\"\n", Device,
	    Unit, OwnerName);
#endif

	if (!Device)
		return;

	ObtainSemaphore(&MasterListSema);

	if (Master = FindMasterNode(Device, Unit)) {
		if (Master->mn_OwnerTask == FindTask(0L)) {
#ifdef DEBUG
			kprintf("NDU: Master owned by this task.  Setting name\n");
#endif
			Master->mn_OwnerName = OwnerName;
		}
	}

	ReleaseSemaphore(&MasterListSema);
}

/* AvailDevUnit() will return the availability status of a node very
   quickly.  This may be preferable over AttemptDevUnit(), which can
   take up 5 seconds.
*/

BOOL __saveds __asm AvailDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit)
{
	MasterListNode_t *Master;

	if (!Device)
		return FALSE;

#ifdef DEBUG
	kprintf("AvailDevUnit called \"%s\" %ld\n", Device, Unit);
#endif
	ObtainSemaphore(&MasterListSema);

	/* we simply check to see if a node for this device/unit
	   exists.  if it does, we return FALSE (not available)
	   because it is either owned or someone else will camp
	   on it very soon. */
	Master = FindMasterNode(Device, Unit);

	ReleaseSemaphore(&MasterListSema);

	if (Master)
		return FALSE;
	else
		return TRUE;
}

/* OwnerDevUnit() will return the name of the current owner of a particular
   lock.  If the device/unit pair isn't locked, we return ODUERR_UNKNOWN */

UBYTE *__saveds __asm OwnerDevUnit(register __a0 UBYTE *Device,
	register __d0 ULONG Unit)
{
	MasterListNode_t *Master;
	STRPTR Owner = NULL;

	if (!Device)
		return ODUERR_UNKNOWN;

#ifdef DEBUG
	kprintf("OwnerDevUnit called \"%s\" %ld\n", Device, Unit);
#endif
	ObtainSemaphore(&MasterListSema);

	Master = FindMasterNode(Device, Unit);

	if (Master)
		Owner = Master->mn_OwnerName;

	ReleaseSemaphore(&MasterListSema);

	if (Owner)
		return Owner;
	else
		return ODUERR_UNKNOWN;
}

/* RexxQuery function handles ARexx requests when OwnDevUnit.library has
   been added as an ARexx function host */

ULONG __saveds __asm RexxQuery(register __a0 struct RexxMsg *RMsg,
			      register __a1 UBYTE **ReturnArgString)
{
	UBYTE *Error = NULL;
	UBYTE NumArgs;
	BOOL  DidAFunc = FALSE;
	BOOL  WeDidALock = FALSE;
	BOOL  NoRexxLib = FALSE;

	ULONG Unit;
	UBYTE NotifyBit;

	struct RxsLib *RexxSysBase;

#ifdef DEBUG
	kprintf("RexxQuery called\n");
#endif

	/* set ReturnArgString to NULL so that if we return early
	   ARexx will know it doesn't point at anything */
	*ReturnArgString = NULL;

	/* grab the number of arguments they passed us */
	NumArgs = RMsg->rm_Action & 0xFF;

	/* now look and see if this is a function we can do.  Note that
	   we don't use a lookup table here as we have very few funcs
           to implement and this is easier... signed, his laziness */

	if (!stricmp(RMsg->rm_Args[0], "LockDevUnit")) {
		if (NumArgs == 3) {
			/* this is a LockDevUnit call */
			Unit = atoi(RMsg->rm_Args[2]);
			NotifyBit = atoi(RMsg->rm_Args[3]);
			Error = LockDevUnit(RMsg->rm_Args[1], Unit,
					    "ARexx", NotifyBit);
			WeDidALock = DidAFunc = TRUE;
		} else
			return WRONG_NUM_OF_ARGS;
	} else
	if (!stricmp(RMsg->rm_Args[0], "AttemptDevUnit")) {
		if (NumArgs == 3) {
			/* this is an AttemptDevUnit call */
			Unit = atoi(RMsg->rm_Args[2]);
			NotifyBit = atoi(RMsg->rm_Args[3]);
			Error = AttemptDevUnit(RMsg->rm_Args[1], Unit,
					       "ARexx", NotifyBit);
			WeDidALock = DidAFunc = TRUE;
		} else
			return WRONG_NUM_OF_ARGS;
	} else
	if (!stricmp(RMsg->rm_Args[0], "FreeDevUnit")) {
		if (NumArgs == 2) {
			/* this is a FreeDevUnit call */
			Unit = atoi(RMsg->rm_Args[2]);
			FreeDevUnit(RMsg->rm_Args[1], Unit);
			DidAFunc = TRUE;
		} else
			return WRONG_NUM_OF_ARGS;
	} else
	if (!stricmp(RMsg->rm_Args[0], "AvailDevUnit")) {
		if (NumArgs == 2) {
			/* this is an AvailDevUnit call */
			Unit = atoi(RMsg->rm_Args[2]);
			if (AvailDevUnit(RMsg->rm_Args[1], Unit))
				Error = "1";
			else
				Error = "0";
			DidAFunc = TRUE;
		} else
			return WRONG_NUM_OF_ARGS;
	} else
	if (!stricmp(RMsg->rm_Args[0], "OwnerDevUnit")) {
		if (NumArgs == 2) {
			/* this is an OwnerDevUnit call */
			Unit = atoi(RMsg->rm_Args[2]);
			Error = OwnerDevUnit(RMsg->rm_Args[1], Unit);
			DidAFunc = TRUE;
		} else
			return WRONG_NUM_OF_ARGS;
	}

	if (!DidAFunc)
		return 1L;	/* not a function we recognize */

	/* if we get here then we did a function.  try an allocate an
	   argstring to return the result string in.  if that fails
	   then check if we did a lock.  if we did, and it was succesful
	   then we want to free the lock before returning an error saying
	   we couldn't allocate the argstring. */

	/* try and get rexxsyslib.library */
	if (RexxSysBase = (struct RxsLib *)OpenLibrary("rexxsyslib.library", 0)) {
		{
			ULONG Length;

			if (Error)
				Length = strlen(Error);
			else
				Length = 0;

			*ReturnArgString = CreateArgstring(Error, Length);
		}

		CloseLibrary((struct Library *)RexxSysBase);

		if (*ReturnArgString)
			return 0L;	/* no low level error so return argstring */
	} else
		NoRexxLib = TRUE;

	/* if we did a lock successfully then free it before returning */
	if (WeDidALock && Error == NULL)
		FreeDevUnit(RMsg->rm_Args[1], Unit);

	if (NoRexxLib)
		return NO_REXX_LIB;
	else
		return NO_MEM_FOR_ARGSTRING;
}

/***************************************************************************/
/***************************************************************************/
/**     These are the support routines used by the top level functions    **/
/***************************************************************************/
/***************************************************************************/

/* DoDevUnit() is by in large the most important support routine.  it is
   what actually handles the LockDevUnit() and AttemptDevUnit() requests.
   The difference in the above two calls is the MaxAttempts and
   AttemptDelay that they use.

   LockDevUnit() passes -1 in MaxAttempts.  It would take near eternity
   to reach $FFFFFFFF so LockDevUnit() basically blocks forever.  It uses
   an AttemptDelay of 3 seconds to keep from hammering the list constantly.

   AttemptDevUnit() passes 5 in MaxAttempts.  Since AttemptDelay is 1
   second for AttemptDevUnit(), this effectively means that this call will
   wait no longer than five seconds for a lock and will return the name
   of the current owner if it can't get it within time.
*/

UBYTE *DoDevUnit(UBYTE *Device, ULONG Unit, UBYTE *OwnerName,
	UBYTE NotifyBit, ULONG MaxAttempts, ULONG AttemptDelay)
{
	MasterListNode_t *Master;
	WaitListNode_t WaitNode;

	struct MsgPort ReplyPort;
	struct timerequest TimeRequest;

	UBYTE AttemptNumber;
	UBYTE ReSignalCount;

	UBYTE *Error = NULL;

#ifdef DEBUG
	kprintf("DoDevUnit called \"%s\" %ld \"%s\" %ld %ld %ld\n",
	    Device, Unit, OwnerName, NotifyBit, MaxAttempts, AttemptDelay);
#endif

	if (!Device || !*BaseName(Device))
		return ODUERR_BADNAME;

	if (NotifyBit == -1)
		return ODUERR_BADBIT;

	ObtainSemaphore(&MasterListSema);

	/* if the node doesn't exist then we try and create it */
	if (!(Master = FindMasterNode(Device, Unit))) {
		if (Master = CreateMasterNode(Device, Unit))
			SetMasterNode(Master, OwnerName, NotifyBit);
		else
			Error = ODUERR_NOMEM;

		ReleaseSemaphore(&MasterListSema);

		return Error;
	}

	/* if we already own this task then simply return that the
	   device is successfully locked.  */
	if (Master->mn_OwnerTask == FindTask(0L)) {
#ifdef DEBUG
		kprintf("DDU: Task already owns this Master\n");
#endif
		ReleaseSemaphore(&MasterListSema);
		return (UBYTE *)NULL;
	}

	/* we are going to need the timer to do things right so open
	   it up.  If we can't init the timer for use then return
	   the appropriate lock failed message */
	if (!OpenTimer(&TimeRequest, &ReplyPort)) {
		ReleaseSemaphore(&MasterListSema);
		return ODUERR_NOTIMER;
	}

	/* setup up our wait node and add it onto the wait list */
	WaitNode.wn_Task = FindTask(0L);
	AddTail((struct List *)&Master->mn_WaitList, (struct Node *)&WaitNode);

	/* if the current owner has provided a NotifyBit then we
	   signal the task to let it know someone wants the list */
	if (Master->mn_OwnerTask && Master->mn_NotifyBit) {
#ifdef DEBUG
		kprintf("DDU: Notifying %08lx on SigBit %ld\n",
		    Master->mn_OwnerTask, Master->mn_NotifyBit);
#endif
		Signal(Master->mn_OwnerTask, 1L << Master->mn_NotifyBit);
	}

	/* now we wait for the node to become free (mn_OwnerTask = NULL)
	   and then check to see if we are the head of the wait list.
	   if we are the head then we can own the device.  else someone
	   else gets to use it before us so continue waiting.

	   we must release the MasterListSema and wait before checking
	   or the owner will never be able to unlock the node! */
	ReleaseSemaphore(&MasterListSema);

	Sleep(&TimeRequest, &ReplyPort, 1L);	/* our initial wait is very short */

	for (AttemptNumber = ReSignalCount = 0; AttemptNumber < MaxAttempts;
	    ReSignalCount++, AttemptNumber++) {
		ObtainSemaphore(&MasterListSema);

		if (Master->mn_OwnerTask) {	/* still owned by someone */
			if (ReSignalCount > RE_SIGNAL) {
				if (Master->mn_NotifyBit)
					Signal(Master->mn_OwnerTask,
					    1L << Master->mn_NotifyBit);
				ReSignalCount = 0;
			}

			ReleaseSemaphore(&MasterListSema);
			Sleep(&TimeRequest, &ReplyPort, AttemptDelay);
			continue;
		}

		/* device is free.  if we aren't the head of the wait
		   list we must let someone else go first */
		if (Master->mn_WaitList.mlh_Head != (struct MinNode *)&WaitNode) {
			ReleaseSemaphore(&MasterListSema);
			Sleep(&TimeRequest, &ReplyPort, AttemptDelay);
			continue;
		}

		/* we can own the node!  yeah! */
		RemHead((struct List *)&Master->mn_WaitList);
		SetMasterNode(Master, OwnerName, NotifyBit);

		/* if this new owner supplied a NotifyBit and there are
		   other people waiting on this node then it is only
		   fair to trigger the NotifyBit. */
		if (Master->mn_WaitList.mlh_TailPred != (struct MinNode *)
		   &Master->mn_WaitList.mlh_Head)
			if (NotifyBit)
				Signal(FindTask(0L), 1L << NotifyBit);

		/* finally, shut things down and return a successful lock */
		ReleaseSemaphore(&MasterListSema);
		CloseTimer(&TimeRequest, &ReplyPort);

		return (UBYTE *)NULL;
	}

	/* if we get here then someone else has the device and refuses to
	   give it up.  so we return a pointer to the owner's name.
           we also need to remove our WaitNode from the Master's waitlist. */

	ObtainSemaphore(&MasterListSema);

	Remove((struct Node *)&WaitNode);

	Error = Master->mn_OwnerName;

	if (!Error)
		Error = ODUERR_UNKNOWN;

	ReleaseSemaphore(&MasterListSema);
	CloseTimer(&TimeRequest, &ReplyPort);

	return Error;
}

/* attempt to find a node in the master list.  returns NULL if not found.
   does not arbitrate for access to the master list.  This must be done
   by the caller! */
MasterListNode_t *FindMasterNode(UBYTE *Device, ULONG Unit)
{
	MasterListNode_t *MasterNode;

	Device = BaseName(Device);

	for (MasterNode = (MasterListNode_t *)MasterList.mlh_Head;
	    MasterNode->mn_Node.mln_Succ; MasterNode = (MasterListNode_t *)
	    MasterNode->mn_Node.mln_Succ) {

#ifdef DEBUG
		kprintf("FMN: compare \"%s\" %ld with requested \"%s\" %ld\n",
		    MasterNode->mn_Device, MasterNode->mn_Unit, Device, Unit);
#endif
		if (strcmp(MasterNode->mn_Device, Device) == 0 &&
		    MasterNode->mn_Unit == Unit) {
#ifdef DEBUG
			kprintf("FMN: Match found Master = %08lx\n", MasterNode);
#endif
			return MasterNode;
		}
	}

	return (MasterListNode_t *)NULL;
}

/* this will try to create a new MasterListNode.  returns NULL if it
   fails (mainly due to an out of memory error).  does not arbitrate
   for access to the master list */
MasterListNode_t *CreateMasterNode(UBYTE *Device, ULONG Unit)
{
	MasterListNode_t *MasterNode;
	ULONG DevNameSize;
	ULONG AllocSize;

#ifdef DEBUG
	kprintf("CreateMasterNode called \"%s\" %ld\n", Device, Unit);
#endif

	/* failsafe:  if the node already exists then return a
	   pointer to it instead of creating a new one. */

	if (MasterNode = FindMasterNode(Device, Unit))
		return MasterNode;

	/* only store the basename of the device */
	Device = BaseName(Device);

	/* alloc some memory to hold the node.  return NULL if the
	   alloc fails. we copy the device name onto the end of the
	   node so that an original owner can quit and we don't
	   lose the device name */

	DevNameSize = strlen(Device);
	AllocSize = sizeof(MasterListNode_t) + DevNameSize;

#ifdef DEBUG
	kprintf("CMN: DevNameSize = %ld\tAllocSize = %ld\n", DevNameSize, AllocSize);
#endif

	if (!(MasterNode = (MasterListNode_t *)MyAllocVec(AllocSize)))
		return (MasterListNode_t *)NULL;

	/* copy device name over and fill in a few fields */

	MasterNode->mn_SizeOf = AllocSize;
	MasterNode->mn_Unit = Unit;
	CopyMem(Device, MasterNode->mn_Device, DevNameSize + 1);
	NewList((struct List *)&MasterNode->mn_WaitList);

	/* add the master node to the master list */
	AddTail((struct List *)&MasterList, (struct Node *)MasterNode);

	/* return a pointer to the newly added node */

#ifdef DEBUG
	kprintf("CMN: Master %08lx created\n", MasterNode);
#endif

	return MasterNode;
}

void __inline SetMasterNode(MasterListNode_t *Node, UBYTE *OwnerName, UBYTE NotifyBit)
{
#ifdef DEBUG
	kprintf("SetMasterNode called Master %08lx \"%s\" %ld\n", Node, OwnerName, NotifyBit);
#endif
	
	Node->mn_OwnerTask = FindTask(0L);
	Node->mn_OwnerName = OwnerName;
	Node->mn_NotifyBit = NotifyBit;
}

/* find the filename part of a full path spec */
UBYTE * __inline BaseName(UBYTE *Path)
{
	register UBYTE *Ptr;
	register UBYTE Char;

	Ptr = Path;

	while (Char = *Ptr++)
		if (Char == '/' || Char == ':')
			Path = Ptr;

	return Path;
}

/***************************************************************************/
/***************************************************************************/
/**        These routines manage the timer.device for doing delays        **/
/***************************************************************************/
/***************************************************************************/

BOOL OpenTimer(struct timerequest *TimeRequest, struct MsgPort *ReplyPort)
{
	ULONG SigBit;

	memset((char *)TimeRequest, 0, sizeof(struct timerequest));
	memset((char *)ReplyPort, 0, sizeof(struct MsgPort));

	if ((SigBit = AllocSignal(-1L)) == -1L)
		return FALSE;

	ReplyPort->mp_Node.ln_Type = NT_MSGPORT;

	ReplyPort->mp_Flags   = PA_SIGNAL;
	ReplyPort->mp_SigBit  = SigBit;
	ReplyPort->mp_SigTask = FindTask(0L);

	NewList(&(ReplyPort->mp_MsgList));

	if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)
	    TimeRequest, 0)) {
		FreeSignal(SigBit);
		ReplyPort->mp_SigTask         = (struct Task *) -1;
		ReplyPort->mp_MsgList.lh_Head = (struct Node *) -1;
		return FALSE;
	}

	TimeRequest->tr_node.io_Message.mn_ReplyPort = ReplyPort;

	return TRUE;
}

void CloseTimer(struct timerequest *TimeRequest, struct MsgPort *ReplyPort)
{
	CloseDevice((struct IORequest *)TimeRequest);
	FreeSignal(ReplyPort->mp_SigBit);
	ReplyPort->mp_SigTask         = (struct Task *) -1;
	ReplyPort->mp_MsgList.lh_Head = (struct Node *) -1;
}

void __inline Sleep(struct timerequest *TimeRequest, struct MsgPort *ReplyPort, ULONG Seconds)
{
	TimeRequest->tr_node.io_Command = TR_ADDREQUEST;
	TimeRequest->tr_time.tv_secs = Seconds;
	TimeRequest->tr_time.tv_micro = 0;

	TimeRequest->tr_node.io_Message.mn_ReplyPort = ReplyPort;

	DoIO((struct IORequest *)TimeRequest);
}
