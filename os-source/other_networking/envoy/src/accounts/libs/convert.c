/*
** $Id: convert.c,v 1.1 92/10/13 11:25:04 kcd Exp $
**
** Module that contains routines to convert between usernames and userid's and
** to convert between groupnames and groupid's.
**
*/

/*--------------------------------------------------------------------------*/

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif

#ifndef ENVOY_ERRORS_H
#include <envoy/errors.h>
#endif

#include "accountsbase.h"
#include "/accounts.h"
#include "/transactions.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>

/*--------------------------------------------------------------------------*/

ULONG ASM NameToUser(REG(a0) STRPTR userName,
		     REG(a1) struct UserInfo *user)
{
    struct Transaction *trans;
    UserData *userdata;
    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(UserData),
    				TAG_DONE))
    {
    	userdata = (UserData *)trans->trans_RequestData;
    	stccpy(userdata->UserName,userName,32);

    	trans->trans_ReqDataActual = sizeof(UserData);
    	trans->trans_Command = ACMD_NameToUser;

    	if(DoCommand(trans))
    	{
    	    if(!trans->trans_Error)
    	    {
    	    	stccpy(user->ui_UserName,userdata->UserName,32);
    	    	user->ui_UserID = userdata->UserID;
    	    	user->ui_PrimaryGroupID = userdata->PrimaryGroupID;
    	    	user->ui_Flags = userdata->Flags;
    	    }
    	    error = trans->trans_Error;
    	}
    	FreeTransaction(trans);
    }
    return(error);
}

ULONG ASM NameToGroup(REG(a0) STRPTR groupName,
		     REG(a1) struct GroupInfo *group)
{
    struct Transaction *trans;
    GroupData *groupdata;
    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(GroupData),
    				TAG_DONE))
    {
    	groupdata = (GroupData *)trans->trans_RequestData;
    	stccpy(groupdata->GroupName,groupName,32);

    	trans->trans_ReqDataActual = sizeof(GroupData);
    	trans->trans_Command = ACMD_NameToGroup;

    	if(DoCommand(trans))
    	{
    	    if(!trans->trans_Error)
    	    {
    	    	stccpy(group->gi_GroupName,groupdata->GroupName,32);
    	    	group->gi_GroupID = groupdata->GroupID;
    	    }
    	    error = trans->trans_Error;
    	}
    	FreeTransaction(trans);
    }
    return(error);
}


ULONG ASM IDToUser(REG(d0) UWORD userid,
		   REG(a0) struct UserInfo *user)
{
    struct Transaction *trans;
    UserData *userdata;
    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(UserData),
    				TAG_DONE))
    {
    	userdata = (UserData *)trans->trans_RequestData;
    	userdata->UserID = userid;

    	trans->trans_ReqDataActual = sizeof(UserData);
    	trans->trans_Command = ACMD_IDToUser;

    	if(DoCommand(trans))
    	{
    	    if(!trans->trans_Error)
    	    {
    	    	stccpy(user->ui_UserName,userdata->UserName,32);
    	    	user->ui_UserID = userdata->UserID;
    	    	user->ui_PrimaryGroupID = userdata->PrimaryGroupID;
    	    	user->ui_Flags = userdata->Flags;
    	    }
    	    error = trans->trans_Error;
    	}
    	FreeTransaction(trans);
    }
    return(error);
}

ULONG ASM IDToGroup(REG(d0) UWORD groupid,
		     REG(a0) struct GroupInfo *group)
{
    struct Transaction *trans;
    GroupData *groupdata;
    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(GroupData),
    				TAG_DONE))
    {
    	groupdata = (GroupData *)trans->trans_RequestData;
	groupdata->GroupID = groupid;

    	trans->trans_ReqDataActual = sizeof(GroupData);
    	trans->trans_Command = ACMD_IDToGroup;

    	if(DoCommand(trans))
    	{
    	    if(!trans->trans_Error)
    	    {
    	    	stccpy(group->gi_GroupName,groupdata->GroupName,32);
    	    	group->gi_GroupID = groupdata->GroupID;
    	    }
    	    error = trans->trans_Error;
    	}
    	FreeTransaction(trans);
    }
    return(error);
}


