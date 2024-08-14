
/************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
************************************************************************/


/************************************************************************
*
* fsup.c
*
* Source Control
* ------ -------
* 
* $Header: fsup.c,v 27.1 85/06/24 13:36:59 neil Exp $
*
* $Locker: neil $
*
* $Log:	fsup.c,v $
* Revision 27.1  85/06/24  13:36:59  neil
* Upgrade to V27
* 
* Revision 26.2  85/06/18  13:55:17  neil
* removed dependency on test_rev
* 
* Revision 26.1  85/06/17  15:12:49  neil
* *** empty log message ***
* 
* 
************************************************************************/

/* Amiga MiniFileSys Support */

#include "exec/types.h"
#include "exec/memory.h"
#include "exec/io.h"
#include "libraries/dos.h"
#include "trackdisk.h"

#define BACKSPACE ('H' & 0x1f)

main()
{
    int unit;
    ULONG *buffer;
    unit = 0;
    buffer = 0;
    do {
	if( SetSignal( 0, SIGBREAKF_CTRL_C ) ) break;
    } while( command(&unit, &buffer) );

    freebuff( buffer, *buffer );
}

#define UNKNOWN		-1
#define FORMAT		0
#define READSEC		1
#define READWRITE	2
#define REPETATIVE	3
#define READTRACK	4
#define WRITEPATRN	5
#define SEEKTEST	6
#define SHORTDISC	7
#define MOTORTEST	8
#define DISCCHANGE	9
#define DISCWAIT	10
#define UPDATE		11
#define CLEAR		12
#define KICKSTART	13
#define DOSLABEL	14

/* these next must be at the end */
#define QUIT		15
#define	UNIT		16
#define	CTRL_D		17
#define	DELCHAR		18
#define	NEWLINECHAR	19
#define	RETURNCHAR	20
#define	NULLCHAR	21
#define	BUFFER		22
#define	RAWREAD		23
#define	RAWWRITE	24

#define COM(num)	(commandtab[num])
#define CMD(num,arg0,arg1)	DiskTest( num | (*unit<<16), arg0, arg1 )

char *commandtab[] = {
    "format",
    "readsec"
    "readwrite",
    "passes",
    "readtrack",
    "writepattern",
    "seektest",
    "shortdisc",
    "motortest",
    "discchange",
    "discwait",
    "update",
    "clear",
    "kickstart",
    "doslabel",
    "quit",
    "unit",
    "\4",
    "\177",
    "\n",
    "\r",
    "",
    "buffer",
    "rawread",
    "rawwrite",
    0
};

lookup( line )
char *line;
{
    char **cmd;

    for( cmd = commandtab; *cmd; cmd++ ) {
	if( cmpstr( *cmd, line ) ) return( cmd - commandtab );
    }
    return( UNKNOWN );
}


command(unit, bufferp)
int *unit;
ULONG **bufferp;
{
    char buf[80];
    int size;

    promptline( "disktest> ", buf, sizeof buf );

    switch( lookup( buf ) ) {
    case FORMAT:
	CMD( FORMAT, 0, 0 );
	break;

    case KICKSTART:
	promptline( "starting address? ", buf, sizeof buf );
	CMD( KICKSTART, hex_to_int( buf ), 0 );
	break;

    case READSEC:
	CMD( READSEC, 0, 0 );
	break;

    case READWRITE:
	CMD( READWRITE, 0, 0 );
	break;

    case READTRACK:
	CMD( READTRACK, 0, 0 );
	break;

    case REPETATIVE:
	promptline( "number of passes? ", buf, sizeof buf );
	CMD( REPETATIVE, num_to_int( buf ), 0 );
	break;

    case SEEKTEST:
	promptline( "track to seek to? ", buf, sizeof buf );
	CMD( SEEKTEST, num_to_int( buf ), 0 );
	break;
    
    case MOTORTEST:
	CMD( MOTORTEST, 0, 0 );
	break;

    case DISCCHANGE:
	CMD( DISCCHANGE, 0, 0 );
	break;

    case DISCWAIT:
	CMD( DISCWAIT, 0, 0 );
	break;

    case DOSLABEL:
	CMD( DOSLABEL, 0, 0 );
	break;

    case UPDATE:
	CMD( UPDATE, 0, 0 );
	break;

    case UNIT:
	*unit = (*unit + 1) & 0x3;
	printf( "\tswitching to unit %ld\n", *unit );
	break;

    case CLEAR:
	CMD( CLEAR, 0, 0 );
	break;

    case QUIT:
    case CTRL_D:
    case DELCHAR:
	return( 0 );

    default:
    case UNKNOWN:
	Usage();

    case NEWLINECHAR:
    case RETURNCHAR:
    case NULLCHAR:
	break;

    case BUFFER:
	promptline( "size of buffer? ", buf, sizeof buf );
	size = num_to_int( buf );
	if( size <= 0 ) break;

	/* free the old buffer (if allocated) */
	freebuff( *bufferp, **bufferp );

	*bufferp = (ULONG *) AllocMem( size, MEMF_CHIP );
	if( *bufferp ) {
	    printf( "buffer of %ld bytes is at 0x%lx\n", size, *bufferp );
	    **bufferp = size;
	} else {
	    printf( "could not allocate %ld bytes of CHIP memory\n", size );
	}
	break;

    case RAWREAD:
	if( ! *bufferp ) {
	    printf( "you must have already set up a buffer!\n" );
	} else {
	    promptline( "track number? ", buf, sizeof buf );
	    RawCommon( TD_RAWREAD, *bufferp, **bufferp, num_to_int( buf ),
		*unit );
	}
	break;

    case RAWWRITE:
	if( ! *bufferp ) {
	    printf( "you must have already set up a buffer!\n" );
	} else {
	    promptline( "track number? ", buf, sizeof buf );
	    RawCommon( TD_RAWWRITE, *bufferp, **bufferp, num_to_int( buf ),
		*unit );
	}
	break;
    }
    return( 1 );
}

freebuff( buf, size )
ULONG *buf;
ULONG size;
{
    if( buf ) {
	FreeMem( buf, size );
    }
}

RawCommon( command, buf, size, track, unit )
ULONG command;
UBYTE *buf;
ULONG size;
ULONG track;
ULONG unit;
{
    struct MsgPort *mp = NULL;
    struct IOStdReq *iob = NULL;

    mp = (struct MsgPort *) CreatePort( NULL, 0 );
    if( mp == NULL ) goto end;

    iob = (struct IOStdReq *) CreateStdIO( mp );
    if( iob == NULL ) goto freeport;

    if( OpenDevice( TD_NAME, unit, iob, 0 ) ) {
	printf( "Can't open %s\n", TD_NAME );
	goto freeiob;
    }

    
    iob->io_Offset = track;
    iob->io_Data = buf;
    iob->io_Length = size;
    iob->io_Command = command;

    DoIO( iob );

    printf( "RawCommon: command %ld returned %ld\n", command, iob->io_Error );

closedev:
    CloseDevice( iob );

freeiob:
    DeleteStdIO( iob );

freeport:
    DeletePort( mp );

end:
    ;

}


Usage()
{
    printf( "The following commands are understood:\n" );
    printf( "\tformat -- format the entire disc followed by checks\n" );
    printf( "\treadsec -- read the entire disc sector by sector\n" );
    printf( "\treadwrite -- write the entire disc then read\n" );
    printf( "\tpasses -- many read and write passes\n" );
    printf( "\treadtrack -- read only one sector per track\n" );
    printf( "\tseektest -- seek to the specified track\n" );
    printf( "\tmotortest -- cycle the motor on and off\n" );
    printf( "\tdiscchange -- report when disc is changed\n" );
    printf( "\tdiscwait -- software ints and disc change\n" );
    printf( "\tupdate -- test the update command\n" );
    printf( "\tclear -- test the clear command\n" );
    printf( "\tunit -- switch the unit we are testing\n" );
    printf( "\tkickstart -- write out a kickstart disk\n" );
    printf( "\tdoslabel -- put a dos header block on a disk\n" );
    printf( "\tbuffer -- allocate a memory buffer\n" );
    printf( "\trawread -- read a raw track of data\n" );
    printf( "\trawwrite -- read a raw track of data\n" );
    printf( "\tquit -- go away.  ^D and DEL also do this\n\n" );
}

copystr(dest, source) 
    char    *dest;
    char    *source;
{ 
    while (*dest++ = *source++);
}

copydata(source, dest, length)
	char *source;
	char  *dest;
	int  length;
{
    int     i;

    for (i = 0; i < length; i++)
	*dest++ = *source++;
}

cmpstr(astr, bstr)
	char *astr;
	char *bstr;
{
    while (*astr && *bstr && (*astr == *bstr))
   	astr++, bstr++;
    return ((*astr == *bstr));
}

short
promptline(prompt,input,maxlen)
    char *prompt;
    char *input;
    int maxlen;
{
    char   *ss;

    printf (prompt);

    for (ss = input--; maxlen > 0; maxlen--) {
	switch( *++input = getchar () ) {

	case '\r':
	case '\n':
	    goto out;
	}
    }
out:
    *input = '\0';
}

#ifdef undef
int ask(s)
    char *s;
{
    char    c;

    printf ("%s [y/n] ", s);
    while (1) {
	c = getchar ();
	if (c == 'y' || c == 'Y') {
	    printf ("yes\n");
	    return (1);
	}
	if (c == 'n' || c == 'N') {
	    printf ("no\n");
	    return (0);
	}
    }
}
#endif undef

dec_to_int( s )
char *s;
{
    int     n;
    char    sign;
    int	    ch;

    ch = *s++;

    for( n = 0; ch >= '0' && ch <= '9'; ch = *s++ ) {
	n = 10 * n + ch - '0';
    }
    return( n );
}

hex_to_int( s )
char *s;
{
    short   i;
    int     n;
    char    c;

    for( n = 0; ; ) {
	c = *s++;
	if( (c >= '0') && (c <= '9') ) {
	    n = (n << 4) + c - '0';
	} else {
	    if( (c >= 'a') && (c <= 'f') ) {
		c -= 'a';
	    } else {
		if( (c < 'A') || (c > 'F') ) {
		    break;
		}
		c -= 'A';
	    }
	    n = (n << 4) + c + 10;
	}
    }
    return( n );
}


oct_to_int( s )
char *s;
{
    short   i;
    int     n;
    char    c;

    for( n = 0; ; ) {
	c = *s++;
	if( (c >= '0') && (c <= '7') ) {
	    n = (n << 3) + c - '0';
	} else {
	    break;
	}
    }
    return( n );
}


num_to_int( s )
char *s;
{
    int sign = ' ';
    int ch;
    int n;
    int shift = 0;

    for( ; ; ) {
	switch( ch = *s++ ) {

	case ' ':
	case '\n':
	case '\t':
	    /* white space */
	    break;

	case '+':
	case '-':
	    /* sign bits */
	    if( sign != ' ' ) return( 0 );
	    sign = ch;
	    break;

	case 'k':
	case 'K':
	    /* multiply by 1024 */
	    shift += 10;
	    break;

	case 'm':
	case 'M':
	    /* multiply by 1024 * 1024 */
	    shift += 20;
	    break;

	default:
	    /* on to the numbers! */
	    goto out;
	}
    }


out:
    if( ch == '0' ) {
	if( ( ch = *s ) == 'X' || ch == 'x' ) {
	    /* we are in hex mode */
	    n = hex_to_int( ++s );
	} else {
	    /* we are in octal mode */
	    n = oct_to_int( s );
	}
    } else {
	/* decimal number */
	n = dec_to_int( --s );
    }

    n <<= shift;

    return( (sign == '-') ? -n : n );
}
