/*
** $Id: lists.c,v 1.1 92/10/13 11:23:11 kcd Exp $
**
** Module that contains routines to help generate lists of users, groups or
** members of a group.
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

ULONG ASM NextUser(REG(a0) struct UserInfo *user)
{
    struct Transaction *trans;
    UserData *userdata;
    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(UserData),
				    TAG_DONE))
    {
        userdata = (UserData *)trans->trans_RequestData;
        stccpy(userdata->UserName,user->ui_UserName,32);
	userdata->UserID = user->ui_UserID;

        trans->trans_ReqDataActual = sizeof(UserData);
        trans->trans_Command = ACMD_NextUser;

        if(DoCommand(trans))
	{
	    if(!trans->trans_Error)
	    {
		stccpy(user->ui_UserName,userdata->UserName,32);
		user->ui_UserID	= userdata->UserID;
		user->ui_PrimaryGroupID	= userdata->PrimaryGroupID;
		user->ui_Flags = userdata->Flags;
	    }
	    error = trans->trans_Error;
	}
        FreeTransaction(trans);

    }
    return(error);
}

ULONG ASM NextGroup(REG(a0) struct GroupInfo *group)
{
    struct Transaction *trans;
    GroupData *groupdata;
    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(GroupData),
				    TAG_DONE))
    {
        groupdata = (GroupData *)trans->trans_RequestData;
        stccpy(groupdata->GroupName,group->gi_GroupName,32);
	groupdata->GroupID = group->gi_GroupID;

        trans->trans_ReqDataActual = sizeof(GroupData);
        trans->trans_Command = ACMD_NextGroup;

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

ULONG ASM NextMember(REG(a0) struct GroupInfo *group,
		     REG(a1) struct UserInfo *user)
{
    struct Transaction *trans;
    NextMemberReq *memberdata;

    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(NextMemberReq),
    				TAG_DONE))
    {
    	memberdata =  (NextMemberReq *)trans->trans_RequestData;
    	stccpy(memberdata->User.UserName,user->ui_UserName,32);
    	stccpy(memberdata->Group.GroupName,group->gi_GroupName,32);
    	memberdata->User.UserID = user->ui_UserID;
    	memberdata->Group.GroupID = group->gi_GroupID;

    	trans->trans_ReqDataActual = sizeof(NextMemberReq);
    	trans->trans_Command = ACMD_NextMember;

    	if(DoCommand(trans))
    	{
    	    if(!trans->trans_Error)
    	    {
    	    	stccpy(user->ui_UserName,memberdata->User.UserName, 32);
    	    	user->ui_UserID = memberdata->User.UserID;
    	    	user->ui_PrimaryGroupID = memberdata->User.PrimaryGroupID;
    	    	user->ui_Flags = memberdata->User.Flags;
    	    }
    	    error = trans->trans_Error;
        }
        FreeTransaction(trans);
    }
    return(error);
}
