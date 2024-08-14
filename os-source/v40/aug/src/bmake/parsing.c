/*	Parsing.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <string.h>
#include <ctype.h>

/*	Parse the first word from the source string into the dest string
 *
 *	return: pointer to next whitespace past parsed argument
 */

char *
parse_strtok( char *dest, char *source, int len, int (*isdelim)( int ))
{
	register char *s, *d;
	int i = 0;

	s = source;
	d=dest;
	while( i < len && *s ) {
		if( (*isdelim)( *s ))
			break;
		*d++ = *s++;
		i++;
	}
	*d=(char)0;
	return s;
}


static int
iswhite( int ch )
{
	return( isspace( ch ));	/* macro */
}

/*	Parse the first word from the source string into the dest string
 *
 *	return: pointer to next whitespace past parsed argument
 */
char *
parse_str( char *dest, char *source, int len )
{
	while( isspace( *source )) source++;	/* skip whitespaces */
	return( parse_strtok( dest, source, len - 1, iswhite ));
}

/*	Counts the number of words in the string */
int
count_args( unsigned char *string )
{
	int count, sflag, prev;
	register unsigned char *a = string;

	prev = 1;
	count = 0;
	for( ; *a; a++ ) {
		sflag = isspace( *a );
		if( !sflag && prev )
			count++;
		prev = sflag;
	}
	return count;
}

/*	finds the position of a particular word in a string */
/*	the first word is numbered 1 */
/*	find_word( string, count_args( string )) == last_word */
char *
find_word( char *string, int word )
{
	int count, sflag, prev;
	register char *a = string;

	prev = 1;
	count = 0;
	for( ; *a; a++ ) {
		sflag = isspace( *a );
		if( !sflag && prev )
			if( ++count == word )
				return( a );
		prev = sflag;
	}
	return( NULL ); /* not found */
}

void
strip_trailspace( char *line )
{
	char *cptr;

	cptr = line + strlen( line ) - 1;

	while( cptr >= line && isspace( *cptr )) {
		*cptr-- = (char)0;
	}
}

int
isnotsuf( int ch )
{
	if( !ch || isspace( ch ) || ch == '.' )
		return( 1 );
	return( 0 );
}

int
isemptyline( char *line )
{
	while( *line ) {
		if( !isspace( *line ))
			return( 0 );
		line++;
	}
	return( 1 );
}

/*	locate the next instance of a token that is not escaped by a backslash
 */
char *
find_token( char *line, int tok )
{
	char *delim = strchr( line, tok );
	while( delim && *(delim-1) == '\\' ) {
		delim = strchr( delim + 1, tok );
	}
	return( delim );
}

void
shift_string_left( char *string, int shift )
{
	register char *d, *s;
	char *end = string + strlen( string );

	if( shift > 0 ) {
		d = string;
		s = string + shift;
		while( s <= end ) {
			*d++ = *s++;
		}
	}
}

