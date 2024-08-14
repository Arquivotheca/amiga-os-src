/*
**  $Id: testgroupdb2.c,v 1.3 92/04/06 11:49:24 dlarson Exp Locker: dlarson $
**
**  Authentication server database test program 2.
**
**  Reads a new user from the Authentication Server Database.
**
**  DO NOT RUN THIS PROGRAM ON A SYSTEM RUNNING THE AUTHENTICATION SERVER
**  UNLESS YOU WISH TO LOSE YOUR DATABASE!
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "AuthenticationServer.h"
struct Library *UtilityBase;

int main(int argc, int *argv[])
{
struct Group group;

	if(argc != 2)
	{
		printf("Usage: %s group\n", argv[0]);
		return 0;
	}
	if( !(UtilityBase=OpenLibrary("utility.library", 37L)) )
		exit(-1);
	InitGroupDB();
	strcpy(group.g_GroupName, argv[1]);
	if(!ReadGroup(&group))
	{
		printf("ReadGroup() failed!!!\n");
		FreeGroupDB();
		CloseLibrary(UtilityBase);
		return 0;
	}
	printf("g_GroupName = %s.\n", group.g_GroupName);
	printf("g_GID       = %d.\n", group.g_GID);
	printf("users       = %d %d %d %d.\n", group.g_UserMembers[0], group.g_UserMembers[1], group.g_UserMembers[2], group.g_UserMembers[3]);
	printf("groups      = %d %d %d %d.\n", group.g_GroupMembers[0], group.g_GroupMembers[1], group.g_GroupMembers[2], group.g_GroupMembers[3]);
	FreeGroupDB();
	CloseLibrary(UtilityBase);
}
