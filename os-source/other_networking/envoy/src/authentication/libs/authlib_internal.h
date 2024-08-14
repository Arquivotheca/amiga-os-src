#ifndef AUTHLIB_INTERNAL_H
#define AUTHLIB_INTERNAL_H
/*
**  $ID:$
**
**	Authentication.library INTERNAL versions of public structures and other
**	internal includes.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#define ABSEXECBASE (*(struct Library **)4L)
#define SysBase ABSEXECBASE
#include "/auth_internal.h"
#define LIBENT __asm
/*
** library base
*/
struct AuthenticationBase
{
        struct       Library ml_Lib;
        ULONG        ml_SegList;
        ULONG        ml_Flags;
        APTR         ml_ExecBase;	/* pointer to exec base */
        LONG         ml_Data;		/* Global data */
/***** Below are customizations *****/
	struct Library *NIPCBase;
	APTR ServerEntity;
	APTR SrcEntity;
	ULONG NIPCSignal;
	BOOL OnOff;
};
/*
**  Global variable defines
*/
#define NIPCBase (AuthenticationBase->NIPCBase)
#define ServerEntity (AuthenticationBase->ServerEntity)
#define SrcEntity (AuthenticationBase->SrcEntity)
#define NIPCSignal (AuthenticationBase->NIPCSignal)
#define OnOff	(AuthenticationBase->OnOff)
/*
**  Internal Prototypes
*/
ULONG LIBENT AddGroup(register __a0 struct Group *,
		     register __a1 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT AddToGroup(register __a0 struct Group *,
		       register __a1 struct UserProfile *,
		       register __a2 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT AddUser(register __a0 struct UserProfile *,
		    register __a1 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
struct UserProfile * LIBENT AllocUserProfile(register __a6 struct AuthenticationBase
				     *AuthenticationBase);
struct UserProfile * LIBENT AuthenticateUser(register __a0 UBYTE *,
					 register __a1 UBYTE *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT ChangePassWord(register __a0 struct UserProfile *,
			 register __a1 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT ChooseServer(register __a0 UBYTE *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT DeleteGroup(register __a0 struct Group *,
			register __a1 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT DeleteUser(register __a0 struct UserProfile *,
		       register __a1 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
BOOL LIBENT AuthErrorText(register __d0 ULONG,
		      register __a0 UBYTE *,
		      register __d1 ULONG,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT FindGroupID(register __a0 struct Group *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT FindGroupName(register __a0 struct Group *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT FindUserName(register __a0 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
VOID LIBENT FreeUserProfile(register __a0 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT ListGroups(register __a0 struct MinList *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT ListMembers(register __a0 struct MinList *,
			register __a1 struct Group *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT ListUsers(register __a0 struct MinList *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG LIBENT RemoveFromGroup(register __a0 struct Group *,
			    register __a1 struct UserProfile *,
			    register __a2 struct UserProfile *,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
BOOL  LIBENT MemberOf(register __a0 struct Group *group,
		    register __a1 struct UserProfile *user,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
struct Group * LIBENT AllocGroup(register __a6 struct AuthenticationBase
				     *AuthenticationBase);
VOID LIBENT FreeGroup(register __a0 struct Group *foo,
		register __a6 struct AuthenticationBase *AuthenticationBase);
VOID LIBENT NoSecurity(register __d0 BOOL onoff,
	     register __a6 struct AuthenticationBase *AuthenticationBase);
BOOL __asm CustomLibOpen(register __a0 struct AuthenticationBase *AuthenticationBase);
VOID __asm CustomLibClose(register __a0 struct AuthenticationBase *AuthenticationBase);
ULONG __asm doCmd(register __a0 struct UserProfile *object,
		register __a1 struct UserProfile *authority,
		register __d0 ULONG cmd,
		register __a2 struct Group *group,
		register __a6 struct AuthenticationBase *AuthenticationBase);
ULONG __asm doList(register __a0 struct MinList *list,
		register __d0 ULONG cmd,
		register __a1 struct Group *group,
		register __a6 struct AuthenticationBase *AuthenticationBase);
APTR *MyCreateEntity(struct AuthenticationBase *AuthenticationBase, Tag tag1, ... );
struct Transaction *MyAllocTransaction(struct AuthenticationBase *AuthenticationBase, Tag tag1, ... );
//#include <pragmas/authentication_pragmas.h>
#endif AUTHLIB_INTERNAL_H