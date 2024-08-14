/* record.c */

#include "ram_includes.h"
#include "ram.h"
#include "dos/record.h"

#include "stddef.h"

#define AllocNew(x)	AllocMem(sizeof(struct x),MEMF_CLEAR)

LONG
lockrecord (struct lock *lock,struct DosPacket *dp)
{
	/* dp == lock, offset, length, mode, timeout */

/*
kprintf("locking record (0x%lx, 0x%lx, 0x%lx, %ld, %ld)\n",
lock,dp->dp_Arg2,dp->dp_Arg3,dp->dp_Arg4,dp->dp_Arg5);
*/
	/* can we get the record lock immediately?   */
	if (find_rlock(lock,dp->dp_Arg2,dp->dp_Arg3,dp->dp_Arg4))
		return DOS_TRUE;

	if (dp->dp_Arg4 & 1)	/* Immediate! Careful of mode additions! */
	{
		fileerr = ERROR_LOCK_COLLISION;
		return FALSE;
	}

	return rlock_wait_add(lock,dp);  /* make sure it's not replied!!!! */
}

/* scan the rlock list of the file.  If area available, add to list.	*/
/* If locked shared, and I am type shared, and no one of type write	*/
/* is waiting, add to list, else fail.					*/
/* Ignore requests from the same lock.					*/

LONG
find_rlock (struct lock *lock, ULONG offset, ULONG length, ULONG mode)
{
	struct rlock *rlock;
	ULONG top = offset + length;	/* points past end of region */

/*
kprintf("top = 0x%lx\n",top);
*/
	for (rlock = (void *) (((struct node *) lock->lock.fl_Key)->
							record_list.mlh_Head);
	     rlock->next;
	     rlock = rlock->next)
	{
/*
kprintf("checking against 0x%lx, 0x%lx, 0x%lx, %ld\n",
rlock->lock,rlock->offset,rlock->length,rlock->mode);
*/
		if (rlock->lock != lock &&
		    rlock->offset < top &&
		    rlock->offset + rlock->length > offset)
		{
/*
kprintf("overlap...\n");
*/
			/* they overlap, not same fh */
			if (mode <= REC_EXCLUSIVE_IMMED)
				return FALSE;		/* no overlap allowed */
			else {
				/* mode must be read */
				if (rlock->mode <= REC_EXCLUSIVE_IMMED)
					return FALSE;	/* already exclusive */
			} /* keep scanning */
		}
	}

/*
kprintf("making new lock\n");
*/
	/* no exclusive locks overlap new one, or no read locks overlap */
	/* if it's an exclusive lock, or it's the same fh.		*/
	rlock = AllocNew(rlock);
	if (!rlock)
		return FALSE;		/* out of mem! */

	rlock->lock   = lock;
	rlock->offset = offset;
	rlock->length = length;
	rlock->mode   = mode;

	AddHead((struct List *) &(((struct node *) lock->lock.fl_Key)->
								record_list),
		(struct Node *) rlock);

	return DOS_TRUE;
}

LONG
rlock_wait_add (struct lock *lock, struct DosPacket *dp)
{
	struct rlock_wait *rwait;

/*
kprintf("Adding lock request to timeout list, time = %ld\n",dp->dp_Arg5);
*/
	rwait = AllocNew(rlock_wait);
	if (!rwait)
		return FALSE;
/*
kprintf("rwait at 0x%lx\n",rwait);
*/
	rwait->dp = dp;

	/* play EVIL games, copy the dos timer block! */
	rwait->iob = *(DOSBase->dl_TimeReq);
	rwait->iob.tr_node.io_Message.mn_ReplyPort = timerport;

	rwait->iob.tr_node.io_Command = TR_ADDREQUEST;
	rwait->iob.tr_time.tv_micro   = 
				mult32(rem32(dp->dp_Arg5,TICKS_PER_SECOND),
				       (1000000/TICKS_PER_SECOND));
	rwait->iob.tr_time.tv_secs    = div32(dp->dp_Arg5,TICKS_PER_SECOND);

	SendIO((struct IORequest *) &(rwait->iob));

	/* add it to the tail of requests - queued requests are serviced */
	/* in order from the front.					 */
	AddTail((struct List *) &(((struct node *) lock->lock.fl_Key)->
								rwait_list),
		(struct Node *) rwait);

	return 1;	/* causes start.c not to reply the pkt! */
}

LONG
unlockrecord (struct lock *lock, ULONG offset, ULONG length)
{
	/* what about a read and a write lock from the same lock and the */
	/* same area?  which gets unlocked?				 */
	struct rlock *rlock;

/*
kprintf("unlocking record (0x%lx, 0x%lx, 0x%lx)\n",
lock,offset,length);
*/
	for (rlock = (void *) (((struct node *) lock->lock.fl_Key)->
							record_list.mlh_Head);
	     rlock->next;
	     rlock = rlock->next)
	{
		if (rlock->lock   == lock &&
		    rlock->offset == offset &&
		    rlock->length == length)
		{
/*
kprintf("found record lock\n");
*/
			/* found it - release, then deal with checking */
			Remove((struct Node *) rlock);
			FreeMem((void *) rlock, sizeof(*rlock));

			wakeup((struct node *) lock->lock.fl_Key,offset,
				offset + length);

			return DOS_TRUE;
		}
	}

	/* can't find the lock!!!! */
	fileerr = ERROR_RECORD_NOT_LOCKED;
	return FALSE;
}

void
wakeup (struct node *node, ULONG offset, ULONG top)
{
	struct rlock_wait *rwait,*next;

/*
kprintf("wakeup: node = 0x%lx (%s), offset = 0x%lx, top = 0x%lx\n",
node,node->name,offset,top);
*/
	/* ok, now figure out if anyone needs to be given a lock */
	for (rwait = (void *) (node->rwait_list.mlh_Head);
	     rwait->next;
	     rwait = next)
	{
		next = rwait->next;	/* since we may free it */

/*
kprintf("rwait=0x%lx, rwait->next = 0x%lx, rwait->dp = 0x%lx\n",rwait,
rwait->next,rwait->dp);
*/
/*
kprintf("checking against offset 0x%lx, top 0x%lx\n",rwait->dp->dp_Arg2,
rwait->dp->dp_Arg2 + rwait->dp->dp_Arg3);
*/
		if (rwait->dp->dp_Arg2 < top &&
		    rwait->dp->dp_Arg2 + rwait->dp->dp_Arg3 > offset)
		{
/*
kprintf("they overlap\n");
*/
			/* they overlap */
			if (find_rlock((struct lock *)BADDR(rwait->dp->dp_Arg1),
					rwait->dp->dp_Arg2,
					rwait->dp->dp_Arg3,
					rwait->dp->dp_Arg4))
			{
/*
kprintf("got lock, now kill timer\n");
*/
				/* We got the lock! */
				AbortIO((struct IORequest *) &(rwait->iob));
				WaitIO((struct IORequest *) &(rwait->iob));
				Remove((struct Node *) rwait);

				/* tell him he got it */
				ReplyPkt(rwait->dp,DOS_TRUE,0);

				FreeMem((void *) rwait,sizeof(*rwait));
			} /* else this still needs to wait for something */
		}
	}
}

/* kill a timed out record lock request */

void
kill_rwait (struct timerequest *iob)
{
	struct rlock_wait *rwait;

	rwait = (void *) (((char *) iob) - offsetof(struct rlock_wait,iob));
	Remove((struct Node *) rwait);

/*
kprintf("lock timeout: 0x%lx\n",BADDR(rwait->dp->dp_Arg1));
*/
	wakeup((struct node *)
		(((struct lock *) BADDR(rwait->dp->dp_Arg1))->lock.fl_Key),
	       rwait->dp->dp_Arg2,rwait->dp->dp_Arg2 + rwait->dp->dp_Arg3);

	ReplyPkt(rwait->dp,FALSE,ERROR_LOCK_TIMEOUT);
	FreeMem((void *) rwait,sizeof(*rwait));
}
