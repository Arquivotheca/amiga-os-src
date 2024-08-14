/*
 * $Id: copy.c,v 39.2 93/02/15 15:03:03 mks Exp $
 *
 * $Log:	copy.c,v $
 * Revision 39.2  93/02/15  15:03:03  mks
 * Changed the names of the Copy tasks
 * 
 * Revision 39.1  92/06/11  15:47:15  mks
 * Now use FORBID and PERMIT macros rathern than calling Forbid() and Permit()
 *
 * Revision 38.6  92/05/30  17:59:20  mks
 * Now uses ThisTask() rather than FindTask(NULL) (ThisTask is a stub)
 *
 * Revision 38.5  92/05/30  16:55:57  mks
 * Now uses the GETMSG/PUTMSG stubs
 *
 * Revision 38.4  91/11/12  18:47:12  mks
 * Copy now preserves dates  (Now uses the stub too)
 *
 * Revision 38.3  91/08/27  16:46:16  mks
 * One more time...  (Ooops, got argument wrong)
 *
 * Revision 38.2  91/08/27  16:41:53  mks
 * Copy now preserves the date...
 *
 * Revision 38.1  91/06/24  11:34:02  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "exec/types.h"
#include "exec/ports.h"
#include "exec/memory.h"
#include "dos/dostags.h"
#include "proto.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "wbinternal.h"
#include "global.h"
#include "support.h"
#include "quotes.h"

extern	void	*DosCopySegmentW;
extern	void	*DosCopySegment;

/*
 * Message structure that is passed between the reader and writter code
 */

struct	WBCopyMessage		/* Base copy message */
{
struct	Message	wbc_Message;
	UWORD	wbc_Code;
	UWORD	wbc_pad_long;
	ULONG	wbc_errCode;	/* Used if WBC_Code is WBC_ERROR and startup */
};

/*
 * If we had an error that was already handled, we do this...
 */
#define	WB_SKIP_ERROR	(-1)


#define	WBC_SET_DEST	1	/* Set the destination lock */
struct	WBC_Set_Dest
{
struct	WBCopyMessage	msg;
	BPTR		dlock;	/* Lock on the directory to go into */
	LONG		samedisk;	/* Flag = TRUE if on same disk */
};

#define	WBC_CLEAR_DEST	2	/* Clear destination...  This writer task */
				/* Will then CD back to RAM:   This is */
				/* the same as DONE but the writer does */
				/* not exit.  (Note that the cm_errCode */
				/* field is used for a pointer to a CopyMsg */

#define	WBC_NEW_DIR	3	/* Make a new directory and enter it */
struct	WBC_FileOpen_NewDir	/* Structure of file open or dir message */
{
struct	WBCopyMessage	msg;
	char		*Name;	/* File or directory name */
};

#define	WBC_EXIT_DIR	4	/* Exit the directory and go back up a level */
				/* No extended message needed here */

#define	WBC_FILEOPEN	5	/* File open code... */
				/* Uses the WBC_FileOpen_NewDir message */

#define	WBC_FILEDATA	6	/* Data to be written to the open file */
struct	WBC_FileData
{
struct	WBCopyMessage	msg;
	ULONG		Size;	/* Size of data */
	UBYTE		dat[1];	/* Data array of the size given above */
};

#define	WBC_FILECLOSE	7	/* Close the open file */

#define	WBC_SET_ATTR	8	/* Set the attributes of the named object */
struct	WBC_Set_Attr
{
struct	WBCopyMessage	msg;
	char		*Name;		/* Name of object */
	ULONG		Prot;		/* Protection bits of object */
struct	DateStamp	Date;		/* Date of object */
	char		*Comment;	/* Comment of object */
};

#define	WBC_MUST_GO	9	/* Writer/reader must go now... */
				/* This gets sent when one side is out of */
				/* data and needs the other side to go. */

#define	WBC_ERROR	10	/* This is sent by the writer when there */
				/* is an error. */

#define	WBC_STARTUP	11	/* This message is actually sent to the */
				/* Process port.  The return in errCode */
				/* is the message port generated or NULL */

#define	WBC_QUIT	12	/* This message is sent to the real port */
				/* by the launcher if it wants the reader */
				/* to DIE...  If the wbc_errCode comes */
				/* back TRUE, it really died... */

#define	WBC_COPYMSG	13	/* This is the message that is sent by */
				/* The launch code to start the copy... */
/*
 * The message that is used to start an individual copy
 */
struct	CopyMsg			/* Will have wbc_Code==WBC_COPYMSG */
{
struct	WBCopyMessage	msg;
struct	FileInfoBlock	*fib;	/* File info block ready for use... */
	BPTR		slock;	/* Source DIR lock...  Parent of object */
	char		*sname;	/* Source object name */
	BPTR		dlock;	/* Destination DIR lock... */
	char		*dname;	/* Destination object name */

/*
 * These are needed during the cleanup after the object is copied...
 */
	LONG		curx;	/* Destination X as passed to DosCopyLaunch */
	LONG		cury;	/* Destination Y as passed to DosCopyLaunch */
struct	WBObject	*fromobj;	/* The source drawer object */
struct	WBObject	*toobj;		/* The destination drawer object */

/*
 * This is the source's disk object...
 */
struct	DiskObject	*sobj;	/* If null, no icon... */

/*
 * This FLAG value is used to signal that the copy had started for this
 * object...
 */
	SHORT		Started;
};

struct	TypedCopyMsg
{
	WORD	tcm_type;	/* For Workbench MessageType */
struct	CopyMsg	tcm_CopyMsg;
};

long DoCopyRequest(long qGad,char *name,long third,long error)
{
struct	EasyStruct	es = {sizeof(struct EasyStruct),NULL,SystemWorkbenchName,NULL,NULL};
struct	Window		*win=(struct Window *)(((struct Process *)ThisTask())->pr_WindowPtr);
	char		*args[3];
	char		Buf[80];

	Fault(error,NULL,Buf,80);
	Buf[79]='\0';

	es.es_TextFormat=Quote(Q_COPY_ERROR_TEXT);
	es.es_GadgetFormat=Quote(qGad);

	args[0]=name;
	args[1]=Buf;
	args[2]=&Buf[79];

	if (third) args[2]=Quote(third);

	return(EasyRequestArgs(win,&es,NULL,args));
}

/*
 * CopyWriter - The main Writer process loop...
 *
 * The state machine in here assumes that the CopyReader will only
 * pass valid state transitions.  That is, this state machine is
 * not able to handle invalid state transitions with defined actions.
 * It is up to the state machine in the CopyReader to make sure it all
 * works out...
 */
void CopyWriter(struct MsgPort *myPort,struct FileInfoBlock *fib)
{
struct	Process		*pr=(struct Process *)ThisTask();
struct	WBCopyMessage	*msg;
	APTR		WindowPtr;
	LONG		ErrorState=NULL;/* Last error or NULL for none */
	BPTR		curFile=NULL;
	BPTR		oldDir=NULL;
	LONG		SameDisk=FALSE;	/* If we are on the same disk */
	LONG		ExitFlag=FALSE;
	LONG		WaitState=TRUE;
	LONG		MustGo=FALSE;
	LONG		Going=FALSE;	/* If writing is in progress */
	LONG		KeepIt;
	LONG		Started;	/* If we started the copy... */
	LONG		ReplaceAll=FALSE;
	LONG		DiskFull=FALSE;
struct	MinList		MsgQueue;
struct	MinList		DirStack;

	NewList((struct List *)&MsgQueue);
	NewList((struct List *)&DirStack);

	DP(("Writer is running...\n"));

	while (!ExitFlag)
	{
		if (WaitState)
		{	/* Wait for a message... */
			WAITPORT(myPort);
		}

		/*
		 * First get all new messages and queue them
		 * onto the message list.
		 * At this time, also check if we are given the *MUST GO*
		 * signal...
		 */
		while (msg=(struct WBCopyMessage *)GETMSG(myPort))
		{	/* We got a message, so queue it... */
#ifdef	DEBUGGING
			FORBID;
			DP(("Writer: Got message "));
			switch (msg->wbc_Code)
			{
			case WBC_SET_DEST:	DP(("WBC_SET_DEST"));	break;
			case WBC_CLEAR_DEST:	DP(("WBC_CLEAR_DEST"));	break;
			case WBC_NEW_DIR:	DP(("WBC_NEW_DIR"));	break;
			case WBC_EXIT_DIR:	DP(("WBC_EXIT_DIR"));	break;
			case WBC_FILEOPEN:	DP(("WBC_FILEOPEN"));	break;
			case WBC_FILEDATA:	DP(("WBC_FILEDATA"));	break;
			case WBC_FILECLOSE:	DP(("WBC_FILECLOSE"));	break;
			case WBC_SET_ATTR:	DP(("WBC_SET_ATTR"));	break;
			case WBC_MUST_GO:	DP(("WBC_MUST_GO"));	break;
			case WBC_ERROR:		DP(("WBC_ERROR = %ld",msg->wbc_errCode));	break;
			case WBC_STARTUP:	DP(("WBC_STARTUP"));	break;
			case WBC_QUIT:		DP(("WBC_QUIT"));	break;
			case WBC_COPYMSG:	DP(("WBC_COPYMSG"));	break;
			}
			DP(("\n"));
			PERMIT;
#endif	/* DEBUGGING */
			ADDTAIL((struct List *)&MsgQueue,(struct Node *)msg);

			if (msg->wbc_Code==WBC_MUST_GO)
			{	/* We _must_ write now... */
				MustGo=TRUE;
				DP(("Writer: MustGo!\n"));
			}
		}

		if (msg=(struct WBCopyMessage *)RemHead((struct List *)&MsgQueue))
		{
			if ((!Going) && (oldDir))
			{
				if ((MustGo) || (!SameDisk))
				{
					WindowPtr=pr->pr_WindowPtr;
					if (!MustGo) pr->pr_WindowPtr=(APTR)(-1);

					DP(("Writer: Checking destination volume\n"));
					oldDir=CURRENTDIR(oldDir);
					if (EXAMINE(oldDir,fib)) Going=TRUE;
					else if (MustGo) ErrorState=IOERR();
					oldDir=CURRENTDIR(oldDir);

					pr->pr_WindowPtr=WindowPtr;
				}
			}

			/*
			 * If we are in error state, we wish to make all of the
			 * stuff just go away...
			 */
			if (!ErrorState) ErrorState=DiskFull;
			if (ErrorState) Going=TRUE;

			DP(("Writer: Going=%ld, MustGo=%ld, WaitState=%ld, Error=%ld  : ",(LONG)Going,(LONG)MustGo,(LONG)WaitState,(LONG)ErrorState));

			KeepIt=FALSE;
			switch (msg->wbc_Code)
			{
			/*
			 * We are getting a new destination...
			 * Install it as the current directory...
			 */
			case WBC_SET_DEST:	DP(("WBC_SET_DEST"));
				oldDir=CURRENTDIR(((struct WBC_Set_Dest *)msg)->dlock);
				Going=FALSE;
				Started=FALSE;
				SameDisk=(((struct WBC_Set_Dest *)msg)->samedisk);
				break;

			/*
			 * Done with the current destination...
			 * Change back to where we should be...
			 * Clear the error state flag...
			 */
			case WBC_CLEAR_DEST:	DP(("WBC_CLEAR_DEST"));
				if (!Going) KeepIt=TRUE;
				else
				{
				struct	CopyMsg	*cm=(struct CopyMsg *)(msg->wbc_errCode);

					/*
					 * If no error and there is an icon,
					 * put that icon onto the disk...
					 */
					if (!ErrorState)
					{
						if ((cm->sobj) && (!(cm->msg.wbc_errCode)))
						{
							cm->sobj->do_CurrentX=cm->curx;
							cm->sobj->do_CurrentY=cm->cury;
							Started=TRUE;
							if (!PutDiskObject(cm->dname,cm->sobj))
							{
								cm->msg.wbc_errCode=IOERR();
							}
						}
					}

					/*
					 * Set the started flag...
					 */
					cm->Started=Started;

					/*
					 * Clean up now...
					 */
					CURRENTDIR(oldDir);
					Going=MustGo;
					oldDir=NULL;
					ErrorState=0;
				}
				break;

			/*
			 * We are going to make a new directory...
			 */
			case WBC_NEW_DIR:	DP(("WBC_NEW_DIR"));
				if (!Going) KeepIt=TRUE;
				else
				{
				BPTR	tmpLock;
				char	*name=((struct WBC_FileOpen_NewDir *)msg)->Name;

					msg->wbc_errCode=NULL;

					/*
					 * Check if we are in an error state
					 * and if so, don't make this new directory...
					 */
					if (!ErrorState)
					{
						if (tmpLock=CreateDir(name))
						{
							/*
							 * We have created something
							 */
							Started=TRUE;

							/*
							 * After creating the directory,
							 * we need a read access lock...
							 */
							UNLOCK(tmpLock);
							if (tmpLock=LOCK(name,ACCESS_READ))
							{
								/* Store it here... */
								msg->wbc_errCode=CURRENTDIR(tmpLock);
							}
							else ErrorState=IOERR();
						}
						else
						{
							ErrorState=IOERR();
							if (ErrorState==ERROR_OBJECT_EXISTS)
							{
								DoCopyRequest(Q_CANCEL_TEXT,name,NULL,ERROR_OBJECT_EXISTS);
								ErrorState=WB_SKIP_ERROR;
							}
						}
					}

					/* Store the directory */
					ADDHEAD((struct List *)&DirStack,(struct Node *)msg);
					msg=NULL;	/* No message to return */
				}
				break;

			/*
			 * We are told to exit a directory...
			 *
			 * This message will always end up returning two
			 * messages and it always works...
			 */
			case WBC_EXIT_DIR:	DP(("WBC_EXIT_DIR"));
				{
				struct	WBCopyMessage	*tm;

					tm=(struct WBCopyMessage *)RemHead((struct List *)&DirStack);
					/*
					 * Check if we have a lock stored
					 */
					if (tm->wbc_errCode)
					{
						UNLOCK(CURRENTDIR(tm->wbc_errCode));
						tm->wbc_errCode=NULL;
					}
					REPLYMSG((struct Message *)tm);
					if (Going) Going=MustGo;
				}
				break;

			/*
			 * A new file name has been given to us...
			 * Try to open it...  Check to make sure that
			 * the file is not there already...
			 */
			case WBC_FILEOPEN:	DP(("WBC_FILEOPEN"));
				if (!Going) KeepIt=TRUE;
				else if (!ErrorState)
				{
				char	*name=((struct WBC_FileOpen_NewDir *)msg)->Name;
				BPTR	tmpLock;

					/*
					 * Check if we can write the file...
					 */
					if (tmpLock=LOCK(name,ACCESS_READ))
					{
						if (!Examine(tmpLock,fib)) ErrorState=IOERR();
						UNLOCK(tmpLock);

						if (!ErrorState)
						{
						long	gad;

							/*
							 * Now check for the type of file...
							 */
							gad=Q_REPLACE_CANCEL_TEXT;
							if (fib->fib_DirEntryType > 0)
							{
								ErrorState=ERROR_OBJECT_EXISTS - ERROR_OBJECT_WRONG_TYPE;
								gad=Q_CANCEL_TEXT;
							}

							if ((!ReplaceAll) || (ErrorState))
							{
								ReplaceAll=DoCopyRequest(gad,name,NULL,ERROR_OBJECT_EXISTS - ErrorState);
								if (ReplaceAll) ReplaceAll--;
								else ErrorState=WB_SKIP_ERROR;
							}
						}
					}

					if (!ErrorState)
					{
						/*
						 * If the file does not open, set the
						 * error flag...
						 */
						if (!(curFile=OPEN_fh(name,MODE_NEWFILE)))
						{
							ErrorState=IOERR();
						}
						else Started=TRUE;
					}
				}
				break;

			/*
			 * Some data is coming in for the open file...
			 */
			case WBC_FILEDATA:	DP(("WBC_FILEDATA"));
				if (!Going) KeepIt=TRUE;
				else if (!ErrorState)
				{
				char	*data=((struct WBC_FileData *)msg)->dat;
				LONG	size=((struct WBC_FileData *)msg)->Size;

					if ((size) && (curFile))
					{
						if (size!=WRITE_fh(curFile,data,size))
						{
							ErrorState=IOERR();
						}
					}
				}
				break;

			/*
			 * The currently open file needs to be closed...
			 * If curFile is NULL and we are not in error state
			 * things are rather strange...  Since we would have
			 * to check either curFile or ErrorState, we do the
			 * safer of the two...
			 */
			case WBC_FILECLOSE:	DP(("WBC_FILECLOSE"));
				if (!Going) KeepIt=TRUE;
				else if (curFile)
				{
					if (!CLOSE_fh(curFile)) ErrorState=IOERR();
					curFile=NULL;
					Going=MustGo;
				}
				break;

			/*
			 * Set the attributes of the object given...
			 */
			case WBC_SET_ATTR:	DP(("WBC_SET_ATTR"));
				if (!Going) KeepIt=TRUE;
				else if (!ErrorState)
				{
				struct	WBC_Set_Attr	*p=(struct WBC_Set_Attr *)msg;

					SETCOMMENT(p->Name,p->Comment);
					SETPROTECTION(p->Name,p->Prot);
					SETFILEDATE(p->Name,&(p->Date));
				}
				break;

			/*
			 * We got this message back when we had to start to
			 * write data (for one reason or another)
			 * Now we got to that point where the message came,
			 * we will want to pass it back and clear the MustGo
			 * flag.
			 */
			case WBC_MUST_GO:	DP(("WBC_MUST_GO"));
				MustGo=FALSE;
				Going=FALSE;
				break;

			/*
			 * If we get an error message, we set the state...
			 */
			case WBC_ERROR:		DP(("WBC_ERROR"));
				if (!ErrorState) ErrorState=msg->wbc_errCode;
				break;

			/*
			 * We got a QUIT message...  Since we know that the
			 * only way we can get this is if we are ready to
			 * quit, we don't need any checks...
			 * We FORBID here and then, when we fall off the
			 * process, we know that we are done before the
			 * quit message returns to the system...
			 */
			case WBC_QUIT:		DP(("WBC_QUIT"));
				ExitFlag=TRUE;
				FORBID;	/* We are exiting... */
				break;

			/*
			 * End of switch()
			 */
			}

			if (ErrorState==ERROR_DISK_FULL) DiskFull=ErrorState;
			/*
			 * If we want to keep the message, we add it back
			 * to the top of my list...
			 */
			if (KeepIt)
			{
				ADDHEAD((struct List *)&MsgQueue,(struct Node *)msg);
				DP(("  KeepIt"));
			}
			else
			{
				if (msg)
				{
					/*
					 * If we are in error state, we
					 * return most messages with the error
					 *
					 * The ones listed below are needed for
					 * the state machine in the main code...
					 */
					if ( (msg->wbc_Code != WBC_MUST_GO) &&
					     (msg->wbc_Code != WBC_CLEAR_DEST) &&
					     (msg->wbc_Code != WBC_QUIT) &&
					     (ErrorState) )
					{
						DP(("  RetError %ld",ErrorState));
						msg->wbc_Code=WBC_ERROR;
						msg->wbc_errCode=ErrorState;
					}
					REPLYMSG((struct Message *)msg);
				}
				WaitState=FALSE;
			}

			DP(("\n"));
		}
		else WaitState=TRUE;
	}
}

/*
 * Send message to port...
 */
void SendWBCMsg(struct WBCopyMessage *msg,LONG *count,UWORD code,struct MsgPort *myPort)
{
struct	WorkbenchBase	*wb=getWbBase();

	msg->wbc_Message.mn_ReplyPort=myPort;
	msg->wbc_Code=code;
	if (count) (*count)++;
	PUTMSG(wb->wb_CopyWriterPort,&(msg->wbc_Message));
}

/*
 * This routine returns a Copy message to the free pool.
 */
void FreeMyMsg(void *msg);

/*
 * This routine will only allocate the message as long as it does not
 * reduce the chip memory available below a set minimum
 */
void *AllocMyMsg(ULONG size);

/* When debugging, we use these allocation routines... */
#ifdef	DEBUGGING

void DebugFreeMyMsg(void *msg)
{
	FORBID;
	DP(("Free  copy message %8lx\n",msg));
	PERMIT;
	FreeMyMsg(msg);
}

void *DebugAllocMyMsg(ULONG size)
{
void *tmp;

	if (tmp=AllocMyMsg(size))
	{
		FORBID;
		DP(("Alloc copy message %8lx  size of %ld\n",tmp,size));
		PERMIT;
	}
	return(tmp);
}

#define	FreeMyMsg(x)	DebugFreeMyMsg(x)
#define	AllocMyMsg(x)	DebugAllocMyMsg(x)

#endif	/* DEBUGGING */

/*
 * Allocate a OpenFile/NewDir message
 */
struct WBC_FileOpen_NewDir *AllocMyNameMsg(char *name)
{
struct	WBC_FileOpen_NewDir	*p;

	if (p=AllocMyMsg(sizeof(struct WBC_FileOpen_NewDir)+strlen(name)+1))
	{
		p->Name=(char *)(((ULONG)p)+sizeof(struct WBC_FileOpen_NewDir));
		strcpy(p->Name,name);
	}
	return(p);
}

/*
 * Reader outer state information...
 */
#define	R_EXIT_STATE	0
#define	R_WAITING_STATE	1		/* Nothing to do, so wait on port */
#define	R_SENDING_STATE	(1L << 1)	/* We are still activily sending */
#define	R_FLUSH_STATE	(1L << 2)	/* We are flushing messages */
#define	R_QUIT_STATE	(1L << 3)	/* We wish to quit */

/*
 * These defines are used by the main state loop to define what is to be done
 * next time around...
 */
#define	n_NEWCOPY		0	/* Start a new copy */
#define	n_NEWCOPY_1		1
#define	n_NEWCOPY_2		2

#define	n_NEWCOPY_TREE		3	/* We are going to copy a tree */

#define	n_NEWCOPY_END		4	/* End of a new copy */
#define	n_NEWCOPY_END_1		5

#define	n_NEWCOPY_ERROR		6	/* Error, could not even start */

#define	n_NEWCOPY_DIR		7	/* Start a new copy that is a DIR */

#define	n_NEWCOPY_FILE		8	/* Start a new copy that is a FILE */
#define	n_NEWCOPY_FILE_1	9
#define	n_NEWCOPY_FILE_2	10

#define	n_DIRCOPY		11	/* Main directory copy loop... */

#define	n_DIRCOPY_DIR		12	/* Nest directories... */

#define	n_DIRCOPY_END		13	/* We hit the end of a directory */
#define	n_DIRCOPY_END_1		14

#define	n_DIRCOPY_FILE		15	/* Next file in directory... */
#define	n_DIRCOPY_FILE_1	16

#define	n_FILECOPY		17	/* Main file copy loop... */

#define	n_FILECLOSE		18	/* End of file copy... */
#define	n_FILECLOSE_1		19	/* Close the local file... */

#define	n_SET_ATTR		20	/* Set object attribute state... */

/*
 * Structure that is used to nest directories...
 */
struct	DirNest
{
struct	MinNode		node;
	BPTR		lock;	/* Lock of the directory... */
struct	FileInfoBlock	fib;	/* File info block for directory... */
};

/*
 * CopyReader - The main Reader process loop...
 */
void CopyReader(struct MsgPort *myPort)
{
struct	WBCopyMessage	*msg;
struct	WBCopyMessage	*QuitMsg=NULL;
struct	CopyMsg		*curCopy=NULL;
	ULONG		ReaderState=R_WAITING_STATE | R_SENDING_STATE;
	LONG		DoNext=n_NEWCOPY;
	LONG		LeftOutMessages=0;
	LONG		LeftCopyMsg=0;
	LONG		LeftOutDir=0;
	LONG		newError;
	LONG		*curError=NULL;
	LONG		*OldError=NULL;
	LONG		DiskFull=FALSE;
	BPTR		oldDir;
	BPTR		curDir=NULL;
	BPTR		curFile=NULL;
	char		*NextDirName;
struct	FileInfoBlock	*fib=NULL;
struct	MinList		CopyQueue;
struct	MinList		DoneQueue;
struct	MinList		DirectoryList;
struct	WBCopyMessage	LastChanceMsg;	/* Used to send a message when it */
struct	WBCopyMessage	LastChanceErr;	/* "absolutely, positively must" */
struct	WBCopyMessage	LastChanceExt;	/* in case of needed exit... */

	NewList((struct List *)&CopyQueue);	/* Copy requests as they come in */
	NewList((struct List *)&DoneQueue);	/* Copy requests that were handled but not 100% done */
	NewList((struct List *)&DirectoryList);	/* Directory nesting list */
	LastChanceMsg.wbc_Message.mn_Node.ln_Type=NT_UNKNOWN;

	DP(("Reader is running...\n"));

	while (ReaderState)
	{
		newError=NULL;	/* No new error */

		if (ReaderState & R_WAITING_STATE)
		{	/* Wait for a message since we are not busy */
			WAITPORT(myPort);
		}
		if (msg=(struct WBCopyMessage *)GETMSG(myPort))
		{
#ifdef	DEBUGGING
			FORBID;
			DP(("Reader: Got message "));
			switch (msg->wbc_Code)
			{
			case WBC_SET_DEST:	DP(("WBC_SET_DEST"));	break;
			case WBC_CLEAR_DEST:	DP(("WBC_CLEAR_DEST"));	break;
			case WBC_NEW_DIR:	DP(("WBC_NEW_DIR"));	break;
			case WBC_EXIT_DIR:	DP(("WBC_EXIT_DIR"));	break;
			case WBC_FILEOPEN:	DP(("WBC_FILEOPEN"));	break;
			case WBC_FILEDATA:	DP(("WBC_FILEDATA"));	break;
			case WBC_FILECLOSE:	DP(("WBC_FILECLOSE"));	break;
			case WBC_SET_ATTR:	DP(("WBC_SET_ATTR"));	break;
			case WBC_MUST_GO:	DP(("WBC_MUST_GO"));	break;
			case WBC_ERROR:		DP(("WBC_ERROR = %ld",msg->wbc_errCode));	break;
			case WBC_STARTUP:	DP(("WBC_STARTUP"));	break;
			case WBC_QUIT:		DP(("WBC_QUIT"));	break;
			case WBC_COPYMSG:	DP(("WBC_COPYMSG"));	break;
			}
			DP(("\n"));
			PERMIT;
#endif	/* DEBUGGING */
			switch (msg->wbc_Code)
			{
			case WBC_CLEAR_DEST:	/* End of a copy... */
						DP(("Reader: Returning copy message, error = %ld\n",((struct CopyMsg *)(msg->wbc_errCode))->msg.wbc_errCode));
						/*
						 * Note when the error is a DiskFull error and set as needed...
						 */
						if (!DiskFull)
						{
							DiskFull=((struct CopyMsg *)(msg->wbc_errCode))->msg.wbc_errCode;
							if (DiskFull!=ERROR_DISK_FULL) DiskFull=FALSE;
						}
						REPLYMSG((struct Message *)RemHead((struct List *)&DoneQueue));
						LeftCopyMsg--;
						if (DoneQueue.mlh_Head->mln_Succ)
						{
							OldError=&(((struct CopyMsg *)(DoneQueue.mlh_Head))->msg.wbc_errCode);
						}
						else OldError=curError;
						break;
			case WBC_COPYMSG:	/* Queue up the requests... */
						ADDTAIL((struct List *)&CopyQueue,(struct Node *)msg);
						LeftCopyMsg++;
						msg=NULL;
						break;
			case WBC_QUIT:		/* Time to quit... */
						QuitMsg=msg;
						msg=NULL;
						ReaderState |= R_QUIT_STATE;
						break;
			case WBC_MUST_GO:	/* Writer wants me to go... */
						ReaderState &= ~R_FLUSH_STATE;
						ReaderState &= ~R_WAITING_STATE;
						ReaderState |= R_SENDING_STATE;
						break;
			case WBC_ERROR:		/* We got an error */
						if (!(*OldError)) if (msg->wbc_errCode) *OldError=msg->wbc_errCode;
						break;
			}
		}

		/* Check if we have a message to free */
		if (msg)
		{
			if (!((msg==&LastChanceMsg) || (msg==&LastChanceErr) || (msg==&LastChanceExt)))
			{
				FreeMyMsg(msg);
			}
			LeftOutMessages--;
		}

		/*
		 * Turn off waiting state...
		 */
		ReaderState &= ~R_WAITING_STATE;

		/*
		 * If we are in FLUSH state we also want to be WAITING...
		 */
		if (ReaderState & R_FLUSH_STATE)
		{
			/* If we are flushing we do nothing much */
			/* other than setting the R_WAITING_STATE flag */
			ReaderState |= R_WAITING_STATE;
		}
		else
		{
			switch(DoNext)
			{
			/*
			 * We need to pull in a new copy message if possible
			 */
			case n_NEWCOPY:		DP(("Reader: State n_NEWCOPY : LOM=%ld\n",LeftOutMessages));
				if (curCopy=(struct CopyMsg *)RemHead((struct List *)&CopyQueue))
				{
					curError=&(curCopy->msg.wbc_errCode);

					/*
					 * Set the initial error code to NULL or DiskFull.
					 */
					*curError=DiskFull;
					if (!OldError) OldError=curError;
					DoNext=n_NEWCOPY_1;
				}
				else
				{	/* Set wait, and flush if something there... */
					ReaderState |= R_WAITING_STATE;
					if (LeftOutMessages) ReaderState |= R_FLUSH_STATE;
				}
				break;
			case n_NEWCOPY_1:	DP(("Reader: State n_NEWCOPY_1 : LOM=%ld\n",LeftOutMessages));
				if (*curError) DoNext=n_NEWCOPY_ERROR;
				else
				{
				struct	WBC_Set_Dest	*p;

					if (p=AllocMyMsg(sizeof(struct WBC_Set_Dest)))
					{
						p->dlock=curCopy->dlock;
						/*
						 * Lets see if the source and destination are on
						 * the same device.
						 */
						p->samedisk=SAMEDEVICE(curCopy->dlock,curCopy->slock);
						SendWBCMsg(p,&LeftOutMessages,WBC_SET_DEST,myPort);
						oldDir=CURRENTDIR(curCopy->slock);
						if (curCopy->sname) curCopy->sobj=GetDiskObject(curCopy->sname);
						else curCopy->sobj=GetDefDiskObject(WBDRAWER);
						fib=curCopy->fib;
						DoNext=n_NEWCOPY_2;
					}
					else ReaderState |= R_FLUSH_STATE;
				}
				break;
			case n_NEWCOPY_2:	DP(("Reader: State n_NEWCOPY_2 : LOM=%ld\n",LeftOutMessages));
				if (curCopy->sname) curDir=LOCK(curCopy->sname,ACCESS_READ);
				else curDir=DUPLOCK(curCopy->slock);
				if (curDir)
				{
					if (EXAMINE(curDir,fib))
					{
						if (fib->fib_DirEntryType > 0)
						{	/* We have a directory */
							DoNext=n_NEWCOPY_TREE;
						}
						else
						{
							DoNext=n_NEWCOPY_FILE;
						}
					}
					else
					{
						newError=IOERR();
						UNLOCK(curDir);
						curDir=NULL;
					}
				}
				if (!curDir)
				{
					/*
					 * We could not lock the source but
					 * that may not be an error, so...
					 */
					DoNext=n_NEWCOPY_END;
				}
				break;

			/*
			 * Handle it if there was a directory...
			 */
			case n_NEWCOPY_TREE:	DP(("Reader: State n_NEWCOPY_TREE : LOM=%ld\n",LeftOutMessages));
				{
				struct	DirNest	*dn;

					if (dn=ALLOCVEC(sizeof(struct DirNest),MEMF_PUBLIC|MEMF_CLEAR))
					{
						ADDHEAD((struct List *)&DirectoryList,(struct Node *)dn);
						dn->fib=*fib;
						NextDirName=curCopy->dname;
						DoNext=n_NEWCOPY_DIR;
					}
					else
					{
						newError=IOERR();
						DoNext=n_NEWCOPY_END;
					}
				}
				break;

			/*
			 * End of a copy stream...
			 */
			case n_NEWCOPY_END:	DP(("Reader: State n_NEWCOPY_END : LOM=%ld\n",LeftOutMessages));
				curDir=NULL;
				CURRENTDIR(oldDir);
				ADDTAIL((struct List *)&DoneQueue,(struct Node *)curCopy);
				DoNext=n_NEWCOPY_END_1;	/* Fall into this one... */
			case n_NEWCOPY_END_1:	DP(("Reader: State n_NEWCOPY_END_1 : LOM=%ld\n",LeftOutMessages));
				{
				struct	WBCopyMessage	*p;

					if (!(p=AllocMyMsg(sizeof(struct WBCopyMessage))))
					{
						/*
						 * We need to flush now...
						 */
						ReaderState |= R_FLUSH_STATE;

						/*
						 * If there are no messages still left out,
						 * we need to do something drastic...
						 */
						if (!LeftOutMessages)
						{
							p=&LastChanceExt;	/* Get the LastChance message */
						}
					}
					if (p)
					{
						/*
						 * Note that we are passing the CopyMsg
						 * in the wbc_errCode...
						 */
						p->wbc_errCode=(LONG)curCopy;
						curCopy=NULL;
						curError=NULL;
						SendWBCMsg(p,&LeftOutMessages,WBC_CLEAR_DEST,myPort);
						DoNext=n_NEWCOPY;
					}
				}
				break;

			/*
			 * We had a BIG	problem...  We could not even send the
			 * Set destination message...
			 */
			case n_NEWCOPY_ERROR:	DP(("Reader: State n_NEWCOPY_ERROR : LOM=%ld\n",LeftOutMessages));
				if (OldError==curError) OldError=NULL;
				REPLYMSG(curCopy);
				LeftCopyMsg--;
				DoNext=n_NEWCOPY;
				break;

			/*
			 * We have a directory to copy,	so set it up...
			 */
			case n_NEWCOPY_DIR:	DP(("Reader: State n_NEWCOPY_DIR : LOM=%ld\n",LeftOutMessages));
				if (*curError)
				{
				struct	DirNest	*dn;

					if (dn=(struct DirNest *)RemHead((struct List *)&DirectoryList))
					{
						UNLOCK(curDir);
						*fib=dn->fib;
						curDir=dn->lock;
						FREEVEC(dn);
						DoNext=n_DIRCOPY;
					}
					else DoNext=n_NEWCOPY_END;
				}
				else
				{
				struct	WBC_FileOpen_NewDir	*p;

					if (p=AllocMyNameMsg(NextDirName))
					{
						SendWBCMsg(p,&LeftOutDir,WBC_NEW_DIR,myPort);
						DoNext=n_DIRCOPY;
					}
					else ReaderState |= R_FLUSH_STATE;
				}
				break;

			/*
			 * We have a file to copy, so set it up...
			 */
			case n_NEWCOPY_FILE:	DP(("Reader: State n_NEWCOPY_FILE : LOM=%ld\n",LeftOutMessages));
				UNLOCK(curDir);
				curDir=NULL;
				DoNext=n_NEWCOPY_FILE_1;
			case n_NEWCOPY_FILE_1:	DP(("Reader: State n_NEWCOPY_FILE_1 : LOM=%ld\n",LeftOutMessages));
				if (curFile=OPEN_fh(curCopy->sname,MODE_OLDFILE))
				{
					DoNext=n_NEWCOPY_FILE_2;
				}
				else newError=IOERR();
				break;
			case n_NEWCOPY_FILE_2:	DP(("Reader: State n_NEWCOPY_FILE_2 : LOM=%ld\n",LeftOutMessages));
				if (*curError) DoNext=n_FILECLOSE_1;
				else
				{
				struct	WBC_FileOpen_NewDir	*p;

					if (p=AllocMyNameMsg(curCopy->dname))
					{
						SendWBCMsg(p,&LeftOutMessages,WBC_FILEOPEN,myPort);
						DoNext=n_FILECOPY;
					}
					else ReaderState |= R_FLUSH_STATE;
				}
				break;

			/*
			 * Main	directory copy loop...
			 */
			case n_DIRCOPY:		DP(("Reader: State n_DIRCOPY : LOM=%ld\n",LeftOutMessages));
				if (curDir)
				{
					if (EXNEXT(curDir,fib))
					{
						if (fib->fib_DirEntryType > 0)
						{
							DoNext=n_DIRCOPY_DIR;
						}
						/*
						 * A trick here:  We never copy plain ".info"
						 * files since they are not useful after they
						 * are copied.
						 */
						else if (stricmp(fib->fib_FileName,InfoSuf))
						{
							DoNext=n_DIRCOPY_FILE;
						}
					}
					else DoNext=n_DIRCOPY_END;
				}
				else DoNext=n_NEWCOPY_END;
				break;

			/*
			 * We just got a new directory, so go into it...
			 */
			case n_DIRCOPY_DIR:	DP(("Reader: State n_DIRCOPY_DIR : LOM=%ld\n",LeftOutMessages));
				if (*curError) DoNext=n_DIRCOPY;
				else
				{
				struct	DirNest	*dn;

					if (dn=ALLOCVEC(sizeof(struct DirNest),MEMF_PUBLIC|MEMF_CLEAR))
					{
					BPTR	tmplock;

						ADDHEAD((struct List *)&DirectoryList,(struct Node *)dn);
						dn->fib=*fib;
						dn->lock=curDir;
						curDir=CURRENTDIR(curDir);
						tmplock=LOCK(NextDirName=fib->fib_FileName,ACCESS_READ);
						curDir=CURRENTDIR(curDir);
						if (tmplock)
						{
						struct	DirNest	*tdn;
						/*
						 * We need to check that we are not in a directory loop
						 * These can happen when you have links...  (:-()
						 */
							tdn=(struct DirNest *)(dn->node.mln_Succ);	/* Get first old node... */
							while ((NextDirName) && (tdn->node.mln_Succ))
							{
								if (SAMELOCK(tmplock,tdn->lock)==LOCK_SAME)
								{
									DP(("Found a directory loop\n"));
									NextDirName=NULL;
								}
								tdn=(struct DirNest *)(tdn->node.mln_Succ);	/* Get next node... */
							}

							if (NextDirName)
							{
								curDir=tmplock;
								if (EXAMINE(curDir,fib)) DoNext=n_NEWCOPY_DIR;
								else NextDirName=NULL;
							}
						}
						else
						{
							/*
							 * We could not lock the next
							 * directory...
							 */
							NextDirName=NULL;
							newError=IOERR();
						}

						/*
						 * There was a problem, so clean up...
						 */
						if (!NextDirName)
						{
							UNLOCK(tmplock);	/* UNLOCK() handles NULL locks... */
							curDir=dn->lock;
							*fib=dn->fib;
							RemHead((struct List *)&DirectoryList);
							FREEVEC(dn);
							DoNext=n_DIRCOPY;
						}
					}
					else	/* We could not nest another level! */
					{
						newError=IOERR();
						DoNext=n_DIRCOPY;
					}
				}
				break;

			/*
			 * We are at the end of a directory...
			 * So we un-nest...
			 */
			case n_DIRCOPY_END:	DP(("Reader: State n_DIRCOPY_END : LOM=%ld\n",LeftOutMessages));
				{
				struct	DirNest	*dn;

					if (dn=(struct DirNest *)RemHead((struct List *)&DirectoryList))
					{
						UNLOCK(curDir);
						*fib=dn->fib;
						curDir=dn->lock;
						FREEVEC(dn);
						DoNext=n_DIRCOPY_END_1;
					}
					else DoNext=n_NEWCOPY_END;
				}
				break;
			case n_DIRCOPY_END_1:	DP(("Reader: State n_DIRCOPY_END_1 : LOM=%ld\n",LeftOutMessages));
				{
				struct	WBCopyMessage	*p;

					if (!(p=AllocMyMsg(sizeof(struct WBCopyMessage))))
					{
						/*
						 * We need to flush now...
						 */
						ReaderState |= R_FLUSH_STATE;

						/*
						 * If there are no messages still left out,
						 * we need to do something drastic...
						 */
						if (!LeftOutMessages)
						{
							p=&LastChanceExt;	/* Get the LastChance message */
						}
					}
					if (p)
					{
						SendWBCMsg(p,&LeftOutMessages,WBC_EXIT_DIR,myPort);
						/*
						 * Now, adjust the count of the left out messages,
						 * as the old NEW_DIR message is now going to come
						 * back and we need to count it and the number of
						 * LeftOut NEW_DIR messages has changed...
						 */
						LeftOutMessages++;
						LeftOutDir--;
						DoNext=n_SET_ATTR;
					}
				}
				break;

			/*
			 * We just got a new file so get ready to copy it...
			 */
			case n_DIRCOPY_FILE:	DP(("Reader: State n_DIRCOPY_FILE : LOM=%ld\n",LeftOutMessages));
				if (*curError) DoNext=n_DIRCOPY;
				else
				{
					curDir=CURRENTDIR(curDir);
					/*
					 * If we can open the file, we do,
					 * otherwise we just skip it...
					 *	This is just like COPY...  It can be
					 *	changed later...
					 */
					if (curFile=OPEN_fh(fib->fib_FileName,MODE_OLDFILE)) DoNext=n_DIRCOPY_FILE_1;
					else DoNext=n_DIRCOPY;
					curDir=CURRENTDIR(curDir);
				}
				break;
			case n_DIRCOPY_FILE_1:	DP(("Reader: State n_DIRCOPY_FILE_1 : LOM=%ld\n",LeftOutMessages));
				if (*curError) DoNext=n_FILECLOSE_1;
				else
				{
				struct	WBC_FileOpen_NewDir	*p;

					if (p=AllocMyNameMsg(fib->fib_FileName))
					{
						SendWBCMsg(p,&LeftOutMessages,WBC_FILEOPEN,myPort);
						DoNext=n_FILECOPY;
					}
					else ReaderState |= R_FLUSH_STATE;
				}
				break;

			/*
			 * We have a file in hand, so we start sending the data...
			 */
			case n_FILECOPY:	DP(("Reader: State n_FILECOPY : LOM=%ld\n",LeftOutMessages));
				/*
				 * So, we have a file, let us try to copy some of it...
				 */
				if (*curError) DoNext=n_FILECLOSE;
				else
				{
				LONG	size;

					size=fib->fib_Size - SEEK_fh(curFile,0,OFFSET_CURRENT);
					if (size)
					{
					struct	WBC_FileData	*p=NULL;
						LONG		memFree;

						/*
						 * For speed reasons, cache this...
						 */
						memFree=AvailMem(MEMF_PUBLIC);

						/*
						 * Ok, now make	sure we	don't try for more than
						 * half	the available memory...
						 */
						while ((size+size+1024) > memFree)
						{
							size-=1024;
						}

						/*
						 * If there is something to try for, we
						 * should try to do a safe allocation...
						 */
						if (size>0)
						{
							while ((!(p=AllocMyMsg(size+sizeof(struct WBC_FileData)))) && (size>32))
							{
								size=size >> 1;
							}
						}

						/*
						 * If we have a data packet, try to read and send it
						 */
						if (p)
						{
							size=READ_fh(curFile,p->dat,size);
							p->Size=size;
							if (size > 0)
							{
								SendWBCMsg(p,&LeftOutMessages,WBC_FILEDATA,myPort);
							}
							else
							{
								FreeMyMsg(p);
								if (size < 0)
								{	/* We got a read error... */
									newError=IOERR();
									DoNext=n_FILECLOSE;
								}
								else	/* We hit end of file for some reason... */
								{
									DoNext=n_FILECLOSE;
								}
							}
						}
						else ReaderState |= R_FLUSH_STATE;
					}
					else DoNext=n_FILECLOSE;
				}
				break;

			/*
			 * The file has been copied or there was an error.  Either
			 * way we must send the end of file message...
			 */
			case n_FILECLOSE:	DP(("Reader: State n_FILECLOSE : LOM=%ld\n",LeftOutMessages));
				{
				struct	WBCopyMessage	*p;

					if (!(p=AllocMyMsg(sizeof(struct WBCopyMessage))))
					{
						/*
						 * No memory, so lets try to flush...
						 */
						ReaderState |= R_FLUSH_STATE;

						/*
						 * If there are no messages still left out,
						 * we need to do something drastic...
						 */
						if (!LeftOutMessages)
						{
							p=&LastChanceExt;	/* Get the LastChance message */
						}
					}
					if (p)
					{
						SendWBCMsg(p,&LeftOutMessages,WBC_FILECLOSE,myPort);
						DoNext=n_FILECLOSE_1;
					}
				}
				break;

			/*
			 * The file needs to be closed...
			 */
			case n_FILECLOSE_1:	DP(("Reader: State n_FILECLOSE_1 : LOM=%ld\n",LeftOutMessages));
				CLOSE_fh(curFile);
				curFile=NULL;
				/*
				 * Go back and check for more files...
				 */
				DoNext=n_SET_ATTR;
				break;

			/*
			 * Send the attribute information for the last object...
			 *
			 * If this does not work, it will not care...  (No error)
			 */
			case n_SET_ATTR:	DP(("Reader: State n_SET_ATTR : LOM=%ld\n",LeftOutMessages));
				DoNext=n_DIRCOPY;
				if (!(*curError))
				{
				struct	WBC_Set_Attr	*p;
					LONG		size;
					char		*fname;

					/* Get the right name... */
					fname=fib->fib_FileName;
					if (!curDir) fname=curCopy->dname;

					size=sizeof(struct WBC_Set_Attr) +
						strlen(fib->fib_Comment) +
						strlen(fname) + 2;

					if (p=AllocMyMsg(size))
					{
						p->Name=(char *)(((ULONG)p)+sizeof(struct WBC_Set_Attr));
						strcpy(p->Name,fname);		/* Destination object name */

						p->Comment=&(p->Name[strlen(p->Name)+1]);
						strcpy(p->Comment,fib->fib_Comment);	/* Source comments */

						p->Prot=fib->fib_Protection;	/* Source protection bits */
						p->Date=fib->fib_Date;		/* Source date */

						/*
						 * New files are always deletable
						 * New files are always "changed"
						 */
						p->Prot &= ~(FIBF_DELETE | FIBF_ARCHIVE);

						SendWBCMsg(p,&LeftOutMessages,WBC_SET_ATTR,myPort);
					}

				}
				break;

			/*
			 * End of CASE statement...
			 */
			}

			/*
			 * Check if we asked for a FLUSH when we had no memory to do so
			 */
			if (ReaderState & R_FLUSH_STATE)
			{
				if (!LeftOutMessages)
				{
					DP(("Flush converted to an ERROR - no free store!\n"));
					newError=ERROR_NO_FREE_STORE;
					ReaderState &= ~R_FLUSH_STATE;
				}
			}

			/*
			 * If we want a	flush, go into that state...
			 */
			if (ReaderState & R_FLUSH_STATE)
			{
				SendWBCMsg(&LastChanceMsg,&LeftOutMessages,WBC_MUST_GO,myPort);
			}
			else if (newError)
			{
				if (!(*curError))
				{
					*curError=LastChanceErr.wbc_errCode=newError;
					DP(("Reader: Error #%ld\n",*curError));
					SendWBCMsg(&LastChanceErr,&LeftOutMessages,WBC_ERROR,myPort);
					SendWBCMsg(&LastChanceMsg,&LeftOutMessages,WBC_MUST_GO,myPort);
					ReaderState |= R_FLUSH_STATE;
				}
			}

			/*
			 * Check if we can set the quit state...
			 */
			if (ReaderState & R_QUIT_STATE)
			{
				DP(("Reader: Quit, LOM=%ld, LCM=%ld\n",LeftOutMessages,LeftCopyMsg));
				if ((!LeftOutMessages) && (!LeftOutDir) && (!LeftCopyMsg))
				{
					ReaderState=R_EXIT_STATE;
				}
			}
		}
	}
	/* Ok, now we blow away the Reader... */
	FORBID;
	SendWBCMsg(&LastChanceMsg,0,WBC_QUIT,myPort);
	WAITPORT(myPort);
	GETMSG(myPort);
	REPLYMSG(QuitMsg);
}

struct	WBCopyMessage	*GetStartUpMsg(void)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	Process		*pr=(struct Process *)ThisTask();
struct	MsgPort		*port=&(pr->pr_MsgPort);

	pr->pr_WindowPtr=wb->wb_BackWindow;
	WAITPORT(port);
	return((struct WBCopyMessage *)GETMSG(port));
}

void DosCopyW(void)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	WBCopyMessage	*msg;
struct	MsgPort		*myPort=NULL;
struct	FileInfoBlock	*fib;
	BPTR		lock;

	msg=GetStartUpMsg();
	if (lock=LockRamDisk())
	{
		myPort=CreateMsgPort();
		if (!(fib=ALLOCVEC(sizeof(struct FileInfoBlock),MEMF_PUBLIC)))
		{
			DeleteMsgPort(myPort);
			myPort=NULL;
		}
	}
	msg->wbc_errCode=(ULONG)myPort;
	REPLYMSG(msg);
	if (myPort)
	{
		/* We have started now, so go to main Reader loop... */
		lock=CURRENTDIR(lock);
		CopyWriter(myPort,fib);
		lock=CURRENTDIR(lock);

		/* We are exiting, so we blow away the port... */
		wb->wb_CopyWriterPort=NULL;
		DeleteMsgPort(myPort);
		FREEVEC(fib);
	}
	UNLOCK(lock);	/* UNLOCK checks for NULL */
}

/*
 * Start up the writer
 */
int InitDosCopyW(void)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	MsgPort		*port;
struct	MsgPort		*newport;
struct	WBCopyMessage	startit;
	int		result=FALSE;	/* We did not start */

	if (port=CreateMsgPort())	/* Get our reply port */
	{
		startit.wbc_Message.mn_ReplyPort=port;
		startit.wbc_Code=WBC_STARTUP;

		if (newport=CREATEPROC(CopyTaskName,2,MKBADDR(&DosCopySegmentW),3000))
		{
			PUTMSG(newport,&(startit.wbc_Message));
			WAITPORT(port);
			GETMSG(port);
			if (wb->wb_CopyWriterPort=((struct MsgPort *)(startit.wbc_errCode)))
			{
				result=TRUE;
			}
		}
		DeleteMsgPort(port);
	}

	return(result);
}

void DosCopy(void)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	WBCopyMessage	*msg;
struct	MsgPort		*myPort=NULL;
	BPTR		lock;

	msg=GetStartUpMsg();

	if (lock=LockRamDisk())
	{
		if (myPort=CreateMsgPort())
		{
			if (!InitDosCopyW())
			{
				DeleteMsgPort(myPort);
				myPort=NULL;
			}
		}
	}
	msg->wbc_errCode=(ULONG)myPort;
	REPLYMSG(msg);
	if (myPort)
	{
		/* We have started now, so go to main Reader loop... */
		lock=CURRENTDIR(lock);
		CopyReader(myPort);
		lock=CURRENTDIR(lock);

		/* We are exiting, so clear the port... */
		wb->wb_CopyReaderPort=NULL;
		DeleteMsgPort(myPort);
	}
	UNLOCK(lock);	/* UNLOCK checks for NULL */
}

/*
 * Start up the reader
 */
int InitDosCopy(void)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	MsgPort		*port;
struct	MsgPort		*newport;
struct	WBCopyMessage	startit;
	int		result=FALSE;	/* We did not start */

	if (wb->wb_CopyReaderPort) result=TRUE;
	else if (port=CreateMsgPort())	/* Get our reply port */
	{
		startit.wbc_Message.mn_ReplyPort=port;
		startit.wbc_Code=WBC_STARTUP;

		if (newport=CREATEPROC(CopyTaskName,0,MKBADDR(&DosCopySegment),3500))
		{
			PUTMSG(newport,&(startit.wbc_Message));
			WAITPORT(port);
			GETMSG(port);
			if (wb->wb_CopyReaderPort=((struct MsgPort *)(startit.wbc_errCode)))
			{
				result=TRUE;
			}
		}
		DeleteMsgPort(port);
	}

	return(result);
}

/*
 * Send the QUIT message to the reader...
 */
void UnInitDosCopy(void)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	MsgPort		*port;
struct	WBCopyMessage	stopit;

	if (!(wb->wb_AsyncCopyCnt))
	{
		if (wb->wb_CopyReaderPort)
		{
			if (port=CreateMsgPort())	/* Get our reply port */
			{
				stopit.wbc_Message.mn_ReplyPort=port;
				stopit.wbc_Code=WBC_QUIT;
				PUTMSG(wb->wb_CopyReaderPort,&(stopit.wbc_Message));
				WAITPORT(port);
				GETMSG(port);
				DeleteMsgPort(port);
			}
		}
	}
}

/*
 * Main entry point that starts this whole thing running...
 */
int DosCopyLaunch(char *fromname,char *toname,struct WBObject *fromdrawer,struct WBObject *todrawer,LONG curx, LONG cury,BPTR fromlock,BPTR tolock,UWORD type)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	TypedCopyMsg	*tcm;
struct	CopyMsg		*cm;

	DP(("DosCopyLaunch:  From (%s) to (%s)\n",fromname,toname));

	if (InitDosCopy())	/* Check for the Copy process... */
	{
		if (tcm=ALLOCVEC(sizeof(struct TypedCopyMsg),MEMF_PUBLIC|MEMF_CLEAR))
		{
		short	errFlag;

			/*
			 * Set workbench into IO waiting state
			 * and count another AsyncCopy...
			 */
			if (!wb->wb_AsyncCopyCnt++) BeginDiskIO();

			tcm->tcm_type=MTYPE_COPYEXIT;
			cm=&(tcm->tcm_CopyMsg);
			cm->msg.wbc_Message.mn_ReplyPort=&(wb->wb_WorkbenchPort);
			cm->msg.wbc_Code=WBC_COPYMSG;
			cm->curx=curx;
			cm->cury=cury;
			cm->fromobj=fromdrawer;
			cm->toobj=todrawer;

			/*
			 * Get and check the FIB allocation and the
			 * duplication of the source and destination locks
			 */
			errFlag = !(cm->fib=ALLOCVEC(sizeof(struct FileInfoBlock),MEMF_PUBLIC|MEMF_CLEAR));
			if (!errFlag) errFlag = !(cm->slock=DUPLOCK(fromlock));
			if (!errFlag) errFlag = !(cm->dlock=DUPLOCK(tolock));
			if (!errFlag) errFlag = !(cm->sname=scopy(fromname));
			if (!errFlag) errFlag = !(cm->dname=scopy(toname));

			/*
			 * Check if we have an error
			 */
			if (!errFlag)
			{
				/*
				 * C...
				 */
				if (type==WBDISK)
				{
					FREEVEC(cm->sname);
					cm->sname=NULL;
				}

				/*
				 * Send off this message to the copy reader...
				 */
				PUTMSG(wb->wb_CopyReaderPort,cm);
				cm=NULL;
			}
			else cm->msg.wbc_errCode=IOERR();	/* Error! */

			/*
			 * If the copy message is still here, we will post
			 * it to the reply port to be cleaned up as needed...
			 */
			if (cm) REPLYMSG((struct Message *)cm);
		}
		else
		{
			UnInitDosCopy();	/* Close down the copy */
			NoMem();	/* No memory for the TypedCopyMsg... */
		}
	}
	else NoMem();	/* No copy process was created! */
	return(0);
}

/*
 * When the ioloop gets this message we get called...
 */
int DosCopyExit(struct CopyMsg *cm)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	TypedCopyMsg	*tcm=(struct TypedCopyMsg *)(((WORD *)cm)-1);


	CURRENTDIR(cm->dlock);

	/*
	 * Check for the exit conditions
	 */
	if (cm->msg.wbc_errCode)	/* There was an error */
	{
		DP(("Copy returned error #%ld  (Started: %ld)\n",cm->msg.wbc_errCode,(LONG)(cm->Started)));

		if ((cm->msg.wbc_errCode!=WB_SKIP_ERROR) && ((cm->Started) || (cm->msg.wbc_errCode!=ERROR_DISK_FULL)))
		{
		long	gad;
		long	third;

			/*
			 * Tell user there error...
			 */
			gad=Q_CANCEL_TEXT;
			third=NULL;
			if (cm->Started)
			{
				gad=Q_REMOVE_CANCEL_TEXT;
				third=Q_COPY_DELETE_TEXT;
			}
			if (!DoCopyRequest(gad,cm->dname,third,cm->msg.wbc_errCode))
			{
				cm->Started=FALSE;
			}

			if (cm->Started)
			{
			BPTR	lock;

				/*
				 * If it was an error other than the
				 * object exists error, we will delete the
				 * destination...  (If NULL lock, don't delete)
				 */
				if (lock=LOCK(cm->dname,SHARED_LOCK))
				{
					/*
					 * Remove complete directory...
					 */
					KillDirectory(lock);

					/*
					 * Now try to remove the rest
					 */
					UNLOCK(lock);

					KillFile(cm->dname);
				}
			}
		}
	}
	else			/* No error, so finish with windows */
	{
		/*
		 * Check if the icon would have been written...
		 */
		if (!(cm->sobj))
		{	/* If not, deal with possible fake icons... */
			if (cm->toobj->wo_DrawerData->dd_Flags & DDFLAGS_SHOWALL)
			{
			struct	WBObject	*newobj;

				if (newobj=MasterSearch(FindDupObj,cm->dlock,cm->dname,~ISDISKLIKE))
				{
					FullRemoveObject(newobj);
				}

				if (newobj=CreateIconFromName(cm->dname,cm->toobj,cm->dlock))
				{
					PostObjMove(cm->toobj,newobj,cm->curx,cm->cury,0);
					if (cm->toobj->wo_DrawerData->dd_ViewModes > DDVM_BYICON)
					{
						RefreshDrawer(cm->toobj);
					}
				}
			}
		}
	}

	/* Now, the cleanup... */
	UNLOCK(cm->slock);	/* UNLOCK() checks for NULL */
	UNLOCK(cm->dlock);

	FREEVEC(cm->sname);	/* FREEVEC() checks for NULL */
	FREEVEC(cm->dname);

	/*
	 * If we got the disk object...
	 */
	if (cm->sobj) FreeDiskObject(cm->sobj);

	/*
	 * If we have the FileInfoBlock, free it...
	 */
	FREEVEC(cm->fib);	/* FREEVEC() checks for NULL */
	FREEVEC(tcm);

	if (!(--wb->wb_AsyncCopyCnt))
	{
		/* MKS:  For now, check if we can blow away the copy process */
		UnInitDosCopy();

		EndDiskIO();
	}

	return(0);
}

/*
 * This is old code that may, at some point be moved elsewhere...
 */
Duplicate(struct WBObject *obj)
{
struct WorkbenchBase *wb = getWbBase();
LONG lock, newlock, oldlock;
int result = 0; /* assume success */

    MP(("Duplicate: enter, obj=$%lx (%s), calling IsDisk\n",
	obj, obj->wo_Name));

    if (obj->wo_Type == WBAPPICON)
    {
	ErrorTitle(Quote(Q_CANT_COPY),obj->wo_Name);
    }
    else if (CheckForType(obj,ISDISKLIKE))
    {
	MP(("D: obj is a disk, calling DiskCopy\n"));
	DiskCopy(obj, NULL);
    }
    else
    {
	MP(("D: calling GetParentsLock\n"));
	lock = GetParentsLock(obj);
	MP(("D: cd'ing to $%lx ($%lx)\n", lock, lock << 2));
	oldlock = CURRENTDIR(lock);
	MP(("D:ok, calling DUPLOCK\n"));
	/* dummy call for first 'UNLOCK' in loop below */
	newlock = DUPLOCK(lock);
	strcpy(wb->wb_Buf1, obj->wo_Name); /* init Buf1 to orig. name */
	while (newlock)  /* make sure directory/file doesn't already exist */
	{
	    strcpy(wb->wb_Buf0, wb->wb_Buf1); /* last bad name in Buf0 */
	    MP(("D:unlocking $%lx ($%lx)\n", newlock, newlock << 2));
	    UNLOCK(newlock); /* free last lock */
	    BumpRevision(wb->wb_Buf1, wb->wb_Buf0); /* make revision name in Buf1 */
	    MP(("D:trying to lock %s\n", wb->wb_Buf1));
	    /* try to get a lock on new name */
	    newlock = LOCK(wb->wb_Buf1, ACCESS_READ);
	}

	MP(("D: calling DosCopyLaunch\n"));
	result = DosCopyLaunch(obj->wo_Name, wb->wb_Buf1, obj->wo_Parent, obj->wo_Parent,NO_ICON_POSITION, NO_ICON_POSITION, lock, lock, obj->wo_Type);

        MP(("D:restoring old currentdir of $%lx ($%lx)\n",oldlock, oldlock << 2));
        CURRENTDIR(oldlock);
    }
    MP(("Duplicate: exit, return %ld\n", result));
    return(result);
}
