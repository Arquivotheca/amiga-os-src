#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>
#include <dos/rdargs.h>
#include <libraries/iffparse.h>
#include <envoy/nipc.h>
#include <envoy/errors.h>
#include "/transactions.h"
#include "server_internal.h"
#include "string.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>

#ifndef PROTOS
#include "server_protos.h"
#endif

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>

#define SysBase (*(struct Library **)4L)

extern void kprintf(STRPTR fmt, ...);

void __saveds server(void)
{
    ServerDataPtr sd;
    struct RDArgs *rdargs;
    ULONG args[1];
    BOOL error = FALSE;

    if(sd = (ServerDataPtr) AllocMem(sizeof(struct ServerData),MEMF_CLEAR|MEMF_PUBLIC))
    {
    	sd->sd_SysBase = SysBase;

    	DOSBase = OpenLibrary("dos.library",37L);
    	NIPCBase = OpenLibrary("nipc.library",0L);
	UtilityBase = OpenLibrary("utility.library",37L);

    	if((DOSBase) && (NIPCBase) && (UtilityBase))
    	{
    	    args[0] = 0;
    	    if(rdargs = ReadArgs("InitDB/S",(LONG *)args,NULL))
    	    {
    	    	FreeArgs(rdargs);
    	    }
    	    NewList((struct List *)&sd->sd_UserList);
    	    NewList((struct List *)&sd->sd_GroupList);

    	    ReadUserData(sd);
    	    ReadGroupData(sd);

	    if(args[0])
	    {
	    	UserData ud;

	    	FlushUserData(sd, FALSE);
	    	strcpy(sd->sd_UserHeader.AdminPassword,"commodore");
	    	strcpy(ud.UserName,"Admin");
	    	strcpy(ud.Password,"Admin");
	    	ud.UserID = sd->sd_GroupHeader.NextGroupID = 1;
	    	sd->sd_UserHeader.NextUserID = 2;
	    	ud.PrimaryGroupID = 0;
	    	ud.Flags = UFLAGF_AdminSelf | UFLAGF_AdminGroups | UFLAGF_AdminAll;

	    	if(!InternalAddUser(sd, &ud))
	    	{
	    	    error = TRUE;
	    	    PutStr("Error! Couldn't initialize database!");
	    	}
	    	else
	    	{
	    	    WriteUserData(sd);
	    	    WriteGroupData(sd);
	    	}
	    }
            if(!error && !args[0])
            {
    	    	ProcessRequests(sd);
    	    	FlushUserData(sd, TRUE);
    	    }
    	}
    	if(DOSBase) CloseLibrary(DOSBase);
    	if(NIPCBase) CloseLibrary(NIPCBase);
    	if(UtilityBase) CloseLibrary(UtilityBase);
    }
}

#undef SysBase
#define SysBase	sd->sd_SysBase;

VOID ReadUserData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    BPTR iffstream;
    struct ContextNode *cn;
    LONG error;
    UserNode *newuser;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream=Open("ETC:UserData",MODE_OLDFILE))
	{
	    if(iff = AllocIFF())
	    {
		InitIFFasDOS(iff);

		if(!OpenIFF(iff,IFFF_READ))
		{
		    iff->iff_Stream = iffstream;

		    while(TRUE)
		    {
			error =	ParseIFF(iff, IFFPARSE_STEP);

			if(!error)
			{
			    if(cn = CurrentChunk(iff))
			    {
			    	if((cn->cn_Type == ID_UDAT) && (cn->cn_ID == ID_UHDR))
			    	{
			    	    ReadChunkBytes(iff, &sd->sd_UserHeader, sizeof(struct UserIFFHeader));
			    	}
				if((cn->cn_Type	== ID_UDAT) && (cn->cn_ID == ID_USER))
				{
				    if(newuser = (struct UserNode *) AllocMem(sizeof(struct UserNode),MEMF_PUBLIC|MEMF_CLEAR))
				    {
					if(ReadChunkBytes(iff, &newuser->Data, sizeof(struct UserData))	== sizeof(struct UserData))
					{
					    AddTail((struct List *)&sd->sd_UserList,(struct Node *)newuser);
					}
				    }
				}
			    }
			}
			else if(error == IFFERR_EOC)
			    continue;
			else
			    break;
		    }
		    CloseIFF(iff);
		}
		FreeIFF(iff);
	    }
	    Close(iffstream);
	}
	CloseLibrary(IFFParseBase);
    }
}

VOID WriteUserData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    struct UserNode *user;
    BPTR iffstream;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream = Open("ETC:UserData",MODE_NEWFILE))
    	{
    	    if(iff = AllocIFF())
    	    {
    	    	InitIFFasDOS(iff);

    	    	if(!OpenIFF(iff,IFFF_WRITE))
    	    	{
    	    	    iff->iff_Stream = iffstream;

    	    	    PushChunk(iff, ID_UDAT, ID_FORM, IFFSIZE_UNKNOWN);

		    PushChunk(iff, ID_UDAT, ID_UHDR, sizeof(struct UserIFFHeader));
		    WriteChunkBytes(iff, &sd->sd_UserHeader, sizeof(struct UserIFFHeader));
		    PopChunk(iff);

    	    	    user = (UserNode *) sd->sd_UserList.mlh_Head;

    	    	    while(user->Link.ln_Succ)
    	    	    {
    	    	    	PushChunk(iff, ID_UDAT, ID_USER, sizeof(struct UserIFFChunk));

    	    	    	WriteChunkBytes(iff, &user->Data, sizeof(struct UserIFFChunk));

    	    	    	PopChunk(iff);

    	    	    	user = (UserNode *)user->Link.ln_Succ;
    	    	    }

    	    	    PopChunk(iff);

    	    	    CloseIFF(iff);
    	    	}
    	    	FreeIFF(iff);
    	    }
    	    Close(iffstream);
    	}
    	CloseLibrary(IFFParseBase);
    }
}

VOID ReadGroupData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    BPTR iffstream;
    struct ContextNode *cn;
    LONG error;
    GroupNode *newgroup;
    MinUserNode *minusernode;
    UserNode *user;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream=Open("ETC:UserData",MODE_OLDFILE))
	{
	    if(iff = AllocIFF())
	    {
		InitIFFasDOS(iff);

		if(!OpenIFF(iff,IFFF_READ))
		{
		    iff->iff_Stream = iffstream;

		    while(TRUE)
		    {
			error =	ParseIFF(iff, IFFPARSE_STEP);

			if(!error)
			{
			    if(cn = CurrentChunk(iff))
			    {
			    	if((cn->cn_Type == ID_GDAT) && (cn->cn_ID == ID_GHDR))
			    	{
			    	    ReadChunkBytes(iff, &sd->sd_GroupHeader, sizeof(struct GroupIFFHeader));
			    	}
				else if((cn->cn_Type == ID_GDAT) && (cn->cn_ID == ID_GRUP))
				{
				    if(newgroup = (struct GroupNode *) AllocMem(sizeof(struct UserNode),MEMF_PUBLIC|MEMF_CLEAR))
				    {
				    	NewList((struct List *)&newgroup->UserList);

					if(ReadChunkBytes(iff, &newgroup->Data, sizeof(struct GroupData)) == sizeof(struct GroupData))
					{
					    while(cn->cn_Size - cn->cn_Scan)
					    {
					        UWORD uid;

				    	        ReadChunkBytes(iff, &uid, sizeof(UWORD));

				    	        if(user = FindUserByUID(sd, uid))
				    	        {
					    	    if(minusernode = (MinUserNode *)AllocMem(sizeof(MinUserNode),MEMF_CLEAR|MEMF_PUBLIC))
					    	    {
					    	    	minusernode->User = user;
					    	    	AddTail((struct List *)&newgroup->UserList, (struct Node *)minusernode);
					    	    }
						}
					    }
					    AddTail((struct List *)&sd->sd_GroupList,(struct Node *)newgroup);
					}
				    }
				}
			    }
			}
			else if(error == IFFERR_EOC)
			    continue;
			else
			    break;
		    }
		    CloseIFF(iff);
		}
		FreeIFF(iff);
	    }
	    Close(iffstream);
	}
	CloseLibrary(IFFParseBase);
    }
}

VOID WriteGroupData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    struct GroupNode *group;
    struct MinUserNode *minuser;
    BPTR iffstream;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream = Open("ETC:GroupData",MODE_NEWFILE))
    	{
    	    if(iff = AllocIFF())
    	    {
    	    	InitIFFasDOS(iff);

    	    	if(!OpenIFF(iff,IFFF_WRITE))
    	    	{
    	    	    iff->iff_Stream = iffstream;

    	    	    PushChunk(iff, ID_GDAT, ID_FORM, IFFSIZE_UNKNOWN);

		    PushChunk(iff, ID_GDAT, ID_GHDR, sizeof(struct GroupIFFHeader));
		    WriteChunkBytes(iff, &sd->sd_GroupHeader, sizeof(struct GroupIFFHeader));
		    PopChunk(iff);

    	    	    group = (GroupNode *) sd->sd_GroupList.mlh_Head;

    	    	    while(group->Link.ln_Succ)
    	    	    {
    	    	    	PushChunk(iff, ID_GDAT, ID_GRUP, IFFSIZE_UNKNOWN);

    	    	    	WriteChunkBytes(iff, &group->Data, sizeof(struct UserIFFChunk));

    	    	    	minuser = (MinUserNode *)group->UserList.mlh_Head;

    	    	    	while(minuser->Link.mln_Succ)
    	    	    	{
    	    	    	    WriteChunkBytes(iff, &minuser->User->Data.UserID, sizeof(UWORD));

    	    	    	    minuser = (MinUserNode *)minuser->Link.mln_Succ;
    	    	    	}

    	    	    	PopChunk(iff);

    	    	    	group = (GroupNode *)group->Link.ln_Succ;
    	    	    }

    	    	    PopChunk(iff);

    	    	    CloseIFF(iff);
    	    	}
    	    	FreeIFF(iff);
    	    }
    	    Close(iffstream);
    	}
    	CloseLibrary(IFFParseBase);
    }
}

VOID FlushUserData(ServerDataPtr sd, BOOL flush)
{
    UserNode *user;
    GroupNode *group;
    MinUserNode *minuser;

    if(flush)
    {
	WriteUserData(sd);
	WriteGroupData(sd);
    }

    while(user = (UserNode *) RemHead((struct List *)&sd->sd_UserList))
    	FreeMem(user,sizeof(UserNode));

    while(group = (GroupNode *) RemHead((struct List *)&sd->sd_GroupList))
    {
    	while(minuser = (MinUserNode *)RemHead((struct List *)&group->UserList))
    	    FreeMem(minuser,sizeof(MinUserNode));

    	FreeMem(group,sizeof(GroupNode));
    }
}

struct Entity *CreateEnt(ServerDataPtr sd, Tag firsttag, ...)
{
    return(CreateEntityA((struct TagItem *)&firsttag));
}

VOID ProcessRequests(ServerDataPtr sd)
{
    struct Entity *entity;
    ULONG mySigBit, waitmask, signals;
    struct Transaction *trans;

    if(entity = CreateEnt(sd, ENT_Name,	"Security Server",
    			     ENT_Public, TRUE,
    			     ENT_AllocSignal, &mySigBit,
    			     TAG_DONE))
    {
    	waitmask = (1 << mySigBit) | SIGBREAKF_CTRL_F;

    	while(TRUE)
    	{
    	    signals = Wait(waitmask);

    	    if(signals & SIGBREAKF_CTRL_F)
    	    	break;

    	    while(trans = GetTransaction(entity))
    	    {
    	    	switch(trans->trans_Command)
    	    	{
    	    	    case UD_AddUser:		AddUser(sd, trans);
	    	    	    			break;

    	    	    case UD_RemUser:		RemUser(sd, trans);
    	    	    				break;

    	    	    case UD_AddGroup:		AddGroup(sd, trans);
    	    	    				break;

    	    	    case UD_RemGroup:		RemGroup(sd, trans);
    	    	    				break;

		    case UD_AddMember:		AddMember(sd, trans);
		    				break;

    	    	    case UD_RemoveMember:       RemoveMember(sd, trans);
    	    	    				break;

    	    	    case UD_NextUser:		NextUser(sd, trans);
    	    	    				break;

    	    	    case UD_NextMember:		NextMember(sd, trans);
    	    	    				break;

    	    	    case UD_NextGroup:		NextGroup(sd, trans);
    	    	    				break;

    	    	    case UD_IDToUser:		IDToUser(sd, trans);
    	    	    				break;

    	    	    case UD_NameToUser:		NameToUser(sd, trans);
    	    	    				break;

    	    	    case UD_IDToGroup:		IDToGroup(sd, trans);
    	    	    				break;

    	    	    case UD_NameToGroup:	NameToGroup(sd, trans);
    	    	    				break;

    	    	    case UD_VerifyUser:		VerifyUser(sd, trans);
    	    	    				break;

    	    	    case UD_AdminUser:		AdminUser(sd, trans);
    	    	    				break;

    	    	    case UD_ModifyGroup:	ModifyGroup(sd, trans);
    	    	    				break;

    	    	    case UD_MemberOf:		MemberOf(sd, trans);
						break;

    	    	    default:			trans->trans_Error = ENVOYERR_CMDUNKNOWN;
    	    	    				break;
    	    	}
    	    	ReplyTransaction(trans);
    	    }
    	    if(sd->sd_FlushUserDat)
    	    {
    	    	sd->sd_FlushUserDat = FALSE;
    	    	WriteUserData(sd);
    	    }
    	    if(sd->sd_FlushGroupDat)
    	    {
    	    	sd->sd_FlushGroupDat = FALSE;
    	    	WriteGroupData(sd);
    	    }
    	}
    	DeleteEntity(entity);
    }
}

VOID AddUser(ServerDataPtr sd, struct Transaction *trans)
{
    AdminUserReq *aur;
    UserNode *authority;

    aur = (AdminUserReq *)trans->trans_RequestData;

    if((authority = VerifyAuthority(sd, &aur->Authority)) && (strcmp(sd->sd_UserHeader.AdminPassword,aur->Authority.AuthPW)))
    {
    	if(authority->Data.Flags & UFLAGF_AdminAll)
    	{
    	    if(InternalAddUser(sd,  &aur->User))
    	    {
    	    	trans->trans_Error = 0;
    	    }
    	    else
    	    {
    	    	trans->trans_Error = SECERROR_NORESOURCES;
    	    }
    	}
    	else
    	{
    	    trans->trans_Error = SECERROR_NOPRIVS;
    	}
    }
    else
    {
    	trans->trans_Error = SECERROR_NOAUTHORITY;
    }
}

BOOL InternalAddUser(ServerDataPtr sd, UserData *user)
{
    UserNode *newuser;
    BOOL result = FALSE;

    if(sd->sd_UserHeader.NextUserID <= 65534)
    {
    	if(newuser = AllocMem(sizeof(UserNode),MEMF_CLEAR|MEMF_PUBLIC))
	{
	    newuser->Link.ln_Name = (STRPTR) &newuser->Data;
    	    newuser->Data.UserID = sd->sd_UserHeader.NextUserID++;
    	    CopyMem(user, &newuser->Data, sizeof(UserData));
	    AddUserSorted(sd, &sd->sd_UserList, newuser);
	    sd->sd_FlushUserDat = result = TRUE;
	}
    }
    return(result);
}

VOID AddUserSorted(ServerDataPtr sd, struct MinList *list, UserNode *user)
{
    UserNode *current;

    current = (UserNode *)list->mlh_TailPred;

    while(current->Link.ln_Pred)
    {
    	if(user->Data.UserID > current->Data.UserID)
    	{
    	    Insert((struct List *)list, (struct Node *)user, (struct Node *)current);
    	    break;
    	}
    	current = (UserNode *)current->Link.ln_Pred;
    }
    if(!current->Link.ln_Pred)
    {
    	AddHead((struct List *)list, (struct Node *)user);
    }
}

VOID RemUser(ServerDataPtr sd, struct Transaction *trans)
{
    AdminUserReq *aur;
    UserNode *authority;

    aur = (AdminUserReq *)trans->trans_RequestData;

    if((authority = VerifyAuthority(sd, &aur->Authority)) && (strcmp(sd->sd_UserHeader.AdminPassword,aur->Authority.AuthPW)))
    {
    	if(authority->Data.Flags & UFLAGF_AdminAll)
    	{
    	    if(InternalRemUser(sd, &aur->User))
    	    {
    	    	trans->trans_Error = 0;
    	    }
    	    else
    	    {
    	    	trans->trans_Error = SECERROR_UNKNOWNUSER;
    	    }
    	}
    	else
    	{
    	    trans->trans_Error = SECERROR_NOPRIVS;
    	}
    }
    else
    {
    	trans->trans_Error = SECERROR_NOAUTHORITY;
    }
}

BOOL InternalRemUser(ServerDataPtr sd, UserData *user)
{
    UserNode *olduser;
    GroupNode *group;
    BOOL result = FALSE;
    olduser = FindUserByName(sd, user->UserName);

    if(!olduser)
    	FindUserByUID(sd, user->UserID);

    if(olduser)
    {
    	Remove((struct Node *)olduser);

    	group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    	while(group->Link.ln_Succ)
    	{
    	    InternalRemUserFromGroup(sd, group, olduser);

    	    group = (GroupNode *)group->Link.ln_Succ;
    	}
    	sd->sd_FlushUserDat = result = TRUE;
    }
    return(result);
}

VOID InternalRemUserFromGroup(ServerDataPtr sd, GroupNode *group, UserNode *user)
{
    MinUserNode *minuser;

    minuser = (MinUserNode *)group->UserList.mlh_Head;

    while(minuser->Link.mln_Succ)
    {
    	if(minuser->User == user)
    	{
    	    Remove((struct Node *)minuser);
    	    FreeMem(minuser,sizeof(MinUserNode));
    	    break;
    	}
    	minuser = (MinUserNode *)minuser->Link.mln_Succ;
    }
}

UserNode *FindUserByName(ServerDataPtr sd, STRPTR name)
{
    UserNode *user;

    user = (UserNode *)sd->sd_UserList.mlh_Head;

    while(user->Link.ln_Succ)
    {
    	if(!Stricmp(name,user->Data.UserName))
    	    break;

    	user = (UserNode *) user->Link.ln_Succ;
    }
    if(!user->Link.ln_Succ)
    	user = NULL;

    return(user);
}

UserNode *FindUserByUID(ServerDataPtr sd, UWORD uid)
{
    UserNode *user;

    user = (UserNode *)sd->sd_UserList.mlh_Head;

    while(user->Link.ln_Succ)
    {
    	if(uid == user->Data.UserID)
    	    break;

    	user = (UserNode *) user->Link.ln_Succ;
    }
    if(!user->Link.ln_Succ)
    	user = NULL;

    return(user);
}

GroupNode *FindGroupByName(ServerDataPtr sd, STRPTR name)
{
    GroupNode *group;

    group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    while(group->Link.ln_Succ)
    {
    	if(!Stricmp(name,group->Data.GroupName))
    	    break;

    	group = (GroupNode *) group->Link.ln_Succ;
    }
    if(!group->Link.ln_Succ)
    	group = NULL;

    return(group);
}

GroupNode *FindGroupByGID(ServerDataPtr sd, UWORD gid)
{
    GroupNode *group;

    group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    while(group->Link.ln_Succ)
    {
    	if(gid == group->Data.GroupID)
    	    break;

    	group = (GroupNode *) group->Link.ln_Succ;
    }
    if(!group->Link.ln_Succ)
    	group = NULL;

    return(group);
}


VOID NextUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;

    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    user = (UserNode *)sd->sd_UserList.mlh_Head;

    while(user->Link.ln_Succ)
    {
	if(user->Data.UserID > userdata->UserID)
	{
	    CopyMem(&user->Data,userdata,sizeof(UserData));
	    trans->trans_Error = 0;
	    break;
	}
	user = (UserNode *)user->Link.ln_Succ;
    }
    if(!user->Link.ln_Succ)
	trans->trans_Error = SECERROR_LASTUSER;
}

VOID IDToUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;
    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    if(user = FindUserByUID(sd, userdata->UserID))
    {
    	CopyMem(&user->Data,userdata,sizeof(UserData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = SECERROR_UNKNOWNUSER;
}

VOID NameToUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;
    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    if(user = FindUserByName(sd, userdata->UserName))
    {
    	CopyMem(&user->Data,userdata,sizeof(UserData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = SECERROR_UNKNOWNUSER;
}

VOID IDToGroup(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    GroupData *groupdata;

    groupdata = (GroupData *)trans->trans_RequestData;

    if(group = FindGroupByGID(sd, groupdata->GroupID))
    {
    	CopyMem(&group->Data,groupdata,sizeof(GroupData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = SECERROR_UNKNOWNGROUP;
}

VOID NameToGroup(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    GroupData *groupdata;

    groupdata = (GroupData *)trans->trans_RequestData;

    if(group = FindGroupByName(sd, groupdata->GroupName))
    {
    	CopyMem(&group->Data,groupdata,sizeof(GroupData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = SECERROR_UNKNOWNGROUP;
}

UserNode *VerifyAuthority(ServerDataPtr sd, AuthorityData *ad)
{
    UserNode *user;

    if(user = FindUserByName(sd, ad->UserName))
    {
    	if(strcmp(user->Data.Password, ad->Password))
    	    user = NULL;
    }
    return(user);
}

UserNode *InternalVerifyUser(ServerDataPtr sd, UserData *userdata)
{
    UserNode *user;

    if(user = FindUserByName(sd, userdata->UserName))
    {
    	if(strcmp(user->Data.Password, userdata->Password))
    	    user = NULL;
    }
    return(user);
}

VOID VerifyUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;
    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    if(user = InternalVerifyUser(sd, userdata))
    {
    	CopyMem(&user->Data,userdata,sizeof(UserData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = SECERROR_UNKNOWNUSER;
}

VOID AdminUser(ServerDataPtr sd, struct Transaction *trans)
{
    AdminUserReq *adminreq;
    UserNode *auth;
    UserNode *user;

    adminreq = (AdminUserReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &adminreq->Authority))
    {
    	if(user = FindUserByUID(sd, adminreq->User.UserID))
    	{
    	    if((auth == user) && (user->Data.Flags & UFLAGF_AdminSelf))
    	    {
    	        ULONG flags;
    	        flags = user->Data.Flags;
    	    	CopyMem(&adminreq->User,&user->Data,sizeof(UserData));
    	    	user->Data.Flags = flags;

    	    	sd->sd_FlushUserDat = TRUE;
    	    	trans->trans_Error = 0;
    	    }
	    else if((auth->Data.Flags & UFLAGB_AdminAll) && (strcmp(adminreq->Authority.AuthPW,sd->sd_UserHeader.AdminPassword)))
	    {
	    	CopyMem(&adminreq->User,&user->Data,sizeof(UserData));
	    	sd->sd_FlushUserDat = TRUE;
	    	trans->trans_Error = 0;
	    }
	    else
	    	trans->trans_Error = SECERROR_NOPRIVS;
	}
	else
	    trans->trans_Error = SECERROR_UNKNOWNUSER;
    }
    else
    	trans->trans_Error = SECERROR_NOAUTHORITY;
}

VOID ModifyGroup(ServerDataPtr sd, struct Transaction *trans)
{
    ModifyGroupReq *modreq;
    UserNode *auth;
    GroupNode *group;

    modreq = (ModifyGroupReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &modreq->Authority))
    {
    	if(group = FindGroupByGID(sd, modreq->Group.GroupID))
    	{
     	    if((auth->Data.Flags & UFLAGF_AdminAll) || (auth->Data.UserID = group->Data.AdminID))
     	    {
     	    	strcpy(group->Data.GroupName,modreq->Group.GroupName);
     	    	sd->sd_FlushGroupDat = TRUE;
     	    	trans->trans_Error = 0;
     	    }
     	    else
     	    	trans->trans_Error = SECERROR_NOPRIVS;
     	}
     	else
     	    trans->trans_Error = SECERROR_UNKNOWNGROUP;
     }
     else
     	trans->trans_Error = SECERROR_NOAUTHORITY;
}

VOID AddGroup(ServerDataPtr sd, struct Transaction *trans)
{
    AdminGroupReq *adminreq;
    UserNode *auth;
    GroupNode *group;

    adminreq = (AdminGroupReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &adminreq->Authority))
    {
    	if((group = FindGroupByGID(sd, adminreq->Group.GroupID)) ||
    	   (group = FindGroupByName(sd, adminreq->Group.GroupName)))
    	{
    	    trans->trans_Error = SECERROR_GROUPEXISTS;
    	}
	else if(auth->Data.Flags & (UFLAGF_AdminGroups | UFLAGF_AdminAll))
	{
	    if(sd->sd_GroupHeader.NextGroupID <= 65534)
	    {
    	    	if(group = AllocMem(sizeof(GroupNode),MEMF_CLEAR|MEMF_PUBLIC))
    	    	{
    	    	    NewList((struct List *)&group->UserList);
    	    	    adminreq->Group.GroupID = sd->sd_GroupHeader.NextGroupID++;
    	    	    CopyMem(&group->Data,&adminreq->Group,sizeof(GroupData));
    	    	    AddGroupSorted(sd, &sd->sd_GroupList, group);
		    sd->sd_FlushGroupDat = TRUE;
		    trans->trans_Error = 0;
		}
		else
		    trans->trans_Error = SECERROR_NORESOURCES;
	    }
	    else
	    	trans->trans_Error = SECERROR_NOFREEGROUPS;
	}
	else
	    trans->trans_Error = SECERROR_NOPRIVS;
    }
    else
    	trans->trans_Error = SECERROR_NOAUTHORITY;
}

VOID AddGroupSorted(ServerDataPtr sd, struct MinList *list, GroupNode *group)
{
    GroupNode *current;

    current = (GroupNode *)list->mlh_TailPred;

    while(current->Link.ln_Pred)
    {
    	if(group->Data.GroupID > current->Data.GroupID)
    	{
    	    Insert((struct List *)list, (struct Node *)group, (struct Node *)current);
    	    break;
    	}
    	current = (GroupNode *)current->Link.ln_Pred;
    }
    if(!current->Link.ln_Pred)
    {
    	AddHead((struct List *)list, (struct Node *)group);
    }
}

VOID RemGroup(ServerDataPtr sd, struct Transaction *trans)
{
    AdminGroupReq *adminreq;
    GroupNode *group;
    MinUserNode *minuser;
    UserNode *auth;

    adminreq = (AdminGroupReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &adminreq->Authority))
    {
    	group = FindGroupByGID(sd, adminreq->Group.GroupID);
    	if(!group)
    	    group = FindGroupByName(sd, adminreq->Group.GroupName);

    	if(group)
    	{
    	    if((auth->Data.UserID == group->Data.AdminID) && (auth->Data.Flags & UFLAGF_AdminAll))
    	    {
    	    	while(minuser = (MinUserNode *)RemHead((struct List *)group->UserList.mlh_Head))
    	    	    FreeMem(minuser,sizeof(MinUserNode));

    	    	Remove((struct Node *)group);
    	    	FreeMem(group,sizeof(GroupNode));

    	    	sd->sd_FlushGroupDat = TRUE;

    	    	trans->trans_Error = 0;
    	    }
    	    else
    	    	trans->trans_Error = SECERROR_NOPRIVS;
    	}
    	else
    	    trans->trans_Error = SECERROR_UNKNOWNGROUP;
    }
    else
    	trans->trans_Error = SECERROR_NOAUTHORITY;
}

VOID NextGroup(ServerDataPtr sd, struct Transaction *trans)
{
    GroupData *groupdat;
    GroupNode *group;

    groupdat = (GroupData *)trans->trans_RequestData;

    group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    while(group->Link.ln_Succ)
    {
	if(group->Data.GroupID > groupdat->GroupID)
	{
    	    CopyMem(&group->Data,groupdat,sizeof(GroupData));
	    trans->trans_Error = 0;
	    break;
	}
	group =	(GroupNode *)group->Link.ln_Succ;
    }

    if(!group->Link.ln_Succ)
    	trans->trans_Error = SECERROR_LASTGROUP;
}

VOID NextMember(ServerDataPtr sd, struct Transaction *trans)
{
    NextMemberReq *nextmember;
    MinUserNode *minuser;
    GroupNode *group;

    nextmember = (NextMemberReq *)trans->trans_RequestData;

    group = FindGroupByGID(sd, nextmember->Group.GroupID);

    if(group)
    {
	minuser	= (MinUserNode *)group->UserList.mlh_Head;

	while(minuser->Link.mln_Succ)
	{
	    if(minuser->User->Data.UserID > nextmember->User.UserID)
	    {
	        CopyMem(&minuser->User->Data,&nextmember->User,sizeof(UserData));
    	        trans->trans_Error = 0;
		break;
	    }
	    minuser = (MinUserNode *)minuser->Link.mln_Succ;
	}
    	if(!minuser->Link.mln_Succ)
    	    trans->trans_Error = SECERROR_LASTMEMBER;
    }
    else
    	trans->trans_Error = SECERROR_UNKNOWNGROUP;
}

MinUserNode *FindMember(ServerDataPtr sd, GroupNode *group, UserNode *user)
{
    MinUserNode *current;

    current = (MinUserNode *)group->UserList.mlh_Head;

    while(current->Link.mln_Succ)
    {
    	if(current->User == user)
    	    return(current);

    	current = (MinUserNode *)current->Link.mln_Succ;
    }
    return(NULL);
}

VOID MemberOf(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    UserNode *user;
    NextMemberReq *nextmember;

    nextmember = (NextMemberReq *)trans->trans_RequestData;

    group = FindGroupByGID(sd, nextmember->Group.GroupID);

    if(group)
    {
    	user = FindUserByUID(sd, nextmember->User.UserID);
    	if(!user)
    	    user = FindUserByName(sd, nextmember->User.UserName);

	if(user)
	{
	    if(FindMember(sd, group, user))
	    {
	    	trans->trans_Error = 0;
	    }
	    else
	    {
	    	trans->trans_Error = SECERROR_UNKNOWNMEMBER;
	    }
	}
	else
	    trans->trans_Error = SECERROR_UNKNOWNUSER;
    }
    else
    	trans->trans_Error = SECERROR_UNKNOWNGROUP;
}

VOID NewList(struct List *list)
{
    list->lh_Head = (struct Node *)&list->lh_Tail;
    list->lh_Tail = NULL;
    list->lh_TailPred = (struct Node *)list;
}

VOID AddMember(ServerDataPtr sd, struct Transaction *trans)
{
    return;
}

VOID RemoveMember(ServerDataPtr sd, struct Transaction *trans)
{
    return;
}