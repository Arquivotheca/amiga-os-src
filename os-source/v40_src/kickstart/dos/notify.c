/* notify.c */

#include <exec/types.h>
#include <exec/memory.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include <string.h>

#include "libhdr.h"
#include "dos/notify.h"

#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "notify_protos.h"
#include "path_protos.h"
#include "klib_protos.h"
#include "blib_protos.h"

#define AllocNew(x) ((struct x *) AllocMem(sizeof(struct x),\
					   MEMF_PUBLIC|MEMF_CLEAR));


/* FIX! how do we handle assigns?????????!!!!!!!! */
/* FIX!!!!! */
/*
 * possible solution: give it a pathname, have it expand them as needed and
 * split the name.  It should be expanded via NameFromLock on any lock returned
 * by GetDevProc.
 * We should also move to a Tags interface, and have StartNotify allocate the
 * notifyrequest and return it to you.  The application should never modify or
 * allocate a notifyrequest structure.
 * This should make simple use of notify easier, while making it easier to
 * expand in the future.
 * As for multi-directory assigns, either only use the first, or look for a
 * file in any and use it if it exists, otherwise set it on the first.
 *
 * Currently expands, but not in a way I like.  Discuss with Steve.  FIX!
 * Handlers must use nr_Private instead of nr_DirName.
 */

LONG ASM
StartNotify (REG(d1) struct NotifyRequest *nreq)
{
	struct DevProc *dp = NULL;
	LONG rc;
	char temp[256], *name, *ptr;	/* FIX?  DANGER - path length */

	while (1) {
		dp = getdevproc(nreq->nr_Name,dp);
		if (dp == NULL)
			return 0;

		nreq->nr_Handler  = dp->dvp_Port;
		nreq->nr_FullName = nreq->nr_Name;
		name = NULL;

		/* FIX! expand path for others as well! */
		/* handle assigns - expand */
		if (dp->dvp_Lock)
		{
			/* assign - Use NameFromLock() to get real path */
			if (!NameFromLock(dp->dvp_Lock,temp,256))
				goto error;

			/* add rest of name to end */
			rc = strlen(temp);
			if (temp[rc-1] != ':')
				temp[rc++] = '/';

			/* pointer to rest of string - handle no ':'! */
			ptr = strchr(nreq->nr_Name,':');
			if (ptr)
				ptr++;
			else
				ptr = nreq->nr_Name;

			/* only allocate as much as we need */
			name = AllocVecPub(rc + 1 + strlen(ptr));
			if (!name)
				goto error;

			/* Use copymem - temp may not be terminated! */
			copyMem(temp,name,rc);
			strcpy(&name[rc],ptr);

			nreq->nr_FullName = name;
		}

		rc = sendpkt1(dp->dvp_Port,ACTION_ADD_NOTIFY,(LONG) nreq);

		if (rc || err_report(REPORT_LOCK,NULL,dp->dvp_Port))
		{
			if (!rc)
			{
error:				rc = 0;		/* other errors come here */
				if (name)
					freeVec(name);
				nreq->nr_FullName = NULL;
			}
			freedevproc(dp);
			return rc;
		}
		if (name)
			freeVec(name);
		freedevproc(dp);
		dp = NULL;
	}
}

LONG ASM
EndNotify (REG(d1) struct NotifyRequest *nreq)
{
	LONG rc;
	struct NotifyMessage *msg,*nextmsg;

	while (1) {
		rc = sendpkt1(nreq->nr_Handler,ACTION_REMOVE_NOTIFY,
			      (LONG) nreq);
		if (rc || err_report(REPORT_LOCK,NULL,nreq->nr_Handler))
		{
			if (nreq->nr_FullName &&
			    nreq->nr_FullName != nreq->nr_Name)
			{
				freeVec(nreq->nr_FullName);
				nreq->nr_FullName = NULL;
			}

			/* now, remove all messages from the port is needed */
			if (nreq->nr_Flags & NRF_SEND_MESSAGE)
			{
				if (nreq->nr_Flags &
				    (NRF_WAIT_REPLY /*| NRF_COPY_DATA*/))
				{
					if (nreq->nr_MsgCount == 0)
						goto done;
				}

				/* remove modified flag */
				nreq->nr_Flags &= ~NRF_MAGIC;

				/* must disable to scan msg list */
				/* downcode to assembler! FIX! */
				Disable();
				for (msg = (struct NotifyMessage *)
					   nreq->nr_stuff.nr_Msg.nr_Port->
							    mp_MsgList.lh_Head;
				     msg->nm_ExecMessage.mn_Node.ln_Succ &&
				      nreq->nr_MsgCount > 0;
				     msg = nextmsg)
				{
					nextmsg = (struct NotifyMessage *)
					    msg->nm_ExecMessage.mn_Node.ln_Succ;

					if (msg->nm_Class == NOTIFY_CLASS &&
					    msg->nm_Code == NOTIFY_CODE &&
					    msg->nm_NReq == nreq)
					{
					  Remove(&msg->nm_ExecMessage.mn_Node);
					  ReplyMsg(&msg->nm_ExecMessage);
					  nreq->nr_MsgCount--;
					}
				}
				Enable();
			}
done:
			return rc;
		}
	}
	/*NOTREACHED*/
}
