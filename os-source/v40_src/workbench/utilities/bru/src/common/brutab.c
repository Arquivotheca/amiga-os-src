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
 *	brutab.c    brutab interface routines
 *
 *  SCCS
 *
 *	@(#)brutab.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Bru derives information about system devices from a file,
 *	commonly kept in /etc/brutab.  Early versions of bru used
 *	a simple, multifield, one line per device, format for the
 *	brutab file.  Unfortunately, this format has severe limitations
 *	with respect to readability and expansion.
 *
 *	This file implements a new brutab format which allows for easier
 *	expansion, more flexable formatting, and enhanced readability.
 *	It is unfortunate that there is no devcap (or devinfo) database
 *	equivalent to the termcap (or terminfo) database for terminals.
 *	It would be great for disk formatters, file system makers, etc.
 *
 */

#include "globals.h"

#define COMMENTCHAR '#'		/* Device table comment character */

#ifndef isspace
#define isspace(a) ((a)==' '||(a)=='\t'||(a)=='\n'||(a)=='\f'||(a)=='\r')
#endif

#if amigados
#define NAMEMATCH(a,b) (s_tolower(a) == s_tolower(b))
#else
#define NAMEMATCH(a,b) (a == b) 
#endif

#include "deftab.h"

static FILE *brutabfp;
static char *brutabname;
static char *brutab;
static int brutablinno;
static struct device newdev;

static VOID InitNewDev PROTO((void));
static VOID FindTable PROTO((void));
static BOOLEAN FindEntry PROTO((char *devname, char *bp));
static BOOLEAN NamedEntry PROTO((char *name, char *bp));
static VOID NextEntry PROTO((char *bp));
static BOOLEAN FirstEntry PROTO((char *bp));
static BOOLEAN NameMatch PROTO((char *name, char *bp));
static char *fgetlr PROTO((char *bp, int bpsize, FILE *fp));
static char *sgetlr PROTO((char *bp, int bpsize, char *sp));
static char *sgets PROTO((char *bp, int bpsize, char *sp));
static struct device *decode PROTO((char *bp));
static long dgetnum PROTO((char *id, char *bp));
static char *dgetstr PROTO((char *id, char *bp));
static BOOLEAN dgetflag PROTO((char *id, char *bp));
static int symbolic PROTO((char *cp));
static S32BIT dv_atol PROTO((char *cp));

/*
 *	A translation table to translate strings in device table entries
 *	into appropriate values.  Rather than ifdef for all possible
 *	combinations, just ifdef each one.  Kinda ugly but maximally
 *	portable.
 *
 *	Also, the translation table used to be built with the aid of
 *	a macro like:
 *
 *		#define TRANSPAIR(a) {"a", a}
 *
 *	such that
 *
 *		TRANSPAIR(EIO)	expanded to	{"EIO", EIO}
 *
 *	but this was dependent on having a Reiser style cpp, and gave no
 *	warnings if it instead expanded as:
 *
 *		TRANSPAIR(EIO)	expanded to	{"a", EIO}
 *
 *	So, we're back to doing it the hard way until ANSI C blesses
 *	this usage and everyone conforms (by about the year 2000).
 *
 */

struct trans {
    char *t_name;
    int t_value;
};

static struct trans translations[] = {
#ifdef EPERM
    {"EPERM", EPERM},
#endif
#ifdef ENOENT
    {"ENOENT", ENOENT},
#endif
#ifdef ESRCH
    {"ESRCH", ESRCH},
#endif
#ifdef EINTR
    {"EINTR", EINTR},
#endif
#ifdef EIO
    {"EIO", EIO},
#endif
#ifdef ENXIO
    {"ENXIO", ENXIO},
#endif
#ifdef E2BIG
    {"E2BIG", E2BIG},
#endif
#ifdef ENOEXEC
    {"ENOEXEC", ENOEXEC},
#endif
#ifdef EBADF
    {"EBADF", EBADF},
#endif
#ifdef ECHILD
    {"ECHILD", ECHILD},
#endif
#ifdef EAGAIN
    {"EAGAIN", EAGAIN},
#endif
#ifdef ENOMEM
    {"ENOMEM", ENOMEM},
#endif
#ifdef EACCES
    {"EACCES", EACCES},
#endif
#ifdef EFAULT
    {"EFAULT", EFAULT},
#endif
#ifdef ENOTBLK
    {"ENOTBLK", ENOTBLK},
#endif
#ifdef EBUSY
    {"EBUSY", EBUSY},
#endif
#ifdef EEXIST
    {"EEXIST", EEXIST},
#endif
#ifdef EXDEV
    {"EXDEV", EXDEV},
#endif
#ifdef ENODEV
    {"ENODEV", ENODEV},
#endif
#ifdef ENOTDIR
    {"ENOTDIR", ENOTDIR},
#endif
#ifdef EISDIR
    {"EISDIR", EISDIR},
#endif
#ifdef EINVAL
    {"EINVAL", EINVAL},
#endif
#ifdef ENFILE
    {"ENFILE", ENFILE},
#endif
#ifdef EMFILE
    {"EMFILE", EMFILE},
#endif
#ifdef ENOTTY
    {"ENOTTY", ENOTTY},
#endif
#ifdef ETXTBSY
    {"ETXTBSY", ETXTBSY},
#endif
#ifdef EFBIG
    {"EFBIG", EFBIG},
#endif
#ifdef ENOSPC
    {"ENOSPC", ENOSPC},
#endif
#ifdef ESPIPE
    {"ESPIPE", ESPIPE},
#endif
#ifdef EROFS
    {"EROFS", EROFS},
#endif
#ifdef EMLINK
    {"EMLINK", EMLINK},
#endif
#ifdef EPIPE
    {"EPIPE", EPIPE},
#endif
#ifdef EDOM
    {"EDOM", EDOM},
#endif
#ifdef ERANGE
    {"ERANGE", ERANGE},
#endif
#ifdef EWOULDBLOCK
    {"EWOULDBLOCK", EWOULDBLOCK},
#endif
#ifdef EINPROGRESS
    {"EINPROGRESS", EINPROGRESS},
#endif
#ifdef EALREADY
    {"EALREADY", EALREADY},
#endif
#ifdef ENOTSOCK
    {"ENOTSOCK", ENOTSOCK},
#endif
#ifdef EDESTADDRREQ
    {"EDESTADDRREQ", EDESTADDRREQ},
#endif
#ifdef EMSGSIZE
    {"EMSGSIZE", EMSGSIZE},
#endif
#ifdef EPROTOTYPE
    {"EPROTOTYPE", EPROTOTYPE},
#endif
#ifdef ENOPROTOOPT
    {"ENOPROTOOPT", ENOPROTOOPT},
#endif
#ifdef EPROTONOSUPPORT
    {"EPROTONOSUPPORT", EPROTONOSUPPORT},
#endif
#ifdef ESOCKTNOSUPPORT
    {"ESOCKTNOSUPPORT", ESOCKTNOSUPPORT},
#endif
#ifdef EOPNOTSUPP
    {"EOPNOTSUPP", EOPNOTSUPP},
#endif
#ifdef EPFNOSUPPORT
    {"EPFNOSUPPORT", EPFNOSUPPORT},
#endif
#ifdef EAFNOSUPPORT
    {"EAFNOSUPPORT", EAFNOSUPPORT},
#endif
#ifdef EADDRINUSE
    {"EADDRINUSE", EADDRINUSE},
#endif
#ifdef EADDRNOTAVAIL
    {"EADDRNOTAVAIL", EADDRNOTAVAIL},
#endif
#ifdef ENETDOWN
    {"ENETDOWN", ENETDOWN},
#endif
#ifdef ENETUNREACH
    {"ENETUNREACH", ENETUNREACH},
#endif
#ifdef ENETRESET
    {"ENETRESET", ENETRESET},
#endif
#ifdef ECONNABORTED
    {"ECONNABORTED", ECONNABORTED},
#endif
#ifdef ECONNRESET
    {"ECONNRESET", ECONNRESET},
#endif
#ifdef ENOBUFS
    {"ENOBUFS", ENOBUFS},
#endif
#ifdef EISCONN
    {"EISCONN", EISCONN},
#endif
#ifdef ENOTCONN
    {"ENOTCONN", ENOTCONN},
#endif
#ifdef ESHUTDOWN
    {"ESHUTDOWN", ESHUTDOWN},
#endif
#ifdef ETIMEDOUT
    {"ETIMEDOUT", ETIMEDOUT},
#endif
#ifdef ECONNREFUSED
    {"ECONNREFUSED", ECONNREFUSED},
#endif
#ifdef ELOOP
    {"ELOOP", ELOOP},
#endif
#ifdef ENAMETOOLONG
    {"ENAMETOOLONG", ENAMETOOLONG},
#endif
#ifdef ENOTEMPTY
    {"ENOTEMPTY", ENOTEMPTY},
#endif
#ifdef ENOMSG
    {"ENOMSG", ENOMSG},
#endif
#ifdef EIDRM
    {"EIDRM", EIDRM},
#endif
#ifdef ECHRNG
    {"ECHRNG", ECHRNG},
#endif
#ifdef EL2NSYNC
    {"EL2NSYNC", EL2NSYNC},
#endif
#ifdef EL3HLT
    {"EL3HLT", EL3HLT},
#endif
#ifdef EL3RST
    {"EL3RST", EL3RST},
#endif
#ifdef ELNRNG
    {"ELNRNG", ELNRNG},
#endif
#ifdef EUNATCH
    {"EUNATCH", EUNATCH},
#endif
#ifdef ENOCSI
    {"ENOCSI", ENOCSI},
#endif
#ifdef EL2HLT
    {"EL2HLT", EL2HLT},
#endif
#ifdef EDEADLOCK
    {"EDEADLOCK", EDEADLOCK},
#endif
    NULL				/* Used to mark end of list */
};


/*
 *  FUNCTION
 *
 *	get_ardp    look up and translate one entry in the device table
 *
 *  SYNOPSIS
 *
 *	struct device *get_ardp (devname)
 *	char *devname;
 *
 *  DESCRIPTION
 *
 *	Given the name of a device being used as an archive, looks
 *	it up in the device table and returns a pointer to the structure
 *	found.  If not found in table then returns NULL.  Also, if
 *	the name is not found in the table and it names a character
 *	or block special device, then a warning message is printed,
 *	to encourage administrators to include a brutab entry.  The
 *	warning does not prevent use of the device, however such use
 *	is still subject to the other normal unix permission checking.
 *
 *	If the name is not specified (devname is NULL), then the first device
 *	in the table is selected.  This allows the first device to be
 *	the default device.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin get_ardp
 *	    Default is no device found
 *	    Clear out any leftover entries from last device
 *	    Find a device table somewhere (can't fail)
 *	    If the requested entry is found then
 *		Decode the entry
 *	    End if
 *	    If entry was found in a file
 *		Close the file, we are done with it for now
 *	    End if
 *	    Return pointer to the decoded entry
 *	End get_ardp
 *
 */

struct device *get_ardp (devname)
char *devname;
{
    char buf[BRUTABSIZE];
    struct device *newardp = NULL;

    DBUG_ENTER ("get_ardp");
    InitNewDev ();
    FindTable ();
    if (FindEntry (devname, buf)) {
	newardp = decode (buf);
    }
    if (brutabfp != NULL) {
	(VOID) s_fclose (brutabfp);
    }
    DBUG_RETURN (newardp);
}


/*
 *  FUNCTION
 *
 *	InitNewDev    initialize all fields of the device structure to default
 *
 *  SYNOPSIS
 *
 *	static VOID InitNewDev ();
 *
 *  DESCRIPTION
 *
 *	Insures that all fields of the device structure are reinitialized to
 *	zero, so there is no possibility of "leftovers" from the previous
 *	device.
 *
 */
 
static VOID InitNewDev ()
{
    DBUG_ENTER ("InitNewDev");
    newdev.dv_dev = NULL;
    newdev.dv_handler = NULL;
    newdev.dv_flags = 0;
    newdev.dv_msize = 0;
    newdev.dv_bsize = 0;
    newdev.dv_seek = 0;
    newdev.dv_prerr = 0;
    newdev.dv_pwerr = 0;
    newdev.dv_zrerr = 0;
    newdev.dv_zwerr = 0;
    newdev.dv_frerr = 0;
    newdev.dv_fwerr = 0;
    newdev.dv_wperr = 0;
    newdev.dv_shmseg = 0;
    newdev.dv_shmmax = 0;
    newdev.dv_shmall = 0;
    newdev.dv_unit = 0;
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	FindTable    locate either the external table or internal table
 *
 *  SYNOPSIS
 *
 *	static VOID FindTable ()
 *
 *  DESCRIPTION
 *
 *	Find the external table and make it available for reading.  If
 *	this fails, then find the internal table.  Note that after this
 *	routine is called, some table will be available for reading,
 *	even if it is the internal table, and appropriate warning
 *	messages will have been issued.
 *
 *	If BRUTAB is defined and begins with a '/' character then
 *	it is taken to be the pathname of the brutab file and
 *	an attempt is made to open it, otherwise the BRUTAB string
 *	itself is taken to be the device table.  If the table is
 *	not found, either directly or indirectly, through the BRUTAB
 *	variable, then the default brutab file is used instead.  If
 *	this also fails, then the internal default table is used as
 *	a last resort.
 *
 *  NOTES
 *
 *	Because this code is called each time a volume switch is done,
 *	to rescan the device table for the (possibly new) device name,
 *	it is possible that a warning message will be issued at each
 *	volume switch.  Adding a static switch to only allow this
 *	printing to only occur the first time we are called will solve
 *	this problem, but will then leave a hidden time bomb in the code
 *	whereby if the brutab file suddenly becomes unreadable for some
 *	reason, later during the same execution, we will never be told
 *	about it.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin FindTable
 *	    Forget any previous table found in a file
 *	    Forget any previous internal/environment table
 *	    Start table line number count at zero
 *	    If there is a BRUTAB environment string then
 *		If the string is not null then
 *		    If the string is not a pathname then
 *			Remember string as bru table
 *			Remember name as environ string
 *		    Else
 *			If file is accessable by user for read then
 *			    Try to open file with that pathname.
 *			    If not successful then
 *				Notify user can't open brutab file
 *			    End if
 *			End if
 *		    End if
 *		End if
 *	    End if
 *	    If no file open and entry not found in environment then
 *		Use the default bru table file name
 *		If default file accessable by user then
 *		    Attempt to open the default file.
 *		    If not successful then
 *			Notify user can't open brutab file
 *		    End if
 *		End if
 *	    End if
 *	    If still no file or environment table found then
 *		Remember internal table as bru table
 *		Remember we used internal table as bru table
 *		Issue warning about using default table
 *	    Endif
 *	End FindTable
 *
 */

static VOID FindTable ()
{
    struct bstat bs;

    DBUG_ENTER ("FindTable");
    brutabfp = NULL;
    brutab = NULL;
    brutablinno = 0;
    if ((brutabname = info.bru_brutab) != NULL) {
	DBUG_PRINT ("brutab", ("BRUTAB=%s", brutabname));
	if (*brutabname != EOS) {
	    if (!bstat (brutabname, &bs, FALSE) || !IS_REG (bs.bst_mode)) {
		brutab = brutabname;
		brutabname = "BRUTAB environment string";
		DBUG_PRINT ("brutab", ("use BRUTAB env string as table"));
	    } else {
		DBUG_PRINT ("brutab", ("open '%s' as brutab file", brutabname));
		if (file_access (brutabname, A_READ, TRUE)) {
		    if ((brutabfp = s_fopen (brutabname, "r")) == NULL) {
			bru_message (MSG_OPEN, brutabname);
		    }
		}
	    }
	}
    }
    if (brutabfp == NULL && brutab == NULL) {
	brutabname = BRUTAB;
	DBUG_PRINT ("brutab", ("use default brutab %s", brutabname));
	if (file_access (brutabname, A_READ, TRUE)) {
	    if ((brutabfp = s_fopen (brutabname, "r")) == NULL) {
		bru_message (MSG_OPEN, brutabname);
	    }
	}
    }
    if (brutabfp == NULL && brutab == NULL) {
	brutab = deftab;
	brutabname = "internal device table";
	bru_message (MSG_BRUTAB);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	FindEntry    find the desired entry in the bru table
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN FindEntry (devname, buf)
 *	char *devname;
 *	char *buf;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a device name, locates that entry in the table
 *	and copies it to buf.  Buf must be large enough to handle the
 *	maximum entry size (BUFTABSIZE).  If devname is NULL, the first
 *	entry in the table is selected as the default.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin FindEntry
 *	    Default return will be failure
 *	    If looking for a specific entry then
 *		If specific entry found and read into buffer then
 *		    Remember were we are going to put the name
 *		    Return status will be success
 *		    Make a local copy of the device name
 *		Endif
 *	    Else
 *		If the first entry was found and read into buffer then
 *		    Remember were we are going to put the name
 *		    Return status will be success
 *		    Set up copy pointer into local buffer
 *		    While there is any leading whitespace on entry
 *			Skip over leading whitespace
 *		    End while
 *		    While there is another valid name character
 *			Stash character away in local buffer
 *		    End while
 *		    Terminate local copy of name
 *		Endif
 *	    Endif
 *	    Return status
 *	End FindEntry
 *
 */

static BOOLEAN FindEntry (devn, bp)
char *devn;
char *bp;
{
    char *dnamep;
    BOOLEAN status;
    static char dname[128];
    struct bstat bsbuf;

    DBUG_ENTER ("FindEntry");
    status = FALSE;
    if (devn != NULL) {
	DBUG_PRINT ("devn", ("look for device %s", devn));
	if (NamedEntry (devn, bp)) {
	    newdev.dv_dev = dname;
	    status = TRUE;
	    (VOID) s_strcpy (dname, devn);
	} else if (bstat (devn, &bsbuf, FALSE) && (IS_SPEC (bsbuf.bst_mode))) {
	    bru_message (MSG_NOENTRY, devn, brutabname);
	}
    } else {
	DBUG_PRINT ("devn", ("get first entry in table as default device"));
	if (FirstEntry (bp)) {
	    newdev.dv_dev = dname;
	    status = TRUE;
	    dnamep = dname;
	    while (isspace (*bp)) {
		bp++;
	    }
	    while ((*bp != EOS) && (*bp != '|') && !isspace (*bp)) {
		*dnamep++ = *bp++;
	    }
	    *dnamep = EOS;
	    DBUG_PRINT ("devn", ("got device name '%s'", newdev.dv_dev));
	}
    }
    DBUG_RETURN (status);
}


/*
 *  FUNCTION
 *
 *	NamedEntry   load buffer with entry for specified device
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN NamedEntry (name, bp)
 *	char *name;
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Extracts the entry for device <name> from the brutab table
 *	and places it in the character buffer <bp>.   It is currently
 *	assumed that bp is at least BRUTABSIZE characters.  If the entry in
 *	the brutab table is larger than BRUTABSIZE-1 characters the excess
 *	characters will be discarded and an appropriate warning message
 *	will be issued.
 *
 *  RETURNS
 *
 *	FALSE if no entry matches <name>
 *	TRUE if an entry is found
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin NamedEntry
 *	    Default status will be failure
 *	    Do
 *		Clear any previous contents from buffer
 *		Get next entry from table
 *		If a table entry was found
 *		    Test entry for name wanted
 *		End if
 *	    While an entry found and not the one wanted
 *	    Return status
 *	End NamedEntry
 *
 */

static BOOLEAN NamedEntry (name, bp)
char *name;				/* Pointer to device entry to find */
char *bp;				/* Pointer to buffer[BRUTABSIZE] */
{
    BOOLEAN status;

    DBUG_ENTER ("NamedEntry");
    status = FALSE;
    DBUG_PRINT ("brutab", ("look for named device '%s'", name));
    do {
	*bp = EOS;
	NextEntry (bp);
	if (*bp != EOS) {
	    status = NameMatch (name, bp);
	}
    } while ((*bp != EOS) && !status);
    DBUG_PRINT ("nentry", ("returned status %d", status));
    DBUG_RETURN (status);
}

/*
 *  FUNCTION
 *
 *	NextEntry    extract next entry from the bru table
 *
 *  SYNOPSIS
 *
 *	static VOID NextEntry (bp)
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Grab the next entry out of the bru table and stuff it away in
 *	the entry buffer.
 *
 */

/*
 *  PSEUDO CODE
 *
 *	Begin NextEntry
 *	    Do
 *		Clear out any previous contents of entry buffer
 *		If table is from a file then
 *		    Get the next entry from file and put in buffer
 *		Else if table is from a string then
 *		    Get the next entry from string and put in buffer
 *		Else
 *		    Report internal bug
 *		Endif
 *	    While another entry and not a comment or blank line
 *	End NextEntry
 */

static VOID NextEntry (bp)
char *bp;				/* Pointer to buffer[BRUTABSIZE] */
{
    DBUG_ENTER ("NextEntry");
    do {
	*bp = EOS;
	if (brutabfp != NULL) {
	    DBUG_PRINT ("nxtent", ("get next entry from brutab file"));
	    (VOID) fgetlr (bp, BRUTABSIZE, brutabfp);
	} else if (brutab != NULL) {
	    DBUG_PRINT ("nxtent", ("get next entry from brutab string"));
	    brutab = sgetlr (bp, BRUTABSIZE, brutab);
	} else {
	    bru_message (MSG_BUG, "NextEntry");
	}
    } while (*bp != EOS && (*bp == COMMENTCHAR || *bp == '\n'));
    DBUG_PRINT ("btline", ("got entry ending at line %d", brutablinno));
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	FirstEntry    extract first entry from table and place in buffer
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN FirstEntry (bp)
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Extract the first entry from the bru table and place it in the
 *	entry buffer.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin FirstEntry
 *	    Get the next (really first) entry
 *	    If no entry found then
 *		Result is failure
 *	    Else
 *		Result is success
 *	    Endif
 *	End FirstEntry
 */

static BOOLEAN FirstEntry (bp)
char *bp;				/* Pointer to buffer[BRUTABSIZE] */
{
    BOOLEAN status;
    
    DBUG_ENTER ("FirstEntry");
    NextEntry (bp);
    if (*bp == EOS) {
	status = FALSE;
    } else {
	status = TRUE;
    }
    DBUG_RETURN (status);
}


/*
 *  FUNCTION
 *
 *	NameMatch   test to see if entry is for specified device
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN NameMatch (name, bp)
 *	char *name;
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Tests to see if the entry in buffer bp matches the device
 *	specified by name.  Returns TRUE if match is detected, FALSE
 *	otherwise.  First discards any leading whitespace.  Then examines
 *	the next string found, up to whitespace or a '|' character, to
 *	see if the string matches the given name.  If so, then returns
 *	true.  If not, and the last scan was terminated by a '|', then
 *	recursively calls itself with the rest of the buffer to look
 *	at the next string as a name.  This continues until a match
 *	is found, or until the scan is terminated by something other than
 *	a '|' character.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin NameMatch
 *	    Default return status is FALSE
 *	    Initialize name scan pointer.
 *	    While there is any leading whitespace on entry
 *		Skip over the whitespace
 *	    End while
 *	    While match is found and not end of name
 *		Go on to next name character
 *		Go on to next buffer (entry) character
 *	    End while
 *	    If used all of name and ended up at non-name buffer character
 *		Status is TRUE since a match was found.
 *	    Else
 *		While not yet at start of next alternate name
 *		    Resync by skipping buffer character
 *		End while
 *		If an alternate name separater character
 *		    Test next name and return results.
 *		End if
 *	    End if
 *	    Return status
 *	End NameMatch
 *
 */

static BOOLEAN NameMatch (name, bp)
char *name;
char *bp;
{
    char *np;
    BOOLEAN status;
 
    DBUG_ENTER ("NameMatch");
    status = FALSE;
    np = name;
    DBUG_PRINT ("nname", ("look for name %s", name));
    while (isspace (*bp)) {
	bp++;
    }
    while (NAMEMATCH (*np, *bp) && (*np != EOS)) {
	np++;
	bp++;
    }
    if ((*np == EOS) && (isspace (*bp) || (*bp == EOS) || (*bp == '|'))) {
	status = TRUE;
    } else {
	while ((*bp != EOS) && (*bp != '|') && !isspace (*bp)) {
	    bp++;
	}
	while (isspace (*bp)) {
	    bp++;
	}
	if (*bp == '|') {
	    status = NameMatch (name, ++bp);
	}
    }
    DBUG_PRINT ("new", ("return status %d", status));
    DBUG_RETURN (status);
}


/*
 *  FUNCTION
 *
 *	fgetlr    get logical record from a file
 *
 *  SYNOPSIS
 *
 *	char *fgetlr (bp, bpsize, fp)
 *	char *bp;
 *	int bpsize;
 *	FILE *fp;
 *
 *  DESCRIPTION
 *
 *	Reads the next logical record from stream "fp" into buffer "bp"
 *	until next unescaped newline, "bpsize" minus one characters
 *	have been read, end of file, or read error.
 *	The last character read is followed by an EOS.
 *
 *	A logical record may span several physical records by having
 *	each newline escaped with the standard C escape character
 *	(backslash).
 *
 *	This is particularly useful for things like the termcap
 *	file, where a single entry is too long for one physical
 *	line, yet needs to be treated as a single record.
 *
 *	Returns its first argument unless an end of file or read
 *	error occurs prior to any characters being read.
 *
 *  BUGS
 *
 *	The only way to know if read was terminated due to buffer size
 *	limitation is to test for a newline before the terminating
 *	null.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin fgetlr
 *	    If read fails then
 *		Return will be NULL.
 *	    Else
 *		Find out how many characters were read.
 *		Initialize pointer to terminating null.
 *		If last char read was newline then
 *		    If newline was escaped then
 *			Replace backslash with the newline.
 *			Replace newline with null.
 *			Read and append more.
 *		    End if
 *		End if
 *		Return buffer pointer.
 *	    End if
 *	    Return result.
 *	End fgetlr
 *
 */

static char *fgetlr (bp, bpsize, fp)
char *bp;
int bpsize;
FILE *fp;
{
    int numch;
    char *cp;
    char *rtnval;

    DBUG_ENTER ("fgetlr");
    brutablinno++;
    if (s_fgets (bp, bpsize, fp) == NULL) {
	rtnval = NULL;
    } else {
	numch = s_strlen (bp);
	cp = &bp[numch];
	if (*--cp == '\n') {
	    if (numch > 1 && *--cp == '\\') {
		*cp++ = '\n';
		*cp = EOS;
		(VOID) fgetlr (cp, bpsize - numch + 1, fp);
	    }
	}
	rtnval = bp;
    }
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	sgetlr    get logical record from a string
 *
 *  SYNOPSIS
 *
 *	char *sgetlr (bp, bpsize, sp)
 *	char *bp;
 *	int bpsize;
 *	char *sp;
 *
 *  DESCRIPTION
 *
 *	Reads the next logical record from string "sp" into buffer "bp"
 *	until next unescaped newline, "bpsize" minus one characters
 *	have been read, or end of string.
 *
 *	The last character read is followed by an EOS.
 *
 *	A logical record may span several physical records by having
 *	each newline escaped with the standard C escape character
 *	(backslash).
 *
 *	This is particularly useful for things like the termcap
 *	file, where a single entry is too long for one physical
 *	line, yet needs to be treated as a single record.
 *
 *  BUGS
 *
 *	The only way to know if read was terminated due to buffer size
 *	limitation is to test for a newline before the terminating
 *	null.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin sgetlr
 *	    If at end of input string
 *		Return will be NULL.
 *	    Else
 *		Find out how many characters were read.
 *		Initialize pointer to terminating null.
 *		If last char read was newline then
 *		    If newline was escaped then
 *			Replace backslash with the newline.
 *			Replace newline with null.
 *			Read and append more.
 *		    End if
 *		End if
 *		Return buffer pointer.
 *	    End if
 *	    Return result.
 *	End sgetlr
 *
 */

static char *sgetlr (bp, bpsize, sp)
char *bp;
int bpsize;
char *sp;
{
    int numch;
    char *cp;

    DBUG_ENTER ("sgetlr");
    brutablinno++;
    sp = sgets (bp, bpsize, sp);
    if (*bp != EOS) {
	numch = s_strlen (bp);
	cp = &bp[numch];
	if (*--cp == '\n') {
	    if (numch > 1 && *--cp == '\\') {
		*cp++ = '\n';
		*cp = EOS;
		sp = sgetlr (cp, bpsize - numch + 1, sp);
	    }
	}
    }
    DBUG_RETURN (sp);
}


/*
 *  FUNCTION
 *
 *	sgets    get next line from a string
 *
 *  SYNOPSIS
 *
 *	static char *sgets (bp, bpsize, sp)
 *	char *bp;
 *	int bpsize;
 *	char *sp;
 *
 *  DESCRIPTION
 *
 *	Is the analog of fgets for files, but works on strings instead.
 *
 */

static char *sgets (bp, bpsize, sp)
char *bp;
int bpsize;
char *sp;
{
    DBUG_ENTER ("sgets");
    while ((*sp != EOS) && (--bpsize > 0)) {
	if ((*bp++ = *sp++) == '\n') {
	    break;
	}
	*bp = EOS;
    }
    DBUG_RETURN (sp);
}


/*
 *  FUNCTION
 *
 *	decode    decode a new format device entry
 *
 *  SYNOPSIS
 *
 *	static struct device *decode (bp)
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Decode a new style entry and return a pointer
 *	to the internal format.
 *
 */

static struct device *decode (bp)
char *bp;
{
    DBUG_ENTER ("decode");
    newdev.dv_handler = dgetstr ("device", bp);
    if (dgetflag ("reblocks", bp)) {
	DBUG_PRINT ("dvflag", ("found reblocks"));
	newdev.dv_flags |= D_REBLOCKS;
    }
    if (dgetflag ("ignoreclose", bp)) {
	DBUG_PRINT ("dvflag", ("found ignoreclose"));
	newdev.dv_flags |= D_IGNORECLOSE;
    }
    if (dgetflag ("rawfloppy", bp)) {
	DBUG_PRINT ("dvflag", ("found rawfloppy"));
	newdev.dv_flags |= D_RAWFLOPPY;
    }
    if (dgetflag ("shmcopy", bp)) {
	DBUG_PRINT ("dvflag", ("found shmcopy"));
	newdev.dv_flags |= D_SHMCOPY;
    }
    if (dgetflag ("format", bp)) {
	DBUG_PRINT ("dvflag", ("found format"));
	newdev.dv_flags |= D_FORMAT;
    }
    if (dgetflag ("eject", bp)) {
	DBUG_PRINT ("dvflag", ("found eject"));
	newdev.dv_flags |= D_EJECT;
    }
    if (dgetflag ("qfwrite", bp)) {
	DBUG_PRINT ("dvflag", ("found qfwrite"));
	newdev.dv_flags |= D_QFWRITE;
    }
    if (dgetflag ("reopen", bp)) {
	DBUG_PRINT ("dvflag", ("found reopen"));
	newdev.dv_flags |= D_REOPEN;
    }
    if (dgetflag ("noreopen", bp)) {
	DBUG_PRINT ("dvflag", ("found noreopen"));
	newdev.dv_flags |= D_NOREOPEN;
    }
    if (dgetflag ("tape", bp)) {
	DBUG_PRINT ("dvflag", ("found tape"));
	newdev.dv_flags |= D_ISTAPE;
    }
    if (dgetflag ("rawtape", bp)) {
	DBUG_PRINT ("dvflag", ("found rawtape"));
	newdev.dv_flags |= D_RAWTAPE;
    }
    if (dgetflag ("norewind", bp)) {
	DBUG_PRINT ("dvflag", ("found norewind"));
	newdev.dv_flags |= D_NOREWIND;
    }
    if (dgetflag ("advance", bp)) {
	DBUG_PRINT ("dvflag", ("found advance"));
	newdev.dv_flags |= D_ADVANCE;
    }
    newdev.dv_msize = (ULONG) dgetnum ("size", bp);
    DBUG_PRINT ("msize", ("msize = %lu", newdev.dv_msize));
    newdev.dv_bsize = (UINT) dgetnum ("bufsize", bp);
    if (newdev.dv_bsize == 0) {
	newdev.dv_bsize = BUFSIZE;
    }
    newdev.dv_maxbsize = (UINT) dgetnum ("maxbufsize", bp);
    newdev.dv_blocksize = (UINT) dgetnum ("blocksize", bp);
    newdev.dv_shmseg = (UINT) dgetnum ("shmseg", bp);
    newdev.dv_shmmax = dgetnum ("shmmax", bp);
    newdev.dv_shmall = dgetnum ("shmall", bp);
    DBUG_PRINT ("bsize", ("bsize = %ld", newdev.dv_bsize));
    newdev.dv_seek = (int) dgetnum ("seek", bp);
    DBUG_PRINT ("seek", ("minseek = %d", newdev.dv_seek));
    newdev.dv_prerr = (int) dgetnum ("prerr", bp);
    DBUG_PRINT ("prerr", ("prerr = %d", newdev.dv_prerr));
    newdev.dv_pwerr = (int) dgetnum ("pwerr", bp);
    DBUG_PRINT ("pwerr", ("pwerr = %d", newdev.dv_pwerr));
    newdev.dv_zrerr = (int) dgetnum ("zrerr", bp);
    DBUG_PRINT ("zrerr", ("zrerr = %d", newdev.dv_zrerr));
    newdev.dv_zwerr = (int) dgetnum ("zwerr", bp);
    DBUG_PRINT ("zwerr", ("zwerr = %d", newdev.dv_zwerr));
    newdev.dv_frerr = (int) dgetnum ("frerr", bp);
    DBUG_PRINT ("frerr", ("frerr = %d", newdev.dv_frerr));
    newdev.dv_fwerr = (int) dgetnum ("fwerr", bp);
    DBUG_PRINT ("fwerr", ("fwerr = %d", newdev.dv_fwerr));
    newdev.dv_wperr = (int) dgetnum ("wperr", bp);
    DBUG_PRINT ("wperr", ("wperr = %d", newdev.dv_wperr));
    newdev.dv_ederr = (int) dgetnum ("ederr", bp);
    DBUG_PRINT ("ederr", ("ederr = %d", newdev.dv_ederr));
    newdev.dv_unit = (int) dgetnum ("unit", bp);
    DBUG_PRINT ("unit", ("unit = %d", newdev.dv_unit));
#if amigados
    if ((newdev.dv_flags & D_RAWTAPE) || (newdev.dv_flags & D_RAWFLOPPY)) {
	if (newdev.dv_handler != NULL) {
	    (VOID) AddRawDevice (&newdev);
	}
    }
#endif
    DBUG_RETURN (&newdev);
}


/*
 *  FUNCTION
 *
 *	dgetnum   extract numeric option from brutab entry
 *
 *  SYNOPSIS
 *
 *	static long dgetnum (id, bp)
 *	char *id;
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Returns numeric value of capability <id>, or 0 if <id>
 *	is not found.
 *
 */

static long dgetnum (id, bp)
char *id;
char *bp;
{
    char *bufp;
    long value;
    char buf[256];

    DBUG_ENTER ("dgetnum");
    value = 0;
    while (*bp != EOS) {
	while (isspace (*bp)) {
	    bp++;
	}
	if (*bp != EOS) {
	    bufp = buf;
	    while (*bp != EOS && !isspace (*bp)) {
		*bufp++ = *bp++;
	    }
	    *bufp = EOS;
	    DBUG_PRINT ("dgetnum", ("examine field '%s' for '%s'", buf, id));
	    bufp = s_strchr (buf, '=');
	    if (bufp != NULL) {
		*bufp++ = EOS;
		if (STRSAME (id, buf)) {
		    value = dv_atol (bufp);
		    break;
		}
	    }
	}
    }
    DBUG_RETURN (value);
}


/*
 *  FUNCTION
 *
 *	dgetstr   extract string option from brutab entry
 *
 *  SYNOPSIS
 *
 *	static char *dgetnum (id, bp)
 *	char *id;
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Returns string value of capability <id>, or NULL if <id>
 *	is not found.  The string is stored in memory allocated
 *	by malloc.
 *
 */

static char *dgetstr (id, bp)
char *id;
char *bp;
{
    char *bufp;
    char *result;
    char buf[256];

    DBUG_ENTER ("dgetstr");
    result = NULL;
    while (*bp != EOS) {
	while (isspace (*bp)) {
	    bp++;
	}
	if (*bp != EOS) {
	    bufp = buf;
	    while (*bp != EOS && !isspace (*bp)) {
		*bufp++ = *bp++;
	    }
	    *bufp = EOS;
	    DBUG_PRINT ("dgetstr", ("examine field '%s' for '%s'", buf, id));
	    bufp = s_strchr (buf, '=');
	    if (bufp != NULL) {
		*bufp++ = EOS;
		if (STRSAME (id, buf)) {
		    result = s_strdup (bufp);
		    DBUG_PRINT ("dgetstr", ("got string '%s'", result));
		    break;
		}
	    }
	}
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	dgetflag   extract boolean option from brutab entry
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN dgetflag (id, bp)
 *	char *id;
 *	char *bp;
 *
 *  DESCRIPTION
 *
 *	Returns boolean value of capability <id>.
 *
 */

static BOOLEAN dgetflag (id, bp)
char *id;
char *bp;
{
    char *bufp;
    BOOLEAN result;
    char buf[256];

    DBUG_ENTER ("dgetflag");
    result = FALSE;    
    while (*bp != EOS) {
	while (isspace (*bp)) {
	    bp++;
	}
	if (*bp != EOS) {
	    bufp = buf;
	    while (*bp != EOS && !isspace (*bp)) {
		*bufp++ = *bp++;
	    }
	    *bufp = EOS;
	    DBUG_PRINT ("dgetnum", ("examine field '%s' for '%s'", buf, id));
	    if (STRSAME (id, buf)) {
		result = TRUE;
		break;
	    }
	}
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	symbolic    convert symbolic value using translation table
 *
 *  SYNOPSIS
 *
 *	static int symbolic (cp)
 *	char *cp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a symbolic value string, converts the
 *	string to internal numeric format using a translation
 *	table of known strings and their values.
 *
 *	The table is searched in the simplest manner possible,
 *	with a linear search.  It is not expected that the
 *	time to search the table is significant.
 *
 *	Symbolic fields separated with the "|" character are
 *	translated individually and "or'd" with the previous value.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin symbolic
 *	    Initialize value to zero
 *	    Initialize string scan pointer to start of string
 *	    Do
 *		Find end of current symbolic name
 *		Remember the separator/terminator character
 *		Replace separator/terminator with end-of-string
 *		For each entry in the translation table
 *		    If entry matches the current symbolic then
 *			Accumulate arithmetic or of values
 *			Break translation table scan
 *		    End if
 *		End for
 *		Set up to translate any remaining symbolics
 *	    While there are more symbolic names
 *	    Return final value
 *	End symbolic
 *
 */
 
static int symbolic (cp)
char *cp;
{
    struct trans *tp;
    char *scan;
    int value;
    char separator;

    DBUG_ENTER ("symbolic");
    value = 0;
    scan = cp;
    do {
	while (*scan != '|' && *scan != EOS) {scan++;}
	separator = *scan;
	*scan++ = EOS;
	for (tp = translations; tp -> t_name != NULL; tp++) {
	    if (STRSAME (tp -> t_name, cp)) {
		value |= tp -> t_value;
		break;
	    }
	}
	cp = scan;
    } while (separator != EOS);
    DBUG_RETURN (value);
}    


/*
 *  FUNCTION
 *
 *	dv_atol    convert numeric or symbolic field to internal value
 *
 *  SYNOPSIS
 *
 *	static S32BIT dv_atol (cp)
 *	char *cp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a string representing either a scaled numeric
 *	value, or symbolic value, converts to the internal numeric form,
 *	using a translation table for the symbolics if necessary, and
 *	returns the resulting value.
 *
 *	Symbolic values may be combined with the form "sym|sym|...",
 *	and the resulting value is the arithmetic "or" of the
 *	individual values.  This is primarily useful for specifying
 *	capabilities flag values.
 *
 */
 

/*
 *  PSEUDO CODE
 *
 *	Begin dv_atol
 *	    If the value is a numeric string then
 *		Convert string using scaled numeric conversion
 *	    Else
 *		Convert string using translation table
 *	    End if
 *	    Return converted value
 *	End dv_atol
 *
 */
 
static S32BIT dv_atol (cp)
char *cp;
{
    S32BIT value;
    
    DBUG_ENTER ("dv_atol");
    DBUG_PRINT ("dv_atol", ("translate string \"%s\"", cp));
    if ('0' <= *cp && *cp <= '9') {
	value = getsize (cp);
    } else {
	value = symbolic (cp);
    }
    DBUG_PRINT ("dv_atol", ("value is %ld", value));
    DBUG_RETURN (value);
}
