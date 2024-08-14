/*	pattern.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "make.h"
#include "depend.h"


int
pattern_match( char *pattern, char *cmpstr )
{
	char *delim = pattern;
	int prelen, postlen, complen = strlen( cmpstr);

	while( *delim && *delim != PATMATCH_CHAR )
		delim++;
	if( !*delim )
		return( stricmp( pattern, cmpstr ) ? 0 : 1 );
	prelen = (int)(delim - pattern);
	postlen = strlen( delim + 1 );
	if( prelen + postlen > complen ||
		strnicmp( pattern, cmpstr, prelen ) ||
		stricmp( delim + 1, cmpstr + complen - postlen ))
		return( 0 );
	return( 1 ); /* match */
}

static struct patternrule *
find_patternrule( char *dep_pat, char *tar_pat )
{
	struct patternrule *sr = NULL;

	for( struct patternrule *ln = Global.patternlist.lh_Head; ln->node.ln_Succ;
		ln = ln->node.ln_Succ ) {
		if( !stricmp( dep_pat, ln->dep_pat ) &&
			!stricmp( tar_pat, ln->tar_pat )) {
			sr = ln;
			break;
		}
	}

	return( sr );
}

struct patternrule *
new_patternrule( char *dep_pat, char *tar_pat )
{
	int found_flag = 0;
	long size = sizeof(struct patternrule);
	struct patternrule *new = find_patternrule( dep_pat, tar_pat );
	if( new )
		found_flag = 1;
	else
		new = (struct patternrule *)malloc( size );

	if( new ) {
		strcpy( new->tar_pat, tar_pat );
		if( dep_pat )
			strcpy( new->dep_pat, dep_pat );
		else
			*new->dep_pat = (char)0;
		if( !found_flag )
			AddTail( &Global.patternlist, &new->node );
	}
	return( new );
}

int
delete_patternrule( struct patternrule *rule )
{
	free( rule );
	return( 0 );
}

void
delete_patternlist( struct List *list )
{
	for_list( list, delete_patternrule );
}

struct patternrule *
add_pattern_rule( struct target *tar )
{
	struct patternrule *sr = NULL;
	struct depend *dep = (struct depend *)tar->dependlist.lh_Head;
	char *depname;
	depname = (dep) ? (dep->name) : NULL;
	sr = new_patternrule( depname, tar->name );
	if( sr ) {
		sr->targ = tar;
		if( tar->flags & TF_ADDED )
			Remove( &tar->node );
		tar->flags |= (TF_PATTERN | TF_ADDED);
		AddTail( &Global.speciallist, &tar->node );
	}
	return( sr );
}

struct patternrule *
add_suffix_targets( char *suf )
{
	char *next, tar_pat[ MAXSUFFIX+2 ], dep_pat[ MAXSUFFIX+2 ];	
	struct patternrule *sr, *first = NULL;
	struct target *ln, *succ;

	for( ln = (struct target *)Global.targetlist.lh_Head;
		ln->node.ln_Succ; ln = succ ) {
		succ = ln->node.ln_Succ;

		next = ln->name;
		*dep_pat = (char)0; /* default to match nothing */
		strcpy( tar_pat, "%." );

		if( *next++ == '.' ) {
			next = parse_strtok( tar_pat+2, next, sizeof(suf)-1, isnotsuf );
			if( *next++ == '.' ) {
				strcpy( dep_pat, tar_pat ); /* double-suffix */
				next = parse_strtok( tar_pat+2, next, sizeof(suf)-1, isnotsuf );
			}
		}
		if( !stricmp( suf, tar_pat+2 )) {
			/* transform a target rule into a suffix rule */
			sr = new_patternrule( dep_pat, tar_pat );
			if( sr ) {
				sr->targ = ln;
				ln->flags |= TF_PATTERN;
				Remove( &ln->node );
				AddTail( &Global.speciallist, &ln->node );
			}
			else
				return( first ); /* error */
			if( !first )
				first = sr;
		}
	}

	return( first );
}

int
map_to_pattern( char *name, char *from_pat, char *to_pat, char *string )
{
	char *f_delim = from_pat;
	char *t_delim = to_pat;
	int len, prelen;

	while( *f_delim && *f_delim != PATMATCH_CHAR ) f_delim++;
	while( *t_delim && *t_delim != PATMATCH_CHAR ) t_delim++;
	if( !*f_delim )
		return( 1 );
	if( !*t_delim ) {
		strcpy( name, to_pat );
		return( 0 );
	}

	len = (int)(t_delim - to_pat );
	if( len > 0 ) {
		strncpy( name, to_pat, len );
		name += len;
	}

	prelen = (int)(f_delim - from_pat);
	len = strlen( string ) - strlen( f_delim + 1 ) - prelen;

	if( len > 0 ) {
		strncpy( name, string + prelen, len );
		name += len;
	}
	strcpy( name, t_delim + 1 );
	return( 0 );
}
