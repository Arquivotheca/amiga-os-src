/* notify.c */

#include "ram_includes.h"
#include "ram.h"

#define AllocNew(x)	((struct x *) AllocMem(sizeof(struct x),MEMF_CLEAR))

/* Currently requires nr_Private to be COMPLETE path back to root! */
/* We ignore nr_Name.  Dos sets up nr_Private to be the full path for us */
/* FIX!? */

/* #define NRF_COPY_DATA 4 */

LONG
addnotify (req)
	struct NotifyRequest *req;
{
	struct notify *notify;
	struct node *node;

	/* make sure we get memory */
	notify = AllocNew(notify);
	if (!notify)
		return FALSE;

	notify->Req	  = req;	/* returned when notify removed */
	req->nr_MsgCount  = 0;

	/* notify->node is NULL */

	/* set up name pointer to last part of name - for use in orphans */
	notify->Name = tail(req->nr_FullName);

	/* see if there's a node to attach it to */
	if (node = exists(req->nr_FullName))
	{
		/* YES! add it to the list of waiting notifies */
		AddHead((struct List *)
			&(current_node->notify_list),(struct Node *) notify);

		notify->node = current_node;

		/* deal with NOTIFY_INITIAL - notify if it exists */
		if (req->nr_Flags & NRF_NOTIFY_INITIAL)
			do_notify(notify);

		return DOS_TRUE;
	}

	/* failure - add to orphaned list */
	/* For a real fs, we might want to attach it as far down the */
	/* directory tree as possible, to reduce the number of times we */
	/* have to check it.  This would imply having an orphan list with */
	/* each directory in memory (or associated with the disk key of */
	/* the directory entry more likely). */

	AddHead((struct List *) &orphaned,(struct Node *) notify);

	return DOS_TRUE;
}

/* remove a notify request */

LONG
remnotify (req)
	struct NotifyRequest *req;
{
	struct notify *notify;
	struct node *node;

	if (node = exists(req->nr_FullName))
	{
		/* YES! it MUST be in the list */
		notify = (struct notify *) node->notify_list.mlh_Head;
	} else {
		notify = (struct notify *) orphaned.mlh_Head;
	}


	for ( ; notify->Succ; notify = notify->Succ)
	{
		if (notify->Req == req)
		{
			/* GOT IT */
			req = notify->Req;

			/* deal with WAIT_REPLY, so we're safe */
			/* careful! */
#ifdef USE_COPY_DATA
			if ((req->nr_Flags & (NRF_WAIT_REPLY|NRF_COPY_DATA)) &&
			    req->nr_MsgCount)
#else
			if ((req->nr_Flags & NRF_WAIT_REPLY) &&
			    req->nr_MsgCount)
#endif
				notify->msg->nm_DoNotTouch &= ~NRF_WAIT_REPLY;

			Remove((struct Node *) notify);
			FreeMem((char *)notify,sizeof(*notify));

			return DOS_TRUE;
		}
	}

	return NULL;
}

/* support routine for above */
struct node *
exists (file)
	char *file;
{
	return findnode(root,file,TRUE);
}

/* notify all interested parties about change to locked object */

void
notify (lock)
	struct lock *lock;
{
	lock->flags &= ~LOCK_MODIFY;

	/* assume lock is valid */
	notifynode((struct node *) lock->lock.fl_Key);
}

void
notifynode (node)
	struct node *node;
{
	struct notify *notify;

	for (notify = (struct notify *) node->notify_list.mlh_Head;
	     notify->Succ;
	     notify = notify->Succ)
	{
		do_notify(notify);
	}
}

/* notify someone */

void
do_notify (notify)
	struct notify *notify;
{
	struct NotifyMessage *msg;
	struct NotifyRequest *req;
#ifdef USE_COPY_DATA
	struct lock *lock;
#endif

	req = notify->Req;

	/* check for outstanding message */
#ifdef USE_COPY_DATA
	if ((req->nr_Flags & (NRF_WAIT_REPLY | NRF_COPY_DATA)) &&
	    req->nr_MsgCount > 0)
#else
	if ((req->nr_Flags & NRF_WAIT_REPLY) &&
	    req->nr_MsgCount > 0)
#endif
	{
		req->nr_Flags |= NRF_MAGIC; /* so we can do delayed notify */
		return;
	}

#ifdef USE_COPY_DATA
	/* COPY_DATA implies WAIT_REPLY! */

	if ((req->nr_Flags & NRF_COPY_DATA) &&
	    (notify->node->type == ST_FILE))
	{
		/* get lock */
		lock = getlock(notify->node,SHARED_LOCK);
		if (!lock)
			return;	/* must be under exclusive write lock - FIX? */

		/* equivalent to openmakefile */
		lock->block  = (struct data *) (notify->node->list);
		/* rest of the lock fields are set up already */

		/* transfer the data */
		req->nr_Actual = read(lock,req->nr_Data+4,req->nr_Length);
		req->nr_FileSize = notify->node->size;

		freelock(lock);
		/* often used with messages or signals */
	}
#endif

	if (req->nr_Flags & NRF_SEND_SIGNAL)
	{
		Signal(req->nr_stuff.nr_Signal.nr_Task,
		       (1L << req->nr_stuff.nr_Signal.nr_SignalNum));
	}

	if ((req->nr_Flags & NRF_SEND_MESSAGE) && 
	    ((msg = FindMsg()) != NULL))
	{
/* use jimm's method! FIX */
		msg->nm_ExecMessage.mn_ReplyPort = MyReplyPort;
		msg->nm_Class       = NOTIFY_CLASS;	/* FIX */
		msg->nm_Code        = NOTIFY_CODE;		/* FIX */
		msg->nm_NReq	    = req;
#ifdef USE_COPY_DATA
		msg->nm_DoNotTouch  = (req->nr_Flags &
					    (NRF_WAIT_REPLY | NRF_COPY_DATA)) ?
				      NRF_WAIT_REPLY : 0;
#else
		msg->nm_DoNotTouch  = req->nr_Flags & NRF_WAIT_REPLY;
#endif
		msg->nm_DoNotTouch2 = (LONG) notify;

		PutMsg(req->nr_stuff.nr_Msg.nr_Port,&(msg->nm_ExecMessage));

		req->nr_MsgCount++;
		notify->msg      =  msg;
	}
}

/* take a node, and attach any orphaned notifications to it */

void
find_notifies (node,flag)
	struct node *node;
	long flag;
{
	char filename[MAX_FILENAME+1];

	struct node *dptr;
	struct notify *notify,*next;

	for (notify = (struct notify *) orphaned.mlh_Head;
	     notify->Succ;
	     notify = next)
	{
		/* we may change what list notify is in */
		next = notify->Succ;

		/* do filenames match? */
		if (stricmp(notify->Name,node->name) == SAME)
		{
			/* now we really have to check directories */
			dptr = finddir(root,(char *) notify->Req->nr_FullName,
				       filename);
			if (dptr == node->back)
			{
				/* got it! */
				Remove((struct Node *) notify);
				AddHead((struct List *) &(node->notify_list),
					(struct Node *) notify);

				notify->node = node;

				/* tell person about it */
				if (flag)
					do_notify(notify);
			}
		}
	}
}

/* find or create a NotifyMessage */

struct NotifyMessage *
FindMsg ()
{
	struct NotifyMessage *msg;

	if (!(((struct Node *) FreeMessages.mlh_Head)->ln_Succ))
	{
		/* no free messages, create one */
		return AllocNew(NotifyMessage);
	}

	msg = (struct NotifyMessage *) FreeMessages.mlh_Head;
	Remove(&(msg->nm_ExecMessage.mn_Node));	/* remove from Message list */

	return msg;
}

void
rem_notifies (node,flag)
	struct node *node;
	long flag;
{
	struct notify *notify,*next;

	/* move notifynodes from node to the orphaned list and notify */
	/* note we MUST remove it first, in case of data in notify    */

	for (notify = (struct notify *) node->notify_list.mlh_Head;
	     notify->Succ;
	     notify = next)
	{
		/* careful, must get next link before removing. */
		next = notify->Succ;

		Remove((struct Node *) notify);
		AddHead((struct List *) &orphaned,(struct Node *) notify);
		notify->node = NULL;

		if (flag)
			do_notify(notify);
	}
}

void
AddFreeMsg (msg)
	struct NotifyMessage *msg;
{
	struct NotifyRequest *req;

	/* WAIT_REPLY is cleared by remnotify */
	if (msg->nm_DoNotTouch & NRF_WAIT_REPLY)
	{
		req = msg->nm_NReq;
		req->nr_MsgCount--;

		/* if it was modified, notify again */
		if (req->nr_Flags & NRF_MAGIC)
		{
			do_notify((struct notify *) msg->nm_DoNotTouch2);
			req->nr_Flags &= ~NRF_MAGIC;
		}
	}
	AddHead((struct List *) &FreeMessages,(struct Node *) msg);
}

/* return ptr to last file component */

char *
tail (str)
	char *str;
{
	char *end;

	end = str + strlen(str);
	while (end > str)
	{
		end--;
		if (*end == '/' || *end == ':')
		{
			end++; /* past separator */
			break;
		}
	}
	return end;
}
