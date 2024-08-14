/*
 * Amiga Grand Wack
 *
 * io.c -- Wack's interface functions.
 *
 * $Id: io.c,v 39.20 93/09/08 17:17:29 peter Exp $
 *
 * These routines build and manage the Wack interface, including its
 * window and character I/O.  As well, a lot of the Amiga-specific
 * initialization stuff is placed here.
 *
 */

#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <dos/dos.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "std.h"
#include "symbols.h"
#include "parse.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern APTR LinkLibBase;

#include "sys_proto.h"
#include "asm_proto.h"
#include "parse_proto.h"
#include "define_proto.h"
#include "io_proto.h"
#include "ops_proto.h"
#include "menus_proto.h"
#include "rexxhandler_proto.h"
#include "wackbase_proto.h"
#include "simplerexx.h"

/*---------------------------------------------------------------------------*/

/* These globals are Amiga system and user-interface resources: */

struct Library *SysBase;
struct Library *DOSBase = NULL;
struct Library *GadToolsBase = NULL;
struct Library *IntuitionBase = NULL;
struct Window *iowin = NULL;
struct Screen *mysc = NULL;
struct VisualInfo *vi = NULL;
struct WackLibBase *WackBase = NULL;
struct Task *WackTask;

extern UWORD far VERSION;
extern UWORD far REVISION;

/* These globals are used to handle our input/output: */
BPTR inputfh = NULL;
BPTR outputfh = NULL;


static BPTR capturefh = NULL;
#define OUTBUFFERSIZE 512
#define INBUFFERSIZE 120
static struct MsgPort *prport = NULL;
static struct MsgPort *userport = NULL;
static struct RDArgs *rdargs = NULL;
static char WinTitle[80];
static ULONG rexxsig = 0;
static char inbuffer[ INBUFFERSIZE ];
static char outbuffer[ OUTBUFFERSIZE ];
static char async_ch;
static struct DosPacket *read_packet = NULL;
#define ERRORBUFFERSIZE 100
static char errorbuffer[ ERRORBUFFERSIZE ];

#define PROMPTLEN 32
static char prompt[ PROMPTLEN+1 ] = "\n=> ";


/* These globals are for processing of remote errors: */

static long freshline = 1;
long remote_error = 0;
extern long store_invalid_address;
extern void *invalid_address;

extern struct ARexxContext *RexxContext;

static char *rexx_startup_file;
long default_rexx_startup = 1;

extern LONG proceed;

extern long CurrAddr;

static struct IBox zoombox =
{
    -1, -1,
    32767, 32767,
};

/*---------------------------------------------------------------------------*/

/* This is the command template. */
#define TEMPLATE    "TARGET,PUBSCREEN/K,FROM/K"

#define OPT_TARGET	0
#define OPT_PUBSCREEN	1
#define OPT_FROM	2
#define OPT_COUNT	3

static LONG opts[OPT_COUNT];

struct Window *
OpenWindowTags( struct NewWindow *nw, ULONG firsttag, ... )
{
    return( OpenWindowTagList( nw, (struct TagItem *) &firsttag ) );
}


/* Get the basic environment, such as libraries, ARexx, and arguments
 * up and running.  Returns the OPT_TARGET argument, or -1 for failure.
 */
STRPTR
openEnvironment( void )
{
    STRPTR errorstr = "Failed to open V37+ DOS library";

    SysBase = (*((struct Library **) 4));

    WackTask = FindTask( NULL );

    if ( DOSBase = OpenLibrary("dos.library", 37 ) )
    {
	if ( rdargs = ReadArgs( TEMPLATE, opts, NULL ) )
	{
	    errorstr = "Failed to open V37+ Intuition library";
	    if ( IntuitionBase = OpenLibrary("intuition.library", 37 ) )
	    {
		errorstr = "Failed to open V37+ GadTools library";
		if ( GadToolsBase = OpenLibrary("gadtools.library", 37 ) )
		{
		    errorstr = "Failed to initialize Wack port";
		    if ( WackBase = InitWackBase() )
		    {
			errorstr = NULL;
			rexxsig = ARexx_Init();
		    }
		}
	    }
	}
        else
	{
	    Fault( IoErr(), NULL, errorbuffer, ERRORBUFFERSIZE );
	    errorstr = errorbuffer;
	}
    }
    return( errorstr );
}


/* Return the target command line option */
STRPTR
getTarget( void )
{
    return( (STRPTR) opts[ OPT_TARGET ] );
}


/* Brings up the window itself */
STRPTR
openWin()
{
    STRPTR errorstr = "Failed to allocate resources";

    if ( ( userport = CreateMsgPort() ) && ( prport = CreateMsgPort() ) )
    {
	errorstr = "Public screen not found";
	if ( mysc = LockPubScreen( (STRPTR)opts[ OPT_PUBSCREEN ] ) )
	{
	    errorstr = "Failed to allocate resources";

	    if ( vi = GetVisualInfoA( mysc, NULL ) )
	    {
		sprintf( WinTitle, "Amiga Wack %ld.%ld (%s) [%s]",
		    VERSION, REVISION, LinkConnection(), WackPortName() );

		errorstr = "Failed to open window";

		if ( iowin = OpenWindowTags( NULL,
		    WA_Width, mysc->Width,
		    WA_Height, mysc->Height/2,
		    WA_MinWidth, 80,
		    WA_MinHeight, 20,
		    WA_MaxWidth, ~0,
		    WA_MaxHeight, ~0,
		    WA_Title, WinTitle,
		    WA_SizeGadget, TRUE,
		    WA_DragBar, TRUE,
		    WA_CloseGadget, TRUE,
		    WA_DepthGadget, TRUE,
		    WA_SimpleRefresh, TRUE,
		    WA_NewLookMenus, TRUE,
		    WA_Activate, TRUE,
		    WA_PubScreen, mysc,
		    WA_Zoom, &zoombox,
		    WA_SizeBBottom, TRUE,
		    TAG_DONE ) )
		{
		    char conname[100];
		    iowin->UserPort = userport;
		    ModifyIDCMP( iowin, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK );

		    sprintf( conname, "CON://///WINDOW%lx", iowin );
		    if ( inputfh = Open(conname, MODE_OLDFILE ) )
		    {
			struct MsgPort *contask = GetConsoleTask();
			SetConsoleTask( ((struct FileHandle *)BADDR(inputfh))->fh_Type );
			if ( outputfh = Open("*", MODE_NEWFILE ) )
			{
			    errorstr = "Failed to allocate resources";
			    if ( init_Menus() )
			    {
				rexx_startup_file = "rexx:startup.wack";
				if ( opts[ OPT_FROM ] )
				{
				    default_rexx_startup = 0;
				    rexx_startup_file = (char *)opts[ OPT_FROM ];
				    if ( !stricmp( rexx_startup_file, "nil:" ) )
				    {
					rexx_startup_file = NULL;
				    }
				}
				errorstr = NULL;
			    }
			}
			SetConsoleTask( contask );
		    }
		    showCursor( FALSE );
		}
	    }
	}
    }
    return( errorstr );
}


/* Take down the window */
void
closeWin( void )
{
    exit_Menus();

    if ( capturefh )
    {
	Close( capturefh );
    }

    if ( outputfh )
    {
	Close( outputfh );
	outputfh = NULL;
    }
    if ( inputfh )
    {
	/* Closing a CON: filehandle closes the window!  AAACK. 
	 * Can you believe it?  Worse, since that window is
	 * closed on CON:'s task, the wrong signal would be freed
	 * if we let CloseWindow() free the UserPort.  Thus, we
	 * have to do it ourselves.  This little snippet is really
	 * a part of CloseWindowSafely(), but we skip the message
	 * stripping (since the port is not shared) and we skip
	 * the closing, since CON: does it "for" us. :-(
	 */
	Forbid();
	iowin->UserPort = NULL;
	ModifyIDCMP( iowin, 0 );
	Permit();

	iowin = NULL;
	Close( inputfh );
	inputfh = NULL;
    }

    if ( userport )
    {
	DeleteMsgPort( userport );
    }

    if ( prport )
    {
	/* Is there an outstanding read request? */
	if ( read_packet )
	{
	    FreeDosObject( DOS_STDPKT, read_packet );
	    read_packet = NULL;
	}
	DeleteMsgPort( prport );
    }

    if ( iowin )
    {
	showCursor( TRUE );
	CloseWindow( iowin );
	iowin = NULL;
    }

    if ( vi )
    {
	FreeVisualInfo( vi );
	vi = NULL;
    }
    if ( mysc )
    {
	UnlockPubScreen( NULL, mysc );
	mysc = NULL;
    }
}


/* Perform final shutdown */
void
closeEnvironment( STRPTR errorstr )
{
    ARexx_Exit();
    FreeWackBase( WackBase );

    if ( GadToolsBase )
    {
	CloseLibrary( GadToolsBase );
	GadToolsBase = NULL;
    }
    if ( IntuitionBase )
    {
	CloseLibrary( IntuitionBase );
	IntuitionBase = NULL;
    }
    if ( rdargs )
    {
	FreeArgs( rdargs );
    }
    if ( DOSBase )
    {
	if ( errorstr )
	{
	    PutStr( errorstr );
	    PutStr( "\n" );
	}
	CloseLibrary( DOSBase );
	DOSBase = NULL;
    }
}


/* Execute the startup.wack or specified FROM file */
void
executeStartupFile( void )
{
    BPTR startup_lock;

    if ( rexx_startup_file )
    {
	if ( startup_lock = Lock( rexx_startup_file, ACCESS_READ ) )
	{
	    UnLock( startup_lock );
	    ARexx_Execute( rexx_startup_file, "" );
	}
	else
	{
	    if ( !default_rexx_startup )
	    {
		PPrintf( "Wack startup file %s not found.\n", rexx_startup_file );
	    }
	}
    }
}

/* Print a new-line */
void
NewLine( void )
{
    PPrintf("\n");
}

void
writeCapture( STRPTR buffer, LONG len )
{
    if ( capturefh )
    {
	Write( capturefh, buffer, len );
    }
}


void
writeConsole( STRPTR buffer, LONG len )
{
    struct FileHandle *outhandle = (struct FileHandle *) BADDR(outputfh);

    DoPkt3( outhandle->fh_Type, ACTION_WRITE,
	(LONG) outhandle->fh_Arg1, (LONG) buffer, len );
}


/* Regargs based vprintf()-style routine that outputs text to the Wack
 * window.  Special features include control-C detection and suppressing
 * output if an error has occurred, until the error is reset.  Also
 * handles file capture.
 */
void
VPPrintf( char *fmt, ULONG *arglist )
{
    LONG writelen;

    if ( SetSignal( 0, SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C )
    {
	remote_error = WACKERROR_BREAK;
    }

    /* We suppress printing after an error has occurred */
    if ( !remote_error )
    {
	sprintfA( outbuffer, fmt, arglist );

	if ( writelen = strlen( outbuffer ) )
	{
	    writeConsole( outbuffer, writelen );
	    writeCapture( outbuffer, writelen );

	    freshline = ( outbuffer[ writelen-1 ] == '\n' );
	}
    }
}

/* Varargs based printf()-sytle routine to output to the Wack window
 * using VPPrintf().
 */
void
PPrintf( char *fmt, ... )
{
    VPPrintf( fmt, ((ULONG *)&fmt)+1 );
}


/* Single-character write to Wack window */
void
Putchar( char ch )
{
    PPrintf("%lc", ch );
}


/* This is used to reset the error condition before every command.
 * The target-communicator can set an error to note when a remote operation
 * errored out, causing output to be skipped until reset.
 */
void
resetError( void )
{
    long error = remote_error;
    remote_error = 0;
    store_invalid_address = TRUE;
    if ( ( error ) && (!freshline ) )
    {
	NewLine();
    }
}


/* If any error occurred, reset the error condition and display
 * descriptive text.
 */
void
showError( void )
{
    long error = remote_error;
    void *addr = invalid_address;
    STRPTR errorstr;

    /* We reset the error to allow us to print again */
    resetError();

    /* Let's see if the local/remote handler recognizes this error.
     * If it does, it will print a description and return zero,
     * else it'll return the error number for us to worry about.
     */
    if ( errorstr = LinkError( error ) )
    {
	PPrintf( errorstr, error );
    }
    else if ( error )
    {
	switch ( error )
	{
	case WACKERROR_ALLOCFAIL:
	    PPrintf( "Allocation failure\n" );
	    break;

	case WACKERROR_ILLEGALACCESS:
	    PPrintf( "Illegal memory access to address %lx\n", addr );
	    break;

	case WACKERROR_BREAK:	/* ctrl-C */
	    PPrintf( "***Break\n" );
	    break;

	default:
	    PPrintf( "Unidentified error %lx\n", error );
	    break;
	}
    }
}


/* Post an asynchronous request to receive a line: */
LONG
RequestLineAsync( void )
{
    LONG success = 1;
    struct FileHandle *inhandle = (struct FileHandle *) BADDR(inputfh);

    /* If we already have an asynchronous read pending, we do nothing
     * but report success.
     */
    if ( !read_packet )
    {
	/* Allocate and post an asynchronous read into our input buffer */
	if ( read_packet = AllocDosObjectTagList( DOS_STDPKT, NULL ) )
	{
	    read_packet->dp_Action = ACTION_READ;
	    read_packet->dp_Arg1 = (LONG) inhandle->fh_Arg1;
	    read_packet->dp_Arg2 = (LONG) inbuffer;
	    read_packet->dp_Arg3 = (LONG) INBUFFERSIZE;

	    SendPkt( read_packet, inhandle->fh_Type, prport );
	}
	else
	{
	    success = 0;
	}
    }
    return( success );
}

/* Handle the line once it arrives: */
char *
ReceiveLineAsync( void )
{
    struct Message *pktmsg;
    struct DosPacket *packet;
    char *result = NULL;

    if ( pktmsg = GetMsg( prport ) )
    {
	packet = (struct DosPacket *) (pktmsg->mn_Node.ln_Name);
	if ( packet->dp_Status > 0 )
	{
	    long i = 0;
	    while ( ( i < INBUFFERSIZE-1 ) && ( inbuffer[i] != '\n' ) )
	    {
		i++;
	    }
	    inbuffer[i] = '\0';
	    result = inbuffer;
	}
	FreeDosObject( DOS_STDPKT, packet );
	read_packet = 0;
    }

    return( result );
}


/* Turn on/off the Wack window's cursor. NB: We don't use PPrintf(),
 * so that we can keep these control codes out of the capture file
 */
void
showCursor( BOOL showit )
{
    static BOOL cursoron = TRUE;
    STRPTR string = NULL;

    if ( showit )
    {
	if ( !cursoron )
	{
	    /* Turn cursor on */
	    string = "[ p";
	    cursoron = TRUE ;
	}
    }
    else
    {
	if ( cursoron )
	{
	    /* Turn cursor off */
	    string = "[0 p";
	    cursoron = FALSE;
	}
    }
    if ( string )
    {
	writeConsole( string, strlen( string ) );
    }
}


#define CURRADDR_CHAR	3

/* Separate showPrompt() command to avoid sending the prompt to the
 * capture file
 */
void
showPrompt( void )
{
    LONG writelen;

    if ( prompt )
    {
	char *start;
	char *ch;

	ch = prompt;
	while ( *ch )
	{
	    start = ch;
	    writelen = 0;
	    while ( ( *ch ) && ( *ch != CURRADDR_CHAR ) )
	    {
		ch++;
		writelen++;
	    }
	    writeConsole( start, writelen );
	    writeCapture( start, writelen );
	    if ( *ch == CURRADDR_CHAR )
	    {
		char addrbuff[ 9 ];
		sprintf( addrbuff, "%08lx", CurrAddr );
		writeConsole( addrbuff, 8 );
		writeCapture( addrbuff, 8 );
		ch++;
	    }
	}
    }
}


/* The main event-processing loop.  Handles prompting, and handling
 * of IntuiMessages, ARexx messages, and keyboard entry.
 */
void
processInput( void )
{
    ULONG intuisig = 1 << iowin->UserPort->mp_SigBit;
    ULONG prsig = 1 << prport->mp_SigBit;
    ULONG sigs;
    ULONG reprompt = TRUE;
    ULONG waitsigs;
    char *input_buffer;

    while ( ( proceed ) || ( ARexx_NumPending() != 0 ) )
    {
	if ( ARexx_NumPending() == 0 )
	{
	    if ( reprompt )
	    {
		showPrompt();
		reprompt = FALSE;
	    }

	    showCursor( TRUE );
	}

	/* RequestLineAsync() will post an asynchronous read request
	 * for a line of data.  It's smart enough to do nothing
	 * if a request is already pending.
	 */
	if ( RequestLineAsync() )
	{
	    /* Don't handle command-line reads while ARexx stuff
	     * is outstanding
	     */
	    waitsigs = intuisig | rexxsig;
	    if ( ARexx_NumPending() == 0 )
	    {
		waitsigs |= prsig;
	    }
	    sigs = Wait( waitsigs );

	    showCursor( FALSE );

	    /* prsig denotes the arrival of a line of text input
	     * from the con-handler:
	     */
	    if ( sigs & prsig )
	    {
		reprompt = TRUE;
		if ( input_buffer = ReceiveLineAsync() )
		{
		    resetError();
		    writeCapture( input_buffer, strlen( input_buffer ) );
		    writeCapture( "\n", 1 );
		    if ( !processLine( input_buffer ) )
		    {
			PPrintf("Unknown command\n");
		    }
		    showError();
		}
		else
		{
		    proceed = 0;
		}
	    }

	    /* intuisig denotes the arrival of an IntuiMessage.
	     * Currently, we listen for menu and window-close events.
	     */
	    if ( sigs & intuisig )
	    {
		struct IntuiMessage *imsg;

		while ( imsg = ((struct IntuiMessage *)GetMsg( iowin->UserPort ) ) )
		{
		    switch ( imsg->Class )
		    {
			case IDCMP_CLOSEWINDOW:
			    proceed = 0;
			    break;

			case IDCMP_MENUPICK:
			    resetError();
			    reprompt = EvalMenus( imsg->Code );
			    showError();
			    break;
		    }
		    ReplyMsg( (struct Message *)imsg );
		}
	    }

	    /* rexxsig means that an ARexx message has arrived */
	    if ( sigs & rexxsig )
	    {
		ARexx_Handler();
	    }
	}
	else
	{
	    proceed = 0;
	}
    }
}


/* malloc()/free() are widely used throughout the existing Wack source,
 * so we provide simple wrappers here.
 */
APTR
malloc( ULONG size )
{
    return( AllocVec( size, MEMF_CLEAR ) );
}

void
free( APTR addr )
{
    FreeVec( addr );
}


/* Handles the output capture command */
STRPTR
Capture( char *arg )
{
    char arg1[ MAXTOKENLEN ];
    char arg2[ MAXTOKENLEN ];
    BPTR openfh;
    int append = 0;

    if ( arg = parseToken( arg, arg1 ) )
    {
	if ( !stricmp( arg1, "off" ) )
	{
	    if ( capturefh )
	    {
		Close( capturefh );
		capturefh = NULL;
		PPrintf("Output capture off\n");
	    }
	    else
	    {
		PPrintf("Output capture was not on\n");
	    }
	}
	else
	{
	    if ( parseToken( arg, arg2 ) && ( !stricmp( arg2, "append" ) ) )
	    {
		append = 1;
	    }
	    if ( capturefh )
	    {
		PPrintf("Output capture already on\n");
	    }
	    else if ( openfh = Open( arg1, MODE_READWRITE ) )
	    {
		if ( append )
		{
		    Seek( openfh, 0, OFFSET_END );
		}
		else
		{
		    SetFileSize( openfh, 0, OFFSET_BEGINNING );
		}
		capturefh = openfh;
	    }
	    else
	    {
		PPrintf("Failed to open %s for capturing output\n", arg1 );
	    }
	}
    }
    else
    {
	if ( capturefh )
	{
	    PPrintf("Capture is on\n");
	}
	else
	{
	    PPrintf("Capture is off\n");
	}
    }

    return( NULL );
}


STRPTR
SetWackPrompt( char *arg )
{
    long i = 0;
    char newprompt[ MAXTOKENLEN ];
    char *src;
    if ( parseToken( arg, newprompt ) )
    {
	src = newprompt;
	while ( ( *src ) && ( i < PROMPTLEN ) )
	{
	    if ( ( *src == '*' ) || ( *src == '\\' ) )
	    {
		char next = *(src+1);
		if ( ( next == 'N' ) || ( next == 'n' ) )
		{
		    prompt[i++] = '\n';
		    src += 2;
		}
		else if ( ( next == 'T' ) || ( next == 't' ) )
		{
		    prompt[i++] = '\t';
		    src += 2;
		}
		else if ( ( next == 'C' ) || ( next == 'c' ) )
		{
		    prompt[i++] = CURRADDR_CHAR;
		    src += 2;
		}
		else
		{
		    prompt[i++] = *src;
		    if ( next == *src )
		    {
			src += 2;
		    }
		    else
		    {
			src++;
		    }
		}
	    }
	    else
	    {
		prompt[i++] = *src++;
	    }
	}
	prompt[i] = '\0';
    }

    return( NULL );
}
