/*
** $Id: verify.c,v 1.1 92/10/13 11:25:28 kcd Exp Locker: kcd $
**
** Module that contains routines to verify a user's username and
** password and to verify a user's group membership.
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
#include <clib/utility_protos.h>
#include <clib/nipc_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/nipc_pragmas.h>

/*--------------------------------------------------------------------------*/

ULONG VerUser(STRPTR userName, STRPTR password, struct UserInfo *user, UBYTE cmd)
{
    struct Transaction *trans;
    UserData *userdata;
    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(UserData),
				    TAG_DONE))
    {
        userdata = (UserData *)trans->trans_RequestData;
        stccpy(userdata->UserName,userName,32);
        stccpy(userdata->Password,password,32);

        trans->trans_ReqDataActual = sizeof(UserData);
        trans->trans_Command = cmd;

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

/*--------------------------------------------------------------------------*/

ULONG ASM MemberOf(REG(a0) struct GroupInfo *group,
		   REG(a1) struct UserInfo *user)
{
    struct Transaction *trans;
    NextMemberReq *memberdata;

    ULONG error = ENVOYERR_NORESOURCES;

    if(trans = AllocTransaction(TRN_AllocReqBuffer, sizeof(NextMemberReq),
    				TAG_DONE))
    {
    	memberdata = (NextMemberReq *)trans->trans_RequestData;
    	stccpy(memberdata->User.UserName,user->ui_UserName,32);
    	stccpy(memberdata->Group.GroupName,group->gi_GroupName,32);
    	memberdata->User.UserID = user->ui_UserID;
    	memberdata->Group.GroupID = group->gi_GroupID;

    	trans->trans_ReqDataActual = sizeof(NextMemberReq);
    	trans->trans_Command = ACMD_MemberOf;

    	if(DoCommand(trans))
    	    error = trans->trans_Error;

        FreeTransaction(trans);
    }
    return(error);
}

/*--------------------------------------------------------------------------*/

ULONG ASM VerifyUser(REG(a0) STRPTR userName,
		    REG(a1) STRPTR password,
		    REG(a2) struct UserInfo *user)
{
    return(VerUser(userName, password, user, ACMD_VerifyUser));
}

/*--------------------------------------------------------------------------*/

ULONG ASM VerifyUserCrypt(REG(a0) STRPTR userName,
		    REG(a1) STRPTR password,
		    REG(a2) struct UserInfo *user)
{
    return(VerUser(userName, password, user, ACMD_VerifyUserCrypt));
}

/*--------------------------------------------------------------------------*/

#define OSIZE 12

STRPTR ASM ECrypt(REG(a0) STRPTR buffer,
		  REG(a1) STRPTR password,
		  REG(a2) STRPTR user)
{
	int i ;
	int k ;
	long d1 ;
	unsigned int buf[OSIZE];
        UBYTE username[32],*uptr;
	uptr = username;

        for(i=0; i<32; i++)
        {
            username[i] = ToLower(user[i]);
        }

        for(i = 0; i < OSIZE; i++)
        {
                buf[i] = 'A' + (*password? *password++:i) + (*uptr? *uptr++:i);
        }

        for(i = 0; i < OSIZE; i++)
        {
                for(k = 0; k < OSIZE; k++)
                {
                        buf[i] += buf[OSIZE-k-1];
                        UDivMod32((long)buf[i], 53L) ;
                        d1 = getreg(1) ;
                        buf[i] = (unsigned int)d1 ;
                }
                buffer[i] = buf[i] + 'A' ;
        }
        buffer[OSIZE-1] = 0;
        return(buffer) ;
}

