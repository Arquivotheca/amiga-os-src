/*
 * Amiga Grand Wack
 *
 * link.c -- Generic functions to communicate with the target machine.
 *
 * $Id: link.c,v 39.11 93/11/05 14:58:56 jesup Exp $
 *
 * These routines connect to the target machine, regardless of
 * connection type.  As well, the memory functions invoke
 * the range-checking tests before proceeding.
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include "std.h"
#include "link.h"

#include "sys_proto.h"
#include "asm_proto.h"
#include "main_proto.h"
#include "link_proto.h"
#include "envoylink_proto.h"
#include "sadlink_proto.h"
#include "locallink_proto.h"
#include "validmem_proto.h"
#include "io_proto.h"
#include "parse_proto.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
extern struct Library *SysBase;

extern struct LinkLibBase *LinkLibBase;

extern long remote_error;

short nestcount = 0;
BOOL protect = TRUE;

LONG
ValidAddress( APTR addr )
{
    return( validAddress( LinkLibBase->ll_ValidMem, addr ) );
}

LONG
ValidAddresses( APTR addr, ULONG len )
{
    return( validAddresses( LinkLibBase->ll_ValidMem, addr, len ) );
}

BYTE
ReadByte( APTR addr )
{
    BYTE result = 0;
    if ( ( !protect ) || validAddress( LinkLibBase->ll_ValidMem, addr ) )
    {
	result = LinkReadByte( addr );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
    return( result );
}


WORD
ReadWord( APTR addr )
{
    WORD result = 0;

    if ( ( !protect ) || validAddresses( LinkLibBase->ll_ValidMem, addr, 2 ) )
    {
	result = LinkReadWord( addr );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }

    return( result );
}


LONG
ReadLong( APTR addr )
{
    LONG result = 0;

    if ( ( !protect ) || validAddresses( LinkLibBase->ll_ValidMem, addr, 4 ) )
    {
	result = LinkReadLong( addr );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
    return( result );
}


void
ReadBlock( APTR addr, APTR buffer, ULONG size )
{
    if ( ( !protect ) || validAddresses( LinkLibBase->ll_ValidMem, addr, size ) )
    {
	LinkReadBlock( addr, buffer, size );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
}


ULONG
ReadString( APTR addr, STRPTR buffer, ULONG maxlen )
{
    ULONG result = 0;

    if ( ( !protect ) || validAddress( LinkLibBase->ll_ValidMem, addr ) )
    {
	result = LinkReadString( addr, buffer, maxlen );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
    return( result );
}


void
WriteByte( APTR addr, BYTE value )
{
    if ( ( !protect ) || validAddress( LinkLibBase->ll_ValidMem, addr ) )
    {
	LinkWriteByte( addr, value );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
}


void
WriteWord( APTR addr, WORD value )
{
    if ( ( !protect ) || validAddresses( LinkLibBase->ll_ValidMem, addr, 2 ) )
    {
	LinkWriteWord( addr, value );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
}


void
WriteLong( APTR addr, LONG value )
{
    if ( ( !protect ) || validAddresses( LinkLibBase->ll_ValidMem, addr, 4 ) )
    {
	LinkWriteLong( addr, value );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
}

void
WriteBlock( APTR addr, APTR buffer, ULONG size )
{
    if ( ( !protect ) || validAddresses( LinkLibBase->ll_ValidMem, addr, size ) )
    {
	LinkWriteBlock( addr, buffer, size );
    }
    else
    {
	remote_error = WACKERROR_ILLEGALACCESS;
    }
}


STRPTR
ResumeExecution( void )
{
    if ( LinkResume() )
    {
	PPrintf("Resumed execution on target system\n");
    }
    else
    {
	PPrintf("Cannot resume execution\n");
    }

    return( NULL );
}


STRPTR
CallFunction( APTR addr )
{
    if ( LinkCall(addr) )
    {
	PPrintf("Called function $%lx on target system\n",addr);
    }
    else
    {
	PPrintf("Function $%lx failed to return\n",addr);
    }

    return( NULL );
}


STRPTR
ShowContext( void )
{
    if ( !LinkShowContext() )
    {
	PPrintf("Cannot get context\n");
    }

    return( NULL );
}

/* Obtain an array of node-pointers under Forbid() */
struct Node **
ObtainNodeArray( struct List *addr, ULONG size )
{
    long error;
    struct Node **array;
    long try = TRUE;

    while ( try )
    {
	try = FALSE;
	if ( array = malloc( (size+1) * sizeof( long ) ) )
	{
	    error = LinkReadNodeArray( addr, array, size );
	    if ( error > 0 )
	    {
		/* error > 0 signifies that the array wasn't big
		 * enough, so double its size and try again.
		 */
		free( array );
		array = NULL;
		size *= 2;
		try = TRUE;
	    }
	    else if ( error < 0 )
	    {
		/* error < 0 indicates a problem occurred.
		 * LinkObtainNodeArray() should have set
		 * a remote_error value.
		 */
		free( array );
		array = NULL;
	    }
	}
	else
	{
	    remote_error = WACKERROR_ALLOCFAIL;
	}
    }
    return( array );
}


/* Release an array obtained with ObtainNodeArray() */
void
ReleaseNodeArray( struct Node **array )
{
    free( array );
}


STRPTR
ProtectAddresses( char *arg )
{
    BOOL ok = TRUE;
    if ( arg = parseRemainder( arg ) )
    {
	if ( !stricmp( arg, "on" ) )
	{
	    protect = TRUE;
	}
	else if ( !stricmp( arg, "off" ) )
	{
	    protect = FALSE;
	}
	else
	{
	    PPrintf("Bad syntax.\n");
	    ok = FALSE;
	}
    }

    if ( ok )
    {
	if ( protect )
	{
	    PPrintf("Invalid addresses are protected from access\n");
	}
	else
	{
	    PPrintf("Invalid addresses are accessible\n");
	}
    }

    return( NULL );
}


APTR
OpenLinkLib( STRPTR arg )
{
    struct LinkLibBase *LinkLibBase = NULL;
    struct LinkFakeLibrary *lfl;

    if ( lfl = AllocMem( sizeof( struct LinkFakeLibrary ), MEMF_CLEAR ) )
    {
	long entry = 0;
	long unit;

	initLinkLVO( lfl, entry++, lib_stubReturn );	/* Open */
	initLinkLVO( lfl, entry++, lib_stubReturn );	/* Close */
	initLinkLVO( lfl, entry++, lib_stubReturn );	/* Expunge */
	initLinkLVO( lfl, entry++, lib_stubReturn );	/* Reserved */
	initLinkLVO( lfl, entry++, lib_stubReturn );	/* ARexx */

	if ( !arg )
	{
	    Local_InitLib( lfl );
	}
	else if ( IsDecNum( arg, &unit ) )
	{
	    SAD_InitLib( lfl );
	}
	else
	{
	    Envoy_InitLib( lfl );
	}

	CacheClearU();
	LinkLibBase = &lfl->lfl_LibBase;
    }
    return( LinkLibBase );
}


void
CloseLinkLib( APTR linkbase )
{
    struct LinkFakeLibrary *lfl;

    if ( linkbase )
    {
	lfl = (struct LinkFakeLibrary *)
	    (((ULONG)linkbase) - ((ULONG)&((struct LinkFakeLibrary *)0)->lfl_LibBase));
	FreeMem( lfl, sizeof( struct LinkFakeLibrary ) );
    }
}


void
initLinkLVO( struct LinkFakeLibrary *lfl, ULONG count, APTR funcptr )
{
    ULONG lvo = LINKTABLESIZE - 6*(count+1);
    *((UWORD *)&lfl->lfl_JumpTable[ lvo ]) = 0x4EF9;	/* jump instruction */
    *((ULONG *)&lfl->lfl_JumpTable[ lvo+2 ]) = (ULONG) funcptr;
}


ULONG
lib_stubReturn( void )
{
    return( 0 );
}
