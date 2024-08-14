/* ram-handler.c */

#include "ram_includes.h"
#include "ram.h"

/*
// Ram disk handler.

// Objects are held as a data block which contains the name, a link, a backlink,
// a flag to distinguish files or directories, the time & date of creation
// and a pointer to a list of further file/directory nodes or the data.
// SCBs associated with open RAM files contain the lock in scb.arg1

   Converted to C, 1/19/88 REJ

// Changes: 14-Jan-88
// Fixed node name so you can talk to it by RAM Disk
// Added test so you can't have 2 files with same name
// Fixed rename directory under itself bug
// Fixed rename file1 file2/file1 bug
// Added renamedisk function
// Made root node valid, so standard file requesters will work
// Added action read/write (new V1.2 packet)
// Supports date stamp of the root
// fills lock with ptr to volumenode rather than devicenode

*/

/*
// This is called with a standard loadable device
// startup packet (same as, for example, cohand).
// The format is 
// Arg1 Name (ignored - passed to open as well)
// Arg2 Startup argument (not used)
// Arg3 Pointer to device node
*/

void
start (struct DosPacket *dp)
{
	struct Message    *msg;
	struct DeviceList *devnode,*newnode;
	struct node *node;
	LONG res1;
	LONG arg1;	/* dp->dp_Arg1 << 2 */
	LONG waitmask,sigs,mrp_sig,time_sig;
	int flag;
	char str1[256], str2[256];	/* FIX!!!!!!!!!! */

	DBUG("In handler");

	/* guaranteed not to go to disk */
	DOSBase = (struct DosLibrary *) TaggedOpenLibrary(OLTAG_DOS);
	UtilityBase = TaggedOpenLibrary(OLTAG_UTILITY);

	/* setup stuff - get packet from DOS */
	MyPort = &(((struct Process *) FindTask(NULL))->pr_MsgPort);

	DBUG1("got packet at 0x%lx",(LONG)dp);

	root = (struct node *) AllocVec(sizeof(struct node),MEMF_CLEAR);
	if (!root)
		goto punt;

	/* reply port for notification messages */
	MyReplyPort = CreateMsgPort();
	if (!MyReplyPort)
		goto punt;
	NewList((struct List *)
		&(MyReplyPort->mp_MsgList));	/* createport doesn't init(?) */

	timerport = CreateMsgPort();
	if (!timerport)
		goto punt;
	NewList((struct List *)
		&(timerport->mp_MsgList));	/* createport doesn't init(?) */

	NewList((struct List *) &(root->notify_list));
	NewList((struct List *) &(root->record_list));
	NewList((struct List *) &(root->rwait_list));
	NewList((struct List *) &orphaned);
	NewList((struct List *) &FreeMessages);
	root->type = ST_USERDIR;
	DBUG1("root is at 0x%lx",(LONG)root);

	strcpy(root->name,"Ram Disk");

	lock_list  = NULL;
	spaceused  = 1;


	/* Set up first, before filling in task id. */
	DBUG("adding device...");

	newnode = (struct DeviceList *) MakeDosEntry("Ram Disk",DLT_VOLUME);
	if (!newnode)
		goto punt;			/* exiting is permission */

	DBUG1("newnode is at 0x%lx",(LONG)newnode);
	newnode->dl_Task     = MyPort;
	newnode->dl_DiskType = ID_DOS_DISK;
	DateStamp((struct DateStamp *) &(newnode->dl_VolumeDate));

	if (!AddDosEntry((struct DosList *) newnode))
	{
		/* some sort of error, probably name collision */
		FreeDosEntry((struct DosList *) newnode);
		goto punt;
	}

	/* fill in device node with process pointer, etc */
	/* arg1 = string, arg2 = startup msg, arg3 = device node */
	/* these fields are inaccurate, it's a union */

	devnode = (struct DeviceList *) (dp->dp_Arg3 << 2);
	DBUG1("Devnode is at 0x%lx",(LONG)devnode);

	devnode->dl_Task = MyPort;
	devnode->dl_LockList = 0;
	devnode->dl_DiskType = 0;
	devnode->dl_unused   = 0;		/* globvec */
	DateStamp((struct DateStamp *) root->date);

	/* device node set up */

	volumenode = newnode;

	DBUG("returning packet...");
	ReplyPkt(dp,DOS_TRUE,0);
	DBUG("returned packet");


	waitmask = (1L << MyPort->mp_SigBit) |
		   (mrp_sig  = (1L << MyReplyPort->mp_SigBit)) |
		   (time_sig = (1L << timerport->mp_SigBit));
	while (1) {

	    sigs = Wait(waitmask);

	    /* handle returning notify messages */
	    if (sigs & mrp_sig)
		    while ((msg = GetMsg(MyReplyPort)) != NULL)
		    {
			AddFreeMsg((struct NotifyMessage *) msg);
		    }

	    /* timeouts for record locking */
	    if (sigs & time_sig)
		    while ((msg = GetMsg(timerport)) != NULL)
		    {
			kill_rwait((struct timerequest *) msg);
		    }

	    /* handle dospackets */
	    while ((msg = GetMsg(MyPort)) != NULL)
	    {
		dp = (struct DosPacket *) msg->mn_Node.ln_Name;
		DBUG2("Action %ld (packet at 0x%lx)",dp->dp_Action,
		      (LONG) dp);

		/* shift to save code later - used a lot */
		arg1 = dp->dp_Arg1 << 2;

		/* clear secondary result */
		res1 = fileerr = 0;

		switch (dp->dp_Action) {
		case ACTION_WRITE:
			res1 = write((struct lock *) arg1,
				     (CPTR) dp->dp_Arg2, dp->dp_Arg3);
			break;
		case ACTION_READ:
			res1 = read((struct lock *) arg1,
				    (CPTR) dp->dp_Arg2, dp->dp_Arg3);
			break;
		case ACTION_EXAMINE_OBJECT:
		case ACTION_EXAMINE_FH:
			res1 = exnext(dp,0);
			break;
		case ACTION_EXAMINE_NEXT:
			res1 = exnext(dp,1);
			break;
		case ACTION_EXAMINE_ALL:
			res1 = exnext(dp,2);
			break;
		case ACTION_SET_FILE_SIZE:
			flag = TRUE;
			goto seekit;
		case ACTION_SEEK:
			flag = FALSE;
seekit:
			res1 = seek((struct lock *) arg1,
				    dp->dp_Arg2, dp->dp_Arg3,flag);
			break;
		case ACTION_LOCATE_OBJECT:
			res1 = (LONG) locate((struct lock *) arg1,
					     BtoC(str1,dp->dp_Arg2),
					     dp->dp_Arg3);
			goto common_bptr;
		case ACTION_FREE_LOCK:
			res1 = freelock((struct lock *) arg1);
			break;
		case ACTION_PARENT_FH:
		case ACTION_COPY_DIR_FH:
			/* a filehandle arg1 is a lock - nice */
		case ACTION_PARENT:
		case ACTION_COPY_DIR:	/* aka DupLock */
			res1 = (LONG) parentfh((struct lock *) arg1,
					       dp->dp_Action);
			goto common_bptr;
		case MODE_READWRITE:
			/* DateStamp(root->date); */	/* why? FIX */
			flag = 2;
			goto open;
		case MODE_OLDFILE:
			flag = 1;
			goto open;
		case MODE_NEWFILE:
			flag = 0;
open:
			res1 = openmakefile(
				       (struct FileHandle *) arg1,
				       (struct lock *) (dp->dp_Arg2 << 2),
				       BtoC(str1,dp->dp_Arg3),
				       flag);
			break;
		case ACTION_END:	/* aka Close(), does notify */
			res1 = closefile((struct lock *) arg1);
			break;
		case ACTION_READ_LINK:
			res1 = readlink((struct lock *) arg1,
					(char *) dp->dp_Arg2,
					(UBYTE *) dp->dp_Arg3,
					(ULONG) dp->dp_Arg4);
			break;
		case ACTION_RENAME_OBJECT:
			res1 = rename((struct lock *) arg1,
				      BtoC(str1,dp->dp_Arg2),
				      (struct lock *) (dp->dp_Arg3 << 2),
				      BtoC(str2,dp->dp_Arg4));
			break;
		case ACTION_DELETE_OBJECT:
			node = checklock((struct lock *) arg1);
			if (node)
			{
				res1 = delete(node,
					      BtoC(str1,dp->dp_Arg2),
					      TRUE);
			}
			break;
		case ACTION_SAME_LOCK:
			node = checklock((struct lock *) arg1);
			if (node)
			{
				arg1 = (LONG) checklock((struct lock *)
							BADDR(dp->dp_Arg2));
				if (arg1 == (LONG) node)
					res1 = TRUE;
			}
			break;
		case ACTION_CURRENT_VOLUME:
			res1 = (LONG) volumenode;
			goto common_bptr;
		case ACTION_SET_COMMENT:
		case ACTION_SET_PROTECT:
		case ACTION_SET_DATE:
			res1 = comment(dp,BtoC(str1,dp->dp_Arg3));
			break;
		case ACTION_INFO:
			if (!checklock((struct lock *) arg1))
			{
				fileerr = ERROR_OBJECT_IN_USE;
				break;
			}
			/* fall through */
		case ACTION_DISK_INFO:
			res1 = diskinfo(dp);
			break;
		case ACTION_FLUSH:
			res1 = DOS_TRUE;
			break;
		case ACTION_RENAME_DISK:
			res1 = renamedisk(dp);
			break;
		case ACTION_ADD_NOTIFY:
			res1 = addnotify((struct NotifyRequest *) dp->dp_Arg1);
			break;
		case ACTION_REMOVE_NOTIFY:
			res1 = remnotify((struct NotifyRequest *) dp->dp_Arg1);
			break;
		case ACTION_DIE:
			fileerr = ERROR_OBJECT_IN_USE;
			break;
		case ACTION_CREATE_DIR:
			/* by default, creates a directory */
			res1 = (LONG) create((struct lock *) arg1,
					     BtoC(str1,dp->dp_Arg2),
					     EXCLUSIVE_LOCK,TRUE);
			goto common_bptr;
		case ACTION_MAKE_LINK:
			res1 = makelink((struct lock *) arg1,
					 BtoC(str1,dp->dp_Arg2),
					 dp->dp_Arg3,
					 dp->dp_Arg4);
			break;
		case ACTION_IS_FILESYSTEM:
			res1 = DOS_TRUE;
			break;
		case ACTION_FH_FROM_LOCK:
			res1 = openfromlock((struct FileHandle *) arg1,
					    (struct lock *) BADDR(dp->dp_Arg2));
			break;
		case ACTION_CHANGE_MODE:
			/* check type for correctness - note that for */
			/* ramdisk, lock == filehandle!! */

			if (arg1 == CHANGE_LOCK << 2)
			{
				res1 = change_lock((struct lock *)
						    BADDR(dp->dp_Arg2),
						   dp->dp_Arg3);

			} else if (arg1 == CHANGE_FH << 2) {

				/* NASTY! FH is passed, not arg1! */
				res1 = change_lock((struct lock *)
					   BADDR(((struct FileHandle *)
					    BADDR(dp->dp_Arg2))->fh_Arg1),
				  dp->dp_Arg3 == MODE_NEWFILE ? EXCLUSIVE_LOCK :
								SHARED_LOCK);
				/* FIX! handle other opening modes! */
			} else
				fileerr = ERROR_OBJECT_WRONG_TYPE;
			break;
		case ACTION_LOCK_RECORD:
			res1 = lockrecord((struct lock *) arg1,dp);
			if (res1 == 1)
				continue;	/* Don't reply pkt!!! */
			break;

		case ACTION_FREE_RECORD:
			res1 = unlockrecord((struct lock *) arg1,
					    dp->dp_Arg2,dp->dp_Arg3);
			break;

common_bptr:	/* change res1 to a bptr - to save code space */
			res1 = TOBPTR(res1);
			break;

		case ACTION_INHIBIT:
		case ACTION_GET_BLOCK:
		case ACTION_MORE_CACHE:
		default:
			fileerr = ERROR_ACTION_NOT_KNOWN;
		} /* switch */

		/* all go here, except sometimes lockrecord */
		DBUG2("returning packet: res1 0x%lx  res2 %ld",res1,fileerr);
		ReplyPkt(dp,res1,fileerr);

	   } /* while (msg = Getmsg) */
	} /* while (1) */

punt:
	if (MyReplyPort)
		DeleteMsgPort(MyReplyPort);
	if (timerport)
		DeleteMsgPort(timerport);

	if (root)
		freevec(root);

	ReplyPkt(dp,FALSE,FALSE);

	/* exit!  Implicit Permit() */
}
