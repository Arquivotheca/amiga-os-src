/*
**  $Id: stubs.c,v 1.3 92/04/09 11:58:10 dlarson Exp Locker: dlarson $
**
**	Special nipc.library stubs for calling from within a library with
**	specially #defined NIPCBase.
**
**	(c) Copyright 1992 Commodore-Amiga, Inc.
**		All Rights Reserved.
*/
#include "authlib_internal.h"

APTR *MyCreateEntity(struct AuthenticationBase *AuthenticationBase, Tag tag1, ... )
{
	return( CreateEntityA((struct TagItem *)&tag1) );
}

struct Transaction *MyAllocTransaction(struct AuthenticationBase *AuthenticationBase, Tag tag1, ... )
{
	return( AllocTransactionA((struct TagItem *)&tag1) );
}