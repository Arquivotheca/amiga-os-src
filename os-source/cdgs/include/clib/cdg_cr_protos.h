#ifndef  CLIB_CDG_CR_PROTOS_H
#define  CLIB_CDG_CR_PROTOS_H

/*
**	$Id: cdg_cr_protos.h,v 1.2 93/04/15 08:51:03 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  LIBRARIES_CDGPREFS_H
#include <libraries/cdgprefs.h>
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
