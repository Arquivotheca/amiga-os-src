/*
 * Amiga Grand Wack
 *
 * parse.c -- Centralized parsing routines.
 *
 * $Id: parse.c,v 39.2 93/08/23 19:31:52 peter Exp $
 *
 * Routines to handle various parsing chores.
 */

#include <exec/types.h>
#include "parse.h"
#include "parse_proto.h"
#include "asm_proto.h"
#include "sys_proto.h"

extern long CurrAddr;

static long
isDelimeter( char ch )
{
    return( ( ch == ' ' ) || ( ch == '\t' ) ||
	( ch == '\n' ) || ( ch == '\0' ) );
}


static long
isBlank( char ch )
{
    return( ( ch == ' ' ) || ( ch == '\t' ) );
}


#define DOUBLE_QUOTE	'\"'
#define SINGLE_QUOTE	'\''

/* This function takes the supplied input string arg and skips over
 * any leading blanks.  If anything remains, the result will be
 * the remainder of the string.
 *
 * A NULL result indicates that no non-blank characters were found.
 */
STRPTR
parseRemainder( STRPTR arg )
{
    if ( arg )
    {
	/* skip any leading blanks */
	while ( isBlank( *arg ) )
	{
	    arg++;
	}

	if ( !*arg )
	{
	    arg = NULL;
	}
    }
    return( arg );
}


/* This function takes the supplied input string arg and skips over
 * any leading blanks.  If anything remains, the next word is copied
 * into the supplied token buffer.  The return value is a pointer
 * to the next non-blank character in the buffer.
 *
 * A NULL result indicates that no token was found.
 */
STRPTR
parseToken( STRPTR arg, STRPTR token )
{
    STRPTR argstart;
    STRPTR argend;
    ULONG bytecount;

    /* Find the first non-blank character.  Returns NULL if
     * there are no non-blanks before the \0.
     */
    if ( arg = parseRemainder( arg ) )
    {
	argstart = arg;
	if ( ( *arg == DOUBLE_QUOTE ) || ( *arg == SINGLE_QUOTE ) )
	{
	    /* If the first non-blank is a quote character, then
	     * skip over it and set our termination condition
	     * to be the matching quote or the end-of-string.
	     */
	    char quote = *arg;
	    argstart++;
	    arg++;

	    while ( ( *arg != '\0' ) && ( *arg != quote ) )
	    {
		arg++;
	    }
	    argend = arg;
	    if ( *arg == quote )
	    {
		arg++;
	    }
	}
	else
	{
	    /* This is a non-quoted argument, so just keep going
	     * until a non-delimeter is found.
	     */
	    while ( !isDelimeter( *arg ) )
	    {
		arg++;
	    }
	    argend = arg;
	}

	/* skip any trailing blanks */
	while ( isBlank( *arg ) )
	{
	    arg++;
	}

	/* Copy this argument into the token storage and
	 * NULL-terminate it:
	 */
	bytecount = 0;
	while ( ( argstart < argend ) && ( ++bytecount < MAXTOKENLEN ) )
	{
	    *token++ = *argstart++;
	}
	*token = '\0';
    }
    return( arg );
}


/* Parses the supplied input string according to the same rules as
 * parseToken().  The word extracted is assumed to be a hexadecimal
 * string and converted to a ULONG.  The return value is a pointer
 * to the next non-blank character in the buffer.
 *
 * A NULL result indicates that no token was found or that an
 * invalid hexadecimal token was found.
 */
STRPTR
parseHexNum( STRPTR arg, ULONG *value, ULONG *inputsize )
{
    STRPTR result;
    char token[ MAXTOKENLEN ];

    if ( result = parseToken( arg, token ) )
    {
	if ( !IsHexNum( token, value ) )
	{
	    result = NULL;
	}
	else
	{
	    if ( inputsize )
	    {
		ULONG len = strlen( token );
		*inputsize = 4;
		if ( len <= 2 )
		{
		    *inputsize = 1;
		}
		else if ( len <= 4 )
		{
		    *inputsize = 2;
		}
	    }
	}
    }
    return( result );
}


/* A variant on parseToken(), this routine parses the supplied input
 * string according to the same rules as parseToken().  The word
 * extracted is assumed to be a hexadecimal string and converted to
 * a ULONG.  The return value is a pointer to the next non-blank
 * character in the buffer.
 *
 * If no token is found, then the CurrAddr pointer is returned
 * as the result.
 *
 * A NULL result indicates an invalid hexadecimal token was found.
 */
STRPTR
parseAddress( STRPTR arg, ULONG *value )
{
    STRPTR result;
    char token[ MAXTOKENLEN ];

    if ( result = parseToken( arg, token ) )
    {
	if ( !IsHexNum( token, value ) )
	{
	    result = NULL;
	}
    }
    else
    {
	*value = CurrAddr;
	result = arg;
    }
    return( result );
}


/* Parses the supplied input string according to the same rules as
 * parseToken().  The word extracted is assumed to be a decimal
 * string and converted to a ULONG.  The return value is a pointer
 * to the next non-blank character in the buffer.
 *
 * A NULL result indicates that no token was found or that an
 * invalid decimal token was found.
 */
STRPTR
parseDecNum( STRPTR arg, ULONG *value )
{
    STRPTR result;
    char token[ MAXTOKENLEN ];

    if ( result = parseToken( arg, token ) )
    {
	if ( !IsDecNum( token, value ) )
	{
	    result = NULL;
	}
    }
    return( result );
}


