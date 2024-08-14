/*	input.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <ctype.h>
#include <clib/exec_protos.h>

#include "make.h"
#include "depend.h"
#include "cond.h"

int line_number;

struct List Mstack =
{
	(struct Node *)&Mstack.lh_Tail,	/* lh_Head */
	(struct Node *)NULL,			/* lh_Tail */
	(struct Node *)&Mstack.lh_Head,	/* lh_TailPred */
	(UBYTE)NT_USER,
	(UBYTE)0
};

static int
drctv_include( char *string, struct List *stack )
{
	int retval = 0;
	int saved_line_number, state;
	while( isspace( *string )) string++;
	if( *string ) {
		saved_line_number = line_number;
		retval = input_makefile( string );
		line_number = saved_line_number;
	}
	return( 0 );	/* return success even if .include file not found */
}

static int
drctv_pragma( char *string, struct List *stack )
{
	int i, arguments, retval = 1;
	char **argv = NULL;

	while( isspace( *string )) string++;
	if( arguments = count_args( string ) + 1) {
		if( argv = (char **)malloc( (arguments + 1) * sizeof(char *)) ) {
			for( i = 1; i < arguments; i++ ) {
				argv[ i ] = find_word( string, i );
			}
			argv[ 0 ] = version_string + 7; /* kludge */
			argv[ arguments ] = NULL;
			retval = parse_parameters( arguments, argv );
			free( argv );
		}
	}
	debugprintf( 3,( "Pragma(%d,%s)\n", arguments, string ));
	return( retval );
}

static int
drctv_phony( char *string, struct List *stack )
{
	char name[ MAXPATHNAME ];
	char *expansion = NULL;
	char *nexttar = string;
	struct target *targ;

	if( !(expansion = (char *)malloc( Param.MaxLine )))
		goto death;

	while( isspace( *nexttar )) nexttar++;
	if( *nexttar != ':' ) {
		logfile( "WARNING: missing colon in .PHONY\n" );
	}
	else nexttar++;

	*expansion = (char)0;
	/* expand macros in rhs */
	if( *nexttar && expand_macros( expansion, nexttar, Param.MaxLine ))
		goto death;
	nexttar = expansion;

	for( ; ; ) {
		nexttar = parse_str( name, nexttar, MAXPATHNAME );
		if( !*name )
			break;
		if( targ = find_target( name )) {
			targ->flags |= TF_PHONY;
			debugprintf(4, ( "Marking %s as phony target\n", name ));
		}
		else {
			logprintf( "Unknown phony target %s\n", name );
			goto death;
		}
	}
	free( expansion );
	return( 0 );
death:
	if( expansion )
		free( expansion );
	return( 1 );
}

static int
drctv_suffixes( char *string, struct List *stack )
{
	char suf[ MAXSUFFIX ];
	char *expansion = NULL;
	char *nexttar = string;

	if( !(expansion = (char *)malloc( Param.MaxLine )))
		goto death;

	while( isspace( *nexttar ))
		nexttar++;
	if( *nexttar != ':' ) {
		logfile( "WARNING: missing colon in .SUFFIXES\n" );
	}
	else nexttar++;

	*expansion = (char)0;
	/* expand macros in rhs */
	if( *nexttar && expand_macros( expansion, nexttar, Param.MaxLine ))
		goto death;
	nexttar = expansion;

	for( ; ; ) {
		while( isspace( *nexttar )) nexttar++;
		if( *nexttar != '.' ) {
			if( *nexttar ) {
				logprintf( "bad suffix rule [%s] on line %d\n%s\n",
					nexttar, line_number, string );
			}
			break;
		}
		nexttar = parse_strtok( suf, ++nexttar, sizeof(suf)-1, isnotsuf );
		if( !*suf )
			break;
		add_suffix_targets( suf );
	}
	free( expansion );
	return( 0 );
death:
	if( expansion )
		free( expansion );
	return( 1 );
}

/* keep it sorted for binary search */
#define MAX_MDRCTVS 7
static struct drctvs darray[ MAX_MDRCTVS ] = {
	{ ".INCLUDE", drctv_include },
	{ ".PHONY",	drctv_phony },
	{ ".SUFFIXES", drctv_suffixes },
	{ "else",	drctv_else	},
	{ "endif",	drctv_endif	},
	{ "if",		drctv_if	},
	{ "pragma",	drctv_pragma }
};

/*	get a non-comment line
 */
static int
get_ncline( char *buf, int sz, FILE *in )
{
	char *inbuf, *cptr;
	int total, len;

	do {
		if( feof( in ))
			return( 1 );
		if( !fgets( buf, sz, in ))
			return( 1);
		line_number++;
		total = strlen( buf );
		inbuf = buf;
		while( cptr = strrchr( inbuf, '\\' ) ){
			if( cptr[1] != '\n' )
				break;
			total -= 2; /* subtract backslash newline */
			inbuf = cptr;
			if( total >= sz )
				break;
			if( feof( in ))
				return( 1 );
			if( !fgets( inbuf, sz - total, in ))
				return( 1 );
			line_number++;
			total += strlen( inbuf );
		}
	} while( *buf == '#' );
	strip_trailspace( buf );
	return( 0 );
}

/*	get the next valid line from a Makefile
 */
int
getline( char *buf, int sz, FILE *in )
{
	struct drctvs *found = NULL;
	int st, state = 0;
	do {
		if( state && !found )
			debugprintf( 4, ("skipped[state=%d] %s\n", state, buf ));
		st = get_ncline( buf, sz, in );
		if( !st ) {
			if( found = find_drctvs( darray, MAX_MDRCTVS, buf )) {
				state = get_directive_state( &Mstack );

				if( (void *)found->call != (void *)drctv_else &&
					(void *)found->call != (void *)drctv_endif &&
					(state == STATE_IF_F || state == STATE_EL_T ))
					continue;

				if( st = (*found->call)( buf + strlen( found->directive),
					&Mstack )) {
					clear_stack( &Mstack );
				}
			}
		}
		state = get_directive_state( &Mstack );
	} while( !st && (state == STATE_IF_F || state == STATE_EL_T || found ));

	return( st );
}

