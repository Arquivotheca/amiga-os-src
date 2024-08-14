/*	depend.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <clib/exec_protos.h>

#include "make.h"
#include "depend.h"

struct target *default_target = NULL;

struct target *
find_target( char *targetname )
{
	struct target *targ = NULL;

	for( struct target *ln = Global.targetlist.lh_Head; ln->node.ln_Succ;
		ln = ln->node.ln_Succ ) {
		if( !stricmp( targetname, ln->name )) {
			targ = ln;
			break;
		}
	}

	return( targ );
}

struct target *
new_target( char *targetname )
{
	long size;
	struct target *new = NULL;

	if( targetname && *targetname ) {
		size = sizeof(struct target) + strlen( targetname );
		new = (struct target *)malloc( size );
		if( new ) {
			NewList( &new->dependlist );
			NewList( &new->commandlist );
			new->alternate = NULL;
			new->mtime = 0L;
			new->flags = 0L;
			if( targetname )
				strcpy( new->name, targetname );
			else
				*targetname = (char)0;
		}
	}
	return( new );
}

int
delete_target( struct target *targ )
{
	if( (targ->flags & TF_OWNER) && targ->commandlist.lh_Head )
		delete_commandlist( &targ->commandlist );
	if( targ->dependlist.lh_Head )
		delete_dependlist( &targ->dependlist );
	free( targ );
	return( 0 );
}

void
delete_targetlist( struct List *list )
{
	for_list( list, delete_target );
	NewList( list );
}

void
set_default_target( struct target *targ )
{
	if( default_target ) {
		delete_target( default_target );
	}
	default_target = targ;
	debugprintf( 4, ( "Setting .DEFAULT target\n" ));
}

struct depend *
new_depend( char *dependname )
{
	struct depend *new = NULL;
	long size;

	if( dependname ) {
		size = sizeof(struct depend) + strlen( dependname );
		if( new = (struct depend *)malloc( size )) {
			strcpy( new->name, dependname );
		}
	}
	return( new );
}

int
delete_depend( struct depend *dep )
{
	free( dep );
	return( 0 );
}

void
delete_dependlist( struct List *list )
{
	for_list( list, delete_depend );
	NewList( list );
}

struct command *
new_command( char *cmd )
{
	long size;
	struct command *new = NULL;
	if( cmd && *cmd ) {
		size = sizeof(struct command) + strlen( cmd );
		new = (struct command *)malloc( size );
		if( new ) {
			if( *cmd == '-' ) {
				new->flags = CF_IGNORE;
				cmd++;
			}
			else if( *cmd == '@' ) {
				new->flags = CF_NOECHO;
				cmd++;
			}
			else new->flags = 0;
			strcpy( new->cmd, cmd );
		}
	}
	return( new );
}

int
delete_command( struct command *cmd )
{
	free( cmd );
	return( 0 );
}

void
delete_commandlist( struct List *list )
{
	for_list( list, delete_command );
	NewList( list );
}

