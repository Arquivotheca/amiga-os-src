/*
 * Amiga Grand Wack
 *
 * rexxhandler.c -- ARexx handling code for Wack.
 *
 * $Id: rexxhandler.c,v 39.13 93/11/05 14:58:37 jesup Exp $
 *
 * Wack is an ARexx command host.  It implements a small number
 * of specific-to-ARexx commands.  Anything not recognized as one
 * of those is attempted as a regular Wack command.  From the
 * Wack command line, it is also possible to invoke ARexx scripts
 * and "string files".
 *
 * This module contains code to initialize the ARexx stuff, handle
 * incoming ARexx messages, bind the ARexx-specific Wack commands, and
 * launch ARexx scripts from within Wack.  The code for the
 * ARexx-specific Wack commands is also here.
 *
 */

#include <dos/dos.h>
#include <dos/rdargs.h>

extern struct Library *DOSBase;

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern APTR LinkLibBase;

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include "symbols.h"
#include "simplerexx.h"
#include "wackbase.h"
#include "parse.h"

#include "sys_proto.h"
#include "io_proto.h"
#include "asm_proto.h"
#include "ops_proto.h"
#include "define_proto.h"
#include "parse_proto.h"
#include "show_proto.h"
#include "find_proto.h"
#include "special_proto.h"
#include "link_proto.h"
#include "wackbase_proto.h"
#include "rexxhandler_proto.h"

extern UWORD far VERSION;
extern UWORD far REVISION;
extern APTR CurrAddr;
extern APTR SpareAddr;
extern ULONG rexx_error;
extern struct WackLibBase *WackBase;

struct ARexxContext *RexxContext;
struct SymbolMap RexxSymMap = {0};
ULONG rexx_error = 0;

#define MAX(a,b) ((a)>(b)?(a):(b))

extern long remote_error;

/* Holds a copy of the command string to execute */
#define REXXBUFFERSIZE	512
char rexxcmdbuffer[ REXXBUFFERSIZE ];

/* Used for result strings */
#define REXXOUTBUFFERSIZE 512
char rexxoutbuffer[ REXXOUTBUFFERSIZE ];

/* Initialize an ARexx port named "WACK.n".  The default extension
 * for our ARexx scripts will be ".wack".  Returns the signal mask
 * of the port signal, or zero for failure.
 */
ULONG
ARexx_Init( void )
{
    ULONG sigmask = 0;
    if ( RexxContext = InitARexx( &WackBase->wl_MsgPort, "wack", "wack") )
    {
	sigmask = ARexxSignal(RexxContext);
	ARexx_Bind();
    }
    return( sigmask );
}

void
ARexx_Exit( void )
{
    /* Shut down ARexx */
    FreeARexx( RexxContext );
    UnBindSymbolMap( &RexxSymMap );
    RexxContext = NULL;
}

ULONG
ARexx_NumPending( void )
{
    ULONG outstanding = 0;
    if ( RexxContext )
    {
	outstanding = RexxContext->Outstanding;
    }
    return( outstanding );
}


void
ARexx_Handler( void )
{
    struct RexxMsg *rmsg;
    char *result = NULL;
    ULONG error = 0;
    if ( RexxContext )
    {
	while ( rmsg = GetARexxMsg( RexxContext ) )
	{
	    if (rmsg == REXX_RETURN_ERROR)
	    {
		/* We got an error back...  What do we do now?
		 * The error result was discarded inside
		 * GetARexxMsg()
		 */
		PPrintf("ARexx script failed\n");
	    }
	    else
	    {
		char *arg;
		char command[ MAXTOKENLEN ];

		if ( arg = parseToken( rmsg->rm_Args[0], command ) )
		{
		    struct Symbol *sym;
		
		    /* Is it a special Rexx-only command?  If so, we
		     * execute it here...
		     */
		    if ( sym = FindSymbol( command, &RexxSymMap ) )
		    {
			if ( !( arg = parseRemainder( arg ) ) )
			{
			    arg = "";
			}

			rexx_error = 0;
			resetError();

			result = evaluateSymbol( sym, arg );
			error = rexx_error;
			if ( ( !error ) && ( remote_error ) )
			{
			    error = 10;
			}
			resetError();
		    }
		    else
		    {
			/* It's not a Rexx-specific command, so pass it
			 * off to anyone else.
			 */
			if ( !processLine( rmsg->rm_Args[0] ) )
			{
			    error = 20;
			}
		    }
		}
	    }
	    ReplyARexxMsg( RexxContext, rmsg, result, error );
	}
    }
}


void
BindRexxPrim( char *name, rexxfunc function )
{
    struct Symbol *symbol;

    if ( ( symbol = FindSymbol( name, &RexxSymMap ) ) ||
	( symbol = NewSymbol( name, &RexxSymMap ) ) )
    {
	symbol->sym_Type = ACT_COMMAND;
	symbol->sym_Function = function;
    }
}


/* This routine creates a result string in a printf()-way */
char *
RexxPrintfResult( char *fmt, ... )
{
    char *result = NULL;
    /* We suppress printing after an error has occurred */
    if ( !remote_error )
    {
	sprintfA( rexxoutbuffer, fmt, ((ULONG *)&fmt)+1 );
	result = rexxoutbuffer;
    }
    return( result );
}


/* We are called to execute the named ARexx script.
 * If we got here through the use of the "rx" keyword,
 * then arg1 contains both the script name and the arguments.
 * If we got here as the result of the "bindrx" command,
 * arg1 contains the script name and any arguments specified
 * at bindrx time, while arg2 contains arguments added
 * at invocation time.
 */
STRPTR
ARexx_Execute( char *arg1, char *arg2 )
{
    if ( RexxContext )
    {
	strcpy( rexxcmdbuffer, arg1 );
	if ( ( arg2 ) && ( *arg2 ) )
	{
	    strcat( rexxcmdbuffer, " " );
	    strcat( rexxcmdbuffer, arg2 );
	}
 
	if ( !SendARexxMsg( RexxContext, rexxcmdbuffer, FALSE ) )
	{
	    PPrintf("Couldn't send ARexx message\n");
	}
    }
    else
    {
	PPrintf("ARexx not available\n");
    }

    return( NULL );
}

/* We are called to interpret the specified ARexx command. */
STRPTR
ARexx_ExecuteString( char *arg )
{
    if ( RexxContext )
    {
	if ( !SendARexxMsg( RexxContext, arg, TRUE ) )
	{
	    PPrintf("Couldn't send rexx message\n");
	}
    }
    else
    {
	PPrintf("ARexx not available\n");
    }

    return( NULL );
}


void
ARexx_Bind( void )
{
    BindRexxPrim("currentaddr", rxcmd_CurrentAddr );
    BindRexxPrim("spareaddr", rxcmd_SpareAddr );
    BindRexxPrim("version", rxcmd_Version );
    BindRexxPrim("print", rxcmd_Print );

    BindRexxPrim("readbyte", rxcmd_ReadByte );
    BindRexxPrim("readword", rxcmd_ReadWord );
    BindRexxPrim("readlong", rxcmd_ReadLong );
    BindRexxPrim("readblock", rxcmd_ReadBlock );
    BindRexxPrim("readstring", rxcmd_ReadString );
    BindRexxPrim("readbstr", rxcmd_ReadBSTR );

    BindRexxPrim("writebyte", rxcmd_WriteByte );
    BindRexxPrim("writeword", rxcmd_WriteWord );
    BindRexxPrim("writelong", rxcmd_WriteLong );
    BindRexxPrim("writeblock", rxcmd_WriteBlock );

    BindRexxPrim("findlibrary", rxcmd_FindLibrary );

    BindRexxPrim("connection", rxcmd_Connection );
    BindRexxPrim("contextaddr", rxcmd_Context );

    BindRexxPrim("call", rxcmd_Call );
}


char *
rxcmd_CurrentAddr( char *arg )
{
    long *address;
    char *result;

    result = RexxPrintfResult( "%lx", CurrAddr );

    if ( arg = parseRemainder( arg ) )
    {
	if ( parseHexNum( arg, (long *)&address, NULL ) )
	{
	    CurrAddr = address;
	}
	else
	{
	    rexx_error = 10;
	}
    }
    return( result );
}


char *
rxcmd_SpareAddr( char *arg )
{
    long *address;
    char *result;

    result = RexxPrintfResult( "%lx", SpareAddr );

    if ( arg = parseRemainder( arg ) )
    {
	if ( parseHexNum( arg, (long *)&address, NULL ) )
	{
	    SpareAddr = address;
	}
	else
	{
	    rexx_error = 10;
	}
    }
    return( result );
}


char *
rxcmd_Version( char *arg )
{
    return( RexxPrintfResult( "Wack %ld.%ld", VERSION,REVISION ) );
}


char *
rxcmd_Print( char *arg )
{
    PPrintf("%s\n", arg ? arg : "" );
    return( NULL );
}


/* Returns zero if there is a parsing problem, >0 if a hexadecimal
 * conversion is requested, <0 if a decimal conversion is requested.
 * Legal formats:
 *	address
 *	address decimal
 *	decimal
 *	(nothing)
 */
long
parseReadTemplate( char *arg, long **addressp )
{
    long ok = 1;
    char arg1[ MAXTOKENLEN ];

    *addressp = CurrAddr;
    if ( arg = parseToken( arg, arg1 ) )
    {
	if ( IsHexNum( arg1, (long *)addressp ) )
	{
	    arg = parseToken( arg, arg1 );
	}
	if ( arg )
	{
	    if ( !stricmp( arg1, "decimal" ) )
	    {
		ok = -1;
	    }
	    else
	    {
		ok = 0;
	    }
	}
    }
    return( ok );
}


char *
rxcmd_ReadByte( char *arg )
{
    long *address;
    unsigned char data;
    char *result = NULL;
    long parse;

    if ( parse = parseReadTemplate( arg, &address ) )
    {
	data = ReadByte( address );
	if ( parse > 0 )
	{
	    result = RexxPrintfResult("%02lx", data );
	}
	else
	{
	    result = RexxPrintfResult("%ld", data );
	}
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}


char *
rxcmd_ReadWord( char *arg )
{
    long *address;
    unsigned short data;
    char *result = NULL;
    long parse;

    if ( parse = parseReadTemplate( arg, &address ) )
    {
	data = ReadWord( address );
	if ( parse > 0 )
	{
	    result = RexxPrintfResult("%04lx", data );
	}
	else
	{
	    result = RexxPrintfResult("%ld", data );
	}
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}


char *
rxcmd_ReadLong( char *arg )
{
    long *address;
    long data;
    char *result = NULL;
    long parse;

    if ( parse = parseReadTemplate( arg, &address ) )
    {
	data = ReadLong( address );
	if ( parse > 0 )
	{
	    result = RexxPrintfResult("%08lx", data );
	}
	else
	{
	    result = RexxPrintfResult("%ld", data );
	}
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}


/* Legal formats:
 *	address hexvalue
 *	address decvalue decimal
 *	hexvalue
 *	decvalue decimal
 */
long
parseWriteTemplate( char *arg, long **addressp, long *data )
{
    char arg1[ MAXTOKENLEN ];
    char arg2[ MAXTOKENLEN ];
    char arg3[ MAXTOKENLEN ];
    char *addrarg = arg1;
    char *valuearg = arg2;
    BOOL decimal = FALSE;
    long ok = TRUE;

    if ( arg = parseToken( arg, arg1 ) )
    {
	if ( arg = parseToken( arg, arg2 ) )
	{
	    if ( parseToken( arg, arg3 ) )
	    {
		if ( !stricmp( arg3, "decimal" ) )
		{
		    decimal = TRUE;
		}
		else
		{
		    ok = FALSE;
		}
	    }
	    else if ( !stricmp( arg2, "decimal" ) )
	    {
		decimal = TRUE;
		addrarg = NULL;
		valuearg = arg1;
	    }
	}
	else
	{
	    addrarg = NULL;
	    valuearg = arg1;
	}
    }
    else
    {
	ok = FALSE;
    }

    if ( ok )
    {
	*addressp = CurrAddr;
	if ( addrarg )
	{
	    if ( !IsHexNum( addrarg, (long *)addressp ) )
	    {
		ok = FALSE;
	    }
	}
	if ( valuearg )
	{
	    if ( decimal )
	    {
		if ( !IsDecNum( valuearg, data ) )
		{
		    ok = FALSE;
		}
	    }
	    else
	    {
		if ( !IsHexNum( valuearg, data ) )
		{
		    ok = FALSE;
		}
	    }
	}
    }
    return( ok );
}


char *
rxcmd_WriteByte( char *arg )
{
    long *address;
    long data;

    if ( parseWriteTemplate( arg, &address, &data ) )
    {
	WriteByte( address, data );
    }
    else
    {
	rexx_error = 10;
    }
    return( NULL );
}


char *
rxcmd_WriteWord( char *arg )
{
    long *address;
    long data;

    if ( parseWriteTemplate( arg, &address, &data ) )
    {
	WriteWord( address, data );
    }
    else
    {
	rexx_error = 10;
    }
    return( NULL );
}


char *
rxcmd_WriteLong( char *arg )
{
    long *address;
    long data;

    if ( parseWriteTemplate( arg, &address, &data ) )
    {
	WriteLong( address, data );
    }
    else
    {
	rexx_error = 10;
    }
    return( NULL );
}


/* Legal formats:
 *	[nothing]
 *	address
 */
char *
rxcmd_ReadString( char *arg )
{
    void *address;
    char *result = "";

    if ( parseAddress( arg, (long *)&address ) )
    {
	GetString( address, rexxoutbuffer, REXXOUTBUFFERSIZE );
	result = rexxoutbuffer;
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}


/* Legal formats:
 *	[nothing]
 *	address
 */
char *
rxcmd_ReadBSTR( char *arg )
{
    void *address;
    char *result = "";

    if ( parseAddress( arg, (long *)&address ) )
    {
	GetBSTR( address, rexxoutbuffer, REXXOUTBUFFERSIZE );
	result = rexxoutbuffer;
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}


char *
rxcmd_FindLibrary( char *arg )
{
    struct Library *library;
    char *result;
    char *libname;
    result = "0";

    if ( libname = parseRemainder( arg ) )
    {
	if ( library = FindBase( arg ) )
	{
	    result = RexxPrintfResult( "%lx", library );
	}
	else
	{
	    rexx_error = 5;
	}
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}

char *
rxcmd_Connection( char *arg )
{
    return( (char *)LinkConnection() );
}

char *
rxcmd_Context( char *arg )
{
    return( RexxPrintfResult("%lx", LinkContext() ) );
}

char *
rxcmd_ReadBlock( char *arg )
{
    UBYTE *address;
    UBYTE *buffer;
    ULONG size;
    char *result = NULL;

    if (parseArrayTemplate( arg, &address, &buffer, &size ) )
    {
	ReadBlock( address, buffer, size );
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}


char *
rxcmd_WriteBlock( char *arg )
{
    UBYTE *address;
    UBYTE *buffer;
    ULONG size;
    char *result = NULL;

    if (parseArrayTemplate( arg, &address, &buffer, &size ) )
    {
	WriteBlock( address, buffer, size );
    }
    else
    {
	rexx_error = 10;
    }
    return( result );
}


/* Legal formats:
 *	address buffer_address length
 *	buffer_address length
 */
long
parseArrayTemplate( char *arg, UBYTE **addressp, UBYTE **data, ULONG *size )
{
    char arg1[ MAXTOKENLEN ];
    char arg2[ MAXTOKENLEN ];
    char arg3[ MAXTOKENLEN ];
    char *addrarg = arg1;
    char *bufferarg = arg2;
    char *sizearg = arg3;
    long ok = TRUE;

    if ( arg = parseToken( arg, arg1 ) )
    {
	if ( arg = parseToken( arg, arg2 ) )
	{
	    if ( parseToken( arg, arg3 ) )
	    {
		addrarg = arg1;
		bufferarg = arg2;
		sizearg = arg3;
	    } else {
		addrarg = NULL;
		bufferarg = arg1;
		sizearg = arg2;
	    }
	    *addressp = CurrAddr;
	    if ( addrarg )
	    {
		if ( !IsHexNum( addrarg, (long *)addressp ) )
		{
			ok = FALSE;
		}
	    }
	    if ( !IsHexNum( bufferarg, (long *)data ) )
	    {
		ok = FALSE;
	    }
	    if ( !IsHexNum( sizearg, (long *)size ) )
	    {
		ok = FALSE;
	    }
	} else
	    ok = FALSE;
    } else
	ok = FALSE;

    return( ok );
}


char *
rxcmd_Call( char *arg )
{
    long *address = CurrAddr;
    char *result = NULL;

    if ( arg = parseRemainder( arg ) )
    {
	if ( parseHexNum( arg, (long *)&address, NULL ) )
	{
		;
	}
	else
	{
	    rexx_error = 10;
	}
    }
    if (rexx_error == 0)
	if (!CallFunction(address))
	    rexx_error = 10;

    return( result );
}

