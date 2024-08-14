/*
**  $ID:$
**
**  Private authentication.library functions for generalizable server calls.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "authlib_internal.h"

/****i* authentication.library/doList ******************************************
*
*   NAME
*	doList -- Process a generalizable server call.
*
*   SYNOPSIS
*	doCmd(list, cmd)
*
*   FUNCTION
*	Sends a command to the server and returns a result (either from the
*	server or an error code from NIPC).
*
*	If there is a user, all info will be copied to the user structure and
*	sent over the wire.  If there is no user, there must be a group.
*
*	If there is no user, but there is an authority, all info will be
*	copied to the authority structure and sent over the wire.
*
*	The last option is that there is only a group to be sent over the wire.
*
*   INPUTS
*	user		- Pointer to UserProfile structure to operate on, may
*			  be NULL.
*	authority	- Pointer to UserProfile structure on who's authority
*			  cmd is to be executed, may be NULL.
*	cmd		- Command to be executed.
*	group		- Pointer to Group structure to operate on, NULL
*			  if not applicable to cmd.
*	base		- Pointer to AuthenticationBase.
*
*   RESULT
*	error		- Error code (see <Envoy/errors.h>).
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*	This function does no checking for NULL pointers.  Perhaps the library
*	functions should...
*
*   SEE ALSO
*
*****************************************************************************
*
*/
ULONG __asm doList(register __a0 struct MinList *list,
		register __d0 ULONG cmd,
		register __a1 struct Group *group,
		register __a6 struct AuthenticationBase *AuthenticationBase)
{
struct Transaction *trans;
APTR srcEntity;
ULONG scratch;

BUG(("doListCommand().\n"));
	if(OnOff)
	{
	BUG(("Security is off, server not called.\n"));
		return ENVOYERR_NOERROR;
	}
	if(!ServerEntity)
	{
	BUG(("No ServerEntity!  Bailing.\n"));
		return ENVOYERR_NORESOURCES;
	}
	srcEntity = MyCreateEntity(AuthenticationBase,
				   ENT_AllocSignal, &scratch,
				   TAG_DONE);
	if(!srcEntity)
	{
	BUG(("Can't Create srcEntity! Bailing.\n"));
		return ENVOYERR_NORESOURCES;
	}
BUG(("Got a srcEntity.\n"));
	trans = MyAllocTransaction(AuthenticationBase,
				TRN_AllocRespBuffer, 10000, /*  should be configurable or adaptable.  */
				TAG_DONE);
	if(!trans)
	{
	BUG(("Couldn't MyAllocTransaction().\n"));
		LoseEntity(srcEntity);
		return ENVOYERR_NORESOURCES;
	}
	if(group)
	{
		trans->trans_RequestData = group;
		trans->trans_ReqDataActual = sizeof(struct Group);
	}else
	{
		trans->trans_RequestData = NULL;
		trans->trans_ReqDataActual = 0;
	}
	trans->trans_Command = cmd;
	trans->trans_Timeout = 10;  /* 10 seconds should be plenty, but maybe should be configurable */
BUG(("About to DoTransaction().\n"));
	DoTransaction(ServerEntity, srcEntity, trans);
BUG(("Did transaction.\n"));
	scratch = trans->trans_Error;
	if(scratch)
	{
		goto ERROR1;
	}
	switch(cmd)
	{
	case LIST_USERS:
	case LIST_MEMBERS:
		{
		struct UserProfile *node;
		int n=0;

BUG(("Doing ListUsers/Members while loop.\n"));
			while(n<trans->trans_RespDataActual)
			{
				if( !(node=AllocMem(sizeof(struct UserProfile), MEMF_ANY|MEMF_CLEAR)) )
				{
					scratch = ENVOYERR_NORESOURCES;
					; /* free nodes here!!! */
					goto ERROR1;
				}
				CopyMem(((UBYTE *)trans->trans_ResponseData)+n, &(node->up_UserName), PROFILE_DSIZE);
				AddHead((struct List *)list, (struct Node *)node);
				n += PROFILE_DSIZE;
				BUG(("Added a node to the list.\n"));
			}
			break;
		}
	case LIST_GROUPS:
		{
		struct Group *node;
		int n=0;

BUG(("Doing ListGroups while loop.\n"));
			while(n<trans->trans_RespDataActual)
			{
				if( !(node=AllocMem(sizeof(struct Group), MEMF_ANY|MEMF_CLEAR)) )
				{
					scratch = ENVOYERR_NORESOURCES;
					; /* free nodes here!!! */
					goto ERROR1;
				}
				CopyMem(((UBYTE *)trans->trans_ResponseData)+n, &(node->g_GroupName), GROUP_DSIZE);
				AddHead((struct List *)list, (struct Node *)node);
				n += GROUP_DSIZE;
				BUG(("Added a node to the list.\n"));
			}
			break;
		}
	default:
		scratch = ENVOYERR_CMDUNKNOWN;
	}
ERROR1:
	DeleteEntity(srcEntity);
	FreeTransaction(trans);
BUG(("DoTransaction() returning: %lu.\n", scratch));
	return scratch;
}
