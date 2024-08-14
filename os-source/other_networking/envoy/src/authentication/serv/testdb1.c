/*
**  $Id: testdb1.c,v 1.4 92/04/09 12:01:24 dlarson Exp Locker: dlarson $
**
**  Authentication server database test program 1.
**
**  Creates a new Authentication Server Database with user admin.  This
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
extern struct MinList *UserDBList;
extern USHORT HighUID;

main()
{
struct UserProfile admin;

/*
**  In lieu of InitUserDB():
*/
	if( !(UtilityBase=OpenLibrary("utility.library", 37L)) )
		exit(-1);
	if( !(UserDBList = AllocMem(sizeof(struct MinList), MEMF_ANY)) )
	{
		printf("Can't allocate memory.\n");
		return 0;
	}
	NewList((struct List *)UserDBList);
	HighUID = 0;
	strcpy(admin.up_UserName, "admin");
	strcpy(admin.up_FullName, "System Administrator");
	strcpy(admin.up_PassWord, "changeme");
	admin.up_GID = GROUP_ADMIN;
	if(!WriteUser(&admin))
	{
		printf("WriteUser() failed!!!\n");
		FreeUserDB();
		CloseLibrary(UtilityBase);
		return 0;
	}
	printf("UID     = %d.\n", admin.up_UID);
	printf("HighUID = %d.\n", HighUID);
	FreeUserDB();
	CloseLibrary(UtilityBase);
}
