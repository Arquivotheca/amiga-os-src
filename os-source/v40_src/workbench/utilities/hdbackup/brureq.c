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

#include "bailout.h"
#include "brushell.h"
#include "mainwin.h"
#include "rexxcom.h"
#include "dbug.h"
#include "usermsg.h"
#include "reqs.h"
#include "brureq.h"


#define MONITOR		0
	/* Define as 1 for error and query printouts.  Development only. */

#define BUFSIZE		128
	/* Size for misc text buffers used here. */

	

/*
 *	Local function prototypes.
 */


static void bru_msg_parse PROTO((char *));

static void msg_loadvol( char *msg );

static void action_file_failure( void );

static void err_dataovw( void );
static void query_dataovw( void );

static void err_wrongvol( void );
static void query_wrongvol( void );

static void err_bsize( void );
static void err_wrtprot( void );
static void err_waccess( void );
static void err_aropen( void );
static void err_aeov( void );
static void err_stack( void );
static void err_unfmt( void );
static void err_ftrunc( void );




/*-----------------------------------------------------------------*/

static int msg_num;
static char *msg_text;
	/* These are set via a call to bru_msg_parse() */
	

static int lasterr = 0;
static int next_to_lasterr = 0;
static char lasterr_text[BUFSIZE] = "";
	/* These contain the number and text from the last error
	 * message received from BRU.  This will be 0 and the null
	 * string respectivly if the last error message was handled
	 * completely at that time.
	 */
	 
static char querybuf[BUFSIZE] = "";
	/* This is also set by the last error message received if
	 * appropriate.  Otherwise it is a null string.
	 */


	 
#define NUM_ACTIONS		129

static struct {
	void	(*error_action)(void);	/* Action done when error message
									 * received.
									 */
	void	(*query_action)(void);	/* Action done when query message
									 * pertaining to this error msg
									 * is received.
									 */
} action_table[NUM_ACTIONS] = {
	{ NULL,	NULL	},
		/* MSG_MODE		1	Confusion over mode */
	{ err_aropen,	NULL	},
		/* MSG_AROPEN	2	Error opening archive */
	{ NULL,	NULL	},
		/* MSG_ARCLOSE	3	Error closing archive */
	{ NULL,	NULL	},
		/* MSG_ARREAD	4	Error reading archive */
	{ NULL,	NULL	},
		/* MSG_ARWRITE	5	Error writing archive */
	{ NULL,	NULL	},
		/* MSG_ARSEEK	6	Error seeking on archive */
	{ NULL,	NULL	},
		/* MSG_BUFSZ	7	Media smaller than I/O buffer */
	{ NULL,	NULL	},
		/* MSG_MAXBUFSZ	8	Buffer size exceeds max for device */
	{ NULL,	NULL	},
		/* MSG_BALLOC	9	Can't allocate block buffers */
	{ NULL,	NULL	},
		/* MSG_BSEQ		10	Block sequence error */
	{ NULL,	NULL	},
		/* MSG_DSYNC	11	File header error; resync */
	{ action_file_failure,	NULL	},
		/* MSG_EACCESS	12	File does not exist */
	{ NULL,	NULL	},
		/* MSG_STAT		13	Can't stat file */
	{ NULL,	NULL	},
		/* MSG_BIGPATH	14	Pathname too big */
	{ NULL,	NULL	},
		/* MSG_BIGFAC	15	Blocking factor too big */
	{ action_file_failure,	NULL	},
		/* MSG_OPEN		16	Can't open file */
	{ NULL,	NULL	},
		/* MSG_CLOSE	17	Error closing file */
	{ action_file_failure,	NULL	},
		/* MSG_READ		18	Error reading from file */
	{ err_ftrunc,	NULL	},
		/* MSG_FTRUNC	19	File was truncated */
	{ action_file_failure,	NULL	},
		/* MSG_FGREW	20	File grew while archiving */
	{ NULL,	NULL	},
		/* MSG_SUID		21	Not owner, can't set user id */
	{ NULL,	NULL	},
		/* MSG_SGID		22	Not in group, can't set group id */
	{ NULL,	NULL	},
		/* MSG_EXEC		23	Can't exec a file */
	{ NULL,	NULL	},
		/* MSG_FORK		24	Can't fork */
	{ NULL,	NULL	},
		/* MSG_BADWAIT	25	Unrecognized wait return */
	{ NULL,	NULL	},
		/* MSG_EINTR	26	Interrupted system call */
	{ NULL,	NULL	},
		/* MSG_CSTOP	27	Child process stopped */
	{ NULL,	NULL	},
		/* MSG_CTERM	28	Child process terminated */
	{ NULL,	NULL	},
		/* MSG_CORE		29	Child process dumped core */
	{ NULL,	NULL	},
		/* MSG_WSTATUS	30	Inconsistent wait status */
	{ NULL,	NULL	},
		/* MSG_SETUID	31	Error setting real & effective uid */
	{ NULL,	NULL	},
		/* MSG_SETGID	32	Error setting real & effective gid */
	{ NULL,	NULL	},
		/* MSG_SUM		33	Archive checksum error */
	{ NULL,	NULL	},
		/* MSG_BUG		34	Internal bug detected */
	{ NULL,	NULL	},
		/* MSG_MALLOC	35	Error allocating space */
	{ NULL,	NULL	},
		/* MSG_WALK		36	Internal consistency error in tree walk */
	{ NULL,	NULL	},
		/* MSG_DEPTH	37	Pathname too big in recursive save */
	{ NULL,	NULL	},
		/* MSG_SEEK		38	Error seeking on file */
	{ NULL,	NULL	},
		/* MSG_ISUM		39	Checksum error in info block */
	{ action_file_failure,	NULL	},
		/* MSG_WRITE	40	Error writing to file */
	{ NULL,	NULL	},
		/* MSG_SMODE	41	Error setting file mode */
	{ NULL,	NULL	},
		/* MSG_CHOWN	42	Error setting uid/gid */
	{ NULL,	NULL	},
		/* MSG_STIMES	43	Error setting times */
	{ NULL,	NULL	},
		/* MSG_MKNOD	44	Error making a non-regular file */
	{ NULL,	NULL	},
		/* MSG_MKLINK	45	Error making link */
	{ NULL,	NULL	},
		/* MSG_ARPASS	46	Inconsistent phys block addresses */
	{ NULL,	NULL	},
		/* MSG_IMAGIC	47	Bad info block magic number */
	{ NULL,	NULL	},
		/* MSG_LALLOC	48	Lost linkage for file */
	{ NULL,	NULL	},
		/* MSG_URLINKS	49	Unresolved links */
	{ NULL,	NULL	},
		/* MSG_TTYOPEN	50	Can't open tty */
	{ NULL,	NULL	},
		/* MSG_NTIME	51	Error converting time */
	{ NULL,	NULL	},
		/* MSG_UNAME	52	Error getting unix name */
	{ NULL,	NULL	},
		/* MSG_LABEL	53	Archive label string too big */
	{ NULL,	NULL	},
		/* MSG_GUID		54	Error converting uid for -o option */
	{ NULL,	NULL	},
		/* MSG_CCLASS	55	Botched character class pattern */
	{ NULL,	NULL	},
		/* MSG_OVRWRT	56	Can't overwrite file */
	{ err_waccess,	NULL	},
		/* MSG_WACCESS	57	Can't access file for write */
	{ action_file_failure,	NULL	},
		/* MSG_RACCESS	58	Can't access file for read */
	{ NULL,	NULL	},
		/* MSG_COPEN	59	File will not be contiguous */
	{ NULL,	NULL	},
		/* MSG_CNOSUP	60	No support for contiguous files */
	{ NULL,	NULL	},
		/* MSG_STDIN	61	Illegal use of standard input */
	{ NULL,	NULL	},
		/* MSG_PEOV		62	Premature end of volume */
	{ err_unfmt,	NULL	},
		/* MSG_UNFMT	63	Media appears to be unformatted */
	{ action_file_failure,	NULL	},
		/* MSG_FIRST	64	General first read/write error */
	{ NULL,	NULL	},
		/* MSG_BRUTAB	65	Can't find device table file */
	{ NULL,	NULL	},
		/* MSG_SUPERSEDE	66	File not superseded */
	{ err_wrtprot,	NULL	},
		/* MSG_WRTPROT	67	Media appears to be write protected */
	{ NULL,	NULL	},
		/* MSG_IGNORED	68	File not in archive or ignored */
	{ NULL,	NULL	},
		/* MSG_FASTMODE	69	May need to use -F option on read */
	{ NULL,	NULL	},
		/* MSG_BACKGND	70	Abort if any interaction required */
	{ NULL,	NULL	},
		/* MSG_MKDIR	71	Mkdir system call failed */
	{ NULL,	NULL	},
		/* MSG_RDLINK	72	Readlink system call failed */
	{ NULL,	NULL	},
		/* MSG_NOSYMLINKS	73	System does not support symbolic links */
	{ NULL,	NULL	},
		/* MSG_MKSYMLINK	74	Could not make the symbolic link */
	{ NULL,	NULL	},
		/* MSG_MKFIFO	75	Could not make a fifo */
	{ NULL,	NULL	},
		/* MSG_SYMTODIR	76	Hard link to a directory if no symlinks */
	{ NULL,	NULL	},
		/* MSG_HARDLINK	77	Target for a hard link does not exist */
	{ NULL,	NULL	},
		/* MSG_FIFOTOREG	78	Extracted a fifo as a regular file */
	{ NULL,	NULL	},
		/* MSG_ALINKS	79	Additional links added while running */
	{ NULL,	NULL	},
		/* MSG_OBTF		80	Obsolete brutab entry */
	{ NULL,	NULL	},
		/* MSG_DEFDEV	81	No default device in bru device table */
	{ NULL,	NULL	},
		/* MSG_NOBTF	82	No support for obsolete brutab format */
	{ err_bsize,	NULL	},
		/* MSG_BSIZE	83	Attempt to change buffer size on nth vol */
	{ NULL,	NULL	},
		/* MSG_DBLIO	84	General double buffering I/O error */
	{ NULL,	NULL	},
		/* MSG_DBLSUP	85	Error setting up double buffering */
	{ NULL,	NULL	},
		/* MSG_EJECT	86	Error attempting to eject media */
	{ NULL,	NULL	},
		/* MSG_NOSHRINK	87	Compressed version was larger, saved
				   				uncompressed */
	{ NULL,	NULL	},
		/* MSG_ZXFAIL	88	Extraction of compressed file failed */
	{ NULL,	NULL	},
		/* MSG_NOEZ		89	Estimate mode ignores compression */
	{ NULL,	NULL	},
		/* MSG_UNLINK	90	Failed to unlink a file */
	{ NULL,	NULL	},
		/* MSG_ZFAILED	91	Compression failed for some nonspecific
				   				reason */
	{ NULL,	NULL	},
		/* MSG_BADBITS	92	Bad number of compression bits */
	{ NULL,	NULL	},
		/* MSG_SHMSIZE	93	Buffer size too large for double buffering */
	{ NULL,	NULL	},
		/* MSG_BUFADJ	94	Buffer size automatically adjusted */
	{ NULL,	NULL	},
		/* MSG_SHMGET	95	Call to shmget failed */
	{ NULL,	NULL	},
		/* MSG_SHMAT	96	Call to shmat failed */
	{ NULL,	NULL	},
		/* MSG_MSGGET	97	Could not get message queue */
	{ NULL,	NULL	},
		/* MSG_IOPT		98	Garbled -I option */
	{ NULL,	NULL	},
		/* MSG_MAXSEGS	99	Need at least two shared memory regions */
	{ NULL,	NULL	},
		/* MSG_SBRK		100	Got error from sbrk call */
	{ NULL,	NULL	},
		/* MSG_NOZ		101	Compression suppressed, can't setup */
	{ NULL,	NULL	},
		/* MSG_CLDUNK	102	Unknown child died */
	{ NULL,	NULL	},
		/* MSG_SLVDIED	103	Double buffer slave died */
	{ NULL,	NULL	},
		/* MSG_SLVERR	104	Double buffer slave error */
	{ NULL,	NULL	},
		/* MSG_REAP		105	No child to reap */
	{ NULL,	NULL	},
		/* MSG_SHMCOPY	106	Warn that device may need shmcopy in 
				   				brutab */
	{ action_file_failure,	NULL	},
		/* MSG_UWERR	107	Unrecoverable write error */
	{ NULL,	NULL	},
		/* MSG_UFORWP	108	Unformatted or write protected media */
	{ err_aeov,	NULL	},
		/* MSG_AEOV		109	Assuming end of volume */
	{ err_wrongvol,	query_wrongvol	},
		/* MSG_WRONGVOL	110	Found wrong volume number */
	{ NULL,	NULL	},
		/* unused		111	*/
	{ NULL,	NULL	},
		/* MSG_WRONGTIME	112	Volume from different archive */
	{ err_dataovw, query_dataovw },
		/* MSG_DATAOVW	113	All data on volume will be overwritten */
	{ NULL,	NULL	},
		/* MSG_NEWPASS	114	Ready for a new pass over archive */
	{ NULL,	NULL	},
		/* MSG_TTYNL	115	Terminate current tty output line */
	{ NULL,	NULL	},
		/* MSG_VERBOSITY	116	A verbosity message */
	{ NULL,	NULL	},
		/* MSG_NOREWIND	117	Can't rewind archive device */
	{ NULL,	NULL	},
		/* MSG_RERUN	118	Rerun suggestion */
	{ NULL,	NULL	},
		/* MSG_CONFIRM	119	A confirm query message */
	{ NULL,	NULL	},
		/* MSG_ACTION	120	A query message to select action */
	{ NULL,	NULL	},
		/* MSG_LOADVOL	121	Load a new volume */
	{ NULL,	NULL	},
		/* MSG_TOOLARGE	122	File size exceeds ulimit maximum */
	{ NULL,	NULL	},
		/* MSG_ULIMITSET	123	Can't set ulimit */
	{ NULL,	NULL	},
		/* MSG_NODBLBUF	124	No double buffering support */
	{ NULL,	NULL	},
		/* MSG_DBLBUFOFF	125	Problem with shared mem in kernel */
	{ NULL,	NULL	},
		/* MSG_MSGSND	126	Problem sending message */
	{ NULL,	NULL	},
		/* MSG_MSGRCV	127	Problem receiving message */
	{ NULL,	NULL	},
		/* MSG_FDATACHG	128	File data changed (but size didn't) */
	{ err_stack, NULL	},
		/* MSG_STACK	129	Stack size less than recommended min */
};



/*-----------------------------------------------------------------*/

static void set_lasterr( int num )
{
	next_to_lasterr = lasterr;
	lasterr = num;
}



/*-----------------------------------------------------------------*/


static void action_file_failure( void )
{
	char *p;

	
	/* printf( "FF: '%s'\n", msg_text ); */

	/* Truncate to get just the filename in error */
	p = strchr( msg_text, ':' );
	if( p )
	{
		*p = '\0';
	}
	
	file_failure( msg_text );

	/* set_lasterr( 0 ); */
}


static void err_dataovw( void )
{
    strcpy( lasterr_text, msg_text );
    strcpy( querybuf, "c" );
}

static void query_dataovw( void )
{
#if 1
	struct EasyStruct es = {
		sizeof( struct EasyStruct ),
		0,
		"HDBackup Message",
		"Warning!  All data on %s will be overwritten",
		"Continue|Quit"
	};


	if( next_to_lasterr == MSG_LOADVOL )
	{
#if MONITOR
		printf( "Suppressing a Data Overwrite requester\n" );
#endif
		return;
	}
	
	switch(  EasyRequest( mainwin, &es, NULL, lasterr_text )  )
	{
#if 0
		case 2:		/* Reload */
			strcpy( querybuf, "r" );
			/* set_lasterr( 0 ); */		/* Don't suppress the loadvol
										 * requestor.
										 */
			break;
#endif		
		case 1:		/* Continue */
			strcpy( querybuf, "c" );
			break;
			
		case 0:		/* Quit */
			strcpy( querybuf, "q" );
			break;
	}
#endif
}



static void hybrid_loadvol( void )
{
	struct EasyStruct es = {
		sizeof( struct EasyStruct ),
		0,
		"HDBackup Message",
		"Warning!  All data on %s will be overwritten",
		"Continue|Quit"
	};

	
	switch(  EasyRequest( mainwin, &es, NULL, lasterr_text )  )
	{
		case 1:		/* Continue */
			strcpy( querybuf, "c" );
			break;
			
		case 0:		/* Quit */
			strcpy( querybuf, "q" );
			break;
	}
}



static void err_wrongvol( void )
{
	int got, need;


	sscanf( msg_text, "%d %d", &got, &need );

	sprintf( lasterr_text, "Ready for volume %d.\n\
Please load this volume before proceeding.\n\n\
(volume in drive now is %d)", need, got );
}

static void query_wrongvol( void )
{
	struct EasyStruct es = {
		sizeof( struct EasyStruct ),
		0,
		"HDBackup Message",
		"%s",
		"Proceed|Quit"
	};

	
	switch(  EasyRequest( mainwin, &es, NULL, lasterr_text )  )
	{
		case 1:		/* Proceed */
			strcpy( querybuf, "r" );
			break;
			
		case 0:		/* Quit */
			strcpy( querybuf, "q" );
			break;
	}
}



static void err_bsize( void )
{
	char buf[80];

	
    sprintf( buf, "Error attempting to change buffer size." );
    mention_error( buf, 0 );
	/* set_lasterr( 0 ); */
}


static void err_wrtprot( void )
{
    sprintf( lasterr_text,
	    "Error.  The destination media appears to be write-protected." );
    strcpy( querybuf, "DF0:" );
}


static void err_waccess( void )
{
    sprintf( lasterr_text, "Error.  Cannot access file for write." );
}


static void err_aropen( void )
{
    sprintf( lasterr_text, "Error.  Cannot open Archive." );
}


static void err_aeov( void )
{
    sprintf( lasterr_text, "Assuming end-of-volume." );
}


static void err_stack( void )
{
	char buf[80];

	
    sprintf( buf, "BRU stack (%s) may be too small.", msg_text );
    mention_error( buf, 0 );
	/* set_lasterr( 0 ); */
}



static void err_ftrunc( void )
{
	char buf[80];

	
    sprintf( buf, "Error.  File '%s' is truncated.", msg_text );
    mention_error( buf, 0 );
}



static void err_unfmt( void )
{
    mention_error( "Error.  Media appears to be unformatted.", 0 );
}



void errormsg_from_bru( char *msg )
{
	char buf[256];

	
    DBUG_ENTER ("errormsg_from_bru");

    DBUG_PRINT ("rexx", ("error message '%s' from bru", msg));

    /* Clear some text buffers */
    lasterr_text[0] = '\0';
    querybuf[0] = '\0';
    
    /* Parse the error string. */
    bru_msg_parse( msg );

#if MONITOR
	printf( "ERROR %d '%s'\n", msg_num, msg_text );
#endif

    if(  ( msg_num < 1 ) || ( msg_num > NUM_ACTIONS )  )
    {
    	/* Invalid error number! */
    	set_lasterr( 0 );
    	mention_error( "Invalid Message Code from BRU",
			ERC_NONE | ERR71 );
		DBUG_VOID_RETURN;
    }
#if 0
	set_lasterr( msg_num );	/* The default.  If a specific function
							 * is complete in and of itself (i.e. not
							 * just setting up for a query message
							 * to follow) it can zero this itself.
							 */
#endif
    if( action_table[msg_num-1].error_action == NULL )
    {
    	/* Take default action */
	    sprintf( buf, "BRU error %d: %s", msg_num, msg_text );
	    mention_error( buf, ERC_NONE | ERR70 );
	    strcpy( lasterr_text, msg_text );
    }
    else
    {
    	/* Vector to specific function. */
    	(action_table[msg_num-1].error_action)();
    }

    set_lasterr( msg_num );
    
    DBUG_VOID_RETURN;
}



char *querymsg_from_bru( char *msg )
{
	char query_msg_buf[BUFSIZE];

	
    DBUG_ENTER ("querymsg_from_bru");

    DBUG_PRINT ("rexx", ("query message '%s'", msg));

    bru_msg_parse( msg );

#if MONITOR
	printf( "QUERY %d '%s'\n", msg_num, msg_text );
#endif

    switch( msg_num )
	{
		case MSG_ACTION:
			/* vector to appropriate routine */
		    if(  ( lasterr == 0 ) ||
				( action_table[lasterr-1].query_action == NULL )  )
    		{
    			/* Take default action */
			    strcpy (query_msg_buf, "Query ->  ");
			    strcat (query_msg_buf, msg_text);
    			do_bru_query_req( lasterr_text, query_msg_buf, querybuf );
		    }
		    else
		    {
    			/* Vector to specific function. */
		    	(action_table[lasterr-1].query_action)();
		    }
			break;
 
		case MSG_LOADVOL: 
			msg_loadvol( msg );
		    break;

		case MSG_CONFIRM:
		default: 
		    strcpy (query_msg_buf, "Query ->  ");
		    strcat (query_msg_buf, msg_text);
    		do_bru_query_req( lasterr_text, query_msg_buf, querybuf );
		    break;
    }

    set_lasterr( msg_num );
    lasterr_text[0] = '\0';
		    
	strcat( querybuf, "\n" );		/* Make sure we end with newline */

    DBUG_RETURN( querybuf );
}



static void msg_loadvol( char *msg )
{
	char *p, *pp;
	char buf[BUFSIZE];
	int vol;
	struct EasyStruct es = {
		sizeof( struct EasyStruct ),
		0,
		"HDBackup Message",
		"%s",
		"Proceed"
	};


	vol = atoi( msg_text );

	/* Get the default device */
	p = strchr( msg, ' ' )+1;
	p = strchr( p, ' ' )+1;
	p = strchr( p, ' ' )+1;
	while( isspace(*p) )
		p++;
	pp = querybuf;
	while( ! isspace(*p) )
		*pp++ = *p++;
	*pp = '\0';

#if 0
	if( lasterr_text[0] != '\0' )
	{
	   	sprintf( buf, "%s\nLoad Volume %d into drive %s",
			lasterr_text, vol, querybuf );
	}
	else
	{
#endif
	   	sprintf( buf, "Load volume %d into drive %s",
			vol, querybuf );
	/* } */

	switch( next_to_lasterr )
	{
		/* case MSG_DATAOVW: */
		case MSG_WRONGVOL:
			/* Let's just fall through for the sake of ease of
			 * use.  User friendly and all that.
			 */
#if MONITOR
			printf( "Suppressing a Load Volume requester\n" );
#endif
			break;

		default:
		    /* do_bru_query_req( lasterr_text, buf, querybuf ); */
			EasyRequest( mainwin, &es, NULL, buf );
		    break;
	}

    strcat( querybuf, "\n" );
}



#if 0
static void msg_loadvol( char *msg )
{
	char *p, *pp;
	char buf[BUFSIZE];
	int vol;


	vol = atoi( msg_text );
   	sprintf( buf, "Load Volume %d into drive:", vol );

	/* Get the default device */
	p = strchr( msg, ' ' )+1;
	p = strchr( p, ' ' )+1;
	p = strchr( p, ' ' )+1;
	while( isspace(*p) )
		p++;
	pp = querybuf;
	while( ! isspace(*p) )
		*pp++ = *p++;
	*pp = '\0';

	switch( next_to_lasterr )
	{
		/* case MSG_DATAOVW: */
		case MSG_WRONGVOL:
			/* Let's just fall through for the sake of ease of
			 * use.  User friendly and all that.
			 */
#if MONITOR
			printf( "Suppressing a Load Volume requester\n" );
#endif
			break;

		default:
			strcat( lasterr_text, "{Make me dull!}" );
		    do_bru_query_req( lasterr_text, buf, querybuf );
		    break;
	}

    strcat( querybuf, "\n" );
}
#endif



/*
 *		Takes the text string from BRU and parse it into a msg number
 *		and a text string.
 *		The text string will be "pure" displayable characters.
 */
 
static void bru_msg_parse( char *p )
{
    DBUG_ENTER ("bru_msg_parse");

    /* Index to first numeric character, which MUST exist!!! */
    /* while(  ( *p < '0' ) || ( *p > '9' )  ) */
	while(  isdigit( *p ) == 0  )
	{
		p++;
    }
    
    msg_num = atoi( p );

    /* Index to the text */
    msg_text = strchr( p, ' ' ) + 1;

    if( msg_text != NULL )
    {
	    /* Purify */
    	p = msg_text;
	    while( *p != '\0' )
    	{
    		/* if(  ( *p < ' ' ) || ( *p > '~'  )  ) */
			if(  isprint( *p ) == 0  )
    		{
    			*p = ' ';	/* convert to a blank char */
    		}
			p++;
    	}
    }
    
    DBUG_VOID_RETURN;
}



