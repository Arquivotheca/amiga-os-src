/*	cond.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <ctype.h>
#include <clib/exec_protos.h>

#include <fcntl.h>

#include "make.h"
#include "depend.h"
#include "cond.h"

/*	binary search to find the function */
struct drctvs *
find_drctvs( struct drctvs *array, int array_size, char *name )
{
	int first = 0;
	int last = array_size - 1;
	int mid;
	int diff, len;

	/* binary search */
	while( first <= last ) {
		mid = (first+last) / 2;
		len = strlen( array[ mid ].directive );
		diff = strnicmp( name, array[ mid ].directive, len );
		if( !diff )
			return( &array[ mid ] ); /* found */
		if( first == last )
			break; /* not found */
		if( diff < 0 )
			last = mid - 1;
		else
			first = mid + 1;
	}
	return( NULL ); /* not found */
}

int
push_state( struct List *stack, long state )
{
	struct mstate *new = (struct mstate *)malloc( sizeof(struct mstate));

	if( !new )
		return( 1 ); /* no memory */
	new->node.ln_Type = NT_USER;
	new->state = state;
	AddHead( stack, &new->node );
	return( 0 );
}

int
pop_state( struct List *stack )
{
	struct mstate *new = (struct mstate *)RemHead( stack );
	int state;

	if( !new )
		return( 0 ); /* no state */
	state = new->state;
	free( new );
	return( state );
}

void
clear_stack( struct List *stack )
{
	struct Node *node, *succ;

	for( node = stack->lh_Head; node->ln_Succ; node = succ ) {
		succ = node->ln_Succ;
		free( node );
	}
	NewList( stack );
}

/*	returns true if we are still within a conditional */
int
get_directive_state( struct List *stack )
{
	struct mstate *first = (struct mstate *)stack->lh_Head;
	if( !first->node.ln_Succ )
		return( 0 );
	return( first->state );
}

/* conditions return 0 for false, 1 for true, -1 for error */
int
do_condeq( char *string, int negate )
{
	char *exp1 = NULL, *exp2 = NULL;
	char *lparen = string, *comma, *rparen;
	int retval = -1; /* default to error */

	exp1 = (char *)malloc( Param.MaxLine );
	exp2 = (char *)malloc( Param.MaxLine );
	if( !exp1 || !exp2 )
		goto death;

	while( *lparen && *lparen != '(' )
		lparen++;
	if( *lparen != '(' )
		goto death;
	comma = lparen + 1;
	while( *comma && *comma != ',' )
		comma++;
	if( *comma != ',' )
		goto death;
	rparen = comma + 1;
	while( *rparen && *rparen != ')' )
		rparen++;
	if( *rparen != ')' )
		goto death;

	*comma = *rparen = (char)0;
	if( expand_macros( exp1, lparen + 1, Param.MaxLine ) ||
		expand_macros( exp2, comma + 1, Param.MaxLine ))
		goto death;

	if( negate )
		retval = stricmp( exp1, exp2 ) ? 1 : 0; /* set condition code */
	else
		retval = stricmp( exp1, exp2 ) ? 0 : 1; /* set condition code */

death:
	if( exp1 )
		free( exp1 );
	if( exp2 )
		free( exp2 );
	return( retval );
}

int
do_conddef( char *string, int negate )
{
	char *lparen = string, *rparen;
	int retval = -1; /* default to error */

	while( *lparen && *lparen != '(' )
		lparen++;
	if( *lparen != '(' )
		goto death;
	rparen = lparen + 1;
	while( *rparen && *rparen != ')' )
		rparen++;
	if( *rparen != ')' )
		goto death;

	*rparen = (char)0;
	retval = find_macro( lparen + 1 ) ? 1 : 0;
	if( negate )
		retval = retval ? 0 : 1;
death:
	return( retval );
}

int
do_condexists( char *string, int negate )
{
	char *lparen = string, *rparen;
	int retval = -1; /* default to error */

	while( *lparen && *lparen != '(' )
		lparen++;
	if( *lparen != '(' )
		goto death;
	rparen = lparen + 1;
	while( *rparen && *rparen != ')' )
		rparen++;
	if( *rparen != ')' )
		goto death;

	*rparen = (char)0;
	retval = access( lparen + 1, 0 ) ? 0 : 1;
	if( negate )
		retval = retval ? 0 : 1;
death:
	return( retval );
}

int
cdrctv_eq( char *string )
{
	return( do_condeq( string, 0 ));
}

int
cdrctv_neq( char *string )
{
	return( do_condeq( string, 1 ));
}

int
cdrctv_def( char *string )
{
	return( do_conddef( string, 0 ));
}

int
cdrctv_ndef( char *string )
{
	return( do_conddef( string, 1 ));
}

int
cdrctv_exists( char *string )
{
	return( do_condexists( string, 0 ));
}

int
cdrctv_nexists( char *string )
{
	return( do_condexists( string, 1 ));
}

/* keep it sorted for binary search */
struct drctvs carray[ MAX_CDRCTVS ] = {
	{ "def", 	cdrctv_def	},
	{ "eq",  	cdrctv_eq	},
	{ "exists",	cdrctv_exists },
	{ "ndef",	cdrctv_ndef	},
	{ "neq", 	cdrctv_neq	},
	{ "nexists",	cdrctv_nexists }
};

int
drctv_else( char *string, struct List *stack )
{
	int state = pop_state( stack );
	int newstate = 0;

	if( state != STATE_IF_T && state != STATE_IF_F ) {
		logfile( "ERROR:  else with no matching conditional\n" );
		return( 1 );
	}
	newstate = (state == STATE_IF_T) ? STATE_EL_T : STATE_EL_F;
	debugprintf( 4, ("else changes state from %d to %d\n",
		state, newstate ));
	push_state( stack, newstate );
	return( 0 );
}

int
drctv_endif( char *string, struct List *stack )
{
	int state = pop_state( stack );

	if( !state ) {
		logfile( "ERROR:  endif with no matching conditional\n" );
		return( 1 );
	}
	return( 0 );
}

int
drctv_if( char *string, struct List *stack )
{
	struct drctvs *cdrctv;
	int condition;

	while( isspace( *string )) string++;
	if( !*string ) {
		logfile( "No condition given\n" );
		return( 1 );
	}

	if( cdrctv = find_drctvs( carray, MAX_CDRCTVS, string )) {
		debugprintf( 4, ("condition %s\n", string ));
		condition = (*cdrctv->call)( string + strlen( cdrctv->directive ));
		debugprintf( 4, ("condition returns %d\n", condition ));
		if( condition < 0 ) {
			logprintf( "Error in condition: %s\n", string );
			return( 1 );
		}
		else {
			push_state( stack, ( condition ? STATE_IF_T : STATE_IF_F ));
			return( 0 );
		}
	}
	logprintf( "Unrecognized condition: %s\n", string );
	return( 1 );
}
