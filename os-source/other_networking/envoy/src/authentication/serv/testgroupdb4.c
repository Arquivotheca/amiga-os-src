/*
**  $Id: testgroupdb4.c,v 1.3 92/04/06 11:49:08 dlarson Exp Locker: dlarson $
**
**  Authentication server database test program 4.
**
** Deletes a user from the Authentication Server Database.
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
	if(!RemoveUser(&user))
	{
		printf("RemoveUser() failed!!!\n");
	}
	FreeUserDB();
	CloseLibrary(UtilityBase);
}
