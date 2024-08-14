/*
**  $Id: testgroupdb3.c,v 1.3 92/04/06 11:49:17 dlarson Exp Locker: dlarson $
**
**  Authentication server database test program 3.
**
**  Adds a new user to the Authentication Server Database.
**
**  DO NOT RUN THIS PROGRAM ON A SYSTEM RUNNING THE AUTHENTICATION SERVER
**  UNLESS YOU WISH TO LOSE YOUR DATABASE!
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "AuthenticationServer.h"
struct Library *UtilityBase;

main()
{
struct UserProfile user;

	if( !(UtilityBase=OpenLibrary("utility.library", 37L)) )
		exit(-1);
	InitUserDB();
	strcpy(user.up_UserName, "dlarson");
	strcpy(user.up_FullName, "Dale Lee Larson");
	strcpy(user.up_PassWord, "foo");
	user.up_GID = GROUP_NONE;
	if(!WriteUser(&user))
	{
		printf("WriteUser() failed!!!\n");
		FreeUserDB();
		CloseLibrary(UtilityBase);
		return 0;
	}
	printf("UID     = %d.\n", user.up_UID);
	FreeUserDB();
	CloseLibrary(UtilityBase);
}
