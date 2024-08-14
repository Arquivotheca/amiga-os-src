/*
**  $Id: custom.c,v 1.3 92/04/09 11:58:03 dlarson Exp Locker: dlarson $
**
**  Custom initialization and expunge for authentication.library.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include "authlib_internal.h"

BOOL __asm CustomLibOpen(register __a0 struct AuthenticationBase *AuthenticationBase)
{
BUG(("CustomLibOpen.\n"));
	ServerEntity = NULL;
	NIPCBase = OpenLibrary("nipc.library", 0L);
	if(!NIPCBase)
	{
		return(FALSE);
	}
}


VOID __asm CustomLibClose(register __a0 struct AuthenticationBase *AuthenticationBase)
{
	if(ServerEntity)
	{
		LoseEntity(ServerEntity);
		ServerEntity=NULL;
	}
	if(NIPCBase)
	{
		CloseLibrary(NIPCBase);
		NIPCBase = NULL;
	}
}