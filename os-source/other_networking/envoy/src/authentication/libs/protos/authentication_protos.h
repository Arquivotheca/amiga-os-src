#ifndef  CLIB_AUTHENTICATION_PROTOS_H
#define  CLIB_AUTHENTICATION_PROTOS_H
/*
**	$Id: authentication_protos.h,v 1.3 92/06/24 13:30:56 dlarson Exp Locker: dlarson $
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
#ifndef  ENVOY_AUTHENTICATION_H
#include <envoy/authentication.h>
#endif
/*--- functions in V37 or higher (distributed as Release 2.04) ---*/
ULONG AddGroup( struct Group *group, struct UserProfile *authority );
ULONG AddToGroup( struct Group *group, struct UserProfile *user,
	struct UserProfile *authority );
ULONG AddUser( struct UserProfile *newuser, struct UserProfile *authority );
struct UserProfile *AllocUserProfile( void );
struct UserProfile *AuthenticateUser( UBYTE *username, UBYTE *password );
struct Group *AllocGroup( void );
void FreeGroup( struct Group * );
ULONG ChangePassWord( struct UserProfile *user,
	struct UserProfile *authority );
ULONG ChooseAuthServer( UBYTE *hostname );
ULONG DeleteGroup( struct Group *group, struct UserProfile *authority );
ULONG DeleteUser( struct UserProfile *user, struct UserProfile *authority );
BOOL AuthErrorText( unsigned long code, UBYTE *buffer, unsigned long length );
ULONG FindGroupID( struct Group *group );
ULONG FindGroupName( struct Group *group );
ULONG FindUserName( struct UserProfile *user );
void FreeUserProfile( struct UserProfile *profile );
ULONG ListGroups( struct MinList *groups );
ULONG ListMembers( struct MinList *members, struct Group *groups );
ULONG ListUsers( struct MinList *users );
ULONG RemoveFromGroup( struct Group *group, struct UserProfile *user,
	struct UserProfile *authority );
BOOL MemberOf( struct Group *group, struct UserProfile *user );
void NoSecurity( long onoff );
#endif   /* CLIB_AUTHENTICATION_PROTOS_H */
