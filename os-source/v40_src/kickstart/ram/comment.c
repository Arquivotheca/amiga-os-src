/* comment.c */

#include "ram_includes.h"
#include "ram.h"

/********************************************************/
/* success := comment(n)                                */
/*                                                      */
/* Handle setting of protection (possibly comments too) */
/********************************************************/

LONG
comment (pkt,objname)
	struct DosPacket *pkt;
	char *objname;
{
/*
   // format of packet is:
   //   arg1 - **unused**
   //   arg2 - lock on directory
   //   arg3 - name of object
   //   arg4 - protection bits OR comment string (BSTR) OR datestamp (CPTR)
*/
	struct node *node;
	char *str;
	LONG type,comm;
	ULONG len;

	str  = (char *) (pkt->dp_Arg4);
	type = pkt->dp_Type;
	comm = (type == ACTION_SET_COMMENT);
	if (type == ACTION_SET_COMMENT)
	{
		str  = (char *) BADDR(str);		/* still in BSTR form */
		len  = *str;
	}

	if (comm && (len > MAX_COMMENT))
	{
		fileerr = ERROR_COMMENT_TOO_BIG;
		return FALSE;
	}

	node = locatenode((struct lock *) (pkt->dp_Arg2 << 2),objname,TRUE);
						/* FIX??? maybe not follow? */
	if (!node)
		return FALSE;

	/* fail attempts to comment on root */
	if (node == root)
	{
		fileerr = ERROR_OBJECT_WRONG_TYPE;
		return FALSE;
	}

	if (comm) {
		freevec(node->comment);		/* freevec(NULL) is ok */
		/* don't allocate for a comment of ""! */
		if (comment)
		{
			node->comment = getvec(len+1);
			if (node->comment)
			{
				memcpy(node->comment,str+1,len);
				node->comment[len] = '\0';
			} else {
				return FALSE;
			}
		} else
			node->comment = NULL;

	} else if (type == ACTION_SET_PROTECT) {

		node->prot = (LONG) str;

	} else /* if (type == ACTION_SET_DATE)*/ {
		/* this MUST be cast! */
		/* arg4 is a CPTR to a datestamp! */
		copydate(node->date,((LONG *) str));

		notifynode(node);
	}

	/* I suppose any of these could be considered a change */
	/* copy makes this a PAIN - I'll try just setdate */
	/*notifynode(node);*/

	/* DateStamp(root->date); */	/* this is wierd - it isn't needed */

	return DOS_TRUE;
}
