#ifndef  CLIB_ACCOUNTS_PROTOS_H
#define  CLIB_ACCOUNTS_PROTOS_H
/*
**	$Id: accounts_protos.h,v 1.2 92/10/13 11:20:43 kcd Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  ENVOY_ACCOUNTS_H
#include <envoy/accounts.h>
#endif
/*--- functions in V37 or higher (Release 2.04) ---*/
/* User and Group structure Allocation/Deallocation */
struct UserInfo *AllocUserInfo( void );
struct GroupInfo *AllocGroupInfo( void );
void FreeUserInfo( struct UserInfo * );
void FreeGroupInfo( struct GroupInfo * );
/* User Verification functions * */
ULONG VerifyUser( STRPTR userName, STRPTR password, struct UserInfo *user );
ULONG MemberOf( struct GroupInfo *group, struct UserInfo *user );
/* Functions to find users/groups or build lists of users/groups/members * */
ULONG NameToUser( STRPTR userName, struct UserInfo *user );
ULONG NameToGroup( STRPTR groupName, struct GroupInfo *group );
ULONG IDToUser( unsigned long userID, struct UserInfo *user );
ULONG IDToGroup( unsigned long groupID, struct GroupInfo *group );
ULONG NextUser( struct UserInfo *user );
ULONG NextGroup( struct GroupInfo *group );
ULONG NextMember( struct GroupInfo *group, struct UserInfo *user );
#endif   /* CLIB_ACCOUNTS_PROTOS_H */
