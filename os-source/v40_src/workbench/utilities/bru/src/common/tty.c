/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			     All Rights Reserved			*
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
 *	tty.c    routines for interacting with user's terminal
 *
 *  SCCS
 *
 *	@(#)tty.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines which directly interact with the user's
 *	terminal.
 *
 */


#include "globals.h"

#if CRAMPED
#define PBUFSIZE (256)
#else
#define PBUFSIZE (512)
#endif

static int ttyout = -1;		/* Write requests to this file desc. */
static int ttyin = -1;		/* Read responses from this file desc. */
static char prtbuf[PBUFSIZE];	/* Local tty print buffer */
static char *prtbufp = prtbuf;	/* Current pointer into print buffer */

/*
 *	Locally defined functions.
 */

static VOID tty_close PROTO((void));
static int tty_getc PROTO((void));
static char *tty_read PROTO((char *bufp, int tbufsize));
static VOID tty_inflush PROTO((void));
static VOID tty_open PROTO((void));


/*
 *  FUNCTION
 *
 *	tty_close    close the tty streams if we opened them
 *
 *  SYNOPSIS
 *
 *	static VOID tty_close ()
 *
 *  DESCRIPTION
 *
 *	If the tty streams are open, and we opened them rather
 *	than having them redirected to the error stream, then
 *	close them.
 *
 */

static VOID tty_close ()
{
    DBUG_ENTER ("tty_close");
    if (ttyin != -1 && ttyin != s_fileno (errfp)) {
	(VOID) s_close (ttyin);
    }
    ttyin = -1;
    if (ttyout != -1 && ttyout != s_fileno (errfp)) {
	(VOID) s_close (ttyout);
    }
    ttyout = -1;
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	tty_printf    formatted print to terminal
 *
 *  SYNOPSIS
 *
 *	VOID tty_printf (str, va_alist)
 *	char *str;
 *	va_dcl
 *
 *  DESCRIPTION
 *
 *	Looks like a printf call, but writes the formatted output
 *	to the terminal, or wherever the user expects it.
 *
 *	Because there are times when we need to collect an entire
 *	message before actually sending it (multiple tty_printf's
 *	are used to build the message), we have to keep a local
 *	print buffer and only flush it when explicitly asked to.
 *
 *  BUGS
 *
 *	The static buffer is a fixed size.  We should really have
 *	some way of checking for overflow.  However, since we
 *	sprintf into it, and we have no way of controlling how
 *	much of it the sprintf will use up, there is not much
 *	we could do anyway except detect overflow and report it
 *	somehow.
 *
 */

/*VARARGS1*/
VOID tty_printf (VA_ARG(char *,str), VA_ALIST)
VA_OARGS(char *str;)
VA_DCL
{
    va_list args;

    VA_START (args, str);
    (VOID) s_vsprintf (prtbufp, str, args);
    prtbufp += s_strlen (prtbufp);
    va_end (args);
}


/*
 *  FUNCTION
 *
 *	tty_flush    flush formatted print to terminal
 *
 *  SYNOPSIS
 *
 *	VOID tty_flush ()
 *
 *  DESCRIPTION
 *
 *	Flushes the current content of the tty_printf buffer to
 *	the terminal (or whatever I/O stream the user is expecting
 *	errors and queries on).
 *
 *	Sometimes we will be writing the output to a fifo.  If we
 *	open ttyout and it is a fifo, and there is noone there to
 *	read the output, the write will block.  However, if someone
 *	previously was reading the fifo, and they exit, the write
 *	will fail with a SIGPIPE.  Thus we set up to ignore a SIGPIPE,
 *	and if the write fails for this reason, errno will get set
 *	to EPIPE.  If this happens, we close the ttyin and ttyout
 *	streams, reopen them, and try again.  We will then block
 *	until someone else comes along and opens up the fifos again.
 *
 */

VOID tty_flush ()
{
    int nbytes;
    int obytes;
    int retries = 0;

    if (ttyout == -1) {
	tty_open ();
    }
    nbytes = s_strlen (prtbuf);
    do {
	errno = 0;
	sig_pipe (TRUE);
	obytes = s_write (ttyout, prtbuf, (UINT) nbytes); 
	sig_pipe (FALSE);
	if (obytes > 0) {
	    nbytes -= obytes;
	} else {
	    retries++;
	    s_sleep ((UINT) 1);
	    tty_close ();
	    tty_open ();
	}
    } while (nbytes > 0 && retries < 10);
    prtbufp = prtbuf;
    *prtbufp = EOS;
}



/*
 *  FUNCTION
 *
 *	confirmed    confirm an action if confirmation enabled
 *
 *  SYNOPSIS
 *
 *	BOOLEAN confirmed (action, fip)
 *	char *action;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	
 *	Given pointer to a string representing an action to be taken,
 *	and pointer to a file information structure, confirms the action
 *	and returns TRUE or FALSE according to "yes" or "no" response.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin confirmed
 *	    Default result is confirm action
 *	    If confirmation requested and terminal available then
 *		Print action to be taken
 *		Print confirmation request
 *		Get answer from user
 *		Determine result
 *	    End if
 *	    Return result
 *	End confirmed
 *
 */

BOOLEAN confirmed (action, fip)
char *action;
struct finfo *fip;
{
    BOOLEAN result;
    char answer [BRUPATHMAX];

    DBUG_ENTER ("confirmed");
    result = TRUE;
    if (flags.wflag) {
	bru_message (MSG_CONFIRM, action, fip -> fname);
	(VOID) tty_read (answer, sizeof (answer));
	switch (answer[0]) {
	    case 'y':
	    case 'Y':
		break;
	    case 'g':
	    case 'G':
		flags.wflag = FALSE;
		break;
	    default:
		result = FALSE;
		break;
	}
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	response    get response from user to a prompt
 *
 *  SYNOPSIS
 *
 *	char response (prompt, dfault)
 *	char *prompt;
 *	char dfault;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a prompt for user, and a default response,
 *	prompts user and returns a single character indicating the
 *	response.  If the response is simply a carriage return, the
 *	the default is returned.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin response
 *	    Save previous signal handling state
 *	    Set up to exit on interrupt or quit
 *	    Issue prompt to user
 *	    Read the answer
 *	    Pop the previous signal handling state
 *	    If no explicit answer given then
 *		Result is the default
 *	    End if
 *	    Return the answer
 *	End response
 *
 */

char response (prompt, dfault)
char *prompt;
int dfault;
{
    SIGTYPE prevINT;
    SIGTYPE prevQUIT;
    char answer [2];
    
    DBUG_ENTER ("response");
    sig_push (&prevINT, &prevQUIT);
    sig_done ();
    bru_message (MSG_ACTION, prompt, dfault);
    (VOID) tty_read (answer, sizeof (answer));
    sig_pop (&prevINT, &prevQUIT);
    if (answer[0] == EOS) {
	answer[0] = (char) dfault;
    }
    DBUG_RETURN (answer[0]);
}


/*
 *  FUNCTION
 *
 *	tty_getc    get a single character from the tty input stream
 *
 *  SYNOPSIS
 *
 *	static int tty_getc ()
 *
 *  DESCRIPTION
 *
 *	Get a single input character from the tty input stream.  Returns
 *	the input character on success, or EOF on end of file or any
 *	error.
 *
 *  NOTES
 *
 *	When we issue the read for one character, the possible returns
 *	are {-1, 0, 1}.  A return of 1 is success.  A return of 0 means
 *	end of file, and a return of -1 (SYS_ERROR) is a read error.
 *	We hang in a loop retrying reads until we either find an end
 *	of file, or until the read fails for some reason other than an 
 *	interrupted system call.  This works around a possible kernel
 *	or tty driver bug seen on some systems, where the first read after
 *	the input queue flush fails, with errno set to EINTR.  It should
 *	be harmless on systems without the bug.
 *
 *	Note also that we return an integer value from the set of
 *	{EOF, 0, 1, 2, 3, ... 256}, assuming 8 bit bytes.  The input
 *	character is explicitly unsigned to avoid sign extension during
 *	promotion to an int.
 */

static int tty_getc ()
{
    int count;
    unsigned char inchar;
    int rtnval;

    if (flags.Afflag) {
	rtnval = ipc_getc ();
    } else {
	while ((count = s_read (ttyin, (char *) &inchar, 1)) != 1) {
	    if ((count == 0) || (count == SYS_ERROR && errno != EINTR)) {
		return (EOF);
	    }
	}
	rtnval = inchar;
    }
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	tty_read    read a string from the user's terminal
 *
 *  SYNOPSIS
 *
 *	static char *tty_read (bufp, tbufsize)
 *	char *bufp;
 *	int tbufsize;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a buffer in which to store a response string,
 *	and the size of the buffer, reads from the tty up to the
 *	limit imposed by the buffer.  Then flushes the rest of
 *	the input.
 *
 *	The rewind call is necessary to allow mixed reads/writes
 *	on the terminal.  Systems which don't allow this use
 *	two separate streams.
 *
 *	Returns pointer to the terminating null character in buffer
 *	or returns NULL if too much input was read for available
 *	buffer.
 *
 *	Note that it is not an error to provide a NULL pointer for
 *	the buffer.  In this case, all input is discarded up to the
 *	next newline and a NULL is returned.  This is useful in
 *	those situations where operation is suspended until the
 *	user issues an acknowledgement.
 *
 *	The -B option (background) is provided for cases where the prefered
 *	action is to simply give up rather than attempting to interact with
 *	the operator.  This might occur, for example, when bru is
 *	used in a shell script that must be run at night with no operator
 *	present, and the script must not hang waiting for input.
 *	In this case, bru just issues an appropriate message and exits
 *	with an error status.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin tty_read
 *	    If abort rather than interact with operator
 *		Finish interaction line with a newline
 *		Issue appropriate error message
 *		Clean up and exit
 *	    Else
 *		Flush any pending output to terminal
 *		Flush any pending input if possible
 *		Rewind the terminal (huh!)
 *		If no buffer or illegal buffer size then
 *		    Flush any input
 *		    Return will be NULL
 *		Else
 *		    While more input and place to put it
 *			Squirrel the character away in buffer
 *		    End while
 *		    Terminate the string in the buffer
 *		    If last character was not end of input then
 *			Remember we overran the buffer
 *			Flush any remaining input on line
 *		    End if
 *		Endif
 *		Rewind the terminal
 *	    End if
 *	    Return result
 *	End tty_read
 *
 */


static char *tty_read (bufp, tbufsize)
char *bufp;
int tbufsize;
{
    char ch;

    DBUG_ENTER ("tty_read");
    if (flags.Bflag) {
	bru_message (MSG_TTYNL);
    	bru_message (MSG_BACKGND);
	done ();
    } else {
	tty_inflush ();
	if (bufp == NULL || tbufsize < 1) {
	    while (tty_getc () != '\n') {;}
	    bufp = NULL;
	} else {
	    while (((ch = tty_getc ()) != '\n') && --tbufsize) {
		*bufp++ = ch;
	    }
	    *bufp = EOS;
	    if (ch != '\n') {
		bufp = NULL;
		while (tty_getc () != '\n') {;}
	    }
	}
    }
    DBUG_RETURN (bufp);
}


/*
 *  FUNCTION
 *
 *	tty_inflush    flush the tty input queue if possible
 *
 *  SYNOPSIS
 *
 *	static VOID tty_inflush
 *
 *  DESCRIPTION
 *
 *	This function is called to flush the input queue, if
 *	possible, before each interactive prompt.  This insures
 *	that no stray typeaheads will cause unintentional results.
 *	This function can be null without any serious adverse
 *	affects on functionality.
 *
 *	Under BSD 4.2, be sure to flush only the input queue.
 */

static VOID tty_inflush ()
{
    DBUG_ENTER ("tty_inflush");

#ifdef TCFLSH		/* System V or Xenix */
    (VOID) s_ioctl (ttyin, TCFLSH, 0);
#else
#ifdef TIOCFLUSH	/* BSD */
    {
	int tflags;
	tflags = FREAD;
	(VOID) s_ioctl (ttyin, (int) TIOCFLUSH, (int) &tflags);
    }
#endif
#endif

    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	tty_open    open the tty file for possible interaction with user
 *
 *  SYNOPSIS
 *
 *	static VOID tty_open ()
 *
 *  DESCRIPTION
 *
 *	Opens the user's terminal file for possible interaction later.
 *	Not every invocation of bru will require interaction.  Failure
 *	to open the prefered file will result in a warning and
 *	execution will continue using standard error instead.
 *
 *	This function is generally called only once per bru
 *	invocation, when it is discovered that the current tty pointer
 *	is NULL.
 *
 *	Note that if the -B option has been specified, there is to
 *	be no interaction with the operator.  Thus we simply allow the
 *	interaction stream to default to errfp so that the message
 *	we need an answer to will get written to wherever the errors
 *	are going.
 *
 *	For the amiga, we only open a single stream for both read
 *	write, without checking access first, since the name might
 *	be a window spec like "con:0/0/400/100/Query Window".
 *
 */

static VOID tty_open ()
{
    DBUG_ENTER ("tty_open");
#if amigados
    if (!flags.Bflag && ttyout == -1) {
	if ((ttyout = s_open (info.bru_ttyout, O_WRONLY, 0)) == -1) {
	    bru_message (MSG_TTYOPEN, info.bru_ttyout);
	} else {
	    ttyin = ttyout;
	}
    }
#else
    if (!flags.Bflag && ttyout == -1) {
	if (file_access (info.bru_ttyout, A_WRITE, TRUE)) {
	    if ((ttyout = s_open (info.bru_ttyout, O_WRONLY, 0)) == -1) {
		bru_message (MSG_TTYOPEN, info.bru_ttyout);
	    }
	}
    }
    if (!flags.Bflag && ttyin == -1) {
	if (file_access (info.bru_ttyin, A_READ, TRUE)) {
	    if ((ttyin = s_open (info.bru_ttyin, O_RDONLY, 0)) == -1) {
		bru_message (MSG_TTYOPEN, info.bru_ttyin);
	    }
	}
    }
#endif
    if (ttyout == -1) {
	ttyout = s_fileno (errfp);
	info.bru_ttyout = "error stream";
    }
    if (ttyin == -1) {
	ttyin = s_fileno (errfp);
	info.bru_ttyin = "error stream";
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	tty_newmedia    prompt for new media and device name
 *
 *  SYNOPSIS
 *
 *	VOID tty_newmedia (volume, dfault, newname, newnamesize)
 *	int volume;
 *	char *dfault;
 *	char *newname;
 *	int newnamesize;
 *
 *  DESCRIPTION
 *
 *	Rings the terminal bell and prompts the user for a new media
 *	for the given volume number.  If dfault is non-null then
 *	the default is also shown.  Any input from the user is placed
 *	in the buffer pointed to by newname, of size newnamesize.
 *
 *	There is a 1 second delay after sending out the BELL character
 *	because some terminals or systems seem to require a long
 *	time to process it and may drop characters in the meantime.
 *	(Update:  This is incompatible with the tty_flush mechanism
 *	since writes are now buffered.  The delay was a kludge anyway,
 *	to help braindamaged terminals or terminal emulators.  Since
 *	the tty_flush mechanism is now a required feature, it takes
 *	precedence over the delay.  We might be able to get the same
 *	effect by padding with nul characters, but for now we just
 *	wait to see what systems break.)
 *
 *	Before bothering the user, we check to see if there is a
 *	user supplied list of device names to cycle through.  If so,
 *	then we call nextdev() to attempt to get the name of the next
 *	device in the list.  If nextdev() returns NULL, it means we
 *	are at the end of the list and firstdev() is called to get
 *	the default name, and prompting occurs as before.  If nextdev
 *	returns a pointer to the name of the next device to use,
 *	the prompting is suppressed and we simply return this name
 *	as if the user typed it in.  Note that we MUST return a
 *	valid name in newname, otherwise the caller will assume
 *	we picked the default name passed in dfault, which may
 *	not be the first name in the device list.  One last thing,
 *	if the user ever types in a name explicitly while device
 *	cycling is in use, it is assumed that he wishes to break
 *	the cycle, and this is accomplished by resetting flags.fflag.
 *
 */

VOID tty_newmedia (volume, dfault, newname, newnamesize)
int volume;
char *dfault;
char *newname;
int newnamesize;
{
    char *nextname;

    DBUG_ENTER ("tty_newmedia");
    *newname = EOS;
    if (flags.fflag > 1) {
	if ((nextname = nextdev ()) == NULL) {
	    dfault = firstdev ();
	} else {
	    (VOID) s_strncpy (newname, nextname, newnamesize);
	}
    }
    if (*newname == EOS) {
	if (dfault == NULL) {
	    dfault = "/dev/null";
	}
	bru_message (MSG_LOADVOL, volume, dfault, BELL);
	(VOID) tty_read (newname, newnamesize);
	if (flags.fflag > 1) {
	    if (*newname == EOS) {
		(VOID) s_strncpy (newname, dfault, newnamesize);
	    } else {
		flags.fflag = 0;	/* Break cycling; kludge? */
	    }
	}
    }
    DBUG_VOID_RETURN;
}

