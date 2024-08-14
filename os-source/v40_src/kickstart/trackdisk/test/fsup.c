
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


#include "exec/types.h"
#include "exec/memory.h"
#include "exec/io.h"
#include "libraries/dos.h"
#include "devices/trackdisk.h"
#include "internal.h"

#define BACKSPACE ('H' & 0x1f)


main()
{
    struct store store;

    store.unit = 0;
    store.buffer = 0;
    store.buffsize = 0;
    store.lastcomm = 0;
    store.lasttrack = 0;

    do {
	if( testflags( SIGBREAKF_CTRL_C ) ) break;
    } while( command( &store ) );

    clearflags( SIGBREAKF_CTRL_C );
    clearflags( SIGBREAKF_CTRL_E );
    freebuff( store.buffer, store.buffsize );
}

clearflags( sig )
ULONG sig;
{
    return( SetSignal( 0, sig ) & sig );
}

testflags( sig )
ULONG sig;
{
    ULONG result;

    return( SetSignal( 0, 0 ) & sig );
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
#define	FINDSECS	25
#define	ALIGNTRACK	26
#define	VALIDATETRACK	27
#define	VALIDATEDISK	28
#define	GETTRACK	29
#define	LASTCOMMAND	30
#define	FIXBUFF		31
#define	READLOOP	32
#define	SYNCEDWRITE	33
#define	SYNCEDREAD	34
#define	SLIP		35
#define	HELP0		36
#define	HELP1		37
#define	HELP2		38
#define	MFMALIGN	39
#define	NUMTRACKS	40
#define	DRIVETYPE	41
#define	VALIDATEEA	42

#define COM(num)	(commandtab[num])
#define CMD(num,arg0,arg1)	DiskTest( num | (st->unit<<16), arg0, arg1 )

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
    "findsecs",
    "aligntrack",
    "validatetrack",
    "validatedisk",
    "gettrack",
    ".",
    "fixbuff",
    "readloop",
    "syncedwrite",
    "syncedread",
    "slip",
    "help",
    "h",
    "?",
    "mfmalign",
    "numtracks",
    "drivetype",
    "validateea",
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


command( st )
struct store *st;
{
    char buf[80];
    int size;
    int command;
    int thiscommand;
    int tmp;

    promptline( "disktest> ", buf, sizeof buf );

    thiscommand = lookup( buf );
    command = thiscommand;
    if( command == LASTCOMMAND ) {
	command = st->lastcomm;
    } else {
	st->lastcomm = command;
    }

    switch( command ) {
    case FORMAT:
	CMD( FORMAT, 0, 0 );
	break;

    case KICKSTART:
	if( buffok( st ) ) {
	    CMD( KICKSTART, st->buffer, 0 );
	}
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
	tmp = getnum( "number of passes? ", buf, sizeof buf );
	CMD( REPETATIVE, tmp, 0 );
	break;

    case SEEKTEST:
	tmp = getnum( "track to seek to? ", buf, sizeof buf );
	CMD( SEEKTEST, tmp, 0 );
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
	st->unit = (st->unit + 1) & 0x3;
	printf( "\tswitching to unit %ld\n", st->unit );
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
	printf( "Command not (yet?) understood.  Type 'help' for help\n" );
	break;

    case HELP0:
    case HELP1:
    case HELP2:
	Usage();
	break;

    case NEWLINECHAR:
    case RETURNCHAR:
    case NULLCHAR:
	break;

    case BUFFER:
	size = getnum( "size of buffer? ", buf, sizeof buf );
	if( size <= 0 ) break;

	/* free the old buffer (if allocated) */
	freebuff( st->buffer, st->buffsize );

	st->buffer = (ULONG *) AllocMem( size, MEMF_CHIP );
	if( st->buffer ) {
	    printf( "buffer of %ld bytes is at 0x%lx\n", size, st->buffer );
	    st->buffsize = size;
	} else {
	    printf( "could not allocate %ld bytes of CHIP memory\n", size );
	}
	break;

    case RAWREAD:
	if( buffok( st ) ) {
	    st->lasttrack = getnum( "track number? ", buf, sizeof buf );
	    RawCommon( TD_RAWREAD, st->buffer, st->buffsize,
		st->lasttrack, st->unit, 1 );
	}
	break;

    case RAWWRITE:
	if( buffok( st ) ) {
	    st->lasttrack = getnum( "track number? ", buf, sizeof buf );
	    RawCommon( TD_RAWWRITE, st->buffer, st->buffsize,
		st->lasttrack, st->unit, 1 );
	}
	break;

    case FINDSECS:
	if( buffok( st ) ) findsecs( st->buffer, st->buffsize );
	break;

    case ALIGNTRACK:
	if( buffok( st ) ) aligntrack( st->buffer, st->buffsize );
	break;

    case VALIDATETRACK:
	if( buffok( st ) ) {
	    validatetrack( st->buffer, st->buffsize, -1 );
	}
	break;

    case VALIDATEDISK:
	validatedisk( st );
	break;

    case VALIDATEEA:
	validateea( st );
	break;

    case GETTRACK:
	if( buffok( st ) ) {
	    if( thiscommand != LASTCOMMAND ) {
		st->lasttrack = getnum( "track number? ", buf, sizeof buf );
	    }
	    RawCommon( TD_RAWREAD, st->buffer, st->buffsize,
		st->lasttrack, st->unit, 1 );
	    aligntrack( st->buffer, st->buffsize );
	    findsecs( st->buffer, st->buffsize );
	}
	break;

    case FIXBUFF:
	if( buffok( st ) ) {
	    fixbuff( st );
	}
	break;

    case READLOOP:
	if( buffok( st ) ) {
	    if( thiscommand != LASTCOMMAND ) {
		st->lasttrack = getnum( "track number? ", buf, sizeof buf );
	    }
	    while( ! clearflags( SIGBREAKF_CTRL_C ) ) {
		RawCommon( TD_RAWREAD, st->buffer, st->buffsize,
		    st->lasttrack, st->unit, 0 );
	    }
	    Motor( 0, st->unit );
	}

    case SYNCEDWRITE:
	if( buffok( st ) ) {
	    if( thiscommand != LASTCOMMAND ) {
		st->lasttrack = getnum( "track number? ", buf, sizeof buf );
	    }
	    while( ! testflags( SIGBREAKF_CTRL_C ) ) {
		SyncedRawCommon( TD_RAWWRITE, st->buffer, st->buffsize,
		    st->lasttrack, st->unit, 0, IOTDF_INDEXSYNC );
	    }
	    Motor( 0, st->unit );
	    clearflags( SIGBREAKF_CTRL_C );
	}

    case SYNCEDREAD:
	if( buffok( st ) ) {
	    st->lasttrack = getnum( "track number? ", buf, sizeof buf );
	    SyncedRawCommon( TD_RAWREAD, st->buffer, st->buffsize,
		st->lasttrack, st->unit, 1, IOTDF_INDEXSYNC );
	}
	break;

    case SLIP:
	if( buffok( st ) ) {
	    Slip( st->buffer, st->buffsize, 1 );
	}
	break;

    case NUMTRACKS:
	printf( "%ld tracks\n", DoCommand( st->unit, TD_GETNUMTRACKS ) );
	break;

    case DRIVETYPE:
	printf( "drive type 0x%lx\n", DoCommand( st->unit, TD_GETDRIVETYPE ) );
	break;

    case MFMALIGN:
	if( buffok( st ) ) {
	    MfmAlign( st->buffer, st->buffsize );
	}
	break;

    }

    return( 1 );
}


buffok( st )
struct store *st;
{
    if( ! st->buffer ) {
	printf( "you must have already set up a buffer!\n" );
    }
    return( (ULONG) st->buffer );
}

freebuff( buf, size )
ULONG *buf;
ULONG size;
{
    if( buf ) {
	FreeMem( buf, size );
    }
}

RawCommon( command, buf, size, track, unit, motoroff )
ULONG command;
UBYTE *buf;
ULONG size;
ULONG track;
ULONG unit;
{
    return( SyncedRawCommon( command, buf, size, track, unit, motoroff, 0 ) );
}

SyncedRawCommon( command, buf, size, track, unit, motoroff, flags )
ULONG command;
UBYTE *buf;
ULONG size;
ULONG track;
ULONG unit;
UBYTE flags;
{
    struct IOStdReq *iob;
    ULONG error = ~0;

    iob = (struct IOStdReq *) CreateTDIOB( unit );
    if( iob == NULL ) goto end;

/*printf(*/
/*"RawCommon: cmd %ld, buf 0x%lx, size %ld, track %ld, unit %ld, motor %ld\n",*/
/*command, buf, size, track, unit, motoroff );*/
    
    iob->io_Offset = track;
    iob->io_Data = buf;
    iob->io_Length = size;
    iob->io_Command = command;
    iob->io_Flags = flags | IOF_QUICK;

    BeginIO( iob );
    WaitIO( iob );
    error = iob->io_Error;

    if( motoroff ) {
	printf( "RawCommon: command %ld returned %ld\n",
	    command, error );

	iob->io_Command = TD_MOTOR;
	iob->io_Length = 0;

	DoIO( iob );
    }

    DeleteTDIOB( iob );

end:
    return( error );
}


GetNumTracks( unit )
ULONG unit;
{
    struct IOStdReq *iob;
    ULONG result = 0;

    iob = (struct IOStdReq *) CreateTDIOB( unit );
    if( iob == NULL ) goto end;

    iob->io_Command = TD_GETNUMTRACKS;

    DoIO( iob );
    result = iob->io_Actual;

    DeleteTDIOB( iob );

end:
    return( result );

}

Motor( onoff, unit )
ULONG onoff;
ULONG unit;
{
    struct IOStdReq *iob;
    ULONG error = ~0;

    iob = (struct IOStdReq *) CreateTDIOB( unit );
    if( iob == NULL ) goto end;

    iob->io_Command = TD_MOTOR;
    iob->io_Length = onoff;

    DoIO( iob );
    error = iob->io_Error;

    DeleteTDIOB( iob );

end:
    return( error );

}



CreateTDIOB( unit )
ULONG unit;
{
    struct MsgPort *mp = NULL;
    struct IOStdReq *iob = NULL;
    ULONG error = ~0;

    mp = (struct MsgPort *) CreatePort( NULL, 0 );
    if( mp == NULL ) goto end;

    iob = (struct IOStdReq *) CreateStdIO( mp );
    if( iob == NULL ) goto freeport;

    if( OpenDevice( TD_NAME, unit, iob, TDF_ALLOW_NON_3_5 ) ) {
	printf( "Can't open %s\n", TD_NAME );
	goto freeiob;
    }

    goto end;

freeiob:
    DeleteStdIO( iob );

freeport:
    DeletePort( mp );
    iob = NULL;

end:
    return( (ULONG) iob );

}


DeleteTDIOB( iob )
struct IOStdReq *iob;
{
    struct MsgPort *mp = iob->io_Message.mn_ReplyPort;

    CloseDevice( iob );

    DeleteStdIO( iob );

    DeletePort( mp );
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
    printf( "\tfindsecs -- find the sector headers\n" );
    printf( "\taligntrack -- allign the track on word boundaries\n" );
    printf( "\tvalidatetrack -- check for errors in the current buffer\n" );
    printf( "\tvalidatedisk -- check for errors anywhere on the disk\n" );
    printf( "\tgettrack -- rawread, aligntrack, & findsecs\n" );
    printf( "\tslip -- ??\n" );
    printf( "\tmfmalign -- ??\n" );
    printf( "\tnumtracks -- ask how many tracks\n" );
    printf( "\tdrivetype -- ask what type of drive\n" );
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


getnum( prompt, buf, sizeofbuf )
char *prompt;
char *buf;
ULONG sizeofbuf;
{
    promptline( prompt, buf, sizeofbuf );
    return( num_to_int( buf ) );
}


Slip( buffer, length, bits )
ULONG *buffer;
ULONG length;
int bits;
{
    ULONG this, next;

    this = *buffer;
    next = buffer[1];
    while( length >= sizeof( ULONG ) ) {

	*buffer++ = (this << bits) | (next >> 32 - bits); 
    
	this = next;
	next = buffer[1];
	length -= sizeof( ULONG );
    }
}


UWORD sliptable[] = {
    0x9254,	/* 0 bits slipped */
    0x492A,	/* 1 bits slipped */
    0x2495,	/* 2 bits slipped */
    0x924A,	/* 3 bits slipped */
    0x4925,	/* 4 bits slipped */
    0xA492,	/* 5 bits slipped */
    0x5249,	/* 6 bits slipped */
    0xA924,	/* 7 bits slipped */
    0x5492,	/* 8 bits slipped */
    0x2A49,	/* 9 bits slipped */
    0x9524,	/* A bits slipped */
    0x4A92,	/* B bits slipped */
    0x2549,	/* C bits slipped */
    0x92A4,	/* D bits slipped */
    0x4952,	/* E bits slipped */
    0x24A9	/* F bits slipped */
};


MfmAlign( buffer, length )
UWORD *buffer;
ULONG length;
{
    ULONG data;
    UWORD *st;
    int bits;


    data = *buffer;

    for( bits = 0, st = sliptable; bits < 16; bits++, st++ ) {
	if( *st == data ) break;
    }

    if( bits == 16 ) {
	printf( "MfmAlign: can't find a match for the buffer\n" );
	return( -1 );
    }

    if( bits ) {
	Slip( buffer, length, bits );
    }

    return( bits );
}


DoCommand( unit, command )
{
    struct IOStdReq *iob;
    long result;

    iob = (struct IOStdReq *) CreateTDIOB( unit );
    if( iob == NULL ) return( -1 );

    iob->io_Command = command;
    iob->io_Flags = IOF_QUICK;

    BeginIO( iob );
    WaitIO( iob );

    result = iob->io_Actual;

    DeleteTDIOB( iob );

    return( result );
    
}

