#ifndef  CLIB_PLAYER_PROTOS_H
#define  CLIB_PLAYER_PROTOS_H
/*
**	$Id: player_protos.h,v 1.2 92/09/21 13:19:28 jerryh Exp Locker: jerryh $
**
**	C prototypes
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "player.library" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
UWORD GetOptions( struct PlayerOptions *options );
UWORD SetOptions( struct PlayerOptions *options );
UWORD GetPlayerState( struct PlayerState *dtate );
UWORD ModifyPlayList( unsigned long state );
struct PlayList *ObtainPlayList( void );
UWORD SubmitKeyStroke( unsigned long keystroke );
#endif   /* CLIB_PLAYER_PROTOS_H */
