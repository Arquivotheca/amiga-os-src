/*
**  $Id: testdb2.c,v 1.3 92/04/06 11:48:49 dlarson Exp Locker: dlarson $
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
struct UserProfile user;

	if(argc != 2)
	{
		printf("Usage: %s user\n", argv[0]);
		return 0;
	}
	if( !(UtilityBase=OpenLibrary("utility.library", 37L)) )
		exit(-1);
	InitUserDB();
	strcpy(user.up_UserName, argv[1]);
	if(!ReadUser(&user))
	{
		printf("ReadUser() failed!!!\n");
		FreeUserDB();
		CloseLibrary(UtilityBase);
		return 0;
	}
	printf("up_UserName = %s.\n", user.up_UserName);
	printf("up_FullName = %s.\n", user.up_FullName);
	printf("up_PassWord = %s.\n", user.up_PassWord);
	printf("UID      = %d.\n", user.up_UID);
	printf("GID      = %d.\n", user.up_GID);
	FreeUserDB();
	CloseLibrary(UtilityBase);
}
