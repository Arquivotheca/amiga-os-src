;/*
lc -b0 -v xwackdemo
blink xwackdemo.o to xwackdemo sc sd nd lib lib:lc.lib
quit
;*/

/* xwackdemo.c
 *
 * (c) Copyright 1992-1993 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 * Example Wack external extension program.  This one has a single
 * function called "libvers".  Its role is to print out the name, version
 * and revision of a library at the specified address.  If no address
 * is specified, the current address is used.
 *
 * Bind it into Wack using:
 *
 *	bindxwack libvers xwackdemo libvers
 *
 */

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "V39:aug/src/wack/wack_protos.h"
#include "V39:aug/src/wack/wack_pragmas.h"

struct Library *SysBase;
struct Library *DOSBase;
struct MsgPort *WackBase;

#define TEMPLATE "wackport/A,libvers/S,arg/F"
#define OPT_WACKPORT	0
#define	OPT_LIBVERS	1
#define OPT_ARG		2
#define OPT_COUNT	3

ULONG stringToHex( STRPTR argstring, ULONG *result );

long
main( void )
{
    long x;
    long opts[OPT_COUNT];
    struct ReadArgs *rdargs;
    long result = 20;

    SysBase = (*((struct Library **) 4));

    if ( DOSBase = OpenLibrary("dos.library", 37 ) )
    {
	for ( x=0; x<OPT_COUNT; opts[x++]=0 )
	{
	}

	if ( rdargs = ReadArgs( TEMPLATE, opts, NULL ) )
	{
	    result = 5;
	    if ( WackBase = FindPort( (STRPTR)opts[OPT_WACKPORT] ))
	    {
		if ( opts[OPT_LIBVERS] )
		{
		    struct Library *library;
		    if ( opts[OPT_ARG ] )
		    {
			if ( stringToHex( (STRPTR)opts[OPT_ARG], (ULONG *)&library ) )
			{
			    result = 0;
			}
		    }
		    else
		    {
			library = Wack_ReadCurrAddr();
			result = 0;
		    }
		    if ( result == 0 )
		    {
#define NAMESIZE 100
			char namebuff[ NAMESIZE ];
			Wack_ReadString( Wack_ReadPointer( &library->lib_Node.ln_Name ),
			    namebuff, NAMESIZE );
			Wack_Printf( "%s %ld.%ld at %lx\n",
			    namebuff,
			    Wack_ReadWord( &library->lib_Version ),
			    Wack_ReadWord( &library->lib_Revision ),
			    library );
			result = 0;
		    }
		}
	    }
	    FreeArgs( rdargs );
	}
	CloseLibrary( DOSBase );
    }
    return( result );
}

void
Wack_Printf( STRPTR fmt, ... )
{
    Wack_VPrintf( fmt, ((ULONG *)&fmt)+1 );
}

ULONG
stringToHex( STRPTR argstring, ULONG *result )
{
    ULONG number = 0;
    ULONG done = 0;
    char ch;
    ULONG digit;

    while ( !done )
    {
	ch = *argstring++;

	if ( ( ch >= '0' ) && ( ch <= '9' ) )
	{
	    digit = ch - '0';
	}
	else if ( ( ch >= 'A' ) && ( ch <= 'F' ) )
	{
	    digit = ch - 'A' + 10;
	}
	else if ( ( ch >= 'a' ) && ( ch <= 'f' ) )
	{
	    digit = ch - 'a' + 10;
	}
	else
	{
	    done = 1;
	    if ( ( ch != '\0' ) && ( ch != '\t' ) && ( ch != '\n' ) && ( ch != ' ' ) )
	    {
		done = 2;
	    }
	}
	if ( !done )
	{
	    number = 16*number + digit;
	}
    }
    if ( done == 1 )
    {
	*result = number;
	return( 1 );
    }
    else
    {
	return( 0 );
    }
}
