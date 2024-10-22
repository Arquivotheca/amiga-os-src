/*
**  $Id: docmd.c,v 1.3 92/04/09 11:57:57 dlarson Exp Locker: dlarson $
**
**  Private authentication.library functions for generalizable server calls.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "authlib_internal.h"

/****i* authentication.library/doCmd ******************************************
*
*   NAME
*	doCmd -- Process a generalizable server call.
*
*   SYNOPSIS
*	doCmd(user, authority, cmd, group, base)
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
ULONG __asm doCmd(register __a0 struct UserProfile *user,
		register __a1 struct UserProfile *authority,
		register __d0 ULONG cmd,
		register __a2 struct Group *group,
		register __a6 struct AuthenticationBase *AuthenticationBase)
{
struct Transaction *trans;
APTR srcEntity;
ULONG scratch;

BUG(("doUserCommand().\n"));
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
	trans = MyAllocTransaction(AuthenticationBase, TAG_DONE);
	if(!trans)
	{
	BUG(("Couldn't MyAllocTransaction().\n"));
		LoseEntity(srcEntity);
		return ENVOYERR_NORESOURCES;
	}
	trans->trans_ReqDataLength  = sizeof(struct UserProfile);
	if(!user)
	{
		if(!authority)
		{
			trans->trans_ReqDataLength  = sizeof(struct Group);
			trans->trans_RequestData = group;
		}else
		{
			trans->trans_RequestData = authority;
			CopyMem(group, authority->Group, sizeof(struct Group));
		}
	}else
	{
		trans->trans_RequestData = user;
		if(authority)
		{
		BUG(("Copying Authority"));
			CopyMem(authority, user->Authority, sizeof(struct MinUserProfile));
		}
		if(group)
		{
		BUG(("Copying Authority"));
			CopyMem(group, user->Group, sizeof(struct Group));
		}
	}
	trans->trans_ResponseData = trans->trans_RequestData;
	trans->trans_ReqDataActual  = trans->trans_ReqDataLength;
	trans->trans_RespDataLength = trans->trans_ReqDataLength;
	trans->trans_Command = cmd;
	trans->trans_Timeout = 10;  /* 10 seconds should be plenty!  */
BUG(("About to DoTransaction().\n"));
	DoTransaction(ServerEntity, srcEntity, trans);
BUG(("Did transaction.\n"));
	scratch = trans->trans_Error;
	DeleteEntity(srcEntity);
	FreeTransaction(trans);
BUG(("DoTransaction() returning: %lu.\n", scratch));
	return scratch;
}
