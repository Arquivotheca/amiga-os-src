/*
**  $Id: testgroupdb1.c,v 1.3 92/04/06 11:48:02 dlarson Exp Locker: dlarson $
**
**  Authentication server database test program 1.
**
**  Creates a new Authentication Server Database with Group group.  This
**  database should be shipped with Envoy disks unless I go to the bother
**  of making a nicer way to get the database going.
**
**  DO NOT RUN THIS PROGRAM ON A SYSTEM RUNNING THE AUTHENTICATION SERVER
**  UNLESS YOU WISH TO LOSE YOUR DATABASE!
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "AuthenticationServer.h"

struct Library *UtilityBase;
extern struct MinList *GroupDBList;
extern USHORT HighGID;

main()
{
struct Group group;
int x;

/*
**  In lieu of InitGroupDB():
*/
	if( !(GroupDBList = AllocMem(sizeof(struct MinList), MEMF_ANY)) )
	{
		printf("Can't allocate memory.\n");
		return 0;
	}
	if( !(UtilityBase=OpenLibrary("utility.library", 37L)) )
		exit(-1);
	NewList((struct List *)GroupDBList);
	HighGID = 0;
	strcpy(group.g_GroupName, "Administrators");
	group.g_GID = GROUP_ADMIN;
	group.g_UserMembers[0]  = 1;
	for(x=1; x<G_MAXUMEMBERS; x++)
	{
		group.g_UserMembers[x]  = 0;
	}
	for(x=0; x<G_MAXGMEMBERS; x++)
	{
		group.g_GroupMembers[x] = 0;
	}
	if(!WriteGroup(&group))
	{
		printf("WriteGroup() failed!!!\n");
		FreeGroupDB();
		CloseLibrary(UtilityBase);
		return 0;
	}
	printf("GID     = %d.\n", group.g_GID);
	printf("HighGID = %d.\n", HighGID);
	FreeGroupDB();
	CloseLibrary(UtilityBase);
}
