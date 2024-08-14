#ifndef ENVOY_AUTHENTICATION_H
#define ENVOY_AUTHENTICATION_H
/*
**  $Id:$
**
**	Authentication.library public structures.
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**		All rights reserved.
*/
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif EXEC_EXEC_H
#ifndef ENVOY_ERRORS_H
#include <envoy/errors.h>
#endif ENVOY_ERRORS_H
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
struct UserProfile
{
	struct MinNode up_Node;
	UBYTE up_UserName[UP_UNLENGTH];
	UBYTE up_FullName[UP_FNLENGTH];
	UBYTE up_PassWord[UP_PWLENGTH];  /* never written to by the library */
	UWORD up_UID;
};
/*
**  Group information structure.
**
**  DO NOT examine, modify or otherwise rely on non-public fields of this
**  structure!!!
*/
struct Group
{
	struct MinNode g_Node;
	UBYTE g_GroupName[G_GNLENGTH];
	UWORD  g_GID;
};
/*
**  Defined groups
*/
#define GROUP_NONE	0
#define GROUP_ADMIN	1

#endif ENVOY_AUTHENTICATION_H