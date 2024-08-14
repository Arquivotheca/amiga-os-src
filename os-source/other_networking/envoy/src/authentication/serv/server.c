/*
**  $Id: server.c,v 1.4 92/04/09 12:00:41 dlarson Exp Locker: dlarson $
**
**  Authentication server main loop.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "AuthenticationServer.h"
#include "AuthenticationServer_rev.h"
#include <dos.h>

VOID main(VOID);


struct Library *NIPCBase;
struct Library *UtilityBase;

UBYTE *vers = VERSTAG;

LONG (*Commands[])(struct Transaction *) =
{
	AuthenticateUser,
	AddUser,
	DeleteUser,
	ChangePassWord,
	AddGroup,
	DeleteGroup,
	AddToGroup,
	RemoveFromGroup,
	FindUserName,
	FindGroupName,
	FindGroupID,
	ListGroups,
	ListUsers,
	ListMembers,
	MemberOf,
};

VOID main(VOID)
{
APTR entity;
struct Transaction *trans;
ULONG sigf_entity, signals;

	UtilityBase = OpenLibrary("utility.library", 37L);
	if(!UtilityBase)
	{
		printf("Can't open utility.library!!!\n");
		exit(-1);
	}
	if(!InitUserDB())
	{
		printf("Can't init userdb.\n");
		CloseLibrary(UtilityBase);
		exit(-1);
	}
	if(!InitGroupDB())
	{
		printf("Can't init groupdb.\n");
		FreeUserDB();
		CloseLibrary(UtilityBase);
		exit(-1);
	}
	NIPCBase = OpenLibrary("nipc.library", 0L);
	if(!NIPCBase)
	{
		printf("Can't open nipc.library!!!\n");
		FreeUserDB();
		FreeGroupDB();
		CloseLibrary(UtilityBase);
		exit(-1);
	}
	entity = CreateEntity(	ENT_Name, "AuthenticationServer",
				ENT_Public, 0L,
				ENT_AllocSignal, (LONG)&sigf_entity,
				TAG_DONE);
	if(!entity)
	{
		printf("Failed to CreateEntity().\n");
		FreeUserDB();
		FreeGroupDB();
		CloseLibrary(NIPCBase);
		exit(-1);
	}
	sigf_entity = 1 << sigf_entity;
	for(EVER)
	{
		signals = Wait(sigf_entity | SIGBREAKF_CTRL_C);
		if(signals & SIGBREAKF_CTRL_C)
		{
			printf("exiting normally.\n");
			FreeUserDB();
			FreeGroupDB();
			DeleteEntity(entity);
			CloseLibrary(NIPCBase);
			exit(0);
		}
		if( !(signals & sigf_entity) )  /*  JUST TO BE SURE  */
		{
			printf("woke up by wierd signal!!!\n");
			continue;
		}
		trans = GetTransaction(entity);
		if(!trans)  /* JUST TO BE SURE  */
		{
			printf("no transaction!!!\n");
			continue;
		}
		if(trans->trans_Command > CMD_LIMIT)
		{
			printf("unknown command...  ");
			trans->trans_Error = ENVOYERR_CMDUNKNOWN;
		}else
		{
			trans->trans_Error = (UBYTE)(*Commands[trans->trans_Command])(trans);
		}
		ReplyTransaction(trans);
	}
}
