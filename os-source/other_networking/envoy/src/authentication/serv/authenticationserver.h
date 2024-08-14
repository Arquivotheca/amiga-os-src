/*
**  $Id: authenticationserver.h,v 1.4 92/04/09 12:02:50 dlarson Exp Locker: dlarson $
**
**  Authentication server common include file.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/

#include "/auth_internal.h"
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/alib_protos.h>
//#include <utility/utility.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *NIPCBase;
extern struct Library *UtilityBase;
/*
**  Command Functions:
*/
ULONG AuthenticateUser(struct Transaction *trans);
ULONG AddUser(struct Transaction *trans);
ULONG DeleteUser(struct Transaction *trans);
ULONG ChangePassWord(struct Transaction *trans);
ULONG AddGroup(struct Transaction *trans);
ULONG DeleteGroup(struct Transaction *trans);
ULONG AddToGroup(struct Transaction *trans);
ULONG RemoveFromGroup(struct Transaction *trans);
ULONG FindUserName(struct Transaction *trans);
ULONG FindGroupName(struct Transaction *trans);
ULONG FindGroupID(struct Transaction *trans);
ULONG ListGroups(struct Transaction *trans);
ULONG ListUsers(struct Transaction *trans);
ULONG ListMembers(struct Transaction *trans);
ULONG MemberOf(struct Transaction *trans);
/*
**  User DataBase Functions:
*/
BOOL InitUserDB(VOID);
VOID FreeUserDB(VOID);
BOOL WriteUser(struct UserProfile *user);
BOOL ReadUser(struct UserProfile *user);
BOOL ReadUID(struct UserProfile *user);
BOOL RemoveUser(struct UserProfile *user);
struct MinList *GetUserList(VOID);
/*
**  Group DataBase Functions:
*/
BOOL InitGroupDB(VOID);
VOID FreeGroupDB(VOID);
BOOL WriteGroup(struct Group *Group);
BOOL ReadGroup(struct Group *Group);
BOOL ReadGroupID(struct Group *group);
BOOL RemoveGroup(struct Group *Group);
struct MinList *GetGroupList(VOID);
