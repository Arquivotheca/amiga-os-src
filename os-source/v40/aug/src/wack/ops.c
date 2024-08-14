/*
 * Amiga Grand Wack
 *
 * ops.c -- Code for minor Wack command operations.
 *
 * $Id: ops.c,v 39.12 93/11/05 14:58:16 jesup Exp $
 *
 * Contains code for many of the smaller Wack operations, including
 * base and BPTR conversion, indirection, token evaluation, etc.
 *
 */

#include "symbols.h"
#include "special.h"
#include "std.h"
#include "parse.h"

#include "sys_proto.h"
#include "asm_proto.h"
#include "sat_proto.h"
#include "find_proto.h"
#include "parse_proto.h"
#include "define_proto.h"
#include "link_proto.h"
#include "decode_proto.h"
#include "ops_proto.h"
#include "io_proto.h"

extern long FrameSize;
extern unsigned long CurrAddr;
extern unsigned long SpareAddr;
extern unsigned long DisasmAddr;

/* In the dump, insert a space every this number of bytes */
long DumpSpacing = 4;
long DumpASCII = 1;
long ReadSize = 4;

#define MAXINDIR 200	/* maximum number of indirections */

long indirStack[MAXINDIR];
short indirCount = 0;


/**********************************************************************/

/* Frame Position Movement */

STRPTR
NextWord( char *arg )
{
    CurrAddr += 2;
    while ( *arg++ == '>' )
    {
	CurrAddr += 2;
    }
    ShowFrame();

    return( NULL );
}

STRPTR
BackWord( char *arg )
{
    CurrAddr -= 2;
    while ( *arg++ == '<' )
    {
	CurrAddr -= 2;
    }
    ShowFrame();

    return( NULL );
}

STRPTR
NextFrame( char *arg )
{
    CurrAddr += FrameSize;
    ShowFrame();
    while ( *arg++ == '.' )
    {
	CurrAddr += FrameSize;
	ShowFrame();
    }

    return( NULL );
}

STRPTR
BackFrame( char *arg )
{
    CurrAddr -= FrameSize;
    ShowFrame();
    while ( *arg++ == ',' )
    {
	CurrAddr -= FrameSize;
	ShowFrame();
    }

    return( NULL );
}

/**********************************************************************/

STRPTR
Disassemble( char *arg )
{
    unsigned long endaddr;
    NewLine();
    do
    {
	endaddr = FrameSize + DisasmAddr;
	while ( DisasmAddr < endaddr )
	{
	    DisasmAddr += (ULONG)do_decode( (caddr_t) DisasmAddr );
	    NewLine();
	}
    }
    while ( *arg++ == ';' );

    return( NULL );
}

/**********************************************************************/

STRPTR
ShiftBPtr( char *arg )
{
    long bptr;

    if ( parseHexNum( arg, &bptr, NULL ) )
    {
	PPrintf (" => %lx\n", bptr<<2);
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

STRPTR
ToDec( char *arg )
{
    long num;

    if ( parseHexNum( arg, &num, NULL ) )
    {
	PPrintf (" => %ld\n", num);
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

STRPTR
ToHex( char *arg )
{
    long num;

    if ( parseDecNum( arg, &num ) )
    {
	PPrintf (" => %lx\n", num);
    }
    else
    {
	PPrintf("Bad syntax: expected decimal argument\n");
    }

    return( NULL );
}

STRPTR
ClearScreen( void )
{
    Putchar ('L'-'A'+1);

    return( NULL );
}

/**********************************************************************/

/* Puts current address on stack, bumps stack down if full
 * (so we lose old stacked values, instead of being unable
 * to do new indirections
 */

void
stackCurrent( void )
{
    if (indirCount < MAXINDIR)
    {
	indirStack[indirCount++] = CurrAddr;
    }
    else
    {
	int i;
	for ( i = 0; i < MAXINDIR-1; i++ )
	{
	    indirStack[ i ] = indirStack[ i+1 ];
	}
	indirStack[MAXINDIR-1] = CurrAddr;
    }
}

STRPTR
Indirect( STRPTR arg )
{
    stackCurrent();

    CurrAddr = ReadLong( (APTR) CurrAddr ) & 0xfffffffe;
    ShowFrame();

    return( arg );
}

/**********************************************************************/

STRPTR
IndirectBptr( STRPTR arg )
{
    stackCurrent();

    CurrAddr = ReadLong( (APTR) CurrAddr ) << 2;
    ShowFrame();

    return( arg );
}

/**********************************************************************/

STRPTR
Exdirect( STRPTR arg )
{
    STRPTR result = arg;
    if (indirCount > 0)
    {
	CurrAddr = indirStack[--indirCount];
	ShowFrame();
    }
    else
    {
	PPrintf("No indirections to undo\n");
	result = NULL;
    }

    return( result );
}

/**********************************************************************/

/* We now keep a spare address pointer you can swap with the
 * current one.  Also, certain functions that display a particular
 * address (eg.  xxxbase, awindow, etc.) set up the SpareAddr
 * variable.  This routine will swap the current address with
 * the spare one.
 */
STRPTR
swapSpareAddr( STRPTR arg )
{
    unsigned long temp;

    temp = CurrAddr;
    CurrAddr = SpareAddr;
    SpareAddr = temp;
    ShowFrame();

    return( arg );
}

/**********************************************************************/

/* These typedefs are used to invoke the single and double argument
 * bound commands.
 */
typedef STRPTR (*CMDFUNC_1ARG)( char *arg );
typedef STRPTR (*CMDFUNC_2ARGS)( char *arg1, char *arg2 );

STRPTR
evaluateSymbol( struct Symbol *sym, char *argStr )
{
    STRPTR result = NULL;

    switch (sym->sym_Type)
    {
	case ACT_CONSTANT: 
	case ACT_PGMSYMBOL:
	    CurrAddr = sym->sym_Value1;
	    ShowFrame();
	    break;

	case ACT_COMMAND:
	    if ( sym->sym_Args )
	    {
		result = (*((CMDFUNC_2ARGS)sym->sym_Function))( sym->sym_Args, argStr );
	    }
	    else
	    {
		result = (*((CMDFUNC_1ARG)sym->sym_Function))( argStr );
	    }
	    break;

	default: 
	    PPrintf("Unknown symbol type\n");
    }
    return( result );
}


/**********************************************************************/

STRPTR
SizeFrame( char *arg )
{
    long newframesize;

    if ( parseHexNum( arg, &newframesize, NULL ) )
    {
	FrameSize = newframesize;
	ShowFrame();
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

/**********************************************************************/

STRPTR
NextCount( char *arg )
{
    long i;

    if ( parseHexNum( arg, &i, NULL ) )
    {
	CurrAddr += i;
	ShowFrame();
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

/**********************************************************************/

STRPTR
BackCount( char *arg )
{
    long i;

    if ( parseHexNum( arg, &i, NULL ) )
    {
	CurrAddr -= i;
	ShowFrame();
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

/**********************************************************************/

STRPTR
Resume( void )
{
    Putchar ('\n');
    freeCPU();

    return( NULL );
}

/**********************************************************************/

STRPTR
Call( char *arg )
{
    long addr;

    if ( arg = parseHexNum( arg, &addr, NULL ) )
    {
	CallFunction((APTR) addr);
    }
    else
    {
	if (*arg)
		BadSyntax();
        else
		CallFunction((APTR) CurrAddr);
    }

    Putchar ('\n');

    return( NULL );
}

/**********************************************************************/

STRPTR
SetBreakPoint( void )
{
#if 000
    long i;

    PPrintf ("%08lx %ld ^", CurrAddr, getBrkCnt (CurrAddr));
    if ( parseAddress(arg, &i) )
    {
	setBrkPt (i);
    }
    else
        Putchar ('\n');
#endif

    return( NULL );
}

/**********************************************************************/

STRPTR
AssignMem( char *arg )
{
    LONG num;
    ULONG writesize;

    if ( parseHexNum( arg, &num, &writesize ) )
    {
	if ( writesize == 1 )
	{
	    WriteByte( (APTR)CurrAddr, num );
	}
	else if ( writesize == 2 )
	{
	    WriteWord( (APTR)CurrAddr, num );
	}
	else
	{
	    WriteLong( (APTR)CurrAddr, num );
	}
        ShowFrame();
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

/**********************************************************************/

STRPTR
Nothing1( void )
{
    return( NULL );
}

STRPTR
Nothing2( void )
{
    PPrintf("Nothing happens.\n");

    return( NULL );
}

STRPTR
Nothing3( void )
{
    PPrintf("You are in a maze of twisty subroutines, all alike.\n");

    return( NULL );
}

void
BadSyntax( void )
{
    PPrintf("Bad syntax: expected hex argument\n");
}

/**********************************************************************/

STRPTR
setDump( char *arg )
{
    char token[ MAXTOKENLEN ];
    int showstatus = 1;

    while ( arg = parseToken( arg, token ) )
    {
	if ( !stricmp( token, "narrow" ) )
	{
	    DumpSpacing = 4;
	    showstatus = 0;
	}
	else if ( !stricmp( token, "wide" ) )
	{
	    DumpSpacing = 2;
	    showstatus = 0;
	}
	else if ( !stricmp( token, "ascii" ) )
	{
	    DumpASCII = 1;
	    showstatus = 0;
	}
	else if ( !stricmp( token, "noascii" ) )
	{
	    DumpASCII = 0;
	    showstatus = 0;
	}
	else if ( !stricmp( token, "word" ) )
	{
	    ReadSize = 2;
	    showstatus = 0;
	}
	else if ( !stricmp( token, "longword" ) )
	{
	    ReadSize = 4;
	    showstatus = 0;
	}
	else
	{
	    PPrintf("Bad syntax: setdump (WORD|LONGWORD) (WIDE|NARROW) (ASCII|NOASCII)\n");
	}
    }
    if ( showstatus )
    {
	PPrintf( "Dump mode: read by %sword, %s output, %s ASCII\n",
	    ReadSize == 4 ? "long" : "",
	    DumpSpacing == 4 ? "narrow" : "wide",
	    DumpASCII ? "show" : "hide" );
    }

    return( NULL );
}

void
asciiByte( UBYTE data, char **strptr )
{
    char first = '.';
    char second = '.';
    if ( ( data ) && ( data < 0x80 ) )
    {
	if ( data > 31 )
	{
	    first = ' ';
	    second = (char) data;
	}
	else
	{
	    first = '^';
	    second = (char) (data + 'A' - 1 );
	}
    }
    *(*strptr)++ = first;
    *(*strptr)++ = second;
}

void
ShowWord( unsigned long data, char **asciibufptr, long bytecount )
{
    PPrintf( "%04lx", data );
    asciiByte( data >> 8, asciibufptr );
    asciiByte( data & 0xFF, asciibufptr );
    if ( ( bytecount % DumpSpacing ) == 0 )
    {
	PPrintf(" ");
    }
}

void
ShowFrame(void)
{
    long count = FrameSize;
    long addr = CurrAddr;
    unsigned long data;
    long bytecount;
    char ascii[33];
    char *asciiptr;

    DisasmAddr = CurrAddr;
    while ( count > 0 )
    {
	bytecount = 0;
	asciiptr = ascii;
	PPrintf("%08lx ", addr );
	while ( ( count > 0 ) && ( bytecount < 16 ) )
	{
	    if ( count > 0 )
	    {
		if ( ( ReadSize == 2 ) || ( count < 4 ) )
		{
		    data = (unsigned short)ReadWord( (void *)addr );
		    bytecount += 2;
		    ShowWord( data, &asciiptr, bytecount );
		    addr += 2;
		    count -= 2;
		}
		else
		{
		    data = ReadLong( (void *)addr );
		    bytecount += 2;
		    ShowWord( data >> 16, &asciiptr, bytecount );
		    bytecount += 2;
		    ShowWord( data & 0xFFFF, &asciiptr, bytecount );
		    addr += 4;
		    count -= 4;
		}
	    }
	}
	*asciiptr = '\0';
	if ( DumpASCII )
	{
	    PPrintf("%s", ascii);
	}
	NewLine();
    }
}

/**********************************************************************/
