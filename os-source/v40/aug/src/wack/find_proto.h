/*
 * Amiga Grand Wack
 *
 * $Id: find_proto.h,v 39.3 93/08/23 19:32:17 peter Exp $
 *
 */

void FindHelp( void );

STRPTR Limit( UBYTE *arg );

STRPTR StackLimit( UBYTE *arg );

STRPTR Find( char *arg );

BOOL match( ULONG *p, ULONG pat, WORD size );

void find( UBYTE *start, UBYTE *limit, ULONG pat, WORD size, ULONG initial );
