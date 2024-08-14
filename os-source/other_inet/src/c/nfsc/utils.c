/* -----------------------------------------------------------------------
 * utils.c  nfsc  SAS
 *
 * $Locker:  $
 *
 * $Id: utils.c,v 1.4 92/10/07 14:10:11 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: AS225:src/c/nfsc/RCS/utils.c,v 1.4 92/10/07 14:10:11 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * various utilities
 */

#include "fs.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <rpc/types.h>
#include <string.h>

void
bst_to_comm(bst, str)
	char	*bst;
	char	*str;
{
	u_char i, outlen;

	bst = btod(bst, char *);
	outlen = (u_char) *bst;
	bst++;

	for (i=0; i < outlen; i++ ) {
		str[i] = *bst;
		bst++;
	}
	str[outlen] = '\0';
}

struct MsgPort	*CreateMyPort (void)
{
	struct	MsgPort	*port;
	extern BYTE nfs_signal;

	port = (struct MsgPort *)AllocMem(sizeof(struct MsgPort), MEMF_PUBLIC | MEMF_CLEAR);
	if (port == 0) {
		return 0;
	}

	port->mp_Node.ln_Name = 0;
	port->mp_Node.ln_Pri = 1;
	port->mp_Node.ln_Type = NT_MSGPORT;
	port->mp_Flags = PA_SIGNAL;
	port->mp_SigBit = nfs_signal;
	port->mp_SigTask = (struct Task *)FindTask(0L);

	NewList(&(port->mp_MsgList));

	return (port);
}

void DeleteMyPort (struct MsgPort *port)
{
	port->mp_SigTask = (struct Task *) -1;
	port->mp_MsgList.lh_Head = (struct Node *) -1;
	FreeMem(port,(ULONG)sizeof(struct MsgPort));
}



/*
 * Catch fprintf from RPC library
 */
void fprintf(f, fmt, a, b, c, d, e)
{
	(void)requester(0L, " OK ", fmt, a, b, c, d, e);
}

/*
 * Catch abort from RPC library
 */
/*
abort()
{
	requester(0L, " OK ", "NFSc: fatal error: giving up");
	cleanup(1);
}
*/
