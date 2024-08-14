/*	builtin.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <fcntl.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "make.h"
#include "depend.h"

extern void NewList( struct List *);

/*	creates a new BUILTIN rule (of any type)
 *	accepts optional command lines as arguments, ending in a NULL
 *	returns the first target created
 */
static struct target *
new_targetline( char *tline, ... )
{
	va_list argptr;
	struct List cmdList;
	struct target *targ;
	struct command *cmd;
	char *nextcmd;

	NewList( &cmdList );
	va_start( argptr, tline );
	while( nextcmd = va_arg( argptr, char *)) {
		cmd = new_command( nextcmd );
		if( !cmd ) break;
		AddTail( &cmdList, &cmd->node );
	}

	targ = process_targetline( tline, &cmdList );
	if( !targ ) delete_commandlist( &cmdList );
	va_end( argptr );
	return( targ );
}

int
input_builtins( void )
{
	static char *name_list[] = {
		"builtins.make",
		"S:builtins.make",
		NULL
	};
	char *filename;
	int i = 0;
	BPTR	lock;

	while( filename = name_list[ i++ ] ) {
		lock = Lock( filename, ACCESS_READ );
		if( lock ) { /* file exists */
			UnLock( lock );
			if( input_makefile( filename )) {
				logprintf( "Error reading file: %s\n", filename );
				return( 1 );
			}
		}
	}
	return( 0 );
}

int
init_builtins( void )
{
	struct target *targ;

	if( Param.no_builtins ) return( 0 );

	set_simplemacro( "CC", "dcc" );
	set_simplemacro( "LN", "dcc" );
	set_simplemacro( "CFLAGS", "-proto" );
	set_simplemacro( "AS", "dcc" );
	set_simplemacro( "CI", "ci" ); /* RCS */
	set_simplemacro( "CO", "co" ); /* RCS */

	set_macro( "@D", "$(dir $@)" );
	set_macro( "*D", "$(dir $*)" );
	set_macro( "<D", "$(dir $<)" );
	set_macro( "@F", "$(nodir $@)" );
	set_macro( "*F", "$(nodir $*)" );
	set_macro( "<F", "$(nodir $<)" );

	targ = new_targetline( "%,v:", NULL ); /* no commands for RCS files */

/*	targ = new_targetline( "%.a: RCS/%.a,v", "$(CO) -u $@", NULL ); */
	targ = new_targetline( "%.a:", NULL ); /* no commands for a files */

/*	targ = new_targetline( "%.c: RCS/%.c,v", "$(CO) -u $@", NULL ); */
	targ = new_targetline( "%.c:", NULL ); /* no commands for c files */

/*	targ = new_targetline( "%.h: RCS/%.h,v", "$(CO) -u $@", NULL ); */
	targ = new_targetline( "%.h:", NULL ); /* no commands for c headers */

/*	targ = new_targetline( "%.i: RCS/%.i,v", "$(CO) -u $@", NULL ); */
	targ = new_targetline( "%.i:", NULL ); /* no commands for asm headers */

	targ = new_targetline( "%.o: %.c", "$(CC) -c $(CFLAGS) -o $@ $<", NULL );
	targ = new_targetline( "%.o: %.a", "$(AS) -c $(AFLAGS) -o $@ $<", NULL );

	if( input_builtins()) return( 1 );

	return( 0 );
}

