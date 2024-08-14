/* blib.c */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <utility/tagitem.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/exall.h"
#include "dos/dostags.h"
#include "dos/rdargs.h"
#include <intuition/intuition.h>
#include <devices/timer.h>

#include <string.h>

#include "libhdr.h"
#include "fault.h"
#include "dos_private.h"

#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "device_protos.h"
#include "blib_protos.h"
#include "klib_protos.h"
#include "crp_protos.h"

#define SAME		0			/* for string compares */
#define TOBPTR(x)	(((LONG) (x)) >> 2)	/* convert to bptr */

/* for doing __asm stuff */
#define ASM __asm
#define REG(x)	register __ ## x	/* ANSI concatenation */


/* include code for dealing with not being booted for testing */
/*#define NOT_REAL*/


/* // Print standard fault message */
/* aka PrintFault */

LONG ASM
fault (REG(d1) LONG code, REG(d2) UBYTE *header)
{
	char *string;
	LONG rc = DOSTRUE;

	if (code)
	{
		if (header)
			writef("%s: ",(LONG) header);

		string = getstring(code);
		if (string)
			writes(string);
		else {
			writes(getstring(STR_ERROR));  /* "Error " */
			writen(code);
			rc = FALSE;
		}
		newline();

		/* restore result, probably not needed */
		setresult2(code);
	}
	return rc;
}

LONG ASM
Fault (REG(d1) LONG code, REG(d2) UBYTE *header, REG(d3) UBYTE *buffer,
       REG(d4) LONG len)
{
	char *string;
	UBYTE *buf = buffer;	/* local copies - we take it's address */
	LONG l = len;
	char number[10];

	if (code)
	{
		if (header)
		{
			cpymax(&buf,header,&l);
			cpymax(&buf,": ",&l);
		}

		string = getstring(code);
		if (string)
			cpymax(&buf,string,&l);
		else {
			mysprintf(number,"%ld",code);
			cpymax(&buf,getstring(STR_ERROR),&l);  /* "Error " */
			cpymax(&buf,number,&l);
		}
	}

	return len-l;
}

/* // A version of LOADSEG which does not put up requestors */

BPTR ASM
noreqloadseg (REG(d1) char *name)
{
	BPTR seg;
	struct Process *tid;
	APTR wp;

	tid = MYPROC;
	wp  = tid->pr_WindowPtr;
	tid->pr_WindowPtr = (APTR) -1L;

	seg = loadseg(name);

	tid->pr_WindowPtr = wp;

	return seg;
}

/*
// Return TRUE if real error; FALSE if we must retry
// Passed a type code and then either a stream, a task or a lock
// depending on the value of type.

// We construct a message of the following form:

//          Disk volume
//          XXX
//          is full
*/

/*
 * dev should be found for Device Not Mounted (if type is report stream),
 * for Not A Dos Disk, for No Disk Present, for No Disk, and for Abort Busy.
 * if it is not found, <unknown> will be displayed. (?)
 *
 * Note that pr_Result2 has the error code to be handled!
 */

LONG ASM
err_report (REG(d1) LONG type, REG(d2) BPTR arg,
	    REG(d3) struct MsgPort *arg2)
{
	/* ErrorReport resets result2 */
	return ErrorReport(getresult2(),type,arg,arg2);
}

/* FIX! need to add "more info" */

LONG ASM
ErrorReport (REG(d1) LONG code,
	     REG(d2) LONG type,
	     REG(d3) BPTR arg,	    /* arg can be fh or lock or volumenode */
	     REG(d4) struct MsgPort *arg2) /* arg2 is always port if passed */
{
	struct DosList *dev;	  /* ptr to device - see above */
	char *s1;
	char *s2;
	char *s3;
	LONG s1code = STR_VOLUME, s3code = 0;	/* "Volume" */
	char unitname[32];
	char *exclam = "";
	char *string;
	LONG fh_arg1 = NULL;		/* was called "cobase" */
	struct DosList *vol = NULL;	/* to volumenode */
	struct MsgPort *task = NULL;
	LONG unit = 0;
	LONG rc,ptr;
	LONG shutdisk = TRUE;

/* s3code must ALWAYS be set unless you make sure unit < 0! (errx) */

	switch (code) {
	case ERROR_DISK_NOT_VALIDATED:
		s3code = STR_IS_NOT_VALIDATED;		/* "is not validated" */
		break;
	case ERROR_DISK_WRITE_PROTECTED:
		s3code = STR_IS_WRITE_PROTECTED;        /*"is write protected"*/
		break;
	case ERROR_DISK_FULL:
		s3code = STR_IS_FULL;			/* "is full" */
		break;
	case ERROR_DEVICE_NOT_MOUNTED:
		if (type == REPORT_INSERT)
			s1code = STR_PLEASE_INSERT;
		else
			s1code = STR_PLEASE_REPLACE;
		s3code = STR_IN_ANY_DRIVE;		/* "in any drive" */
		if (type == REPORT_STREAM)
errx:			unit = -1;
		break;
	case ERROR_NOT_A_DOS_DISK:
		s1code = STR_NOT_A_DOS_DISK;		/* "Not a DOS disk" */
		goto errx;
	case ERROR_NO_DISK:
		s1code = STR_NO_DISK_PRESENT;		/* "No disk present" */
		goto errx;
	case ABORT_DISK_ERROR:
		s3code = STR_HAS_RW_ERROR;	 /* "has a read/write error" */
		goto erry;
	case ABORT_BUSY:
		s1code = STR_MUST_REPLACE;	 /* "You MUST replace volume"*/
		unit = -1;
		exclam = "!!!";
erry:		shutdisk = FALSE;	/* don't do a flush!! */
		break;
	default:
		return DOSTRUE;
	}

	s1 = getstring(s1code);
	s2 = NULL;
	if (s3code)
		s3 = getstring(s3code);

	if (type != REPORT_INSERT)
		arg <<= 2;	/* real ptr - lock or fh or volumenode */

	switch (type) {
	case REPORT_STREAM:
		/* volume from handler */
		task    = ((struct FileHandle *) arg)->fh_Type;
		fh_arg1 = ((struct FileHandle *) arg)->fh_Arg1;

/*??????? FIX!!!!!!!! ??????? */
/*
		((struct FileHandle *) arg)->fh_End   = NULL;
		((struct FileHandle *) arg)->fh_Pos   = NULL;
		((struct FileHandle *) arg)->fh_Func2 = NULL;
*/
		break;

	case REPORT_LOCK:
		if (arg == NULL || unit < 0)	/* Root dir, or unit needed */
			task = arg2;
		else {
			vol = (struct DosList *)
			      BADDR(((struct FileLock *) arg)->fl_Volume);
			task = ((struct FileLock *) arg)->fl_Task;
		}
		break;

	case REPORT_VOLUME:
		vol = (struct DosList *) arg;	/* already CPTR */
		break;

	case REPORT_INSERT:
		/* split off the volume name */
		if (!arg)
			return DOSTRUE;

		/* 30 character name plus NULL   - FIX! MAXFILENAME+1 */
		ptr = splitname((char *) arg,':',unitname,0,31);
		if (ptr == -1)
			s2 = (char *) arg;	/* no ':' */
		else
			s2 = unitname;

	} /* switch */

	/* Find out more about the volume we are worried about */
	if (vol == NULL && task != NULL)
	{
		vol = (struct DosList *)
		      BADDR(sendpkt1(task,ACTION_CURRENT_VOLUME,fh_arg1));
	}

	if (vol)
		s2 = ((char *) BADDR(vol->dol_Name)) + 1;
	/* all device/volume names are null-terminated BSTRs! */

	if (unit < 0)
	{
/*    // when the type is report.volume we are being called
      // from the file handler. So pick up volume number
      // from the FH global.
      // DEVICE NAMES MUST BE NULL-TERMINATED!

	 used to be:	GLOBAL $( unit_no = ug + 1 *)
*/

		string = getstring(STR_IN_UNIT);	/* "in unit %ld%s" */

		/* try to get real name for device */
		if (type == REPORT_VOLUME || task != NULL)
		{
			/* get the device node for the item */
			dev = FindDosEntryByTask(task ? task : vol->dol_Task);
			/* leaves the list locked!! */

			/* get the devname */
			if (dev)
			{
				string = getstring(STR_IN_DEVICE);
							/* "in device %s%s" */
				unit = ((LONG) BADDR(dev->dol_Name)) + 1;
			} else {
				if (task)
					unit = getresult2();
				else
					/* stupid BCPL handler! */
					/* get global #151 from private gvec */
					unit = ((LONG *) 
						MYPROC->pr_GlobVec)[UG+1];
			}

			UnLockDosList(LDF_ALL|LDF_READ);
		} else {
			unit = getresult2();
		}

		mysprintf(unitname,string,unit,exclam);
		s3 = unitname;
	}

	/* Issue the requester */
	if (shutdisk)
		closedisk(task);

	rc = requester2(s1,s2,s3,type == REPORT_VOLUME ? 0 : DISKINSERTED);

	/* Make _certain_ that error code is restored! */
	setresult2(code);

	/* very important this be false or -1 */
	return (rc ? FALSE : DOSTRUE);
}

/* used to flush a drive - usually before asking the user to remove a disk */

void __regargs
closedisk (task)
	struct MsgPort *task;
{
	/* Close down the drive (or at least attempt to) */
	if (task == NULL)
		task = filesystemtask();
	sendpkt0(task,ACTION_FLUSH);
}

/* for BCPL */

void ASM
endtask (REG(d1) BPTR seg)
{
	Forbid();
	unloadseg(seg);
	RemTask(0L);
/* 	abort(AN_EndTask|AT_DeadEnd); */
}

/* MUST be callable from a task!!! */

void ASM
dotimer (REG(d1) LONG request,
	 REG(d2) struct timerequest *iob,
	 REG(d3) LONG secs,
	 REG(d4) LONG micro)
{
	/* copy the request */
	*iob = *(opendosbase()->dl_TimeReq);

	/* set replyport to my process reply port */
	iob->tr_node.io_Message.mn_ReplyPort = 
				&(((struct Process *) FindTask(0))->pr_MsgPort);

	iob->tr_node.io_Command = request;
	iob->tr_time.tv_micro   = micro;
	iob->tr_time.tv_secs    = secs;

	DoIO((struct IORequest *) iob);
}

/* is callable from a task!!! */

void ASM
delay (REG(d1) ULONG ticks)
{
	struct timerequest iob;
	LONG quot,rem;

	if (ticks) {
		quot = divrem32(ticks,TICKS_PER_SECOND,&rem);
		dotimer(TR_ADDREQUEST,&iob,quot,
			rem * (1000000/TICKS_PER_SECOND));
	}
}

/*
    // Takes a device name string and returns the task number of the
    // corresponding device handler. The device name may be a filing
    // system directory, in which case a pointer to a shared directory
    // lock is returned in result2.
*/

/* C/BCPL interface */
struct MsgPort * ASM
deviceproc (REG(d1) char *name)
{
	struct DevProc *dp;
	struct MsgPort *port;

	dp = getdevproc(name,NULL);
	if (!dp)
		return NULL;

	/* old-style deviceproc doesn't work with non-binding assigns! */
	if (dp->dvp_Flags & DVPF_UNLOCK)
	{
		freedevproc(dp);
		setresult2(ERROR_DEVICE_NOT_MOUNTED);
		return NULL;
	}

	/* they want the lock in pr_result2 */
	setresult2(dp->dvp_Lock);
	port = dp->dvp_Port;

	/* FIX! force handler to remain! */
	freedevproc(dp);

	return port;
}

/*
 * Call initially with dp == NULL.  It will return a struct DevProc or NULL.
 * After using the DevProc, if (dp->dvp_Flags & DVPF_ASSIGN) is true and
 * if the object is not found (ERROR_OBJECT_NOT_FOUND), you should call
 * GetDeviceProc again, passing back dp.  It will return you either a modified
 * DevProc to use again, or NULL (with ERROR_NO_MORE_ENTRIES in IoErr()).  If
 * it returns NULL, it has freed the DevProc structure.  Otherwise, you are
 * responsible for calling FreeDeviceProc() when you are done with the entry.
 *
 * It is safe to call FreeDeviceProc(NULL).  It is also safe to call
 * GetDeviceProc(name,dp) even if DVPF_ASSIGN is not set (though it adds
 * overhead, and isn't preferred).
 *
 * This call is complex, evil, and to hard to understand.  It should be broken
 * up at some point, if possible.
 *
 * Device List locking:
 *	For user accesses, we have three cases.  (1) node has task ptr,
 * (2) node has NULL task, has segment, and (3) node has NULL task and
 * segment ptrs.
 *
 * Following assumes two semaphores: doslist semaphore, and entry semaphore
 *
 * Pseudo-code:
 *   
 *   obtain list read lock
 *   find node
 *   determine type
 *   switch (type):
 *	(1): release list read lock
 *	     return devproc
 *	(2):
 *	(3): obtain entry write lock
 *	     release list read lock
 *	     if seglist is NULL
 *		load segment	Note! will call this routine!
 *				Luckily, semaphores nest!
 * 	     if dl_Task is NULL (in case someone else was just starting it)
 *		start task
 * 
 *	     release entry write locks.
 *           return devproc
 *
 *  Possible problem: what if handler on startup wants to add something to
 *  the device list, like a volume????  Solution: release devlist semaphore
 *  after getting entry write semaphore.  require BOTH to remove a node!
 */

struct DevProc * ASM
getdevproc (REG(d1) char *name, REG(d2) struct DevProc *olddp)
{
	char v[32];	/* FIX! MAX_FILENAME+1+1 */
	LONG ptr;
	struct MsgPort *task = NULL;
	struct DosList *volnode = NULL;
	struct DosList *dl = NULL;
	struct AssignList *al;
	struct DevProc *dp;
	char *volname = NULL;
	char *cname;
	BPTR dir;
	LONG flags = 0;
	LONG type;

	dp = olddp;

	/* free existing non-binding assign lock if passed in */
	/* (so we can ignore later).  Unset flag also. */
	if (dp)
	{
		/* unlock non-binding assign lock */
		if (dp->dvp_Flags & DVPF_UNLOCK)
		{
			freeobj(dp->dvp_Lock);
			dp->dvp_Flags &= ~DVPF_UNLOCK;
		}

		/* if dp is passed in, and there are no more entries, exit */
		if (!(dp->dvp_Flags & DVPF_ASSIGN))
			goto null_return;
	} else {
		/* we're going to have to allocate one, do it first in case */
		/* of low mem (we don't want to leave a hanging proc)	    */
		dp = AllocVecPubClear(sizeof(*dp));
		if (!dp)
			goto null_return;
	}

	/* splitname is now 0-based! */
	/* Leaves volume name in v   */
	/* 30 character name plus NULL   - FIX! MAXFILENAME+1 */
	ptr = splitname(name,':',v,0,31);

	while (1)
	{
		dir = currentdir(FALSE,NULL);		/* NULL isn't needed! */
		setresult2(ERROR_DEVICE_NOT_MOUNTED);

		if (ptr > 0)	/* was there a ':'? */
		{
			/* v holds volume name (no ':') */

			if (mystricmp(v,"NIL") == SAME)
				goto null_return;

			if (mystricmp(v,"CONSOLE") == SAME)
			{
				dir = NULL;
				task = consoletask();
			}

			/* FIX! change name */
			if (mystricmp(v,"PROGDIR") == SAME)
			{
				BPTR lock;

				/* ProgDir: works here */
				lock = MYPROC->pr_HomeDir;

				/* FIX! should we use SYS: if no home? */
				if (lock)
				{
				    dir  = lock;
				    task = ((struct FileLock *)
					    BADDR(lock))->fl_Task;
/*kprintf("  lock = 0x%lx, task = 0x%lx\n",lock,task);*/
				}
/*else kprintf("  NULL HOME:!\n");*/
			}
		}

		/* if it isn't relative to home: */
		if (task == NULL)
		{
		    if (ptr == -1 || ptr == 1)	/* no ':' or starts with ':' */
		    {
			if (dir == NULL)
			{
			/* no current directory - take root of default fs */
				task = filesystemtask();
			} else {
				volnode = (struct DosList *)
					BADDR(((struct FileLock *) BADDR(dir))->
					 fl_Volume);
				task    = volnode->dol_Task;
				volname = (char *) BADDR(volnode->dol_Name) + 1;
				if (ptr == 1)
					dir = NULL; /* root of this fs ':' */
			}

		    } else {	/* has ':' */

			/* look in assignment list */
			/* see algorithm above in psuedo-code */

			/* lock list for read, to keep things from changing */
			dl = LockDosList(LDF_ALL|LDF_READ);

			volnode = finddev(v,FALSE);
			if (volnode == NULL) /* volume not known */
			{
cant_find:
				volname = v;	/* v is cstr */
			} else {

			    /* volume/device/assign is known */
			    /* is this a continuation of a multi-assign? */
			    /* Careful! check initial value of dp! */
			    if (olddp)
			    {
				/* if olddp is set, dp == olddp */

				setresult2(ERROR_NO_MORE_ENTRIES);
				if (volnode != dp->dvp_DevNode)
				{
/*requester("Bad node!",((char *)BADDR(volnode->dol_Name))+1,
((char *)BADDR(dp->dvp_DevNode->dol_Name))+1);*/
				    /* got a different node - punt */
				    /* should this put up a requester? FIX! */
				    goto null_return;
				}

				/* find next entry in assign, if any */
				if (volnode->dol_Type != DLT_DIRECTORY)
{
/*requester("Bad type?!","","");*/
				    goto null_return;
}

				al = volnode->dol_misc.dol_assign.dol_List;
				/* is this the first call? */
				if (al && dp->dvp_Lock == volnode->dol_Lock)
					goto got_multi;

				while (al) {
				    /* find the current lock in the list */
				    if (al->al_Lock == dp->dvp_Lock)
				    {
					/* got it! */
					al = al->al_Next;
					if (al)
					{
got_multi:
					    /* NULL lock would be silly, but */
					    dp->dvp_Port = al->al_Lock ?
						((struct FileLock *)
						 BADDR(al->al_Lock))->fl_Task :
						filesystemtask();
					    dp->dvp_Lock = al->al_Lock;

					    /* turn off flag if no more after */
					    if (!al->al_Next)
						dp->dvp_Flags &= ~DVPF_ASSIGN;

					    goto return_success;

					} else
					    goto null_return;
				    }
				    al = al->al_Next;
				}
				/* didn't find it (bizarre) must have changed */
				goto null_return;

			    } /* if (olddp) */

			    type = volnode->dol_Type;

			    /* handle late-binding assigns */
			    if (type == DLT_LATE ||
				type == DLT_NONBINDING)
			    {
/*
if (type == DLT_LATE)
 requester("Handling late-binding assign!",name,
 volnode->dol_misc.dol_assign.dol_AssignName);
else
 requester("Handling non-binding assign!",name,
 volnode->dol_misc.dol_assign.dol_AssignName);
*/
				/* this will call getdevproc!!! */
				/* remove circularity by changing type! */
				/* ok, since list should be locked here FIX */
				/* FIX! recursive! at least check stack!!!! */
				/* leaves a slight hole, may cause object not
				   found */
				if (type == DLT_LATE)
					volnode->dol_Type = DLT_PRIVATE;

				/* CSTR */
				dir = locateobj(volnode->
					dol_misc.dol_assign.dol_AssignName);
				if (!dir)
				{
/* FIX! handle assigns to drives with nothing in them (please insert...) */
/*requester("Failed to find",volnode->dol_misc.dol_assign.dol_AssignName,"");*/
					/* locateobj already did requester */
					volnode->dol_Type = type;
					goto null_return;
				}

				if (type == DLT_LATE)
				{
				    /* got it! convert to regular assign */
				    freeVec(volnode->
					    dol_misc.dol_assign.dol_AssignName);
				    volnode->dol_Type = DLT_DIRECTORY;
				    volnode->dol_Lock = dir;
		/* apparently some people rely on dol_Task, try to help them */
				    volnode->dol_Task = ((struct FileLock *)
							 BADDR(dir))->fl_Task;
				} else {
				    /* type == DLT_NONBINDING */
				    flags |= DVPF_UNLOCK;
				}

			    } /* if late or non-binding */

			    if (type != DLT_NONBINDING)
				dir = volnode->dol_Lock;
			    task = (struct MsgPort *)
			           (dir == NULL ? volnode->dol_Task :
				    ((struct FileLock *) BADDR(dir))->fl_Task);

			    /* set flag to tell caller there are more */
			    if (type == DLT_DIRECTORY &&
				volnode->dol_misc.dol_assign.dol_List)
			    {
				flags |= DVPF_ASSIGN;
			    }

			    if (task == NULL)
			    {
				if (volnode->dol_Type == DLT_DEVICE)
				{
/* grab the Entry semaphore (the "I'm mucking with an entry" lock)	 */
/* we support new-style handlers (-2), which means that they will not do */
/* any dangerous synch IO before replying their packet, or old-style     */
/* handlers, which are protected using Forbid() and dol_Task.		 */
/* -2 is C-style, -3 is bcpl-style					 */
/* RemDosEntry grabs write-locks on BOTH delete and entry!		 */

				    ULONG mode = ((LONG) volnode->
				       dol_misc.dol_handler.dol_GlobVec) < -1 ?
				       LDF_DELETE | LDF_READ /* new */:
				       LDF_ENTRY | LDF_WRITE /* old */;

				    LockDosList(mode);

/* release list semaphore so handler can add volume nodes */
				    UnLockDosList(LDF_ALL | LDF_READ);
				    /* don't unlock later */
				    dl = NULL;

/* since we weren't locked until now, someone else may have started it	    */
/* however, loaddevice checks the task field before trying */
				    	/* loaddevice wants a BSTR! */
				    cname = AllocVecPub(strlen(name)+1);
				    if (cname)
				    {
				    	  CtoB(name,(CPTR) cname);
				    	  task = loaddevice(volnode,
							    TOBPTR(cname));
				    	  freeVec(cname);
				    	  dir = NULL;
				     }

				     UnLockDosList(mode);
				} else {
				    /* not a device - volume probably */
				    volname = (char *)
					     BADDR(volnode->dol_Name)+1;
				}
			    } /* if task == NULL */

			} /* if volnode == NULL... else */

		    } /* if ptr == -1 || ptr == 1.... else */ 

		    /* did we find anything? */
		    if (task == NULL)
		    {
			/* free doslist before looping */
			if (dl)
			{
				dl = NULL;
				UnLockDosList(LDF_ALL | LDF_READ);
			}

			if (volname != NULL)
			{
				/* flush to reduce chances of munged disk */
				/* useless - this is merely the boot volume */
				/* closedisk(task); */

				/* put up "insert disk" requester */
				/* leaves result2 set */
				setresult2(ERROR_DEVICE_NOT_MOUNTED);
				if (!err_report(REPORT_INSERT,(BPTR) volname,
						NULL))
					continue;	/* while loop */
				/* goto null_return was here */
			}
			/* either fell out of above, or loaddevice failed */
			goto null_return;
		    }

		} /* if task == NULL */

		/* FIX! add to count in handler task! */
		dp->dvp_Port    = task;
		dp->dvp_Lock    = dir;
		dp->dvp_Flags   = flags;
		dp->dvp_DevNode = volnode;	/* may be NULL */

return_success:
		/* unlock doslist if locked */
		if (dl)
			UnLockDosList(LDF_READ | LDF_ALL);

		return dp;

	} /* while 1 */

null_return:
	/* save error code... */
	ptr = getresult2();

	/* free dp if passed in, return NULL. */
	if (dp)
		freedevproc(dp);

	/* unlock doslist if locked */
	if (dl)
		UnLockDosList(LDF_READ | LDF_ALL);

	/* restore error and exit */
	setresult2(ptr);
	return NULL;
}

/* MUST be safe for dp == NULL! */

void ASM
freedevproc (REG(d1) struct DevProc *dp)
{
	if (dp) {
		if (dp->dvp_Flags & DVPF_UNLOCK)
			freeobj(dp->dvp_Lock);

		/* FIX! decrement count in handler! */
		freeVec((char *) dp);
	}
}

/* splitname has been downcoded - uses Cstrings */
/* findstream moved to bcplio.c */

/* device management support */
/* finddevice is a define */

struct DosList * ASM
finddev (REG(d1) char *name,
	 REG(d2) LONG devonly)
{
	struct DosList *p;
	LONG flags;

	flags = devonly ? LDF_DEVICES : LDF_ALL;

	/* usually called under read lock from getdeviceproc */

	p = LockDosList(flags|LDF_READ);
	p = FindDosEntry(p,name,flags);
	UnLockDosList(flags|LDF_READ);

	return p;
}

/* adddevice already written for ram-handler */

struct MsgPort * ASM
loaddevice (REG(d1) struct DosList *node,
	    REG(d2) BPTR string)	/* actually we don't care */
{
	struct MsgPort *task;
	struct DosPacket *pkt = NULL;
	LONG *mseg;
	LONG *segl = NULL;
	BSTR name = node->dol_Name;
	BPTR gv   = node->dol_misc.dol_handler.dol_GlobVec;
	BPTR seg  = node->dol_misc.dol_handler.dol_SegList;
	LONG pri  = node->dol_misc.dol_handler.dol_Priority;
	LONG ss   = node->dol_misc.dol_handler.dol_StackSize;

/*kprintf("starting handler %s\n",((char *)BADDR(name))+1);*/

	/* see psuedo-code above for GetDeviceProc */
	/* we have the Entry locked for write, the list itself is unlocked! */
	/* (or we have delete locked for read, see RemDosEntry)		    */

	/* check first */
	if (node->dol_Task)
		return node->dol_Task;

	if (!seg)			/* no segment, load it from disk */
	{
	  /* may end up calling loaddevice!!! */
	  /* potential deadlock if this requires starting a handler?      */
	  /* no deadlock, since this task owns the semaphores, all is OK. */
	  /* deadlock still possible if loading the other handler requires*/
	  /* loading a third handler.  Oh well.				  */

		seg = loadsegbstr(node->dol_misc.dol_handler.dol_Handler);
							/* BPTR to BSTR */

		if (!seg)
			return NULL;			/* exit if error */
		Forbid();
		if (node->dol_misc.dol_handler.dol_SegList)
			unloadseg(seg);
		else 
			node->dol_misc.dol_handler.dol_SegList = seg;
					/* loaded now */
		Permit();

		/* make sure we have the current segment for the handler */
		seg = node->dol_misc.dol_handler.dol_SegList;
	}

/*
   // Now create new task invocation. If GV = -1 then we must create a
   // C task; otherwise we assume a BCPL task, which uses the shared
   // global vector as GV or a private one if GV = 0.
*/
/* assumes device names are null-terminated as well!!!!! */

	/* someone may have started the task already!!! */
	/* probably redundant, but may be useful and doesn't hurt */

	task = node->dol_Task;
	if (task == NULL)
	{
	    pkt = AllocDosObject(DOS_STDPKT,NULL);
	    if (!pkt)
		goto cleanup;

	    /* -1 is cstyle, semaphore locking.  -2 same w/ Forbid locking */
	    if ((LONG) gv == -1 || (LONG) gv == -2)
	    {
		/* name must be null-terminated BSTR! */
		task = createproc((char *) BADDR(name)+1,pri,seg,ss);

	    } else {
		segl = AllocVecPub(4*4);
		if (!segl)
			goto cleanup;

		mseg = (LONG *) BADDR(seglist());	/* my segarray */

		segl[0] = 3;
		segl[1] = mseg[1];	/* must be klib */
		segl[2] = mseg[2];	/* must be blib */
		segl[3] = seg;		/* BCPL program */

		/* make private GV if required */
		/* (isn't for con,raw,par,ser,prt) */
		/* (is for ofs, old ram) */

		/* -3 is forbid() locking, BCPL handler */
		if (gv == NULL || gv == -3)
			gv = TOBPTR(makeglob(segl));

		if (gv)
			task = createtask(TOBPTR(segl),ss,pri,name,gv);
			/* createtask is bcpl-style */
	    }

	    if (!task)
		goto cleanup;
	    segl = NULL;		/* so cleanup won't free it! */

	    /* start it up */
	    /* For DeviceProc? */
	    setresult2(node->dol_Lock);

	    pkt->dp_Action = ACTION_STARTUP;
	    pkt->dp_Arg1   = string;
	    pkt->dp_Arg2   = node->dol_misc.dol_handler.dol_Startup;
	    pkt->dp_Arg3   = TOBPTR(node);
	    pkt->dp_Arg4   = NULL;		/* port to override task ptr */

	    pkt->dp_Port   = task;		/* task to send to */

	    qpkt(pkt);
	    taskwait();	/* assume I got the right one back */
	    /* note: taskwait() doesn't set result2! */

	    if (pkt->dp_Res1)
	    {
		setresult2(node->dol_Lock);

		/* use either port passed back in arg4 or dol_Task (if set) */
		/* This allows handlers to keep their process port for sync */
		/* Dos I/O.						    */

		if (pkt->dp_Arg4)
			task = (struct MsgPort *) pkt->dp_Arg4;
		else if (node->dol_Task)
			task = node->dol_Task;

	    } else {
		task = NULL;
		setresult2(pkt->dp_Res2);
	    }
	} /* if task == NULL */

cleanup:
	FreeDosObject(DOS_STDPKT,pkt);		/* may be null */
	freeVec(segl);				/* may be null */

	return task;
}

/* build a global vector (not filled in).  Walks each seglist in the serarray */
/* to find the highest global used, then allocates space for them. */

LONG * ASM
makeglob (REG(d1) LONG *segl)
{
	LONG max = 15;
	LONG nseg = segl[0];
	LONG *gv,*p;
	LONG i,hrg;

	for (i = 1; i <= nseg; i++)
	{
		p = (LONG *) BADDR(segl[i]);
		while (p != 0) {
			/* seglist points to next segment ptr */
			/* p[1] is the first lw of segment, which has the */
			/* number of longs until the end of the segment */
			hrg = p[p[1]];
			if (hrg > max)
				max = hrg;
			p = (LONG *) BADDR(*p);
		}
	}

	gv = (LONG *) AllocVecPub((max + 1 + NEGATIVE_GLOBAL_SIZE) * 4);
	if (gv)
	{
		gv = gv + NEGATIVE_GLOBAL_SIZE;		/* remember, LONG *!! */
		gv[0] = max;
	}
	return gv;
}

LONG ASM
isinteractive (REG(d1) BPTR s)
{
	/* scb.id == fh_Port */
	return (LONG) ((struct FileHandle *) BADDR(s))->fh_Port;
}

/* MUST be callable from a task!!! */

struct DateStamp * ASM
datstamp (REG(d1) struct DateStamp *v)
{
	struct timeval tv;
	struct DosLibrary *doslib = opendosbase();
	LONG quot,rem;

	/* get the current time (callable from a task)	*/
	/* asm stub to call GetSysTime()		*/
	getSysTime(&tv,doslib->dl_TimeReq->tr_node.io_Device);

	/* write the current time out to the users buffer */
	quot = divrem32(tv.tv_secs,60,&rem);	/* sets rem to remainder */

	v->ds_Tick   = div32(tv.tv_micro,
			     1000000/TICKS_PER_SECOND) +
		       rem * TICKS_PER_SECOND;
	v->ds_Minute = rem32(quot,(60*24));
	v->ds_Days   = div32(tv.tv_secs,(60*60*24));

	/* update the root node to have the current time */
	Forbid();
	doslib->dl_Root->rn_Time = *v;
	Permit();

	return v;
}

void ASM
setdosdate (REG(d1) struct DateStamp *v)
{
	struct timerequest iob;
	LONG secs,micro;
	LONG rem;

	secs  = v->ds_Days * (60*60*24) +
		v->ds_Minute * (60) +
		divrem32(v->ds_Tick,TICKS_PER_SECOND,&rem);
	micro = rem * (1000000/TICKS_PER_SECOND);

	Forbid();

	/* set the timer devices idea of the time */
	dotimer(TR_SETSYSTIME,&iob,secs,micro);

	/* update dos's internal idea of time */
	rootstruct()->rn_Time = *v;

	Permit();
}

/* most calls to this are in support.asm */

LONG ASM
do_lock (REG(d1) BPTR lock,	/* may actually be FH! (for ExamineFH)! */
	 REG(a0) LONG *args,
	 REG(d0) LONG action)
{
	LONG rc;
	struct MsgPort *task;
	struct FileInfoBlock *fib = (void *) BADDR(args[0]);
	struct FileLock *rlock = BADDR(lock);
	LONG arg1 = lock;

#ifndef DONT_CONVERT_EXNEXT
	/* if exnext convert C filename back to BCPL */
	/* FIX I want to ignore this so exnext goes faster! */
	if (action == ACTION_EXAMINE_NEXT)
		CtoBstr((char *) &(fib->fib_FileName[0]));
#endif

	do {
		/* NULL lock handling code here */
		/* bizarre condition code, careful */
		if (action == ACTION_EXAMINE_FH)
		{
			task = ((struct FileHandle *) rlock)->fh_Type;
			arg1 = ((struct FileHandle *) rlock)->fh_Arg1;
		} else {
			task = (rlock ? rlock->fl_Task : filesystemtask());
		}

		rc = sendpkt4(task,action,arg1,args[0],args[1],args[2]);
		if (rc == 0)
		{
			if (action != ACTION_EXAMINE_FH)
			{
				if (err_report(REPORT_LOCK,lock,task))
					return 0;
			} else {
				if (err_report(REPORT_STREAM,lock,0))
					return 0;
			}
		}
	} while (rc == 0);

	/* convert string to C format if required (painful!) */
	if (action == ACTION_EXAMINE_NEXT || action == ACTION_EXAMINE_OBJECT ||
	    action == ACTION_EXAMINE_FH)
	{
		BtoCstr((CPTR) &(fib->fib_FileName[0]));
		BtoCstr((CPTR) &(fib->fib_Comment[0]));
	}

	return rc;
}

/* exall,examine,exnext,info,parent,copydir(duplock) are in support.asm */

/* takes fh or lock for arg2!!! */

LONG ASM
changemode (REG(d1) LONG type, REG(d2) BPTR fh, REG(d3) LONG newmode)
{
	struct MsgPort *port;

	if (!fh)
	{
		setresult2(ERROR_INVALID_LOCK);
		return FALSE;
	}

/* FIX!!! should use a do_lock type interface!!!! */
	if (type == CHANGE_LOCK)
		port = (void *) ((struct FileLock *) BADDR(fh))->fl_Task;
	else /* FIX! should check for error! */
		port = (void *) ((struct FileHandle *) BADDR(fh))->fh_Type;

	return sendpkt3(port,ACTION_CHANGE_MODE,type,fh,newmode);
}


LONG ASM
setcomment (REG(d1) char *name,
	    REG(d2) char *comment)
{
	char *v;
	BPTR myv;
	LONG rc = FALSE;

	v = AllocVecPub(strlen(comment)+1);
	if (v)
	{
		myv = TOBPTR(v);
		CtoB(comment,(CPTR) v);
		rc = objact(name,ACTION_SET_COMMENT,&myv);
		freeVec(v);
	}
	return rc;
}

/* most of the objact routines moved to support.asm */

#ifndef NEW_RENAME
LONG ASM
renameobj (REG(d1) char *fromname,
	   REG(d2) char *toname)
{
	/* rename a file or directory (to the same device) */
	/* does not follow path assigns! FIX!?  */
	/* does not handle soft links!! FIX! */
	struct DevProc *fromdp,*todp;
	char *fname,*tname;
	LONG rc = 0,len;

	while (1) {
		todp = NULL;
		if (!(fromdp = getdevproc(fromname,NULL)))
			break;
		if (!(todp = getdevproc(toname,NULL)))
			break;

		if (fromdp->dvp_Port != todp->dvp_Port)
		{
			setresult2(ERROR_RENAME_ACROSS_DEVICES);
			break;
		}

		/* get space for BSTR versions - do only one alloc */
		/* round len up to next multiple of 4! */
		len = (strlen(fromname)+1+3) & ~0x03;
		fname = AllocVecPub(len + strlen(toname)+1);
		tname = fname + len;
		if (!fname)
			break;

		CtoB(fromname,(CPTR) fname);
		CtoB(toname,(CPTR) tname);

		rc = sendpkt4(fromdp->dvp_Port,ACTION_RENAME_OBJECT,
			      fromdp->dvp_Lock,TOBPTR(fname),
			      todp->dvp_Lock,  TOBPTR(tname));
		freeVec(fname);

		if (rc || err_report(REPORT_LOCK,todp->dvp_Lock,todp->dvp_Port))
			break;

		/* make sure they get freed before looping! */
		freedevproc(fromdp);
		freedevproc(todp);
	} /* while 1 */

cleanup:
	if (fromdp)
		freedevproc(fromdp);
	if (todp)
		freedevproc(todp);
	return rc;
}
#else
LONG ASM
renameobj (REG(d1) char *fromname,
	   REG(d2) char *toname)
{
	/* rename a file or directory (to the same device) */
	/* does not follow path assigns for destination! (of course) */
	/* has to find a source that is on the same volume as the destination */

	struct DevProc *fromdp = NULL,*todp = NULL;
	char *fname,*tname,*newname;
	LONG rc = FALSE;
	LONG fromlinks = MAX_LINKS, tolinks = MAX_LINKS;

	tname = AllocVecPub(strlen(toname)+1+1);
	if (!tname)
		return FALSE;
	CtoB(toname,(CPTR) tname);
	tname[*tname+1] = '\0';

	fname = AllocVecPub(strlen(fromname)+1+1);
	if (!fname)
		goto exit_rename;
	CtoB(fromname,(CPTR) fname);
	fname[*fname+1] = '\0';

	while (1) {
		/* no following of multi-directories on to! */
		todp = getdevproc(tname+1,NULL);
		if (todp == NULL)
			goto exit_rename;

		fromdp = getdevproc(fname+1,fromdp);
		if (fromdp == NULL)
			goto exit_rename;

/*
soft links make this code useless - we must count on the handlers to reject!
		if (fromdp->dvp_Port != todp->dvp_Port)
		{
			setresult2(ERROR_RENAME_ACROSS_DEVICES);
			return 0;
		}
*/
		rc = sendpkt4(fromdp->dvp_Port,ACTION_RENAME_OBJECT,
			      fromdp->dvp_Lock,TOBPTR(fname),
			      todp->dvp_Lock,  TOBPTR(tname));

/*
 * There may be soft links in EITHER the source or destination path.  Try to
 * do a ReadLink on from, then on to.  Keep separate link counts.  Fail only
 * if BOTH fail the readlink.  Evil.
 */

		if (!rc)
		{
			switch (getresult2()) {
			case ERROR_OBJECT_NOT_FOUND:
			  /* multi-dirs only for source */
			  if (fromdp->dvp_Flags & DVPF_ASSIGN)
				goto multi_dir_loop;
			  break;
			case ERROR_IS_SOFT_LINK:
			  /* for the time being, max 255 chars */
			  /* Bstr is allocated in 256 bytes */
			  newname = AllocVecPub(256);
			  if (!newname)
			    break;

			  /* first try the source */
			  if (--fromlinks >= 0)
			  {
			    BtoCstr((CPTR) fname);
			    if (ReadLink(fromdp->dvp_Port,fromdp->dvp_Lock,
					 fname,newname,256) >= 0)
			    {
			      freeVec(fname);
			      fname = newname;
			      CtoBstr(fname);
			      fname[*fname+1] = '\0';
			      /* we read a link for 'from', loop and try again*/
			      goto link_loop;

			    } else {

			      CtoBstr(fname);		/* restore it */

			      /* now try the destination */
			      if (--tolinks >= 0)
			      {
				/* for the time being, max 255 chars */
				/* Bstr is allocated in 256 bytes */
				/* newname already alloced */

				BtoCstr((CPTR) tname);
				if (ReadLink(todp->dvp_Port,todp->dvp_Lock,
					     tname,newname,256) >= 0)
				{
				  freeVec(tname);
				  tname = newname;
				  CtoBstr(tname);
				  tname[*tname+1] = '\0';
				  /* we read a link for 'to', loop and try*/
				  /* again */
				  goto link_loop;

				} else {
				  CtoBstr(tname);	/* probably not needed*/
				  /* we can't deal with >255 chars anyway */
				  freeVec(newname);
				  setresult2(ERROR_INVALID_COMPONENT_NAME);
				}
			      } else {
			        goto counterr;
			      }
			    }
			  } else {
counterr:		    freeVec(newname);
			    setresult2(ERROR_TOO_MANY_LEVELS);
			  } /* fall thru */

default:		  break;
			} /* switch getresult2() */
		} /* if !rc */

		if (rc || err_report(REPORT_LOCK,todp->dvp_Lock,todp->dvp_Port))
		{
exit_rename:
			freeVec(fname);
			freeVec(tname);
			freedevproc(fromdp);
			freedevproc(todp);
			return rc;
		}

		/* make sure they get freed before looping! */
link_loop:
		freedevproc(fromdp);
		fromdp = NULL;
multi_dir_loop:
		freedevproc(todp);

	} /* while 1 */
	/*NOTREACHED*/
}
#endif

void ASM
freeobj (REG(d1) BPTR lock)
{
	LONG res2;
	struct FileLock *l;

	l = BADDR(lock);
	/* don't send UnLocks for locks with NULL tasks */
	if (l && l->fl_Task)
	{
		res2 = getresult2();
		sendpkt1(l->fl_Task,ACTION_FREE_LOCK,lock);
		setresult2(res2);
	}
}

BPTR ASM
OpenFromLock (REG(d1) BPTR lock)
{
	struct FileHandle *scb;
	struct MsgPort *task;
	LONG rc;

	if (!lock)
	{
		setresult2(ERROR_OBJECT_WRONG_TYPE);
		return NULL;
	}

	scb = AllocDosObject(DOS_FILEHANDLE,NULL);
	if (!scb)
		return NULL;	/* no space */

	while (1) {
		task = ((struct FileLock *) BADDR(lock))->fl_Task;
		scb->fh_Type = task;	/* this needs to be set! */

		rc = sendpkt2(task,ACTION_FH_FROM_LOCK,TOBPTR(scb),lock);

		/* open really failed */
		if (rc || err_report(REPORT_LOCK,lock,task))
		{
			if (!rc)
			{
				FreeDosObject(DOS_FILEHANDLE,scb);
				return 0;
			}
			return TOBPTR(scb);
		}
	}
}

LONG ASM
isfilesystem (REG(d1) char *name)
{
	LONG rc;
	char pat[32];	/* FIX! max handler name len + 1 */
	LONG ptr;
	BPTR lock;

	/* FIX! only keep one copy of strings! */
	if (mystricmp("*",name) == SAME ||
	    mystricmp("CONSOLE:",name) == SAME ||
	    mystricmp("NIL:",name) == SAME)
	{
		setresult2(ERROR_ACTION_NOT_KNOWN);
		return FALSE;
	}

	/* first try asking the handler directly */
	rc = devact(name,0,0,ACTION_IS_FILESYSTEM);
	if (rc == 0 && getresult2() == ERROR_ACTION_NOT_KNOWN)
	{
		/* doesn't understand 2.0 packets - Try Lock on root */
		/* 30 char name + NULL -  FIX! MAXFILENAME+1 */
		ptr = splitname((char *) name,':',pat,0,31);
		if (ptr >= 0)
		{
			mystrcat(pat,":");
			lock = locateobj(pat);
			if (!lock)
			{
				/* doesn't support locks - not an FS */
				/* IoErr will be ERROR_ACTION_NOT_KNOWN */
				return FALSE;
			}
			/* supports Lock() on root - assume an FS */
			freeobj(lock);
		}
		return DOSTRUE;	/* either relative path or it supports lock */
	}

	return rc;	/* isfilesystem succeeded */
}

/* most calls to devact moved to support.asm */

LONG ASM
devact (REG(d1) char *name, REG(d2) LONG arg1, REG(d3) LONG arg2,
	REG(d0) LONG action)
{
	struct DevProc *dp = NULL;
	LONG temparg2 = arg2;
	LONG rc;

	while (1) {
		dp = getdevproc(name,dp);
		if (!dp)
			/* getdevproc freed dp for us */
			return FALSE;

		rc = sendpkt2(dp->dvp_Port,action,arg1,temparg2);

		/* should I try subsequent directories? */
		if (!rc && getresult2() == ERROR_OBJECT_NOT_FOUND)
		{
			continue;	/* makes it get the next assign */
		}

		if (rc || err_report(REPORT_LOCK,dp->dvp_Lock,dp->dvp_Port))
		{
			freedevproc(dp);
			return rc;
		}

		/* there was an error and he selected retry - start over */
		freedevproc(dp);
		dp = NULL;
	}
}

/* returns LOCK_SAME for same, LOCK_SAME_HANDLER for same handler, */
/* LOCK_DIFFERENT for different handlers. */

LONG ASM
SameLock (REG(d1) BPTR l1, REG(d2) BPTR l2)
{
	struct FileLock *lock1,*lock2;

	/* simplest check */
	if (l1 == l2)
		return LOCK_SAME;

	/* check for NULL locks! */
	/* not _fully_ correct - could be NULL lock and lock on boot vol */
	/* Fix? */
	if (!(lock1 = (struct FileLock *) BADDR(l1)) ||
	    !(lock2 = (struct FileLock *) BADDR(l2)))
		return LOCK_DIFFERENT;

	/* first check they belong to the same handler */
	/* FIX!!! remove task test - unneeded, but left in due to paranoia */
	if ((lock1->fl_Volume != lock2->fl_Volume) ||
	    (lock1->fl_Task != lock2->fl_Task))
		return LOCK_DIFFERENT;		/* different volumes */

	/* now try the ACTION_SAME_LOCK packet */
	/* returns FALSE (failure) or non-false (success) */

	/* FIX!! should use error checking or do_lock! */
	if (sendpkt2(lock1->fl_Task,ACTION_SAME_LOCK,l1,l2))
		return LOCK_SAME;			/* same */

	/* didn't succeed - check if result2 == ERROR_ACTION_NOT_KNOWN */
	/* for any other error assume they're different */

	if (getresult2() == ERROR_ACTION_NOT_KNOWN)
	{
		/* doesn't support SAME_LOCK - check the fl_Key field */
		if (lock1->fl_Key == lock2->fl_Key)
			return LOCK_SAME;		/* same */
	}

	return LOCK_SAME_VOLUME;	/* same volume, not the same object */
}

/* needs stub rtn because of the number of parameters */
/* NOTE: path is a CSTR, buffer (on success) is a CSTR! */

LONG ASM
ReadLink (REG(d1) struct MsgPort *port, REG(d2) BPTR lock,REG(d3) UBYTE *path,
	  REG(d0) UBYTE *buffer,REG(a0) ULONG size)
{
	return sendpkt4(port,ACTION_READ_LINK,lock,(LONG) path,
			(LONG) buffer,size);
}

/* return last portion of a path */

UBYTE * ASM
FilePart (REG(d1) UBYTE *path)
{
	UBYTE *p;

	p = PathPart(path);
	if (*p == '/')
		p++;

	return p;
}

/* Returns a pointer to the end of the next-to-last component of a path. */
/* i.e. it points past the :, or to the last slash (if preceded by chars), */
/* or past the last slash, if preceded by a null or another slash.  */

UBYTE * ASM
PathPart (REG(d1) UBYTE *path)
{
	UBYTE *p;

	if ((p = strrchr(path,'/')) == NULL)
	{
		if ((p = strchr(path,':')) == NULL)
			return path;
		else
			return p+1;
	}
	if ((p == path)	|| 	/* single leading slash */
	    (*(p-1) == '/') ||
	    (*(p-1) == ':'))	/* multiple slashes (...//foo) or ...:/foo  */
		p++;		/* point past slash - include going up dirs */

	return p;
}

/* Appends a file/dir to the end of a path */

LONG ASM
AddPart (REG(d1) UBYTE *dirname, REG(d2) UBYTE *filename, REG(d3) ULONG size)
{
	UBYTE *p;
	LONG slash = FALSE;

	/* must handle :'s in filename! */

	if (strchr(filename,':'))
	{
		/* filename has a : - if first, tack onto any colon in	*/
		/* dirname, else replace entire string with filename.	*/
		if (*filename == ':')
		{
			p = strchr(dirname,':');
			if (!p)
			{
				p = dirname;	/* overwrite entire path */

			} /* else p points to :, right spot for copy */
		} else
			p = dirname;	/* overwrite entire path */
	} else {
		p = dirname + strlen(dirname);	/* ptr to end of path */
		/* Don't go back past the beginning of the string.	*/
		/* With trailing slashes, you just add the path to it,	*/
		/* because for historical reasons foo/ means foo, while	*/
		/* / means one directory up.				*/
		if (*dirname)
			if (*(p-1) != ':' && *(p-1) != '/')
			{
				slash = TRUE;
				*p++ = '/';  /* we can safely overwrite null */
			}
	}

	/* p points to point to copy filename to */
	/* do length check after finding out about '/' versus ':' */
	if ((p-dirname) + strlen(filename) + 1 > size)	/* 1 for NULL */
	{
		if (slash)
			*--p = '\0';		/* fix string back */
		setresult2(ERROR_LINE_TOO_LONG);
		return FALSE;
	}

	strcpy(p,filename);

	return DOSTRUE;
}

WORD sizes[] = {
	sizeof(struct NewFileHandle),	/* the extended version */
	sizeof(struct ExAllControl),
	sizeof(struct FileInfoBlock),
	sizeof(struct StandardPacket),
	sizeof(struct CommandLineInterface),  /* not used */
	sizeof(struct RDArgs),
};

void * ASM
AllocDosObject (REG(d1) ULONG type, REG(d2) struct TagItem *tags)
{
	void *ptr;

	if (type >= sizeof(sizes)/sizeof(sizes[0]))
		return NULL;

	/* handle DOS_CLI differently */
	if (type == DOS_CLI)
		ptr = (void *) AllocCli(tags);
	else
		ptr = AllocVecPubClear(sizes[type]);
	if (!ptr)
		return NULL;
 
	/* initialize, if needed */
	switch (type) {

	case DOS_FILEHANDLE:
		/* careful!!! execute.c allocates fh's!! */
		init_scb((struct FileHandle *) ptr,
			 getTagData(ADO_FH_Mode,ACTION_FINDINPUT,tags));
		break;

	case DOS_EXALLCONTROL:
	case DOS_FIB:
		break;

	case DOS_STDPKT:
		/* void * can be a pain at times */

		((struct StandardPacket *) ptr)->sp_Msg.mn_Node.ln_Name =
			(UBYTE *) (&(((struct StandardPacket *) ptr)->sp_Pkt));
		((struct StandardPacket *) ptr)->sp_Pkt.dp_Link = ptr;

		/* return ptr to dospacket */
		ptr = &(((struct StandardPacket *) ptr)->sp_Pkt);
		break;

	case DOS_CLI:
	case DOS_RDARGS:
		break;
	}

	return ptr;
}

void ASM
FreeDosObject (REG(d1) ULONG type, REG(d2) void *ptr)
{
	if (!ptr || type >= sizeof(sizes)/sizeof(sizes[0]))
		return;

	/* fix up ptr if needed */
	switch (type) {
	case DOS_STDPKT:
		ptr = ((char *) ptr) - sizeof(struct Message);
		// fall through
	default:
		freeVec(ptr);
		break;
	case DOS_CLI:
		freecli((struct CommandLineInterface *) ptr,0);
	}
}
