#ifndef  CLIB_CDG_CR_PROTOS_H
#define  CLIB_CDG_CR_PROTOS_H
/*
**	$Id: cdg_cr_protos.h,v 1.1 92/04/21 11:12:15 darren Exp $
**
**	C prototypes
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "cdg.library" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  CDTV_CDGPREFS_H
#include <cdtv/cdgprefs.h>
#endif
BOOL CDGBegin( struct CDGPrefs *cdp );
void CDGEnd( void );
void CDGFront( void );
void CDGBack( void );
ULONG CDGDraw( unsigned long types );
void CDGChannel( unsigned long channel );
void CDGPause( void );
void CDGStop( void );
void CDGPlay( long show );
void CDGNextTrack( void );
void CDGPrevTrack( void );
void CDGFastForward( void );
void CDGRewind( void );
void CDGClearScreen( void );
void CDGDiskRemoved( void );
void CDGUserPack( APTR pack );
struct CDGPrefs *CDGAllocPrefs( void );
void CDGFreePrefs( struct CDGPrefs *cdp );
#endif   /* CLIB_CDG_CR_PROTOS_H */
