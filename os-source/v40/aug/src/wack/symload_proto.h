/*
 * Amiga Grand Wack
 *
 * $Id: symload_proto.h,v 39.1 93/07/16 18:25:40 peter Exp $
 *
 */

STRPTR bindHunks( char *arg );

STRPTR showHunks( char *arg );

STRPTR bindSymbols( char *arg );

ULONG hunkFinder( ULONG option, STRPTR taskname, ULONG *table, ULONG *nummodules, ULONG *numhunks );

