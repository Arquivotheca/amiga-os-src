#ifndef  CLIB_REALTIME_PROTOS_H
#define  CLIB_REALTIME_PROTOS_H

/*
**	$Id: realtime_protos.h,v 40.1 93/03/16 11:12:57 vertex Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  LIBRARIES_REALTIME_H
#include <libraries/realtime.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
/*--- functions in V37 or higher (Release 2.04) ---*/

/* Locks */

APTR LockRealTime( unsigned long lockType );
void UnlockRealTime( APTR lock );

/* Conductor */

struct Player *CreatePlayerA( struct TagItem *tagList );
struct Player *CreatePlayer( Tag tag1, ... );
void DeletePlayer( struct Player *player );
BOOL SetPlayerAttrsA( struct Player *player, struct TagItem *tagList );
BOOL SetPlayerAttrs( struct Player *player, Tag tag1, ... );
LONG SetConductorState( struct Player *player, unsigned long state,
	long time );
BOOL ExternalSync( struct Player *player, long minTime, long maxTime );
struct Conductor *NextConductor( struct Conductor *previousConductor );
struct Conductor *FindConductor( STRPTR name );
ULONG GetPlayerAttrsA( struct Player *player, struct TagItem *tagList );
ULONG GetPlayerAttrs( struct Player *player, Tag tag1, ... );
#endif   /* CLIB_REALTIME_PROTOS_H */
