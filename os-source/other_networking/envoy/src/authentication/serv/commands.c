/*
**  $Id: commands.c,v 1.4 92/04/09 12:01:09 dlarson Exp Locker: dlarson $
**
**  Authentication server command processing functions.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "AuthenticationServer.h"


struct Group RootGroup =
{
	0, 0,	/* g_Node */
	"Administrators",
};

BOOL InGroup(struct Group *group, struct UserProfile *user)
{
int x;

	if( !ReadUser(user) )
	{
		return FALSE;
	}
	if( !ReadGroup(group) )
	{
		return FALSE;
	}
	for(x=0; x<G_MAXUMEMBERS; x++)
	{
		if(group->g_UserMembers[x] == user->up_UID)
		{
			return TRUE;
		}
	}
	return FALSE;
}


BOOL AdminGroup(struct Group *group, struct UserProfile *user)
{
	if( !ReadUser(user) )
	{
		return FALSE;
	}
	if( !ReadGroup(group) )
	{
		return FALSE;
	}
	if(group->g_UserMembers[0] == user->up_UID)
	{
		return TRUE;
	}
	if(InGroup(&RootGroup, user))
	{
		return TRUE;
	}
	return FALSE;
}


ULONG AuthenticateUser(struct Transaction *trans)
{
struct UserProfile *user, temp;

	user = (struct UserProfile *)trans->trans_RequestData;
	strcpy(temp.up_UserName, user->up_UserName);
	if( !ReadUser(&temp) )
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if(Stricmp(temp.up_PassWord, user->up_PassWord) == 0)
 	{
 		strcpy(user->up_FullName, temp.up_FullName);
 		user->up_UID = temp.up_UID;
 		user->up_GID = temp.up_GID;
		return ENVOYERR_NOERROR;
	}
	return ENVOYERR_NOAUTHORITY;
}


ULONG AddUser(struct Transaction *trans)
{
struct UserProfile *user, *authority, temp;

	user = (struct UserProfile *)trans->trans_RequestData;
	authority = (struct UserProfile *)
                    ((struct UserProfile *)trans->trans_RequestData)->Authority;
	if( !strlen(user->up_UserName) )
	{
		return ENVOYERR_ZEROLENGTHNAME;
	}
	strcpy(temp.up_UserName, authority->up_UserName);
	if(!ReadUser(&temp) || Stricmp(temp.up_PassWord, authority->up_PassWord))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if(!InGroup(&RootGroup, &temp))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	strcpy(temp.up_UserName, user->up_UserName);
	if( ReadUser(&temp) )
	{
		return ENVOYERR_OBJEXISTS;
	}
	if(WriteUser(user))
	{
		return ENVOYERR_NOERROR;
	}
	return ENVOYERR_IOERR;
}


ULONG DeleteUser(struct Transaction *trans)
{
struct UserProfile *user, *authority, temp;

	user = (struct UserProfile *)trans->trans_RequestData;
	authority = (struct UserProfile *)((struct UserProfile *)trans->trans_RequestData)->Authority;
	if(user->up_UID == 1)
	{
		return ENVOYERR_ID1UNDELETABLE;
	}
	strcpy(temp.up_UserName, authority->up_UserName);
	if(!ReadUser(&temp) || Stricmp(temp.up_PassWord, authority->up_PassWord))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if(!InGroup(&RootGroup, &temp))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if( RemoveUser(user) )
	{
		return ENVOYERR_NOERROR;
	}
	return ENVOYERR_NOOBJECT;
}


ULONG ChangePassWord(struct Transaction *trans)
{
struct UserProfile *user, *authority, temp;

	user = (struct UserProfile *)trans->trans_RequestData;
	authority = (struct UserProfile *)((struct UserProfile *)trans->trans_RequestData)->Authority;
	strcpy(temp.up_UserName, authority->up_UserName);
	if(!ReadUser(&temp) || Stricmp(temp.up_PassWord, authority->up_PassWord))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if( !(InGroup(&RootGroup, &temp) || user->up_UID == temp.up_UID) )
	{
		return ENVOYERR_NOAUTHORITY;
	}
	strcpy(temp.up_UserName, user->up_UserName);
	if(!ReadUser(&temp) )
	{
		return ENVOYERR_NOOBJECT;
	}
	user->up_UID = temp.up_UID;
	if(WriteUser(user))
	{
		return ENVOYERR_NOERROR;
	}
	return ENVOYERR_IOERR;
}


ULONG FindUserName(struct Transaction *trans)
{
struct UserProfile *user, temp;

	user = (struct UserProfile *)trans->trans_RequestData;
	temp.up_UID = user->up_UID;
	if(!ReadUID(&temp))
	{
		return ENVOYERR_NOOBJECT;  /*  But could be an IOERR!  */
	}
	strcpy(user->up_UserName, temp.up_UserName);
	strcpy(user->up_FullName, temp.up_FullName);
	user->up_GID = temp.up_GID;
	return ENVOYERR_NOERROR;
}


ULONG MemberOf(struct Transaction *trans)
{
struct UserProfile *user;
struct Group *group;

	user = (struct UserProfile *)trans->trans_RequestData;
	group = (struct Group *)user->Group;
	if(InGroup(group, user))
	{
		return ENVOYERR_NOERROR;
	}
	return ENVOYERR_NOAUTHORITY;
}


ULONG AddGroup(struct Transaction *trans)
{
struct UserProfile *authority, temp;
struct Group *group;
int x;

	authority = (struct UserProfile *)trans->trans_RequestData;
	group = (struct Group *)authority->Group;
	if( !strlen(group->g_GroupName) )
	{
		return ENVOYERR_ZEROLENGTHNAME;
	}
	strcpy(temp.up_UserName, authority->up_UserName);
	if(!ReadUser(&temp) || Stricmp(temp.up_PassWord, authority->up_PassWord))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	group->g_UserMembers[0] = temp.up_UID;
	for(x=1; x<G_MAXUMEMBERS; x++)
	{
		group->g_UserMembers[x] = 0;
	}
	for(x=0; x<G_MAXGMEMBERS; x++)
	{
		group->g_GroupMembers[x] = 0;
	}
	if(WriteGroup(group))
	{
		return ENVOYERR_NOERROR;
	}
	return ENVOYERR_IOERR;
}


ULONG DeleteGroup(struct Transaction *trans)
{
struct UserProfile *authority, temp;
struct Group *group;

	authority = (struct UserProfile *)trans->trans_RequestData;
	group = (struct Group *)authority->Group;
	if(group->g_GID == 1)
	{
		return ENVOYERR_ID1UNDELETABLE;
	}
	strcpy(temp.up_UserName, authority->up_UserName);
	if(!ReadUser(&temp) || Stricmp(temp.up_PassWord, authority->up_PassWord))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if(!ReadGroup(group))
	{
		return ENVOYERR_NOOBJECT;
	}
	if(!AdminGroup(group, &temp))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if(RemoveGroup(group))
	{
		return ENVOYERR_NOERROR;
	}
	return ENVOYERR_NOOBJECT;  /* could be IOERR!  */
}


ULONG AddToGroup(struct Transaction *trans)
{
struct UserProfile *user, *authority, tempa, tempu;
struct Group *group;
int x;

	user = (struct UserProfile *)trans->trans_RequestData;
	authority = (struct UserProfile *)user->Authority;
	group = (struct Group *)user->Group;
	strcpy(tempa.up_UserName, authority->up_UserName);
	if(!ReadUser(&tempa) || Stricmp(tempa.up_PassWord, authority->up_PassWord))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if(!AdminGroup(group, &tempa))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	strcpy(tempu.up_UserName, user->up_UserName);
	if(!ReadUser(&tempu) || !ReadGroup(group))
	{
		return ENVOYERR_NOOBJECT;
	}
	for(x=0; x<G_MAXUMEMBERS; x++)
	{
		if(group->g_UserMembers[x] == tempu.up_UID)
		{
			return ENVOYERR_OBJEXISTS;
		}
		if(group->g_UserMembers[x] == 0)
		{
			group->g_UserMembers[x] = tempu.up_UID;
			if(WriteGroup(group))
			{
				return ENVOYERR_NOERROR;
			}
			return ENVOYERR_IOERR;
		}
	}
	return ENVOYERR_NORESOURCES;
}


ULONG RemoveFromGroup(struct Transaction *trans)
{
struct UserProfile *user, *authority, tempa, tempu;
struct Group *group;
int x;

	user = (struct UserProfile *)trans->trans_RequestData;
	authority = (struct UserProfile *)user->Authority;
	group = (struct Group *)user->Group;
	strcpy(tempa.up_UserName, authority->up_UserName);
	if(!ReadUser(&tempa) || Stricmp(tempa.up_PassWord, authority->up_PassWord))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	if(!AdminGroup(group, &tempa))
	{
		return ENVOYERR_NOAUTHORITY;
	}
	strcpy(tempu.up_UserName, user->up_UserName);
	if(!ReadUser(&tempu) || !ReadGroup(group))
	{
		return ENVOYERR_NOOBJECT;
	}
	for(x=0; x<G_MAXUMEMBERS; x++)
	{
		if(group->g_UserMembers[x] == tempu.up_UID)
		{
			group->g_UserMembers[x] = 0;
			if(WriteGroup(group))
			{
				return ENVOYERR_NOERROR;
			}
			return ENVOYERR_IOERR;
		}
	}
	return ENVOYERR_NORESOURCES;
}


ULONG FindGroupName(struct Transaction *trans)
{
struct group *group;

	group = (struct Group *)trans->trans_RequestData;
	if(!ReadGroupID(group))
	{
		return ENVOYERR_NOOBJECT;  /*  But could be an IOERR!  */
	}
	return ENVOYERR_NOERROR;
}


ULONG FindGroupID(struct Transaction *trans)
{
struct group *group;

	group = (struct Group *)trans->trans_RequestData;
	if(!ReadGroup(group))
	{
		return ENVOYERR_NOOBJECT;  /*  But could be an IOERR!  */
	}
	return ENVOYERR_NOERROR;
}


ULONG ListUsers(struct Transaction *trans)
{
struct MinList *list;
struct UserProfile *node;
UBYTE *pw;

	list = GetUserList();  /* this has passwords!  */
	trans->trans_RespDataActual = 0;
	if(trans->trans_RespDataLength <  PROFILE_DSIZE)
	{
		return ENVOYERR_SMALLRESPBUFF;
	}
	for(node = (struct UserProfile *)list->mlh_Head;
	    node->up_Node.mln_Succ;
	    node = (struct UserProfile *)node->up_Node.mln_Succ)
	{
		CopyMem(&(node->up_UserName),
			((UBYTE *)trans->trans_ResponseData)+trans->trans_RespDataActual,
			PROFILE_DSIZE);
		pw =  ((UBYTE *)trans->trans_ResponseData)+
			trans->trans_RespDataActual+
			UP_UNLENGTH+UP_FNLENGTH;
		CopyMem("               ", pw, 16); /* blank password */
		trans->trans_RespDataActual += PROFILE_DSIZE;
		if(trans->trans_RespDataLength <  trans->trans_RespDataActual)
		{
			trans->trans_RespDataActual = trans->trans_RespDataLength;
			return ENVOYERR_SMALLRESPBUFF;
		}
	}
	return ENVOYERR_NOERROR;
}


ULONG ListMembers(struct Transaction *trans)
{
struct MinList *list;
struct UserProfile *node;
struct Group *group;
UBYTE *pw;

	list = GetUserList();  /* this has passwords!  */
	group = (struct Group *)trans->trans_RequestData;
	trans->trans_RespDataActual = 0;
	if(trans->trans_RespDataLength <  PROFILE_DSIZE)
	{
		return ENVOYERR_SMALLRESPBUFF;
	}
	for(node = (struct UserProfile *)list->mlh_Head;
	    node->up_Node.mln_Succ;
	    node = (struct UserProfile *)node->up_Node.mln_Succ)
	{
		if(InGroup(group, node))
		{
			CopyMem(&(node->up_UserName),
				((UBYTE *)trans->trans_ResponseData)+trans->trans_RespDataActual,
				PROFILE_DSIZE);
			pw =  ((UBYTE *)trans->trans_ResponseData)+
				trans->trans_RespDataActual+
				UP_UNLENGTH+UP_FNLENGTH;
			CopyMem("               ", pw, 16); /* blank password */
			trans->trans_RespDataActual += PROFILE_DSIZE;
			if(trans->trans_RespDataLength <  trans->trans_RespDataActual)
			{
				trans->trans_RespDataActual = trans->trans_RespDataLength;
				return ENVOYERR_SMALLRESPBUFF;
			}
		}
	}
	return ENVOYERR_NOERROR;
}


ULONG ListGroups(struct Transaction *trans)
{
struct MinList *list;
struct Group *node;

	list = GetGroupList();
	trans->trans_RespDataActual = 0;
	if(trans->trans_RespDataLength <  GROUP_DSIZE)
	{
		return ENVOYERR_SMALLRESPBUFF;
	}
	for(node = (struct Group *)list->mlh_Head;
	    node->g_Node.mln_Succ;
	    node = (struct Group *)node->g_Node.mln_Succ)
	{
		CopyMem(&(node->g_GroupName),
			((UBYTE *)trans->trans_ResponseData)+trans->trans_RespDataActual,
			 GROUP_DSIZE);
		trans->trans_RespDataActual += GROUP_DSIZE;
		if(trans->trans_RespDataLength <  trans->trans_RespDataActual)
		{
			trans->trans_RespDataActual = trans->trans_RespDataLength;
			return ENVOYERR_SMALLRESPBUFF;
		}
	}
	return ENVOYERR_NOERROR;
}
