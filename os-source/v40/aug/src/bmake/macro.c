/*	macro.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 *	USER DEFINABLE variables
 *
 *	$x		= the variable x
 *	$(var)	= the variable var
 *	${var}	= the same variable var
 *
 *	INTERNAL AUTOMATIC variables (not for user definitions)
 *
 *	$$ = $
 *	$@ = target filename
 *	$* = basename of target
 *	$< = dependent filename
 *	$^ = all dependents of target newer than target (not-imp)
 *	$% = dependent member of the target archive (not-imp)
 *	$? = all dependents of the target archive (not-imp)
 */

#include <clib/exec_protos.h>

#include "make.h"
#include "depend.h"

/*	internal function used to find an existing variable */
struct target *
find_macro( char *macroname )
{
	struct macro *mac = NULL;

	for( struct macro *ln = Global.macrolist.lh_Head; ln->node.ln_Succ;
		ln = ln->node.ln_Succ ) {
		if( !strcmp( macroname, ln->name )) {
			mac = ln;
			break;
		}
	}

	return( mac );
}

/*	internal allocation function */
struct macro *
new_macro( char *name, char *expansion )
{
	long size = sizeof(struct macro);
	struct macro *new = (struct macro *)calloc( size, 1 );
	if( new ) {
		if( name )
			if( !(new->name = strdup( name ))) goto death;
		if( expansion )
			if( !(new->expansion = strdup( expansion ))) goto death;
		new->flags = 0;
	}
	return( new );
death:
	if( new ) {
		if( new->name ) free( new->name );
		if( new->expansion ) free( new->expansion );
		free( new );
	}
	return( NULL );
}

/*	internal deallocation function */
int
delete_macro( struct macro *mac )
{
	if( mac->name ) free( mac->name );
	if( mac->expansion ) free( mac->expansion );
	free( mac );
	return( 0 );
}

/*	perform a variable assignment
 *	this is the entry point from outside.
 *	a NULL or null string will delete the variable
 */
struct macro *
set_macro( char *name, char *expansion )
{
	struct macro *mac = NULL;
	int create_flag = (expansion && !isemptyline( expansion ));

	if( mac = find_macro( name )) {
		if( create_flag ) {
			if( mac->expansion ) free( mac->expansion );
			mac->expansion = strdup( expansion );
		}
		else {
			Remove( &mac->node );
			delete_macro( mac );
			mac = NULL;
		}
	}
	else
		if( create_flag ) mac = new_macro( name, expansion );

	if( mac ) {
		mac->flags &= ~MF_SIMPLE;
		if( !(mac->flags & MF_ADDED )) {
			mac->flags |= MF_ADDED;
			AddTail( &Global.macrolist, &mac->node );
		}
	}
	return( mac );
}

/*	Set's the automatic variables $@ $* and $<  */
void
set_target_macros( char *tarname, char *depname )
{
	struct macro *mac;

	if( tarname && *tarname ) {
		char *dupname = strdup( tarname );
		char *next;
		if( dupname ) { /* try the more efficient way first */
			next = strrchr( dupname, '.' );
			if( next ) *next = (char)0; /* remove the extension */
			mac	= set_macro( "*", dupname );
			free( dupname );
		}
		else {
			mac = set_macro( "*", tarname );
			next = strrchr( mac->name, '.' );
			if( next ) *next = (char)0; /* remove the extension */
		}
	}
	else mac = set_macro( "*", NULL );
	mac = set_macro( "@", tarname );
	mac = set_macro( "<", depname );
}

void
delete_macrolist( struct List *list )
{
	for_list( list, delete_macro );
	NewList( list );
}

/* set a variable by expanding all macros in its value */
struct macro *
set_simplemacro( char *name, char *value )
{
	char *expansion = NULL;
	struct macro *mac = NULL;

	expansion = (char *)malloc( Param.MaxLine );
	if( !expansion ) goto death;

	if( expand_macros( expansion, value, Param.MaxLine )) goto death;
	if( mac = set_macro( name, expansion )) {
		mac->flags |= MF_SIMPLE;
	}
death:
	if( expansion ) free( expansion );
	return( mac );
}
