/*
 * Amiga Grand Wack
 *
 * $Id: showlock_proto.h,v 39.3 93/06/02 15:51:39 peter Exp $
 *
 */

void semHeader( void );

ULONG dumpSemReq( APTR addr, struct SemaphoreRequest *sr );

ULONG dumpSem( APTR addr, struct SignalSemaphore *sem, char *name );

STRPTR ShowSem( char *arg );

void ShowSemNoHeader( APTR addr );

void ShowSemListNoHeader( APTR addr );

STRPTR ShowSemList( char *arg );

