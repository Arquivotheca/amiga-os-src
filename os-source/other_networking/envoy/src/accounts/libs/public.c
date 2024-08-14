/*
** $Id: public.c,v 1.1 92/10/13 11:20:55 kcd Exp $
**
** This module contains calls to allocate and free public data structures
** used by accounts.library.
**
*/

/*--------------------------------------------------------------------------*/

#ifndef	EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#include "accountsbase.h"
#include "/accounts.h"

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

/*--------------------------------------------------------------------------*/

struct UserInfo * ASM AllocUserInfo(void)
{
	return((struct UserInfo *)AllocMem(sizeof(struct UserInfo),MEMF_PUBLIC|MEMF_CLEAR));
}

/*--------------------------------------------------------------------------*/

struct GroupInfo * ASM AllocGroupInfo(void)
{
	return((struct GroupInfo *)AllocMem(sizeof(struct GroupInfo),MEMF_PUBLIC|MEMF_CLEAR));
}

/*--------------------------------------------------------------------------*/

VOID ASM FreeUserInfo(REG(a0) struct UserInfo *user)
{
	FreeMem(user,sizeof(struct UserInfo));
}

/*--------------------------------------------------------------------------*/

VOID ASM FreeGroupInfo(REG(a0) struct GroupInfo *group)
{
	FreeMem(group,sizeof(struct GroupInfo));
}

