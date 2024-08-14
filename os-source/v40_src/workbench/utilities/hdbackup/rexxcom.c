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

#include "standard.h"

#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <rexx/errors.h>

#include "backup.h"
#include "bailout.h"
#include "bru_ipc.h"
#include "brureq.h"
#include "brushell.h"
#include "dirtree.h"
#include "eventloop.h"
#include "mainwin.h"
#include "rexxcom.h"
#include "scansort.h"
#include "dbug.h"
#include "usermsg.h"
#include "reqs.h"
#include "libfuncs.h"

#include <clib/alib_protos.h>

#define MONITOR		0
	/* Define as 1 for seeing BRU communications.  Development only. */


/*
 *	These are external Rexx functions not declared in any
 *	header file in prototype form.
 */

extern struct RexxMsg *CreateRexxMsg PROTO((struct MsgPort *, char *, char *));
extern void DeleteRexxMsg PROTO((struct RexxMsg *));
extern struct RexxArg *CreateArgstring PROTO((char *, ULONG));
extern void DeleteArgstring PROTO((struct RexxArg *));
extern BOOL IsRexxMsg PROTO((struct RexxMsg *));

#pragma libcall RexxSysBase CreateArgstring 7e 802
#pragma libcall RexxSysBase DeleteArgstring 84 801
#pragma libcall RexxSysBase CreateRexxMsg 90 a9803
#pragma libcall RexxSysBase DeleteRexxMsg 96 801
#pragma libcall RexxSysBase IsRexxMsg a8 801



/*
 *	Local function prototypes.
 */

static void cmd_status PROTO((void));
static void cmd_filename PROTO((void));
static void cmd_querymsg PROTO((void));
static void cmd_errormsg PROTO((void));
static void cmd_root PROTO((void));
static void cmd_path PROTO((void));
static void cmd_files PROTO((void));
static void cmd_volume PROTO((void));
static void cmd_finished PROTO((void));

static void dispatch PROTO((char *));
static void process_rexx_message PROTO((struct RexxMsg *));
static BOOL talk_to_bru( void );
static void set_result PROTO((int, char *));



struct CmdFunction {
	char	*Name;					/* The function name. */

	void	(*Func)(void);			/* Vector to the function's 'C'
									 * code.
									 */
};



struct MsgPort *ipcport = NULL;		/* Our public port */
ULONG ipc_sig_bit = 0;

struct Task *bru_tcb = NULL;

static int result_code;
static char *result_string;

/* The primary return code will be set to this. */
static LONG rexx_result1;

/* The secondary return code will be set to this. */
static struct RexxArg *rexx_result2;

static BOOL from_arexx;

/* General purpose string buffer */
static char buf[80];

struct RxsLib *RexxSysBase = NULL;



static struct CmdFunction func_table[] = {
	{ "STATUS", cmd_status },
    { "PATH", cmd_path },
    { "ROOT", cmd_root },
    { "FILES", cmd_files },
    { "VOLUME", cmd_volume },

    /* These keywords are only valid when a backup is in progress */
    { "ERRORMSG", cmd_errormsg },
    { "QUERYMSG", cmd_querymsg },
    { "FILENAME", cmd_filename },
    { "FINISHED", cmd_finished },

	/* End of table */
	{ NULL, NULL }
};


static char cmdline[1024];
static char *myargv[10];
static int myargc;



/* Call this whenever we suspect we have received something at
 * the IPC port.
 */

void check_ipc_port()
{
	struct Message *message;
	struct BruMessage *brumsg;


    DBUG_ENTER ("check_ipc_port");

	/* get 'em all */
    while(  ( message = GetMsg( ipcport ) ) != NULL  )
	{
		if(  ( RexxSysBase != NULL ) &&
			IsRexxMsg( (struct RexxMsg *)message )  )
		{
			/* The message is from an ARexx program */
			DBUG_PRINT ("rexx", ("message from ARexx at IPC port"));
			process_rexx_message( (struct RexxMsg *)message );
    	}
		else
		{
			/* This had better be a message in our private format! */

			brumsg = (struct BruMessage *)message;

			if( is_bru_message( brumsg ) == TRUE )
			{
				DBUG_PRINT ("rexx", ("Private msg at IPC port"));
				bru_tcb = message->mn_ReplyPort->mp_SigTask;
				set_result( 0, NULL );
				from_arexx = FALSE;
				dispatch( &brumsg->cmd[0] );
				reply_bru_message( brumsg, result_code, result_string );
		    }
			else
			{
				/* What in the world is this?  Send it back... */
				DBUG_PRINT ("rexx", ("Rogue msg at IPC port"));
				ReplyMsg( message );
			}
		}
	}

    DBUG_VOID_RETURN;
}



/* This routine is called with the message recieved at our public port
 * if it is an ARexx-format message.
 */

static void process_rexx_message (rexxmsg)
struct RexxMsg *rexxmsg;
{
    DBUG_ENTER ("process_rexx_message");


    rexx_result1 = RC_OK;
   	rexx_result2 = 0;
	from_arexx = TRUE;

    DBUG_PRINT ("rexx", ("action %lx", rexxmsg -> rm_Action));
   	DBUG_PRINT ("rexx", ("result %s", rexxmsg -> rm_Args[0]));

    /* 
   	 * Just to be squeaky clean, since Bill says to affect only the
     * result fields, copy this into a buffer for parsing
   	 */
    strlcpy (cmdline, ARG0 (rexxmsg), 1024);


	dispatch( cmdline );


    rexxmsg -> rm_Result1 = rexx_result1;

   	if( FlagIsSet (rexxmsg -> rm_Action, RXFF_RESULT) )
	{
		/* Caller asked for result string, give it if one was set. */

		DBUG_PRINT ("rexx", ("result string requested"));
		DBUG_PRINT ("rexx", ("result is '%s'", rexx_result2));
		rexxmsg -> rm_Result2 = (LONG) rexx_result2;
    }
	else
	{
		/* Caller did not ask for a result string */

		if( (rexx_result1 == RC_OK) && (rexx_result2 != 0) )
		{
		    /* Well, even though we set a result string, the
	    	 * caller does not want it, free it ourselves.
		     * This is also an error!
		     */
		    DeleteArgstring (rexx_result2);
		    rexxmsg -> rm_Result1 = RXERR_RESULTS;
		}
	}

    ReplyMsg ((struct Message *)rexxmsg);

    DBUG_VOID_RETURN;
}



/* When passed a command line, this will parse and dispatch it.
 * The keyword and arguments are placed in an array similar to
 * argv[] by this routine, and are accessed the same way.
 */

static void dispatch (line)
char *line;
{
	int i, match=-1;


    DBUG_ENTER ("dispatch");

    DBUG_PRINT ("IPC", ("command line ->%s<-", line));

#if MONITOR
		printf( "From BRU: \"%s\"\n", line );
#endif

	/* Parse the command line */
    myargc = parse_inplace (line, myargv, 10);

    DBUG_PRINT ("IPC", ("%d args passed", myargc));

    if (myargc == 0) {
	set_result( RXERR_UNKNOWN_CMD, NULL );
	DBUG_VOID_RETURN;
    }


	/* Find the keyword */

	for( i=0; func_table[i].Name != NULL; i++ )
	{
		if(  stricmp( func_table[i].Name, myargv[0] ) == 0  )
		{
			/* A match */
			match = i;
			break;
		}
	}

	if( match == -1 )
	{
		/* Function not found */
	    set_result( RXERR_UNKNOWN_CMD, NULL );
	    DBUG_VOID_RETURN;
	}


	/* Vector to the function */
	(*(func_table[match].Func))();

    DBUG_VOID_RETURN;
}



static void cmd_path()
{
    DBUG_ENTER ("cmd_path");
    set_result( 0, current_path );
    DBUG_VOID_RETURN;
}

static void cmd_root()
{
    DBUG_ENTER ("cmd_root");
    sprintf (buf, "%ld", root_node);
	set_result( 0, buf );
    DBUG_VOID_RETURN;
}

static void cmd_files()
{
    DBUG_ENTER ("cmd_files");
    sprintf (buf, "%d", file_selected_count);
	set_result( 0, buf );
    DBUG_VOID_RETURN;
}

static void cmd_volume()
{
    DBUG_ENTER ("cmd_volume");
	set_result( 0, root_name );
    DBUG_VOID_RETURN;
}

static void cmd_finished()
{
    DBUG_ENTER ("cmd_finished");
    if ( talk_to_bru() == TRUE ) {
	finished_flag = TRUE;
	/* Don't affect result fields, default is OKAY */
    } else {
	set_result( RXERR_FAILED, NULL );
    }
    DBUG_VOID_RETURN;
}

static void cmd_status ()
{
    DBUG_ENTER ("cmd_status");
    if (myargc < 2) {
	set_result( RXERR_NO_ARG, NULL );
	DBUG_VOID_RETURN;
    }
    switch (*myargv[1]) {
	case 'N': 
	case 'n': 
	    node_status (root_node);
	    sprintf (buf, "%d %ld %d %d",
		    node_count, node_memsize, node_entrys, most_entrys);
		set_result( 0, buf );
	    break;
	case 'F': 
	case 'f': 
	    file_status (root_node);
	    sprintf (buf, "%d %ld %d %ld",
		    file_count, file_size,
		    file_selected_count, file_selected_size);
		set_result( 0, buf );
	    break;
	default: 
		set_result( RXERR_INVALID_ARG, NULL );
    }
    DBUG_VOID_RETURN;
}


static void cmd_errormsg ()
{
    DBUG_ENTER ("cmd_errormsg");

    if( talk_to_bru() == FALSE )
	{
		set_result( RXERR_FAILED, NULL );
		DBUG_VOID_RETURN;
    }
    
    if( myargc < 2 )
	{
		set_result( RXERR_NO_ARG, NULL );
		DBUG_VOID_RETURN;
    }

	errormsg_from_bru( myargv[1] );

    DBUG_VOID_RETURN;
}



/*
 * This gets called with the name of a file which was not backed up
 * properly due to an error while BRU tried to back it up.
 */

void file_failure (name)
char *name;
{
    struct TreeEntry *te;


    DBUG_ENTER ("file_failure");

    te = find_filename (root_node, name);
    if (te == NULL)
	{
		/* How odd.  File not found in tree??? */
		mention_error ("A File status's may be invalid.  Internal Error.",
			ERC_NONE | ERR35);
    }
	else
	{
		switch( mode )
		{
			case BACKUP_INPROGRESS:
				sprintf (buf, "* Error *  %s NOT backed up.",
					name);
				mention_error (buf, 0);
				ClearFlag (te -> Flags, ENTRY_BACKEDUP | ENTRY_SELECTED);
				break;

			case RESTORE_INPROGRESS:
				sprintf (buf, "* Error *.  %s NOT properly restored.",
					name);
				mention_error (buf, 0);
				ClearFlag (te -> Flags, ENTRY_RESTORED | ENTRY_SELECTED);
				break;

			default:
				mention_error( "Failure message recieved at invalid time",
					ERC_NONE | ERR57 );
				break;
		}
    }

    DBUG_VOID_RETURN;
}



static void cmd_querymsg ()
{
    DBUG_ENTER ("cmd_querymsg");

    if( talk_to_bru() == FALSE )
	{
		set_result( RXERR_FAILED, NULL );
		DBUG_VOID_RETURN;
    }
    
    if( myargc < 2 )
	{
		set_result( RXERR_NO_ARG, NULL );
		DBUG_VOID_RETURN;
    }

	set_result(  0, querymsg_from_bru( myargv[1] )  );    

    DBUG_VOID_RETURN;
}



/* DTM_NEW */

static BOOL talk_to_bru( void )
{
	if(  ( mode == BACKUP_INPROGRESS ) ||
		( mode == RESTORE_INPROGRESS ) )
	{
		return( TRUE );
	}

	return( FALSE );
}



static void cmd_filename ()
{
    DBUG_ENTER ("cmd_filename");

    if( talk_to_bru() == FALSE )
	{
		/* If a backup is not in progress, this is an illegal command. */
		set_result( RXERR_FAILED, NULL );
    }


    if(  ( tree_end_flag == FALSE ) && ( user_cancel_flag == FALSE )  )
	{
		/* Send a filename */

		if( need_to_embed_logfile == TRUE )
		{
			/* Special case.  Pass the logfile name to BRU. */
			set_result( 0, EMBED_LOGNAME );
			need_to_embed_logfile = FALSE;
			DBUG_VOID_RETURN;
		}

		if ((myargc > 1) && ((*myargv[1] == 'A') || (*myargv[1] == 'a')))
		{
	    	/* Absolute volume/path/filename requested */
			set_result( 0, current_path );
		}
		else
		{
	    	/* Default to relative path and filename */
			set_result( 0, current_file );
		}
    }
	else
	{
		/* Tell BRU there are no more files */
		set_result( 0, "::::" );
    }

    filename_sent = TRUE;

    DBUG_VOID_RETURN;
}

ERRORCODE init_ipc ()
{
    DBUG_ENTER ("init_ipc");
    /*   Open Rexx library   */
    if (!(RexxSysBase = (struct RxsLib *) OpenLibrary (RXSNAME, 0L))) {
	DBUG_PRINT ("rexx", ("no rexx lib"));
    }
    /* Now setup our public port */
    if ((ipcport = CreatePort (ipcport_name, 0L)) == NULL) {
	DBUG_RETURN (ERC_NO_PORT | ERR28);
    }
    /* Set a bit mask that we can wait on */
    ipc_sig_bit = 1L << ipcport -> mp_SigBit;
    DBUG_RETURN (0);
}

void cleanup_ipc ()
{
	struct Message *msg;


    DBUG_ENTER ("cleanup_ipc");
    if (ipcport) {
	Forbid ();
	while (  ( msg = GetMsg (ipcport) ) != NULL  ) {
	ReplyMsg (msg);
	}
	DeletePort (ipcport);
	Permit ();
    }
    if (RexxSysBase) {
	CloseLibrary ((struct Library *) RexxSysBase);
    }
    DBUG_VOID_RETURN;
}



/* DTM_NEW */

/* Call this to set the result.  If the code is non-zero, it
 * will be returned as an error.
 * If zero, the string will be returned.
 */

static void set_result( code, string )
int code;
char *string;
{
	if( code != 0 )
	{
#if MONITOR
		printf( "Result code set: %d\n", code );
#endif
	    if( from_arexx == TRUE )
		{
			rexx_result1 = code;
		}
		else
		{
			result_code = code;
		}

		return;
	}

	if( string != NULL )
	{
#if MONITOR
		printf( "Result string set: \"%s\"\n", string );
#endif
	    if( from_arexx == TRUE )
		{
			rexx_result2 = CreateArgstring( string, strlen(string) );
		}
		else
		{
			result_string = string;
		}
	}
}



