#ifndef  CLIB_AUTHENTICATION_PROTOS_H
#define  CLIB_AUTHENTICATION_PROTOS_H
/*
**	$Id: authentication_protos.h,v 1.2 92/04/09 11:59:09 dlarson Exp Locker: dlarson $
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

BOOL UD_ErrorText( unsigned long code, UBYTE *buffer, unsigned long length );

ULONG GroupIDToName( UWORD gid, STRPTR name);

ULONG NameToGroupID( STRPTR name, WORD *gid);

ULONG UserIDToName( UWORD uid, STRPTR name);

ULONG NameToUserID( STRPTR name, WORD *gid);

struct MinList *UD_CreateList( ULONG listtype );

VOID UD_DeleteList( struct MinList *list, ULONG listtype );

BOOL MemberOfGroup( struct Group *group, struct User *user );

UWORD VerifyUser( STRPTR name, STRPTR password );

#endif   /* CLIB_AUTHENTICATION_PROTOS_H */
