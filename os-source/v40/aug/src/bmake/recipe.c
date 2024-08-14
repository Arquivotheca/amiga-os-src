/*	recipe.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <string.h>
#include <ctype.h>

#include "make.h"
#include "depend.h"
#include "cond.h"

static
struct List Cstack =
{
	(struct Node *)&Cstack.lh_Tail,	/* lh_Head */
	(struct Node *)NULL,			/* lh_Tail */
	(struct Node *)&Cstack.lh_Head,	/* lh_TailPred */
	(UBYTE)NT_USER,
	(UBYTE)0
};


static int
changedir( char *newdir )
{
	BPTR oldlock = 0L, newlock = 0L;

	while( isspace( *newdir )) newdir++;
	newlock = ( *newdir ) ? Lock( newdir, MODE_OLDFILE ) : Global.oldcwd;
	if( newlock ) {
		oldlock = CurrentDir( newlock );
		if( *newdir && !Global.oldcwd )
			Global.oldcwd = oldlock; /* bug if oldlock is the root */
		else UnLock( oldlock );
		return( 0 );
	}
	logprintf( "Unable to Lock directory %s\n", newdir );
	return( 1 );
}

static void
echo_commandline( struct command *cmd, char *cline )
{
	if( !(cmd->flags & CF_NOECHO ) || Param.debug )
		logprintf( "\t%s\n", cline );
}

/* execute the command List to make a target */
int
recipe( const char *goalname, struct List *cmdlist )
{
	char special_cmd[ 10 ];
	struct command *cmd;
	char *expansion = NULL;
	char *next;
	int retval = 0, state;

	if( !cmdlist )
		return( 0 ); /* no command list */

	if( Param.touch_mode ) {
		logprintf( "\ttouch(%s)\n", goalname );
		if( !Param.pretend_mode )
			touch( goalname );
		return( 0 );
	}

	expansion = (char *)malloc( Param.MaxLine );

	for( cmd = (struct command *) cmdlist->lh_Head;	cmd->node.ln_Succ;
		cmd = cmd->node.ln_Succ ) {
		if( next = cmd->cmd )
			while( isspace( *next )) next++;
		if( next && *next ) {

			/* handle variable assignment before macro expansion */
			if( strchr( next, '=' )) {
				echo_commandline( cmd, next );
				if( retval = process_macroline( next ) ? 0 : 1 )
					break;
				continue;
			}

			if( expand_macros( expansion, next, Param.MaxLine )) {
				logprintf( "Error expanding macros on commandline:\n"
					"\t%s\n", next );
				break;
			};
			next = expansion;
			while( isspace( *next )) next++;

			next = parse_str( special_cmd, next, sizeof(special_cmd));
			if( !stricmp( special_cmd, "if" )) {
				echo_commandline( cmd, expansion );
				if( retval = drctv_if( next, &Cstack ))
					break;
				continue;
			}
			else if( !stricmp( special_cmd, "else" )) {
				echo_commandline( cmd, expansion );
				if( retval = drctv_else( next, &Cstack ))
					break;
				continue;
			}
			else if( !stricmp( special_cmd, "endif" )) {
				echo_commandline( cmd, expansion );
				if( retval = drctv_endif( next, &Cstack ))
					break;
				continue;
			}

			state = get_directive_state( &Cstack );
			if( state == STATE_IF_F || state == STATE_EL_T )
				continue;

			echo_commandline( cmd, expansion );

			if( !stricmp( special_cmd, "cd" )) {
				if( retval = changedir( next ))
					break;
				continue;
			}

			retval = (Param.pretend_mode ) ? 0 : xsystem( expansion );
			if( retval ) {
				char *s = (cmd->flags & CF_IGNORE ) ? "(Ignored)" : "" ;
				logprintf( "command returned error %d %s\n", retval, s );
				if( cmd->flags & CF_IGNORE )
					retval = 0;
				else
					break;
			}
		}
	}
	if( !retval && get_directive_state( &Cstack )) {
		logprintf( "Missing endif conditional command for %s\n", goalname );
		retval = 1;
	}
	if( expansion )
		free( expansion );
	return( retval );
}
