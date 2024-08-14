#ifndef AUTH_INTERNAL_H
#define AUTH_INTERNAL_H
/*
**  $ID:$
**
**	Authentication.library INTERNAL versions of public structures and other
**	internal includes.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include <exec/exec.h>
#include <utility/tagitem.h>
#include <envoy/errors.h>
#include <envoy/nipc.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/nipc_protos.h>
#include <pragmas/nipc_pragmas.h>
#include <string.h>

#define EVER ;;
/*
**  Group information structure.
*/
#define GROUP_DSIZE   (sizeof(struct Group) - sizeof(struct MinNode))
#define G_GNLENGTH	16
#define G_MAXUMEMBERS	50
#define G_MAXGMEMBERS	5
struct MinGroup
{
	struct MinNode g_Node;
	UBYTE g_GroupName[G_GNLENGTH];
	WORD  g_GID;
};
struct Group
{
	struct MinNode g_Node;
	UBYTE g_GroupName[G_GNLENGTH];
	WORD  g_GID;
/**** Private, needs to be removed from public header!!! ****/
	UWORD g_UserMembers[G_MAXUMEMBERS];
	WORD  g_GroupMembers[G_MAXGMEMBERS];
};
/*
**  Defined groups
*/
#define GROUP_ANY	0
#define GROUP_NONE	1
#define GROUP_ADMIN	2
#define GROUP_ADDGROUP	3
/*
**  Internal version of user information structure:
**
**    -	MinUserProfile doesn't include space for an authority profile.
**
**    -	UserProfile is the whole sheebang.
**
**    -	We could have a NetUserProfile that doesn't include the MinNode -- it
**	would be all we actually send over the net. 8 extra bytes doesn't
** 	justify the complexity right now.
**
*/
#define PROFILE_DSIZE (sizeof(struct MinUserProfile) - sizeof(struct MinNode))
#define UP_UNLENGTH	16
#define UP_FNLENGTH	64
#define UP_PWLENGTH	16
struct MinUserProfile
{
	struct MinNode up_Node;
	UBYTE up_UserName[UP_UNLENGTH];
	UBYTE up_FullName[UP_FNLENGTH];
	UBYTE up_PassWord[UP_PWLENGTH];  /* this is never written to by the library */
	UWORD up_UID;
	WORD  up_GID;  /* Sign bit of GID is used for modify group permission */
};
struct UserProfile
{
	struct MinNode up_Node;
	UBYTE up_UserName[UP_UNLENGTH];
	UBYTE up_FullName[UP_FNLENGTH];
	UBYTE up_PassWord[UP_PWLENGTH];  /* this is never written to by the library */
	UWORD up_UID;
	WORD  up_GID;  /* array of up to 64k GIDs, NOGROUP terminated */
		     /* The sign bit of GID is used for modify group
		        permission */
/************** The above is public  *********************************/
	UBYTE Authority[sizeof(struct MinUserProfile)];  /* store authority here in request */
		/*  This eats extra memory, but is much more efficient  */
	UBYTE Group[sizeof(struct Group)];  /* store group a group here in request */
		/*  This eats extra memory, but is much more efficient  */
};
/*
**  Actions (commands)
*/
#define AUTHENTICATE_USER 	0
#define ADD_USER		1
#define DELETE_USER		2
#define CHANGE_PASS_WORD	3
#define ADD_GROUP		4
#define DELETE_GROUP		5
#define ADD_TO_GROUP		6
#define REMOVE_FROM_GROUP	7
#define FIND_USER_NAME		8
#define FIND_GROUP_NAME		9
#define FIND_GROUP_ID		10
#define LIST_GROUPS		11
#define LIST_USERS		12
#define LIST_MEMBERS		13
#define MEMBER_OF		14
#define CMD_LIMIT		14
/*
**  Misc #defines:
*/
#if DEBUG > 0
	void KPrintF(char *,...);
	#define BUG(x) KPrintF x
#else
	#define BUG(x)
	#endif DEBUG > 0
#endif AUTH_INTERNAL_H