/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			    All Rights Reserved				*
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
 *	@(#)ipc.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	This file contains Amiga IPC functions.
 *
 */

#include "globals.h"

#define PORTNAME		"HDBACKUP_CBM"
#define BRM_STRINGSIZE		256		/* Should be BRUPATHMAX? */
#define BRM_TAG			"BRUMSG"


struct BruMessage {
    struct Message message;
    char cmd[BRM_STRINGSIZE];		/* Command line from BRU */
    UWORD result_code;			/* Error code return. */
    char result_string[BRM_STRINGSIZE];	/* Result string return. */
};

/* Local Function Declarations */

static int ipctell PROTO((char *string));
static int send_string PROTO((char *string));
static char *recieve_reply PROTO((void));
static struct BruMessage *create_bru_message PROTO((struct MsgPort *replyport));
static void delete_bru_message PROTO((struct BruMessage *brumsg));
static int send_bru_message PROTO((struct BruMessage *brumsg, char *portname, char *string));
static BOOL is_bru_message PROTO((struct BruMessage *brumsg));

/* Variables */

/* Our private reply port */
static struct MsgPort *replyport = NULL;

static struct BruMessage *global_brumsg = NULL;

static char querybuf[BRM_STRINGSIZE] = "";
static char *queryposition = querybuf;

static BPTR oldlock;

/* Functions */

/*
 *	This routine gets the next pathname during filter mode using
 *	an ipc type interface (currently Amiga only), and places it
 *	into the buffer pointed to by "buf".
 *
 *	Returns 1 on success, 0 on failure.
 */

int ipc_getfilename (buf)
char *buf;
{
    char *p;
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
    char *p;
    char scratchbuf[BRM_STRINGSIZE];
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
    char scratchbuf[BRM_STRINGSIZE];
    
    DBUG_ENTER ("ipc_error");
    if (strlen (buf) < (sizeof (scratchbuf) - 11)) {
	strcpy (scratchbuf, "ERRORMSG \042");
	strcat (scratchbuf, buf);
	strcat (scratchbuf, "\042");
	if (ipctell (scratchbuf)) {
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
    char *p;
    BPTR lock;
    int rtnval = 0;
    
    DBUG_ENTER ("ipc_init");
    DBUG_PRINT ("ipc_init", ("open private reply port"));
    if ((replyport = CreatePort (NULL, 0L)) != NULL) {
	global_brumsg = create_bru_message (replyport);
	if (global_brumsg == NULL) {
		DeletePort (replyport);
		DBUG_RETURN (rtnval);
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
    (void) ipctell ("FINISHED");
    CurrentDir (oldlock);
    delete_bru_message (global_brumsg);
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

static int ipctell (string)
char *string;
{
    int rtnval = 0;
    
    DBUG_ENTER ("ipctell");
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
    int rtnval = 0;

    DBUG_ENTER ("send_string");
    DBUG_PRINT ("ipcsend", ("send string '%s'", string));
    if (send_bru_message (global_brumsg, PORTNAME, string) == 0) {
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
    struct Message *msg;
    struct BruMessage *brumsg;
    char *result = NULL;
    
    DBUG_ENTER ("recieve_reply");
    WaitPort (replyport);
    msg = GetMsg (replyport);
    if (is_bru_message ((struct BruMessage *) msg)) {
	brumsg = (struct BruMessage *) msg;
	if (brumsg -> result_code == 0) {
	    result = brumsg -> result_string;
	    DBUG_PRINT ("ipcget", ("got string '%s'", result));
	} else {
	    DBUG_PRINT ("ipcerr", ("bad result %d", brumsg -> result_code));
	}
    }
    DBUG_RETURN (result);
}



/* DTM_NEW */

/*
 * Allocate a BruMessage structure and perform it's low-level
 * initialization.
 */

static struct BruMessage *create_bru_message (struct MsgPort *replyport)
{
    struct BruMessage *brumsg;

    brumsg = (struct BruMessage *) AllocMem (sizeof (struct BruMessage),
	MEMF_PUBLIC | MEMF_CLEAR);
    if (brumsg == NULL) {
	/* Out of memory? */
	return (NULL);
    }
    brumsg -> message.mn_Node.ln_Name = BRM_TAG;
    brumsg -> message.mn_Node.ln_Type = NT_MESSAGE;
    brumsg -> message.mn_ReplyPort = replyport;
    brumsg -> message.mn_Length = sizeof(struct BruMessage);
    return (brumsg);
}

/* DTM_NEW */

static void delete_bru_message (struct BruMessage *brumsg)
{
    if (brumsg != NULL) {
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

static int send_bru_message (struct BruMessage *brumsg, char *portname,
	char *string)
{
    struct MsgPort *port;

    if (strlen (string) >= BRM_STRINGSIZE) {
	/* String too long! */
	return (-1);
    }
    strcpy (brumsg -> cmd, string);
    strcpy (brumsg -> result_string, "");
    brumsg -> result_code = 0;
    Forbid ();
    if ((port = FindPort (portname)) != NULL) {
	/* Found the port, let's send the message */
	PutMsg (port, (struct Message *) brumsg);
    }
    Permit ();
    if (port == NULL) {
	/* Port was not found */
	return (-1);
    }
    return (0);
}


/* DTM_NEW */

/* Verifies that the message actually originated from BRU.
 * This is just an additional bit of safety...
 */

static BOOL is_bru_message (struct BruMessage *brumsg)
{
    BOOL result;
    char *type;

    DBUG_ENTER ("is_bru_message");
    type = brumsg -> message.mn_Node.ln_Name;
    if (strcmp (type, BRM_TAG) == 0) {
	result = TRUE;
    } else {
	result = FALSE;
	DBUG_PRINT ("ipcerr", ("bad message type '%s'", type));
    }
    DBUG_RETURN (result);
}
