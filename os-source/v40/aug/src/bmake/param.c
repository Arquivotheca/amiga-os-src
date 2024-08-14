/*	param.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <ctype.h>
#include <scdir.h>

#include <clib/exec_protos.h>

#include "make.h"

struct parameters Param = {
	{
	(struct Node *)&Param.filelist.lh_Tail,	/* lh_Head */
	(struct Node *)NULL,					/* lh_Tail */
	(struct Node *)&Param.filelist.lh_Head,	/* lh_TailPred */
	(UBYTE)NT_USER,
	(UBYTE)0
	},	/* filelist */
	1024, /* MaxLine */
	1,	/* verbosity */
	0,	/* log */
	0,	/* debug */
	0,	/* touch mode */
	0,	/* all mode */
	0,	/* no builtins */
	0,	/* pretend mode */
	0,	/* print database */
	NULL /* makefile */
};

const char *usage_string =
	"Usage: make -abdhnpt -v# +m# -fmakefile var=value wildcards...";

const char *help_lines[] = {
	"-a\t"	"make all dependents regardless of modification time",
	"-b\t"	"do not use builtin rules",
#if DEBUG
	"-d\t"	"debug mode (enable printf)",
#endif
	"+m#\t"	"sets the maximum line length to # (minimum = 1024)",
	"-n\t"	"prints commands to be executed, but do not execute them",
	"-p\t"	"prints the database, but do not run",
	"-t\t"	"make targets up to date by touching (commands not executed)",
	"-v#\t"	"set verbose level to #; 0=silent",
	"-fmakefile\t"	"specify the makefile to run",
	"var=value\t"	"assigns the value to a variable",
	"wildcards\t"	"specify targets to make",
	NULL
};

void
usage( void )
{
	printf( "%s %s\n", version_string+7, verdate_string );
	puts( usage_string );
}

int
help( void )
{
	int i = 0;
	char *helpline;

	usage();
	puts( copyright_string );
	puts( "" );
	while( helpline = help_lines[ i++ ] ) {
		puts( helpline );
	}

	return( 0 );
}

static int
expand_wildcard( const char *wild )
{
	struct string_node *sptr = NULL;
	char *fn;

	while( fn = scdir( wild )) {
		if( sptr = new_snode( fn )) {
			AddTail( &Param.filelist, &sptr->node );
		}
		else return( 1 );
	}
	return( 0 );
}

int
parse_parameters( int argc, char *argv[] )
{
	char *arg;

	for( int i = 1; i < argc; i++ ) {
		if( !(arg = argv[ i ] ))
			break;
		switch( *arg ) {
		case '-':
			switch( *++arg ) {
			default:
			case '?':
				usage();
				goto death;
			case 'V':
				printf( "%s %s\n", version_string + 7, verdate_string );
				puts( copyright_string );
				goto death;
			case 'a':
				Param.all_mode = 1;
				break;
			case 'b':
				Param.no_builtins = 1;
				break;
			case 'd':
#if DEBUG
				Param.debug = 1;
#else
				puts( "Debugging not available!" );
#endif
				break;
			case 'f':
				if( Param.makefile ) {
					puts( "only one -f argument is allowed" );
					goto death;
				}
				else {
					if( *++arg )
						Param.makefile = strdup( arg );
					else if( ++i < argc ) {
						arg = argv[ i ];
						if( *arg )
							Param.makefile = strdup( arg );
					}
				}
				break;
			case 'h':
				help();
				goto death;
			case 'n':
				Param.pretend_mode = 1;
				break;
			case 'p':
				Param.print_database = 1;
				break;
			case 't':
				Param.touch_mode = 1;
				break;
			case 'v':
				Param.verbosity = atoi( ++arg );
				break;
			}
			break;
		case '+':
			switch( *++arg ) {
			case 'l':
				Param.log = (Param.log) ? 0 : 1;
				break;
			case 'm':
				{
					int maxline = atoi( ++arg );
					Param.MaxLine = max( Param.MaxLine, maxline);
				}
			}
			break;
		case '?':
			usage();
			goto death;
		default: /* a filename */
			if( scdir( arg ) ) {
				scdir_abort();
				if( expand_wildcard( arg ))
					goto death;
			}
			else {
				struct string_node *sptr = new_snode( arg );
				if( sptr ) {
					AddTail( &Param.filelist, &sptr->node );
				}
			}
			break;
		}
	}
	if( !Param.makefile )
		Param.makefile = strdup( "Makefile" );

	return( 0 );
death:
	return( 1 );
}

void
delete_params( void )
{
	if( Param.makefile ) {
		free( Param.makefile );
		Param.makefile = NULL;
	}
	/* free Param */
	delete_slist( &Param.filelist );
	NewList( &Param.filelist );
}
