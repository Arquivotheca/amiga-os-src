/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	arexx.c    AREXX functions
 *
 *  SCCS
 *
 *	@(#)arexx.c	11.5	12/19/88
 *
 *  DESCRIPTION
 *
 *	This file contains AREXX functions.
 *
 */

/* #include "autoconfig.h" */

/* #include <stdio.h> */

/* #include "typedefs.h" */

#include "standard.h"

#include <rexx/storage.h>
#include <rexx/rxslib.h>

#if LATTICE
#include <proto/exec.h>
#include <proto/dos.h>
#endif

#include "dbug.h"

/* External Function Declarations */

struct RexxMsg *CreateRexxMsg();
void DeleteRexxMsg();
struct RexxArg *CreateArgstring();
void DeleteArgstring();
LONG IsRexxMsg();



/* Local Function Declarations */

struct RexxArg *recieve_reply();
void rexx_cleanup();

struct RexxMsg *create_rexxmsg();
void delete_rexxmsg();
struct RexxArg *create_argstring();
void delete_argstring();
BOOL is_rexxmsg();



/* Variables */

struct RxsLib *RexxSysBase = NULL;

/* Our private reply port */
static struct MsgPort *rexx_replyport = NULL;

static char querybuf[200] = "";
static char *queryposition = &querybuf[0];

static BPTR oldlock;



/* Functions */


/* FOR TESTING ONLY */

FILE *dest;

main( argc, argv )
int argc;
char *argv[];
{
	char filebuf[128];
	int i;


	DBUG_ENTER ("main");
	dest = fopen( "CON:10/10/300/70/BRUrexx", "w" );

	for( i=0; i<argc; i++ )
	{
		fprintf( dest, "Cmd Line Arg %d = %s\n", i, argv[i] );
	}

	if( rexx_init() == 0 )
	{
		fprintf( dest, "Cannot do ARexx initialization\n" );
		Delay(200L);
		fclose( dest );
		exit(20);
	}

	fprintf( dest, "Ready to go\n");

	for(;;)
	{
		if( rexx_getfilename( filebuf ) == 0 )
		{
			fprintf( dest, "FAILURE in GETTING FILENAME\n" );
			Delay( 200L );
			fclose( dest );
			rexx_cleanup();
			exit(10);
		}

		fprintf( dest, "--> %s <--\n", filebuf );

		if( filebuf[0] == 't' )
		{
			/* Simulate a file-backup error */
			if( rexx_error( filebuf ) == 0 )
			{
				fprintf( dest, "** Unable to send error message\n" );
			}
		}
	}
	DBUG_RETURN (0);
}



/*
 *	This routine gets the next pathname during filter mode using
 *      a rexx type interface (currently Amiga only), and places it
 *	into the buffer pointed to by "buf".
 *
 *	Returns 1 on success, 0 on failure.
 */

int rexx_getfilename (buf)
char *buf;
{
	struct RexxArg *ra;


	if(  send_string( "FILENAME RELATIVE" ) == 0  )
	{
		/* Failure */
	    return (0);
	}

	ra = recieve_reply();
	if( ra == NULL )
	{
		/* Bad Reply */
		return( 0 );
	}

	if( strncmp( ra, ":::", 3 ) == 0 )
	{
		/* BRUshell said, no more! */
		delete_argstring( ra );
		return( 0 );
	}

	strcpy( buf, ra );

	delete_argstring( ra );

	return( 1 );
}



/*
 *	This routine sends a query string to a rexx type interface
 *	(currently Amiga only).
 *
 *	Returns 1 on success, 0 on failure.
 *
 */

int rexx_query (buf)
char *buf;
{
	struct RexxArg *ra;
	char scratchbuf[128];


	if( strlen(buf) > 118 )
	{
		/* Query string too long */
		return( 0 );
	}

	strcpy( scratchbuf, "QUERYMSG \042" );
	strcat( scratchbuf, buf );
	strcat( scratchbuf, "\042" );

	if(  send_string( scratchbuf ) == 0  )
	{
		/* Failure */
	    return (0);
	}

	ra = recieve_reply();
	if( ra == NULL )
	{
		/* Bad Reply */
		return( 0 );
	}

	strcpy( querybuf, ra );
	queryposition = querybuf;

	delete_argstring( ra );
    return (1);
}



/*
 *	This routine reads a single character from a rexx type interface
 *	(currently Amiga only).  Returns EOF on failure.  Note that the
 *	return type is int, so it can return EOF as an out-of-band
 *	indicator that no character is available.
 *
 */

int rexx_getc ()
{
	if( *queryposition == '\0' )
	{
		/* At end of buffer */
	    return (EOF);
	}

	return( (int)(*queryposition++) );
}



/*
 *	This routine sends an error string out a rexx type interface
 *	(currently Amiga only).
 *
 *	Returns 1 on success, 0 on error;
 *
 *	The return value is currently ignored.
 */

int rexx_error (buf)
char *buf;
{
	char scratchbuf[128];


	if( strlen(buf) > 118 )
	{
		/* Error string too long */
		return( 0 );
	}

	strcpy( scratchbuf, "ERRORMSG \042" );
	strcat( scratchbuf, buf );
	strcat( scratchbuf, "\042" );

	if(  tell( scratchbuf ) == 0  )
	{
		/* Failure */
		return( 0 );
	}

    return (1);
}



/* Open the rexx library (if there) and the appropriate message ports
 * for communications with the Intuition interface.
 * Returns 1 on success, 0 on error.
 */

int rexx_init()
{
	struct RexxMsg *rmsg;
	struct RexxArg *ra;
	struct FileLock *lock;


	/*   Open Rexx library   */
	RexxSysBase = (struct RxsLib *)OpenLibrary( RXSNAME, 0L);

	/* Open a private reply port for reply of messages we send to
	 * BRUshell
	 */
	if(  ( rexx_replyport = CreatePort( NULL, 0L ) ) == NULL )
	{
		return( 0 );
	}

	/* Get volume name/path */
	if(  send_string( "VOLUME" ) == 0  )
	{
		/* Failure */
	    return (0);
	}

	ra = recieve_reply();
	if( ra == NULL )
	{
		/* Bad Reply */
		return( 0 );
	}

	fprintf( dest, "Volume===> %s\n", ra );

	lock = Lock( ra, ACCESS_READ );
	if( lock == NULL )
	{
		/* Cannot find this directory */
		delete_argstring( ra );
		return( 0 );
	}
	oldlock = CurrentDir (lock);
	delete_argstring( ra );
	return( 1 );
}



/* Close any librarys and message ports, etc. which were opened
 * by a call to rexx_init().  Also send a "all done" message to
 * BRUshell.
 */

void rexx_cleanup()
{
	tell( "FINISHED" );

	(void) CurrentDir (oldlock);

	if( rexx_replyport )   DeletePort( rexx_replyport );

	if( RexxSysBase )   CloseLibrary( RexxSysBase );
}



/*-----------------------------------------------------------------*/


/* Send a string to BRUshell, wait for and discard the reply.
 *
 * Returns 1 on success, 0 on failure.
 */

static int tell( string )
char *string;
{
	struct RexxArg *ra;


	if( send_string( string ) == 0 )
	{
		return( 0 );
	}

	ra = recieve_reply();

	if( ra )
	{
		delete_argstring( ra );
	}

	return( 1 );
}



/*
 * Returns 1 on success, 0 on failure.
 */

static int send_string( string )
char *string;
{
	struct RexxMsg *rm;
	struct MsgPort *bp;
	struct RexxArg *ra;


	rm = create_rexxmsg( rexx_replyport, NULL, NULL );
	if( rm == NULL )
	{
		/* Failure */
		return( 0 );
	}

	ra = create_argstring( string, (ULONG)strlen(string) );
	if( ra == NULL )
	{
		delete_rexxmsg( rm );
		return( 0 );
	}

	SetFlag( rm->rm_Action, 1L<<RXFB_RESULT );
	ARG0(rm) = (STRPTR)ra;

	Forbid();
	if(  bp = FindPort( "BRU_SHELL" )  )
	{
		PutMsg( bp, (struct Message *)rm );
	}
	Permit();

	if( bp == NULL )
	{
		/* we could not find the BRUshell port, this failed! */
		delete_rexxmsg( rm );
		delete_argstring( ra );

		return( 0 );
	}

	return( 1 );
}



/*
 * Return is a Rexx Argstring pointer if succesful, or a NULL pointer if
 * failure or CMD DID NOT REQUIRE A RESULT.
 */

static struct RexxArg *recieve_reply()
{
	struct RexxMsg *rm;
	struct RexxArg *ra;


	WaitPort( rexx_replyport );

	while( rm = (struct RexxMsg *)GetMsg( rexx_replyport ) )
	{
		if(  ( rm->rm_Result1 == 0 ) && ( rm->rm_Result2 != 0 )  )
		{
			/* A result string returned */
			ra = (struct RexxArg *)(rm->rm_Result2);
		}
		else
		{
			ra = NULL;	
		}

		delete_argstring( ARG0(rm) );
		delete_rexxmsg( rm );
	}

	return( ra );
}



/*..................................................................*/
/*				ARexx Library Flex Functions						*/


static struct RexxMsg *create_rexxmsg( rport, ext, host )
struct MsgPort *rport;
char *ext, *host;
{
	struct RexxMsg *rm;


	if( RexxSysBase )
	{
		/* Use Bill's routine if ARexx Library was openable */
		return(  CreateRexxMsg( rport, ext, host )  );
	}
	else
	{
		/* Or use our own... */
		rm = AllocMem( (ULONG)sizeof(struct RexxMsg),
			MEMF_PUBLIC | MEMF_CLEAR );
		if( rm == NULL )
		{
			return( NULL );
		}
		
		rm->rm_Node.mn_Length = sizeof(struct RexxMsg);
		rm->rm_FileExt = (STRPTR)ext;
		rm->rm_CommAddr = (STRPTR)host;

		return( rm );
	}
}



static void delete_rexxmsg( rexxmsg )
struct RexxMsg *rexxmsg;
{
	if( rexxmsg == NULL )
	{
		return;
	}

	if( RexxSysBase )
	{
		DeleteRexxMsg( rexxmsg );
	}
	else
	{
		FreeMem( rexxmsg, (ULONG)(rexxmsg->rm_Node.mn_Length) );
	}
}



static struct RexxArg *create_argstring( string, length )
char *string;
ULONG length;
{
	struct RexxArg *ra;
	ULONG size;


	if( RexxSysBase )
	{
		return(  CreateArgstring( string, length )  );
	}
	else
	{
		size = sizeof(struct RexxArg) + length - 7L;

		ra = AllocMem( size, MEMF_PUBLIC | MEMF_CLEAR );
		if( ra == NULL )
		{
			return( NULL );
		}

		ra->ra_Size = size;
		ra->ra_Length = length;
		CopyMem( string, &ra->ra_Buff[0], length );
		/* Insure we end in a NULL, if will be used as 'C' string */
		ra->ra_Buff[length] = '\0';

		return( (struct RexxArg *)&ra->ra_Buff[0] );
	}
}



static void delete_argstring( argstring )
struct RexxArg *argstring;
{
	UBYTE *ra;


	if( RexxSysBase )
	{
		DeleteArgstring( argstring );
	}
	else
	{
		ra = ((UBYTE *)argstring) - 8L;
		FreeMem( ra, ((struct RexxArg *)ra)->ra_Size );
	}
}



static BOOL is_rexxmsg( rexxmsg )
struct RexxMsg *rexxmsg;
{
	if( RexxSysBase )
	{
		return(  IsRexxMsg( rexxmsg )  );
	}
	else
	{
		return( FALSE );
	}
}

