/*	read.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <ctype.h>
#include <clib/exec_protos.h>

#include "make.h"
#include "depend.h"
#include "cond.h"

extern int line_number;

/*	process a line that looks like
 *	target names: dependencies
 */
struct target *
process_targetline( char *line, struct List *cmdlist )
{
	struct target *first_targ = NULL;
	struct target *targ = NULL;
	struct List *ownerlist = NULL;
	char *delim, *nexttar;
	char name[ MAXPATHNAME ];
	char *expandlhs = NULL, *expandrhs = NULL;
	long len;
	int dbl_colon_flag = 0;

	struct depend *dep = NULL;
	struct command *cmd = NULL;
	char *next;

	if( !(expandlhs = (char *)malloc( Param.MaxLine )) ||
		!(expandrhs = (char *)malloc( Param.MaxLine )))
		goto death;

	while( isspace( *line )) line++;
	delim = find_token( line + 1, ':' );
	if( !delim )
		goto death;
	len = (long)(delim - line);
	if( !len )
		goto death;

	if( delim[1] == ':' ) {
		dbl_colon_flag = 1;
		delim++;
	}

	memset( name, 0, sizeof( name ));
	strncpy( name, line, (int)(delim - line));

	/* expand macros in targetnames */
	if( expand_macros( expandlhs, name, Param.MaxLine ))
		goto death;
	nexttar = expandlhs;

	/* for each target name left of the colon */
	for(;;) {
		dep = NULL;
		cmd = NULL;
		
		nexttar = parse_str( name, nexttar, sizeof( name ));
		if( !*name )
			break; /* no more names */

		if( dbl_colon_flag)
			targ = NULL;
		else
			targ = find_target( name );
		if( !targ ) {
			targ = new_target( name );
			if( !targ )
				goto death;
			targ->line_number = line_number;
		}
		if( Global.builtin_flag )
			targ->flags |= TF_BUILTIN;
		if( dbl_colon_flag )
			targ->flags |= TF_DBLCOLON;

		debugprintf( 5, ( "new target: %s\n", targ->name ) );

		if( !first_targ ) {
			first_targ = targ;
			first_targ->flags |= TF_OWNER;
			ownerlist = &targ->commandlist;
		}

		set_target_macros( name, NULL );

		/* find the dependencies */
		next = delim + 1;
		while( isspace( *next )) next++;

		/* expand macros in dependencies */
		if( expand_macros( expandrhs, next, Param.MaxLine ))
			goto death;
		next = expandrhs;

		/* for each dependency add to list */
		for( ;; ) {
			next = parse_str( name, next, sizeof( name ));
			if( !*name )
				break; /* no more names */

			if( *name == '.' ) {
				if( !stricmp( name+1, ONCE_TARGET )) {
					targ->flags |= TF_ONCE;
					continue;
				}
				else if( !stricmp( name+1, INVIS_TARGET )) {
					targ->flags |= TF_INVIS;
					continue;
				}
				else if( !stricmp( name+1, ALWAYS_TARGET )) {
					targ->flags |= TF_ALWAYS;
					continue;
				}
				else if( !stricmp( name+1, NEVER_TARGET )) {
					targ->flags |= TF_NEVER;
					continue;
				}
			}

			/* first check if that dependency already exists */
			for( dep = targ->dependlist.lh_Head; dep->node.ln_Succ;
				dep = dep->node.ln_Succ ) {
				if( !stricmp( dep->name, name ))
					break;
			}

			if( !dep->node.ln_Succ ) { /* doesn't exist already */
				if(  dep = new_depend( name ))
					AddTail( &targ->dependlist, &dep->node );
				else
					goto death;
			}
			else {
				logprintf( "duplicate dependency %s for target %s\n",
					name, targ->name );
			}
		}
		/* for each command add to list */
		if( targ->flags & TF_OWNER ) {
			delete_commandlist( &targ->commandlist );
			attach_list( &targ->commandlist, cmdlist );
		}
		else targ->alternate = ownerlist;

		if( !stricmp( targ->name, DEFAULT_TARGET )) {
			set_default_target( targ );
		}
		else if( !(targ->flags & TF_ADDED )) {
			if( ((dep = targ->dependlist.lh_Head) &&
				strchr( dep->name, PATMATCH_CHAR )) ||
				strchr( targ->name, PATMATCH_CHAR )) {
				add_pattern_rule( targ );
			}
			else {
				targ->flags |= TF_ADDED;
				AddTail( &Global.targetlist, &targ->node );
			}
		}
		targ = NULL;
	} /* for */

death:
	set_target_macros( NULL, NULL ); /* delete $@ $* and $< */

	if( targ )
		free( targ );
	if( expandrhs )
		free( expandrhs );
	if( expandlhs )
		free( expandlhs );
	if( dbl_colon_flag ) {
		logfile( "WARNING:  double colon rules do not work!\n" );
	}
	return( first_targ );
}

/*	process a line that looks like
 *	variable = value
 */
struct macro *
process_macroline( char *line )
{
	struct macro *mac = NULL;
	char *delim, *next;
	char macroname[ MAX_MACRONAME ], *rhs;
	char *expansion = NULL; /* dynamic */
	long namelen, explen;
	int do_expansion = 0;
	int do_addition = 0;

	debugprintf( 5, ( "macroline %d: %s\n", line_number, line ) );

	while( isspace( *line ))
		line++; /* skip whitespace */

	delim = strchr( line, '=' ); /* cannot be escaped by backslash */
	if( !delim )
		goto death;
	rhs = delim + 1;

	namelen = min( (long)(delim - line), sizeof(macroname)-1);
	if( !namelen )
		goto death;

	switch( *(delim-1)) {
	case ':':
		do_expansion = 1;
		delim--;
		namelen--;
		break;
	case '+':
		do_addition = 1;
		delim--;
		namelen--;
		break;
	}

	next = line;
	while( isspace( *next )) next++;
	memset( macroname, 0, sizeof( macroname ));
	for( int i = 0; next < delim && i < 79; i++ ) {
		if( isspace( *next ))
			break;
		macroname[ i ] = *next++;
	}

	while( isspace( *rhs )) rhs++; /* skip whitespace */

	if( do_expansion && *rhs ) {
		expansion = (char *)malloc( Param.MaxLine );
		if( !expansion )
			goto death;
		if( expand_macros( expansion, rhs, Param.MaxLine ))
			goto death;
			/* expand rhs */
		rhs = expansion;
	}

	if( mac = find_macro( macroname )) {
		char *old = mac->expansion;

		mac->flags &= ~MF_SIMPLE;
		if( do_addition && old ) {
			if( mac->expansion = malloc(strlen( old ) + strlen( rhs ) + 1)) {
				strcpy( mac->expansion, old );
				strcat( mac->expansion, " " );
				strcat( mac->expansion, rhs );
			}
		}
		else mac->expansion = strdup( rhs );
		if( old )
			free( old );
	}
	else mac = new_macro( macroname, rhs );

	if( mac ) {
		if( do_expansion )
			mac->flags |= MF_SIMPLE;
		if( !(mac->flags & MF_ADDED )) {
			mac->flags |= MF_ADDED;
			AddTail( &Global.macrolist, &mac->node );
		}
		debugprintf( 5, ( "assigned macro [%s] = [%s]\n",
			mac->name, mac->expansion ));
	}
	else {
		debugprintf( 2, ("error assigning macro [%s]\n", macroname ) );
	}

	if( expansion )
		free( expansion );
	return( mac );
death:
	if( expansion )
		free( expansion );
	if( mac )
		free( mac );
	return( NULL );
}

/*	read in all rules and variables from this makefile
*/
int
input_makefile( const char *makefile )
{
	char *curline = NULL;
	char *commandline = NULL;
	FILE *infile = NULL;
	int targetcount = 0, macrocount = 0;
	int st;

	curline = (char *)malloc( Param.MaxLine );
	commandline = (char *)malloc( Param.MaxLine );
	if( !curline || !commandline )
		goto death;

	infile = fopen( makefile, "r" );
	if( !infile ) {
		logprintf( "Cannot open %s\n", makefile );
		goto death;
	}

	debugprintf( 3,( "input Makefile: %s\n", makefile ));

	line_number = 0;
	if( st = getline( curline, Param.MaxLine, infile ))
		goto death;
	while( !st ) {
		/* debugprintf( 5,( "getline %d: %s\n", line_number, curline )); */
		if( strchr( curline, '=' )) {
			macrocount++;
			process_macroline( curline );

			st = getline( curline, Param.MaxLine, infile );
		}
		else if( isemptyline( curline )) {
			st = getline( curline, Param.MaxLine, infile );
		}
		else {
			struct List cmdList;
			struct target *targ = NULL;
			struct command *cmd = NULL;
			char *delim;

			NewList( &cmdList );

			targetcount++;
			*commandline = (char)0;

			delim = find_token( curline + 1, ':' );
			if( delim ) {
				if( delim = find_token( delim + 1, ';' ) ) {
					*delim++ = (char)0;
					*commandline = '\t'; /* commandlines begin with a tab */
					strcpy( commandline + 1, delim ); /* overlapping copy */
				}
			}

			if( st = ( *commandline ) ? 0 :
				getline( commandline, Param.MaxLine, infile )) break;
			while( !st && isspace( *commandline )) {
				debugprintf( 5,("tabline %d: %s\n",line_number, commandline));
				delim = commandline;
				while( isspace( *delim )) delim++;
				if( *delim ) { /* line is not empty */
					if( cmd = new_command( delim ))
						AddTail( &cmdList, &cmd->node );
					else {
						logprintf( "Could not add command for target\n" ); 
						goto death;
					}
				}
				st = getline( commandline, Param.MaxLine, infile );
			} /* while */

			debugprintf( 4, ("targetline %d is %s\n", line_number, curline));
			targ = process_targetline( curline, &cmdList );
			if( !targ ) {
				logprintf( "Error in Makefile before line %d: %s\n",
					line_number, curline );
				goto death;
			}

			strcpy( curline, commandline );
		} /* else */
	} /* while */

	debugprintf( 2,( "targetcount = %d\nmacrocount = %d\n", targetcount,
		macrocount ));

#if DEBUG
	if( Param.debug && Param.verbosity >= 5 ) {
		dump_all();
	}
#endif

	free( commandline );
	free( curline );
	fclose( infile );
	return( 0 );
death:
	if( commandline )
		free( commandline );
	if( curline )
		free( curline );
	if( infile )
		fclose( infile );
	return( 1 );
}
