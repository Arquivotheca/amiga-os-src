/*
 * Amiga Grand Wack
 *
 * define.c -- Wack key and symbol support
 *
 * $Id: define.c,v 39.9 93/10/08 16:45:11 jesup Exp $
 *
 * Routines to handle building symbol tables, and binding commands to
 * functions in various ways.
 *
 */

#include "symbols.h"
#include "special.h"
#include "parse.h"

#include <exec/types.h>
#include "parse_proto.h"
#include "link_proto.h"
#include "sys_proto.h"
#include "asm_proto.h"
#include "ops_proto.h"
#include "bind_proto.h"
#include "define_proto.h"
#include "find_proto.h"
#include "io_proto.h"
#include "rexxhandler_proto.h"
#include "wackbase_proto.h"

extern long LastNum;

struct SymbolMap TopSymMap = {0};

extern long CurrAddr;

extern LONG Hash();

char *TypeNames[] =
{
    "constant",
    "command",
    "offset",
    "base",
};

char *
UnCtrl( char c, char *s )
{
    char *t = s;

    if (c < ' ')
    {
	*t++ = '^';
	*t++ = 'A' - 1 + c;
    }
    else
	if (c != 0x7f)
	{
	    *t++ = c;
	}
    *t = 0;

    return s;
}

void
BindMacro( char *name, short *type, char *body )
{

}

/* Allocate and return a Symbol of the supplied name and link it
 * into the specified SymbolMap.
 */
struct Symbol *
NewSymbol( char *name, struct SymbolMap *symMap )
{
    char *newName;
    struct Symbol *sym;

    if ( sym = (struct Symbol *) malloc( sizeof( struct Symbol ) ) )
    {
	if ( newName = (char *) malloc( strlen( name ) + 1 ) )
	{
	    strcpy( newName, name );
	    sym->sym_Name = newName;
	    if (symMap->tail[ Hash( name ) ] == 0)
	    {
		symMap->head[ Hash( name ) ] = sym;
	    }
	    else
	    {
		symMap->tail[ Hash( name ) ]->sym_Next = sym;
	    }
	    symMap->tail[ Hash( name ) ] = sym;
	    sym->sym_Next = (struct Symbol *) 0;
	    sym->sym_Type = 0;
	    sym->sym_Value1 = 0;
	    sym->sym_Value2 = 0;
	}
	else
	{
	    free( sym );
	    sym = NULL;
	}
    }
    return( sym );
}


/* Locate a symbol by name in a specified SymbolMap */
struct Symbol *
FindSymbol( char *name, struct SymbolMap *symMap )
{
    struct Symbol *sym;

    for ( sym = symMap->head[ Hash( name ) ]; sym; sym = sym->sym_Next )
    {
	if ( !stricmp( name, sym->sym_Name ) )
	{
	    return( sym );
	}
    }
    return( NULL );
}


static void
ShowSymbol( struct Symbol *sym )
{
    PPrintf( "%10s  %08lx  %08lx  %s\n", TypeNames[ sym->sym_Type ],
	sym->sym_Value1, sym->sym_Value2, sym->sym_Name );
}


static void
ShowSymbolMap( struct SymbolMap *symMap )
{
    struct Symbol *sym;
    int h;

    for ( h = 0; h < HASHSIZE; h++ )
    {
	for ( sym = symMap->head[ h ]; sym; sym = sym->sym_Next )
	{
	    ShowSymbol( sym );
	}
    }
}


STRPTR
ShowTopSymMap( void )
{
    ShowSymbolMap( &TopSymMap );

    return( NULL );
}

void
UnBindSymbols( void )
{
    UnBindSymbolMap( &TopSymMap );
}

void
UnBindSymbolMap( struct SymbolMap *symMap )
{
    struct Symbol *sym;
    struct Symbol *succ;
    int h;

    for ( h = 0; h < HASHSIZE; h++ )
    {
	sym = symMap->head[ h ];
	while ( sym )
	{
	    succ = sym->sym_Next;
	    if ( sym->sym_Type == ACT_COMMAND )
	    {
		free( sym->sym_Args );
	    }
	    free( sym->sym_Name );
	    free( sym );
	    sym = succ;
	}
    }
}


void
dumpNearestSymbol( ULONG addr )
{
    struct Symbol *sym;
    struct Symbol *cp;
    long v;
    long lv;
    int	h;

    cp = 0;
    lv = 0x1000000;

    for ( h = 0; h < HASHSIZE; h++ )
    {
	for ( sym = TopSymMap.head[ h ]; sym; sym = sym->sym_Next )
	{
	    if ( ( sym->sym_Type == ACT_CONSTANT ) || ( sym->sym_Type == ACT_PGMSYMBOL ) )
	    {
		v = sym->sym_Value1;
		if ( ( addr - v >= 0 ) && ( addr - v < lv ) )
		{
		    cp = sym;
		    lv = addr - v;
		}
	    }

	}
    }
    if ( cp != 0 )
    {
	PPrintf( "%s + %lx   (%lx + %lx = %lx)\n", cp->sym_Name, lv,
		cp->sym_Value2, cp->sym_Value1, v );
    }
    else
    {
	PPrintf( "You're lost (%lx).\n", addr );
    }
}


STRPTR
ApproxSym( char *arg )
{
    ULONG addr;

    if ( parseAddress( arg, &addr ) )
    {
	dumpNearestSymbol( addr );
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}


STRPTR
ApproxSymIndirect( char *arg )
{
    ULONG addr;

    if ( parseAddress( arg, &addr ) )
    {
	addr = ReadLong( (APTR)addr ) & 0xFFFFFFFE;
	dumpNearestSymbol( addr );
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}


/* Create or replace the named symbol, giving it the
 * specified type and value.  Will reuse an existing
 * symbol of the same name.
 */
struct Symbol *
BindValue( char *name, short vtype, long value )
{
    struct Symbol *symbol;

    if ( ( symbol = FindSymbol( name, &TopSymMap ) ) ||
	( symbol = NewSymbol( name, &TopSymMap ) ) )
    {
	symbol->sym_Type = vtype;
	symbol->sym_Value1 = value;
    }
    return( symbol );
}


/* Create the named symbol, giving it the specified
 * type and value.  Does not check for duplicate
 * symbols, so it's faster than BindValue().
 */
struct Symbol *
BindValueQuick( char *name, short vtype, long value )
{
    struct Symbol *symbol;

    if ( symbol = NewSymbol( name, &TopSymMap ) )
    {
	symbol->sym_Type = vtype;
	symbol->sym_Value1 = value;
    }
    return( symbol );
}


void
BindCommand( char *name, CMDFUNC function )
{
    BindValue( name, ACT_COMMAND, (long) function );
}

void
BindCommand2( char *name, CMDFUNC function, char *arg1 )
{
    struct Symbol *sym;

    if ( sym = BindValue( name, ACT_COMMAND, (long) function ) )
    {
	char *args;
	if ( arg1 )
	{
	    if ( args = malloc( strlen( arg1 ) + 1 ) )
	    {
		strcpy( args, arg1 );
		sym->sym_Args = args;
	    }
	}
    }
}


void
TryIt( void )
{
    int     h;

    for (h = 0; h < HASHSIZE; h++)
    {
	TopSymMap.head[h] = 0;
	TopSymMap.tail[h] = 0;
    }

    InitTopMap ();
}


long
isAlpha( char ch )
{
    return( ( ( ch >= 'a' ) && ( ch <= 'z' ) ) || ( ( ch >= 'A' ) && ( ch <= 'Z' ) ) );
}


long
processLine( char *line )
{
    char command[ MAXTOKENLEN ];
    char *arg;
    long ok = TRUE;
    char singlekeycmd[ 2 ];
    struct Symbol *sym;

    while ( line )
    {
	if ( arg = parseToken( line, command ) )
	{
	    /* Non-alphanumeric commands (normally punctuation keys)
	     * don't require a delimeter, they just take the whole line
	     * as an argument.  Let's scan for one of those and handle
	     * it if there.
	     */
	    singlekeycmd[ 0 ] = command[ 0 ];
	    singlekeycmd[ 1 ] = '\0';
	    if ( ( !isAlpha( command[ 0 ] ) ) &&
		( sym = FindSymbol( singlekeycmd, &TopSymMap ) ) )
	    {
		/* Found such a command.  Its argument will be everything
		 * on the command line after the single character.  We
		 * use parseRemainder() to strip off the same leading blanks
		 * stripped by parseToken() above, without losing any of
		 * the rest of the line.
		 */
		arg = parseRemainder( line );
		line = evaluateSymbol( sym, arg+1 );
	    }
	    else if ( sym = FindSymbol( command, &TopSymMap ) )
	    {
		line = evaluateSymbol( sym, arg );
	    }
	    else if ( IsHexNum( command, &LastNum ) )
	    {
		CurrAddr = LastNum;
		ShowFrame();
		line = NULL;
	    }
	    else
	    {
		ok = FALSE;
		line = NULL;
	    }
	}
	else
	{
	    /* Just hitting <Enter> causes the current frame
	     * to be redisplayed
	     */
	    ShowFrame();
	    line = NULL;
	}
	line = parseRemainder( line );
    }
    return( ok );
}


/* Makes a symbol which is a synonym for another symbol.
 * argument line is supposed to be "verb equivalentverb"
 */
STRPTR
bindAlias( char *arg )
{
    char symbol[ MAXTOKENLEN ];
    char *equiv;
    struct Symbol *sym;

    if ( equiv = parseRemainder( parseToken( arg, symbol ) ) )
    {
	if ( sym = FindSymbol( equiv, &TopSymMap ) )
	{
	    if ( sym->sym_Type == ACT_COMMAND )
	    {
		BindCommand2( symbol, sym->sym_Function, sym->sym_Args );
	    }
	    else
	    {
		PPrintf("Cannot bind to '%s'\n", equiv);
	    }
	}
	else
	{
	    PPrintf("Symbol '%s' not found\n", equiv);
	}
    }
    else
    {
	PPrintf("Bad syntax\n");
    }

    return( NULL );
}


/* Makes a symbol which will invoke an external Wack program with
 * the supplied arguments.
 * The argument line is supposed to be "verb command [fixed_args]"
 * Later, when the line "verb [args]" is processed, the
 * command emitted will be:
 *
 *	command WACKPORT=wack.n [fixed_args] [args]
 */
STRPTR
bindXWack( char *arg )
{
    char symbol[ MAXTOKENLEN ];
    char *command;

    if ( command = parseRemainder( parseToken( arg, symbol ) ) )
    {
	BindCommand2( symbol, runWackCommand, command );
    }
    else
    {
	PPrintf("Bad syntax\n");
    }

    return( NULL );
}


/* Makes a symbol which will invoke a Shell command with the
 * supplied arguments.
 * The argument line is supposed to be "verb command [fixed_args]"
 * Later, when the line "verb [args]" is processed, the
 * Shell command emitted will be:
 *
 *	command [fixed_args] [args]
 */
STRPTR
bindSystem( char *arg )
{
    char symbol[ MAXTOKENLEN ];
    char *command;

    if ( command = parseRemainder( parseToken( arg, symbol ) ) )
    {
	BindCommand2( symbol, runCommand, command );
    }
    else
    {
	PPrintf("Bad syntax\n");
    }

    return( NULL );
}


/* Makes a symbol which will invoke an ARexx script with the
 * supplied arguments.
 * The argument line is supposed to be "verb command [fixed_args]"
 * Later, when the line "verb [args]" is processed, the
 * command emitted will be:
 *
 *	command [fixed_args] [args]
 */
STRPTR
bindRexx( char *arg )
{
    char symbol[ MAXTOKENLEN ];
    char *command;

    if ( command = parseRemainder( parseToken( arg, symbol ) ) )
    {
	BindCommand2( symbol, ARexx_Execute, command );
    }
    else
    {
	PPrintf("Bad syntax\n");
    }

    return( NULL );
}


/* Makes a symbol which is a constant.  Invoking this symbol
 * will set the current address to that constant.
 */
STRPTR
bindConstant( char *arg )
{
    char symbol[ MAXTOKENLEN ];
    ULONG value;

    if ( parseAddress( parseToken( arg, symbol ), &value ) )
    {
	BindValue( symbol, ACT_CONSTANT, value );
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}
