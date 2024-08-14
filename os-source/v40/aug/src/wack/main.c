/*
 * Amiga Grand Wack
 *
 * main.c -- "Main" file for Wack.
 *
 * $Id: main.c,v 39.16 93/11/05 14:59:13 jesup Exp $
 *
 * This file contains a lot of the Wack globals, and the actual
 * main routine, which calls out a lot of stuff (in particular,
 * initialization in io.c).  Some of the help info is here.
 * 
 */

#include "std.h"
#include "symbols.h"

#include "sys_proto.h"
#include "sat_proto.h"
#include "define_proto.h"
#include "symload_proto.h"
#include "main_proto.h"
#include "link_proto.h"
#include "io_proto.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"

#define DMA_DEV "/dev/bb0"


char *program;
#if 0
FILE  *SymFile;
#endif

short td = 0;
APTR LinkLibBase = NULL;

extern int errno;
short flags;

short Backup;
short LastToken;
long Accum;
long holdRTS;
long LastNum;
long CurrAddr=0;
/* Swappable address ptr.  Also, some commands set this up. "!" to swap */
long SpareAddr=0;
long DisasmAddr=0;
long HoldNum=0;
long FrameSize=16;
char *InputPtr;

char stream[100];
char *streamp;

long values[20];

long proceed = 1;

extern UWORD far VERSION;
extern UWORD far REVISION;

LONG
mymain( void )
{
#if 0
    char fileName[1024];
#endif
    STRPTR machine;
    STRPTR errorstr;
    LONG rc = 20;

    if ( !( errorstr = openEnvironment() ) )
    {
	machine = getTarget();

	if ( LinkLibBase = OpenLinkLib( machine ) )
	{
	    rc = 10;
	    if ( !( errorstr = LinkConnect( machine ) ) )
	    {
		rc = 20;
		if ( !( errorstr = openWin() ) )
		{
		    rc = 0;
		    Banner();

		    TryIt();

		    clearBPT();

		    executeStartupFile();

		    processInput();
		}
		closeWin();
	    }

	    LinkDisconnect();

	    CloseLinkLib( LinkLibBase );
	}
    }

    UnBindSymbols();

    closeEnvironment( errorstr );

    return( rc );
}


STRPTR
Conclude( void )
{
    proceed = 0;
    return( NULL );
}

void
Banner( void )
{
    PPrintf( "\n  Amiga Wack Local/Remote Debugger (Version %ld.%ld)\n  © Copyright 1988-1993, Commodore-Amiga, Inc.\n", VERSION, REVISION );
}

STRPTR
Credits( void )
{
    Banner();
    NewLine();
    PPrintf("Wack the extensible local/remote Amiga debugger, is the work of many people.\n");
    PPrintf("The mastermind of the current incarnation is\n");
    PPrintf("    Peter Cherna\n");
    PPrintf("who was responsible for most of the design and enhancements of the\n");
    PPrintf("past few years, including the native port and all sorts of nifty features...\n");
    PPrintf("Derived from Sun Grand-Wack, last maintained and generally kept cool by\n");
    PPrintf("    Jim Mackraz\n");
    PPrintf("Other contributors:\n");
    PPrintf("    Mike Sinz (help with ARexx and hunk support)\n");
    PPrintf("    Greg Miller (SAD support and help with Envoy support)\n");
    PPrintf("    Ken Dyke (help with Envoy support)\n");
    PPrintf("    Randell Jesup (parallel support, read/write block, call)\n");

    return( NULL );
}
