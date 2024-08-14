#ifndef ACCOUNTSBASE_H
#define ACCOUNTSBASE_H

/*****************************************************************************/

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <utility/tagitem.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos.h>
#include <envoy/nipc.h>
#include "accounts.h"

/*****************************************************************************/

struct AccountsLib
{
    struct Library           ACNTS_Lib;
    APTR                     ACNTS_NIPCBase;
    struct ExecBase         *ACNTS_SysBase;
    BPTR                     ACNTS_SegList;

    struct Entity	    *ACNTS_Entity;

};

#ifndef SERVICES_MANAGER
#define ASM           __asm
#define REG(x)        register __ ## x

#define AccountsBase  ((struct AccountsLib *)getreg(REG_A6))
#define SysBase       AccountsBase->ACNTS_SysBase
#define NIPCBase      AccountsBase->ACNTS_NIPCBase

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);

/*****************************************************************************/

struct UserInfo * ASM AllocUserInfo(void);

struct GroupInfo * ASM AllocGroupInfo(void);

VOID ASM FreeUserInfo(REG(a0) struct UserInfo *user);

VOID ASM FreeGroupInfo(REG(a0) struct GroupInfo *group);

BOOL ASM VerifyUser(REG(a0) STRPTR userName,
		    REG(a1) STRPTR password,
		    REG(a2) struct UserInfo *user);

BOOL ASM MemberOf(REG(a0) struct GroupInfo *group,
		  REG(a1) struct UserInfo *user);

BOOL NameToUser(REG(a0) STRPTR userName,
		REG(a1) struct UserInfo *user);

BOOL NameToGroup(REG(a0) STRPTR groupName,
		 REG(a1) struct GroupInfo *ser);

BOOL IDToUser(REG(d0) userID,
	      REG(a0) struct UserInfo *user);

BOOL IDToGroup(REG(d0) groupID,
	       REG(a0) struct GroupInfo *group);

BOOL NextUser(REG(a0) struct UserInfo *user);

BOOL NextGroup(REG(a0) struct GroupInfo *group);

BOOL NextMember(REG(a0) struct GroupInfo *group,
		REG(a1) struct UserInfo *user);


/*****************************************************************************/

kprintf(STRPTR,...);
sprintf(STRPTR,...);

#endif /* ACCOUNTSBASE_H */
