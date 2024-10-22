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

extern	__far	ULONG	ROMData[];

void
start (struct DosPacket *dp)
{
	struct Message    *msg;
	struct DeviceList *devnode,*newnode;
	struct node *node;
	LONG res1;
	LONG arg1;	/* dp->dp_Arg1 << 2 */
	char str1[256];	/* FIX!!!!!!!!!! */
	ULONG	*romData;
	ULONG	size;

	/* guaranteed not to go to disk */
	DOSBase = (struct DosLibrary *) TaggedOpenLibrary(OLTAG_DOS);
	UtilityBase = TaggedOpenLibrary(OLTAG_UTILITY);

	/* setup stuff - get packet from DOS */
	MyPort = &(((struct Process *) FindTask(NULL))->pr_MsgPort);

	root = (struct node *) AllocVec(sizeof(struct node),MEMF_CLEAR);
	if (!root)
		goto punt;

	root->type = ST_USERDIR;

	root->name="ROM Disk";

	lock_list  = NULL;
	spaceused  = 0;

	/*
	 * Now build the disk structure...
	 * ROMData contains the following:
	 * <size> 0 (ULONG - number of longwords to next block)
	 * <name> 1 (32 bytes)
	 * <data> 9 (includes:  NULL next pointer, SIZE, data...)
	 * <size> ... (repeats until size is 0)
	 */
	romData=ROMData;
	while (size=*romData)
	{
	struct	node	*fileNode;

		if (fileNode=AllocVec(sizeof(struct node),MEMF_CLEAR))
		{
			fileNode->next=root->list;
			root->list=fileNode;
			fileNode->back=root;
			fileNode->list=(struct node *)&(romData[9]);
			spaceused+=(fileNode->size=romData[10]-8);
			DateStamp((struct DateStamp *) fileNode->date);
			fileNode->name=(char *)&romData[1];
			fileNode->type=ST_FILE;
		}
		romData+=size;
	}

	/* Set up first, before filling in task id. */

	newnode = (struct DeviceList *) MakeDosEntry(root->name,DLT_VOLUME);
	if (!newnode)
		goto punt;			/* exiting is permission */

	newnode->dl_Task     = MyPort;
	newnode->dl_DiskType = ID_DOS_DISK;
	DateStamp((struct DateStamp *) &(newnode->dl_VolumeDate));

	/* To let an icon show up */
#if	1
	if (!AddDosEntry((struct DosList *) newnode))
	{
		/* some sort of error, probably name collision */
		FreeDosEntry((struct DosList *) newnode);
		goto punt;
	}
#endif

	/* fill in device node with process pointer, etc */
	/* arg1 = string, arg2 = startup msg, arg3 = device node */
	/* these fields are inaccurate, it's a union */

	devnode = (struct DeviceList *) (dp->dp_Arg3 << 2);

	devnode->dl_Task = MyPort;
	devnode->dl_LockList = 0;
	devnode->dl_DiskType = 0;
	devnode->dl_unused   = 0;		/* globvec */
	DateStamp((struct DateStamp *) root->date);

	/* device node set up */

	volumenode = newnode;

	ReplyPkt(dp,DOS_TRUE,0);

	while (1) {

	    WaitPort(MyPort);

	    /* handle dospackets */
	    while ((msg = GetMsg(MyPort)) != NULL)
	    {
		dp = (struct DosPacket *) msg->mn_Node.ln_Name;

		/* shift to save code later - used a lot */
		arg1 = dp->dp_Arg1 << 2;

		/* clear secondary result */
		res1 = fileerr = 0;

		switch (dp->dp_Action)
		{
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
		case ACTION_SEEK:
			res1 = seek((struct lock *) arg1,
				    dp->dp_Arg2, dp->dp_Arg3);
			break;
		case ACTION_LOCATE_OBJECT:
			res1 = (LONG) locate((struct lock *) arg1,
					     BtoC(str1,dp->dp_Arg2),
					     dp->dp_Arg3);
			goto common_bptr;
		case ACTION_FREE_LOCK:
		case ACTION_END:
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
		case MODE_OLDFILE:
			res1 = openmakefile(
				       (struct FileHandle *) arg1,
				       (struct lock *) (dp->dp_Arg2 << 2),
				       BtoC(str1,dp->dp_Arg3));
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
		case ACTION_DIE:
			fileerr = ERROR_OBJECT_IN_USE;
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

common_bptr:	/* change res1 to a bptr - to save code space */
			res1 = TOBPTR(res1);
			break;

		case ACTION_DELETE_OBJECT:
		case ACTION_WRITE:
		case ACTION_RENAME_OBJECT:
		case ACTION_SET_COMMENT:
		case ACTION_SET_PROTECT:
		case ACTION_SET_DATE:
		case ACTION_SET_FILE_SIZE:
		case MODE_NEWFILE:
		case ACTION_RENAME_DISK:
		case ACTION_CREATE_DIR:
			fileerr = ERROR_DISK_WRITE_PROTECTED;
			break;

		case ACTION_FREE_RECORD:
		case ACTION_LOCK_RECORD:
		case ACTION_ADD_NOTIFY:
		case ACTION_REMOVE_NOTIFY:
		case ACTION_READ_LINK:
		case ACTION_MAKE_LINK:
		case ACTION_INHIBIT:
		case ACTION_GET_BLOCK:
		case ACTION_MORE_CACHE:
		default:
			fileerr = ERROR_ACTION_NOT_KNOWN;
		} /* switch */

		/* all go here, except sometimes lockrecord */
		ReplyPkt(dp,res1,fileerr);

	   } /* while (msg = Getmsg) */
	} /* while (1) */

punt:
	if (root)
		freevec(root);

	ReplyPkt(dp,FALSE,FALSE);

	/* exit!  Implicit Permit() */
}
