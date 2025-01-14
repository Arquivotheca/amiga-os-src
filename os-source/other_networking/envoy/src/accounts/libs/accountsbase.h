#ifndef ACCOUNTSBASE_H
#define ACCOUNTSBASE_H

/*****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

#ifndef	EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#ifndef EXEC_EXECBASE_H
#include <exec/execbase.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif

#ifndef DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif

#ifndef DOS_DOSTAGS_H
#include <dos/dostags.h>
#endif

#ifndef ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif

#include <dos.h>
#include "/accounts.h"

/*****************************************************************************/

struct AccountsLib
{
    struct Library           ACNTS_Lib;
    APTR                     ACNTS_NIPCBase;
    struct ExecBase         *ACNTS_SysBase;
    struct Library	    *ACNTS_DOSBase;
    struct Library	    *ACNTS_UtilityBase;
    BPTR                     ACNTS_SegList;

    struct Entity	    *ACNTS_Entity;
    struct Entity	    *ACNTS_AccEntity;

    struct SignalSemaphore   ACNTS_Lock;
    ULONG		     ACNTS_HTag;
    UBYTE		     ACNTS_HostName[128];
};

#ifndef SERVICES_MANAGER
#define ASM           __asm
#define REG(x)        register __ ## x

#define AccountsBase  ((struct AccountsLib *)getreg(REG_A6))
#define SysBase       AccountsBase->ACNTS_SysBase
#define NIPCBase      AccountsBase->ACNTS_NIPCBase
#define UtilityBase   AccountsBase->ACNTS_UtilityBase

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);

/*****************************************************************************/

struct UserInfo * ASM AllocUserInfo(void);

struct GroupInfo * ASM AllocGroupInfo(void);

VOID ASM FreeUserInfo(REG(a0) struct UserInfo *user);

VOID ASM FreeGroupInfo(REG(a0) struct GroupInfo *group);

ULONG VerUser(STRPTR userName, STRPTR password, struct UserInfo *user, UBYTE cmd);

ULONG ASM VerifyUser(REG(a0) STRPTR userName,
		     REG(a1) STRPTR password,
		     REG(a2) struct UserInfo *user);

ULONG ASM VerifyUserCrypt(REG(a0) STRPTR userName,
		     	  REG(a1) STRPTR password,
		     	  REG(a2) struct UserInfo *user);

ULONG ASM MemberOf(REG(a0) struct GroupInfo *group,
		   REG(a1) struct UserInfo *user);

ULONG ASM NameToUser(REG(a0) STRPTR userName,
		     REG(a1) struct UserInfo *user);

ULONG ASM NameToGroup(REG(a0) STRPTR groupName,
		      REG(a1) struct GroupInfo *ser);

ULONG ASM IDToUser(REG(d0) UWORD userID,
	      	   REG(a0) struct UserInfo *user);

ULONG ASM IDToGroup(REG(d0) UWORD groupID,
	       	    REG(a0) struct GroupInfo *group);

ULONG ASM NextUser(REG(a0) struct UserInfo *user);

ULONG ASM NextGroup(REG(a0) struct GroupInfo *group);

ULONG ASM NextMember(REG(a0) struct GroupInfo *group,
		     REG(a1) struct UserInfo *user);

STRPTR ASM ECrypt(REG(a0) STRPTR buffer,
		 REG(a1) STRPTR password,
		 REG(a2) STRPTR UserName);

BOOL DoCommand(struct Transaction *trans);

/*****************************************************************************/

kprintf(STRPTR,...);
sprintf(STRPTR,...);

#endif /* ACCOUNTSBASE_H */
