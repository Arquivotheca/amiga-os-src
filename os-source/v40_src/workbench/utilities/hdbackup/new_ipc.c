/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
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
 *	ipc.c    Amiga IPC functions
 *
 *  SCCS
 *
 *	@(#)ipc.c	11.20	10/10/89
 *
 *  DESCRIPTION
 *
 *	This file contains Amiga IPC functions.
 *
 */

#include "autoconfig.h"

#include <stdio.h>
#include <exec/memory.h>
#include <libraries/dos.h>

#include "dbug.h"
#include "typedefs.h"


#define PORTNAME		"BRU_SHELL"

#define BRM_STRINGSIZE		256

#define BRM_TAG				"BRUMSG"



struct BruMessage {
	struct Message	message;

	char			cmd[BRM_STRINGSIZE];
		/* Command line from BRU */

	UWORD			result_code;
		/* Error code return. */

	char			result_string[BRM_STRINGSIZE];
		/* Result string return. */
};



/* External Function Declarations */

extern struct FileLock *CurrentDir ();
extern void *GetMsg ();
extern void *AllocMem ();
extern struct MsgPort *FindPort ();
extern struct MsgPort *CreatePort ();
extern void *OpenLibrary ();
extern struct FileLock *Lock ();

/* Local Function Declarations */

static int send_string ();
static char *recieve_reply ();
void ipc_cleanup ();

static struct BruMessage *create_bru_message( struct MsgPort *replyport );
static void delete_bru_message( struct BruMessage *brumsg );
static int send_bru_message( struct BruMessage *brumsg, char *portname,
	char *string );
static BOOL is_bru_message( struct BruMessage *brumsg );



/* Variables */

/* Our private reply port */
static struct MsgPort *replyport = NULL;

static struct BruMessage *global_brumsg = NULL;

static char querybuf[BRM_STRINGSIZE] = "";
static char *queryposition = &querybuf[0];

static struct FileLock *oldlock;

/* Functions */

/*
 *	This routine gets the next pathname during filter mode using
 *  an ipc type interface (currently Amiga only), and places it
 *	into the buffer pointed to by "buf".
 *
 *	Returns 1 on success, 0 on failure.
 */

int ipc_getfilename (buf)
char *buf;
{
    register char *p;
    int rtnval = 0;
    
    DBUG_ENTER ("ipc_getfilename");
    if (send_string ("FILENAME RELATIVE")) {
	if ((p = recieve_reply ()) != NULL) {
	    if (strncmp (p, ":::", 3) == 0) {
		DBUG_PRINT ("ipcgf", ("brushell reports no more filenames"));
	    } else {
		strcpy (buf, p);
		rtnval = 1;
	    }
	}
    }
    DBUG_PRINT ("ipcgf", ("returns %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 *	This routine sends a query string to an ipc type interface
 *	(currently Amiga only).
 *
 *	Returns 1 on success, 0 on failure.
 *
 */

int ipc_query (buf)
char *buf;
{
    register char *p;
    auto char scratchbuf[BRM_STRINGSIZE];
    int rtnval = 0;
    
    DBUG_ENTER ("ipc_query");
    DBUG_PRINT ("ipcq", ("query = '%s'", buf));
    if (strlen (buf) < (sizeof (scratchbuf) - 11)) {
	strcpy (scratchbuf, "QUERYMSG \042");
	strcat (scratchbuf, buf);
	strcat (scratchbuf, "\042");
	if (send_string (scratchbuf)) {
	    if ((p = recieve_reply ()) != NULL) {
		strcpy (querybuf, p);
		queryposition = querybuf;
		rtnval = 1;
	    }
	}
    }
    DBUG_RETURN (rtnval);
}

/*
 *	This routine reads a single character from an ipc type interface
 *	(currently Amiga only).  Returns EOF on failure.  Note that the
 *	return type is int, so it can return EOF as an out-of-band
 *	indicator that no character is available.
 *
 */

int ipc_getc ()
{
    int rtnval;

    DBUG_ENTER ("ipc_getc");
    if (*queryposition == '\0') {
	rtnval = EOF;
    } else {
	rtnval = *queryposition++;
    }
    DBUG_PRINT ("ipc_getc", ("return %d", rtnval));
    DBUG_RETURN (rtnval);
}

/*
 *	This routine sends an error string out an ipc type interface
 *	(currently Amiga only).
 *
 *	Returns 1 on success, 0 on error;
 *
 *	The return value is currently ignored.
 */

int ipc_error (buf)
char *buf;
{
    int rtnval = 0;
    auto char scratchbuf[BRM_STRINGSIZE];
    
    DBUG_ENTER ("ipc_error");
    if (strlen (buf) < (sizeof (scratchbuf) - 11)) {
	strcpy (scratchbuf, "ERRORMSG \042");
	strcat (scratchbuf, buf);
	strcat (scratchbuf, "\042");
	if (tell (scratchbuf)) {
	    rtnval = 1;
	}
    }
    DBUG_PRINT ("ipc_error", ("returns %d", rtnval));
    DBUG_RETURN (rtnval);
}

/* Open the appropriate message ports
 * for communications with the Intuition interface.
 * Also changes us into the appropriate current directory for our
 * backing up or restoring.
 * Returns 1 on success, 0 on error.
 */

int ipc_init ()
{
    register char *p;
    register struct FileLock *lock;
    int rtnval = 0;
    
    DBUG_ENTER ("ipc_init");
    DBUG_PRINT ("ipc_init", ("open private reply port"));
    if ((replyport = CreatePort (NULL, 0L)) != NULL) {

	global_brumsg = create_bru_message( replyport );
	if( global_brumsg == NULL )
	{
		DeletePort( replyport );
		DBUG_RETURN(rtnval);
	}

	DBUG_PRINT ("ipc_init", ("get volume name/path"));
	if (send_string ("VOLUME")) {
	    if ((p = recieve_reply ()) != NULL) {
		DBUG_PRINT ("volume", ("volume %s", p));
		if ((lock = Lock (p, ACCESS_READ)) == NULL) {
		    DBUG_PRINT ("ipc_init", ("can't find this directory"));
		} else {
		    oldlock = CurrentDir (lock);
		    rtnval = 1;
		}
	    }
	}
    }
    DBUG_RETURN (rtnval);
}

/* Close any librarys and message ports, etc. which were opened
 * by a call to ipc_init().  Also send a "all done" message to
 * BRUshell.
 */

void ipc_cleanup ()
{
    DBUG_ENTER ("ipc_cleanup");
    (void) tell ("FINISHED");
    CurrentDir (oldlock);
	delete_bru_message( global_brumsg );
    if (replyport) {
	DeletePort (replyport);
    }
    DBUG_VOID_RETURN;
}

/*
 * Send a string to BRUshell, wait for and discard the reply.
 *
 * Returns 1 on success, 0 on failure.
 */

static int tell (string)
char *string;
{
    int rtnval = 0;
    
    DBUG_ENTER ("tell");
    if (send_string (string)) {
	if (recieve_reply () != NULL) {
	    rtnval = 1;				/* semantic change (fnf) */
	}
    }
    DBUG_RETURN (rtnval);
}

/*
 * Returns 1 on success, 0 on failure.
 */

static int send_string (string)
char *string;
{
	int rtnval=0;

    DBUG_ENTER ("send_string");
	if(  send_bru_message( global_brumsg, PORTNAME, string ) == 0  )
	{
		rtnval = 1;
	}

    DBUG_PRINT ("rxsend", ("returns %d", rtnval));
    DBUG_RETURN (rtnval);
}


/*
 * Return a string pointer if succesful, or a NULL pointer if
 * failure or CMD DID NOT REQUIRE A RESULT.
 */

static char *recieve_reply ()
{
    register struct Message *msg;
    register struct BruMessage *brumsg;

    
    DBUG_ENTER ("recieve_reply");

	WaitPort( replyport );

	msg = GetMsg( replyport );
	if(  is_bru_message( (struct BruMessage *)msg )  )
	{
		brumsg = (struct BruMessage *)msg;

		if( brumsg->result_code == 0 )
		{
			DBUG_RETURN ( &brumsg->result_string );
		}
	}

    DBUG_RETURN (NULL);
}



/* DTM_NEW */

/* Allocate a BruMessage structure and perform it's low-level
 * initialization.
 */

static struct BruMessage *create_bru_message( struct MsgPort *replyport )
{
	struct BruMessage *brumsg;


	brumsg = (struct BruMessage *)AllocMem( sizeof(struct BruMessage),
		MEMF_PUBLIC | MEMF_CLEAR );
	if( brumsg == NULL )
	{
		/* Out of memory? */
		return( NULL );
	}

	brumsg->message.mn_Node.ln_Name = BRM_TAG;
	brumsg->message.mn_Node.ln_Type = NT_MESSAGE;
	brumsg->message.mn_ReplyPort = replyport;
	brumsg->message.mn_Length = sizeof(struct BruMessage);

	return( brumsg );
}



/* DTM_NEW */

static void delete_bru_message( struct BruMessage *brumsg )
{
	if( brumsg != NULL )
	{
		FreeMem( brumsg, brumsg->message.mn_Length );

	}
}



/* DTM_NEW */

/* Initialize the user data fields and send the message to the named port.
 * This expects a message structure created by a call to
 * create_bru_message().
 *
 * Return is 0 for success.
 */

static int send_bru_message( struct BruMessage *brumsg, char *portname,
	char *string )
{
	struct MsgPort *port;


	if( strlen(string) >= BRM_STRINGSIZE )
	{
		/* String too long! */
		return( -1 );
	}

	strcpy( &brumsg->cmd[0], string );
	strcpy( &brumsg->result_string[0], "" );
	brumsg->result_code = 0;

	Forbid();
	if(  ( port = FindPort( portname ) ) != NULL )
	{
		/* Found the port, let's send the message */
		PutMsg( port, (struct Message *)brumsg );
	}
	Permit();

	if( port == NULL )
	{
		/* Port was not found */
		return( -1 );
	}

	return( 0 );
}


/* DTM_NEW */

/* Verifies that the message actually originated from BRU.
 * This is just an additional bit of safety...
 */

static BOOL is_bru_message( struct BruMessage *brumsg )
{
	if(  strcmp( brumsg->message.mn_Node.ln_Name, BRM_TAG ) == 0  )
	{
		return( TRUE );
	}

	return( FALSE );
}

