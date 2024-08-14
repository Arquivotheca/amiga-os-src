/*	expand.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 *	macro expansion
 */

#include <clib/exec_protos.h>

#include <ctype.h>

#include "make.h"
#include "depend.h"

static int expand_macros2( char *dest, char *src, int maxlen );
static int expand_macros3( char *src, int maxlen );

/*	define the criterion for detecting infinitely recursive
 *	macro expansions
 */
static int expand_level = 0;
#define MAX_RECURSION	32
#define MAX_ITERATION	256

static struct macro *
expand_fncall( char *src, int maxlen )
{
	static struct macro *fnmac = NULL;
	struct fncall *fc;
	char fn[ 40 ], *next = src;
	int addlen;

	if( fnmac) {
		if( fnmac->expansion ) {
			free( fnmac->expansion );
			fnmac->expansion = NULL;
		}
	}
	else if( fnmac = new_macro( NULL, NULL ))
		fnmac->flags |= MF_SIMPLE;
	else return( NULL );

	next = parse_str( fn, next, sizeof(fn));
	if( fc = find_fncall( fn )) {
		if( (*fc->call)( fnmac, next )) {
			logprintf( "function call %s returned ERROR\n", fc->name );
			return( NULL );
		}
		fnmac->flags |= MF_SIMPLE;
		return( fnmac );
	}
	return( NULL );
}

/*	expand a macro reference with only a single $(x) instance
 *	handles recursive expansion
 */
static int
expand_macros3( char *src, int maxlen )
{
	char *dest = NULL;
	char *macroname = NULL;
	char *dollar;

	dollar = strchr( src, '$' );
	if( dollar ) {
		struct macro *mac = NULL;
		char *out, *next;
	 	int outlen = 0;
		char delimchar = (char)0;

		debugprintf( 6,( "expand_macros3(%s,%d)\n", src, maxlen ));

		/* could get away with only allocating maxlen to save memory */
		macroname = (char *)malloc( Param.MaxLine );
		dest = (char *)calloc( Param.MaxLine, 1 );
		if( !macroname || !dest )
			goto death;

		out = dest;

		memset( macroname, 0, Param.MaxLine);
		for( next = src; next < dollar && outlen < maxlen; outlen++ )
			*out++ = *next++;

		next = dollar + 1;
		if( outlen >= maxlen )
			goto death;

		switch( *next ) {
		case '(':
			delimchar = ')';
			break;
		case '{':
			delimchar = '}';
			break;
		}

		if( delimchar ) { /* multi letter variable name */
			char *n;
			int i = 0;

			for( n = ++next; *n && i < Param.MaxLine-1; n++ ) {
				if( *n == delimchar ) break;
				macroname[ i++ ] = *n;
			}
			if( *n != delimchar ) {
				logprintf( "macro name is too long [%s]\n", macroname );
				logprintf( "macro names are limited to %d\n",
					Param.MaxLine );
				goto death;
			}
			next = n + 1;
		}
		else { /* single letter variable name */
			macroname[ 0 ] = *next++; /* advance past the macroname */
		}

		if( mac = expand_fncall( macroname, maxlen - outlen )) {
			debugprintf( 4,( "fncall macro [%s] = %s\n", macroname,
				mac->expansion ));
		}
		else if( !(mac = find_macro( macroname )) && getenv( macroname )) {
			/* use getenv() to assign a simple macro */
			debugprintf( 4,( "getenv macro [%s]\n", macroname ));
			if( mac = set_macro( macroname, getenv( macroname ))) {
				mac->flags |= MF_SIMPLE;
			}
		}
		if( mac ) {
			int cdrlen = maxlen - outlen;
			if( mac->flags & MF_EXPANDED ) {
				logprintf( "infinitely recursive macro expansion: %s\n",
					macroname );
				goto death;
			}
			memset( out, 0, cdrlen );
			if( mac->expansion )
				strncpy( out, mac->expansion, cdrlen );
			cdrlen -= strlen( out );
			if( cdrlen < 0 ) {
				logprintf( "expand_macros3 ERROR: cdrlen is %d\n",
					cdrlen );
				goto death;
			}
			strncpy( out + strlen(out), next, cdrlen );
			if( !(mac->flags & MF_SIMPLE )) {
				/* recursively expand nested macro expansion */
				mac->flags |= MF_EXPANDED;
				if( expand_macros2( out, out, cdrlen )) goto death;
				mac->flags &= ~MF_EXPANDED;
			}
		}
		else {
			logprintf( "WARNING:  unknown macro [%s]\n", macroname );
			strncpy( out + strlen(out), next, maxlen - outlen );
		}
		/* for next macro occurrence on the line */
		outlen = min( strlen( dest ), maxlen - 1 );

		debugprintf( 6,( "expand_macros3 returns [%s] %d\n", dest, outlen ));

		strncpy( src, dest, outlen ); /* copy the expansion back */
		src[ outlen  ] = (char)0;
		free( macroname ); macroname = NULL;
		free( dest ); dest = NULL;
	} /* dollar */

	return( 0 );
death:
	if( macroname )
		free( macroname );
	if( dest )
		free( dest );
	return( 1 );
}

/*	find the rightmost occurrence of the character tok
 *	starting at prev in the string
 */
static char *
strrlchr( char *string, char tok, char *prev )
{
	while( prev >= string ) {
		if( *prev == tok ) return( prev );
		prev--;
	}
	return( NULL );
}

/*	expand a line with multiple $(x) instances
 *	expand right to left to avoid recursion
 */
static int
expand_macros2( char *dest, char *src, int maxlen )
{
	char *dollar, *prev;
	int iteration = 0;

	/* increment the recursion level counter */
	if( ++expand_level > MAX_RECURSION ){
		logprintf( "Infinite Macro expansion aborted at %d recursions\n",
			expand_level );
		return( 1 );
	}
	debugprintf( 6,( "expand_macros2(%s,%d)\n", src, maxlen ));
	if( src != dest ) strncpy( dest, src, maxlen );
	prev = dest + strlen(dest);
	while( dollar = strrlchr( dest, '$', prev )) {
		if( ++iteration > MAX_ITERATION ) {
			logprintf( "Infinite Macro expansion aborted at %d iterations\n",
				iteration );
			return( 1 );
		}
		if( dollar[-1] == '$' || dollar[-1] == '\\' ) {
			shift_string_left( dollar - 1, 1 );
			--dollar;
		}
		else if( expand_macros3( dollar, maxlen - (int)(dollar - dest) ))
			return( 1 );
		prev = dollar - 1;
	}
	debugprintf( 6,( "expand_macros2 returns [%s]\n", dest ));
	return( 0 );
}

/*	to reset each variable's flags before expansion begins */
static long
reset_macroflag( struct macro *mac )
{
	mac->flags &= ~MF_EXPANDED;
	return( 0 );
}

/*	top level macro expansion call
 *	this is the entry point called from the outside
 */
int
expand_macros( char *dest, char *src, int maxlen )
{
	expand_level = 0;	/* reset the recursion level counter */
	memset( dest, 0, maxlen );
	for_list( &Global.macrolist, reset_macroflag );
	if( expand_macros2( dest, src, maxlen - 1 )) {
		logprintf( "Error expanding $(%s)\n", src );
		return( 1 );
	}

	debugprintf( 3,( "expand_macros [%s] to [%s]\n", src, dest ));
	return( 0 );
}
