/*	scan directory for filenames matching wildcard
 *  (c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 *	scdir() must be called until it returns NULL
 *	or else scdir_abort() must be called to free up resources
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dosasl.h>
#include <clib/dos_protos.h>
#include <string.h>

extern struct Library *SysBase;
static struct AnchorPath AnPath;
static char *oldwild = NULL;
static LONG failure = 0;
static char *result = NULL;

#include <stdlib.h>

void
scdir_abort( void )
{
	if( result ) {
		free( result ); result = NULL;
	}
	if( oldwild ) {
		free( oldwild ); oldwild = NULL;
		if( !failure && SysBase->lib_Version >= 36L )
			MatchEnd( &AnPath );
		failure = -1;
	}
}

#if 0
/* uncomment this routine if extern basename() does not return a
 * pointer in the same string as pathname
 */
static char *
basename( char *pathname )
{
	char *ptr, *filename;

	for( filename = ptr = pathname; *ptr; ptr++ )
		if( *ptr == '/'		/* directory delimiter */
			|| *ptr == ':'	/* device delimiter */
			) filename = ptr+1;
	return( filename );
}
#else
extern char *basename( char *);
#endif

static char *
scdir_result( char *wild, char *name )
{
	long size;

	if( result ) {
		free( result ); result = NULL;
	}
	if( name ) {
		size = strlen( wild ) + strlen( name ) + 1;
		if( result = (char *)malloc( size )) {
			strcpy( result,	wild );
			strcpy( basename( result ), name );
			return( result );
		}
	   	if( SysBase->lib_Version >= 36L )
			MatchEnd( &AnPath );
	}
	return( NULL );
}

/*
*	NAME
*		scdir -- scans a directory for filenames matching Amiga wildcard
*
*	SYNOPSIS
*		filename = scdir( wildcard )
*
*		char *scdir( const char *scdir )
*
*	FUNCTION
*		This function returns a single filename that matches the AmigaDOS
*		wildcard.  Consecutive calls with the same wildcard will result
*		in the next filename that matches the wildcard.  This function
*		should be called with the same wildcard until all matching
*		filenames are exhausted, at which time NULL is returned.
*
*	INPUTS
*		wildcard - the full pathname with wildcard to be matched
*
*	RESULT
*
*		filename - the next filename matching the wildcard or
*			NULL if there are no more matches found or
*			NULL if an error occurred
*
*   BUGS
*		Resources are not properly freed if scdir_abort() is never called
*		or scdir() is not called until a NULL is returned.  That is why
*		if scdir() is used, the caller should perform an
*		atexit(scdir_abort), to ensure that resources are always freed.
*
*   SEE ALSO
*		scdir_abort()
*
*/

char *
scdir( const char *wild )
{
	int diff = 0;

	if( oldwild ) {
		diff = strcmp( oldwild, wild );
		if( !diff && failure ) return( scdir_result( wild,  NULL ));
	}
	if( SysBase->lib_Version < 36L ) {
		BPTR lock;

		lock = Lock( wild, MODE_OLDFILE );
		if( lock ) UnLock( lock );
		if( oldwild ) free( oldwild );
		oldwild = strdup( wild );
		if( result ) free( result );
		result = strdup( wild );
		failure = -1;
		return( result );
	}
	if( !oldwild || diff ) {
		if( oldwild ) free( oldwild );
		oldwild = strdup( wild );

		AnPath.ap_BreakBits = SIGBREAKF_CTRL_C; /* Break on these bits	*/

		failure = MatchFirst( wild, &AnPath);

		if( !failure && AnPath.ap_Info.fib_DirEntryType <= 0 ) {
    		return( scdir_result( wild, AnPath.ap_Info.fib_FileName ));
		}
	}

	/* same wildcard as before */
	while( !failure ) {
		if( failure = MatchNext( &AnPath )) break;
		if( AnPath.ap_Info.fib_DirEntryType <= 0 ) {
    		return( scdir_result( wild, AnPath.ap_Info.fib_FileName ));
		}
	}
	/* failure */
    /* This absolutely, positively must be called, all of the time. */
   	MatchEnd( &AnPath );
	return( scdir_result( wild, NULL ));
}

