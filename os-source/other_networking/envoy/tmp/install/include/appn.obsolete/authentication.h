#ifndef APPN_AUTHENTICATION_H
#define APPN_AUTHENTICATION_H
/*
**  $ID:$
**
**	Authentication.library public structures.
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**		All rights reserved.
*/
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif EXEC_EXEC_H
#ifndef APPN_ERRORS_H
#include <appn/errors.h>
#endif APPN_ERRORS_H
/*
**  Public part of User information structure
**
**  DO NOT examine, modify or otherwise rely on non-public fields of this
**  structure!!!
*/
#define PROFILE_DSIZE (sizeof(struct UserProfile) - sizeof(struct MinNode))
#define GROUP_DSIZE   (sizeof(struct Group) - sizeof(struct MinNode))
#define UP_UNLENGTH	16
#define UP_FNLENGTH	64
#define UP_PWLENGTH	16
#define G_GNLENGTH	16
#define G_MAXUMEMBERS	50
#define G_MAXGMEMBERS	5
struct UserProfile
{
	struct MinNode up_Node;
	UBYTE up_UserName[UP_UNLENGTH];
	UBYTE up_FullName[UP_FNLENGTH];
	UBYTE up_PassWord[UP_PWLENGTH];  /* this is never written to by the library */
	UWORD up_UID;
	WORD  up_GID;  /* Sign bit of GID is used for modify group permission */
};
/*
**  Group information structure.
*/
struct Group
{
	struct MinNode g_Node;
	UBYTE g_GroupName[G_GNLENGTH];
	WORD  g_GID;
};
/*
**  Defined groups
*/
#define GROUP_ANY	0
#define GROUP_NONE	1
#define GROUP_ADMIN	2
#define GROUP_ADDGROUP	3

#endif APPN_AUTHENTICATION_H