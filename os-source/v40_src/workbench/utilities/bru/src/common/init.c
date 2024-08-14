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
 *	init.c    routines for doing initialization at startup
 *
 *  SCCS
 *
 *	@(#)init.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines for doing initialization at startup.
 *	These routines are called to process command line options,
 *	initialize uid/gid translation table, build the file tree, etc.
 *
 */

#include "globals.h"

/*
 *	Each explicit filename argument on the command line is
 *	added to the tree of files to archive.  The tree_add
 *	function is responsible for adding each name to the tree.
 *	On systems where the shell expands wildcard characters
 *	in filenames (Unix for example), the tree_add routine can be
 *	called directly.  Others, such as the Amiga, must have
 *	the arguments expanded before calling tree_add, so on these
 *	machines first call the appropriate expansion routine, which
 *	call tree_add with each expanded filename.
 */

/*
 *	Locals.
 */

static VOID options PROTO((int argc, char *argv []));
static VOID buildtree PROTO((int argc, char *argv []));
static VOID init_time PROTO((char *cp));
static VOID init_uid PROTO((char *cp));
static VOID process_opts PROTO((char *argv []));
static VOID init_info PROTO((char *argv []));
static VOID setiopt PROTO((char *option));
static VOID selfcheck PROTO((void));

/*
 *	To limit the size of the option parsing routine "options"
 *	option arguments are saved temporarily to be processed
 *	at a later time.  Also, there are some option arguments
 *	which cannot be effectively used until some other processing
 *	is done first, the -o argument for example.  The -o argument
 *	requires that ur_init be called first.
 *
 */

static char *A_arg;		/* Argument given for -A option */
static char *b_arg;		/* Argument given for -b option */
static char *n_arg;		/* Argument given for -n option */
static char *N_arg;		/* Argument given for -N option */
static char *o_arg;		/* Argument given for -o option */
static char *P_arg;		/* Argument given for -P option */
static char *s_arg;		/* Argument given for -s option */
static char *S_arg;		/* Argument given for -S option */
static char *u_arg;		/* Argument given for -u option */

#define OPTSTRING "#:aA:b:BcCdDef:FghiI:lL:mn:N:o:pP:RS:s:tu:vwxZ"


/*
 *  FUNCTION
 *
 *	init    perform initialization procedures
 *
 *  SYNOPSIS
 *
 *	VOID init (argc, argv)
 *	int argc;
 *	char *argv[];
 *
 *  DESCRIPTION
 *
 *	Performs initialization functions which are common to
 *	all operations: create, table, update, extract, etc.
 *
 *	Any errors at this time cause immediate exit.
 *
 *	On the Commodore Amiga, system resources are not automatically
 *	returned to the system when a task exits.  Thus as part of
 *	the cleanup process, we must be sure to return any resouces.
 *	For normal runs, this is taken care of, but for aborted (via
 *	control-C) runs, we must do it in the signal handling function.
 *	Thus we must set up to catch SIG_INT all the time, not just
 *	when it is necessary under UNIX.
 *
 *	Since we might also generate an error message somewhere along
 *	the way, we arbitrarily set our name to argv[0].  We deal with
 *	it more thoroughly in init_info.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin init
 *	    Initialize the execution information structure
 *	    Get command line options
 *	    If need uid/gid translation tables then
 *		Initialize the tables
 *	    End if
 *	    Process command line options
 *	    Build directory tree
 *	End init
 *
 */

VOID init (argc, argv)
int argc;
char *argv[];
{
    DBUG_ENTER ("init");
    info.bru_name = argv[0];
    options (argc, argv);
    init_info (argv);
    if (flags.oflag || flags.tflag || flags.iflag || flags.gflag) {
	ur_init ();
	gp_init ();
    }
    process_opts (argv);
    stackcheck ();
    selfcheck ();
    buildtree (argc, argv);
#if amigados
    (VOID) s_signal (SIGINT, (SIGTYPE) done);
#endif
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	options    process command line options
 *
 *  SYNOPSIS
 *
 *	static VOID options (argc, argv)
 *	int argc;
 *	char *argv[];
 *
 *  DESCRIPTION
 *
 *	Processes command line options up to the first file name
 *	specified on the command line.  Returns an updated argv
 *	pointer with will contain only the file names, starting
 *	with argv[0].
 *
 *	Note that the only processing of the arguments done at this
 *	time is redirection of the log file stream from stdout to the
 *	error stream for the special case of writing an archive to stdout.
 *	This insures that the verbosity/logfile stream can be used
 *	as soon as the command line arguments are squirreled away.
 *	All processing is deferred until later.
 *
 */

static VOID options (argc, argv)
int argc;
char *argv[];
{
    int c;

    DBUG_ENTER ("options");
    if (argc < 2) {
	usage (BRIEF);
	done ();
    }
    while ((c = s_getopt (argc, argv, OPTSTRING)) != EOF) {
	switch (c) {
	case '#':
	    DBUG_PUSH (optarg);
	    break;
	case 'a':
	    flags.aflag = TRUE;
	    break;
	case 'A': 
	    flags.Aflag = TRUE;
	    A_arg = optarg;
	    break;
	case 'b': 
	    flags.bflag = TRUE;
	    b_arg = optarg;
	    break;
	case 'B':
	    flags.Bflag = TRUE;
	    break;
	case 'c': 
	    flags.cflag = TRUE;
	    break;
	case 'C':
	    flags.Cflag = TRUE;
	    break;
	case 'd':
	    flags.dflag++;
	    break;
	case 'D':
	    flags.Dflag = TRUE;
	    break;
	case 'e':
	    flags.eflag = TRUE;
	    break;
	case 'f': 
	    flags.fflag++;
	    savedev (optarg);
	    break;
	case 'F':
	    flags.Fflag = TRUE;
	    break;
	case 'g':
	    flags.gflag = TRUE;
	    break;
	case 'h':
	    flags.hflag = TRUE;
	    break;
	case 'i':
	    flags.iflag = TRUE;
	    break;
	case 'I':
	    setiopt (optarg);
	    break;
	case 'l':
	    flags.lflag = TRUE;
	    break;
	case 'L':
	    flags.Lflag = TRUE;
	    label = optarg;
	    break;
	case 'm':
	    flags.mflag = TRUE;
	    break;
	case 'n':
	    flags.nflag = TRUE;
	    n_arg = optarg;
	    break;
	case 'N':
	    flags.Nflag = TRUE;
	    N_arg = optarg;	
	    break;
	case 'o':
	    flags.oflag = TRUE;
	    o_arg = optarg;
	    break;
	case 'p':
	    flags.pflag = TRUE;
	    break;
	case 'P':
	    flags.Pflag = TRUE;
	    P_arg = optarg;
	    break;
	case 'R':
	    flags.Rflag = TRUE;
	    break;
	case 'S':
	    flags.Sflag = TRUE;
	    flags.Zflag |= Z_AUTO;
	    S_arg = optarg;
	    break;
	case 's':
	    flags.sflag = TRUE;
	    s_arg = optarg;
	    break;
	case 't': 
	    flags.tflag = TRUE;
	    break;
	case 'u': 
	    flags.uflag = TRUE;
	    u_arg = optarg;
	    break;
	case 'v': 
	    flags.vflag++;			/* Multilevel verbosity */
	    break;
	case 'w': 
	    flags.wflag = TRUE;
	    break;
	case 'x': 
	    flags.xflag = TRUE;
	    break;
	case 'Z':
	    flags.Zflag |= Z_ON;
	    break;
	default:
	    usage (BRIEF);
	    done ();
	}
    }
    if (flags.fflag > 0) {
	copyname (afile.fname, firstdev ());
    }
    if ((flags.eflag || flags.cflag) && (flags.fflag > 0)) {
	if (afile.fname != NULL && STRSAME ("-", afile.fname)) {
	    if (logfp == stdout) {
		logfp = errfp;
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	buildtree    build directory tree from files on command line
 *
 *  SYNOPSIS
 *
 *	static VOID buildtree (argc, argv)
 *	int argc;
 *	char *argv[];
 *
 *  DESCRIPTION
 *
 *	Adds each file name specified on the command line to the
 *	directory tree.  If there are no files specified, the
 *	directory tree is "." by default.  If "-" is given
 *	instead of files, a list of files is read from the
 *	standard input and used to build the tree.
 *
 *	The flags to enable directory expansion and automatic
 *	archiving of parent directories default to different
 *	values depending upon the source of the pathnames.
 *	For pathnames read from the command line, expansion and
 *	automatic parent archiving are both enabled.  For
 *	pathnames read from standard input, expansion and automatic
 *	parent archiving are both disabled.  Either of these
 *	functions can be explicitly enabled or disabled by
 *	use of the -P command line option.
 *	
 */


/*
 *  PSEUDO CODE
 *
 *	Begin buildtree
 *	    If read files from standard input then
 *		While there is another name on standard input
 *		    Add that name to the tree
 *		End while
 *	    Else
 *		If no files given and creating archive then
 *		    Tree is simply "."
 *		Else
 *		    For each pathname on command line
 *			Add pathname to the tree
 *		    End for
 *		End if
 *	    End if
 *	End buildtree
 *
 */

static VOID buildtree (argc, argv)
int argc;
char *argv[];
{
    int index;
    char namebuf[BRUPATHMAX];

    DBUG_ENTER ("buildtree");
    if (argv[optind] != NULL && STRSAME (argv[optind], "-")) {
	if (!flags.PFflag) {
	    while (nextname (namebuf, sizeof (namebuf))) {
		if (*namebuf != EOS) {
		    TREE_ADD (namebuf);
		}
	    }
	}
    } else {
	if (argv[optind] == NULL && (flags.cflag || flags.eflag)) {
	    TREE_ADD (".");
	} else {
	    for (index = optind; index < argc; index++) {
		TREE_ADD (argv[index]);
	    }
	}
    }
    DBUG_VOID_RETURN;
}



/*
 *  FUNCTION
 *
 *	init_time    get selection time for use with -n option
 *
 *  SYNOPSIS
 *
 *	static VOID init_time (cp)
 *	char *cp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a pathname or date string, initialize the
 *	selection time for the -n option.  Note that ONLY the
 *	mtime of the file is used.  This facilitates the use
 *	of files whose only purpose is to save a particular
 *	date.  We don't look at the ctime because it is less
 *	likely to be the time intended by the user.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin init_time
 *	    Make a dummy file info structure for testing access
 *	    If the string represents an existing file then
 *		If the file can be stat'd then
 *		    The time is the modification time of the file
 *		Else
 *		    Notify user of error
 *		End if
 *	    Else
 *		Convert the string as if it was a date string
 *	    End if
 *	End init_time
 *
 */

static VOID init_time (cp)
char *cp;
{
    struct bstat bsbuf;
    char *ttime;

    DBUG_ENTER ("init_time");
    DBUG_PRINT ("time", ("convert %s", cp));
    if (file_access (cp, A_EXISTS, FALSE)) {
	if (bstat (cp, &bsbuf, TRUE)) {
	    ntime = bsbuf.bst_mtime;
	} else {
	    bru_message (MSG_NTIME, cp);
	}
    } else {
	ntime = date (cp);
    }
    if (flags.vflag > 1) {
	ttime = s_ctime ((long *) &ntime);
	ttime[s_strlen (ttime) - 1] = EOS;
	voutput ("select files modified since %s", ttime);
	vflush ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	init_uid    initialize the user id for -o option
 *
 *  SYNOPSIS
 *
 *	static VOID init_uid (cp)
 *	char *cp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a string which is either a valid password
 *	file entry name or a numeric uid, initializes the user id
 *	for use with the -o option.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin init_uid
 *	    Initialize a dummy file info structure for testing access
 *	    If the string is the name of an existing file then
 *		If the file can be stat'd then
 *		    Use the file's user as the uid
 *		Else
 *		    Notify user of error 
 *		End if
 *	    Else
 *		Translate the string to a numeric id
 *		If translation successful then
 *		    Use the numeric id as the uid
 *		Else
 *		    If the first character is numeric then
 *			Convert string to numeric value
 *		    Else
 *			Notify user of conversion error
 *		    End if
 *		End if
 *	    End if
 *	    If verbosity greater than level one then
 *		Tranlate uid for user and print name
 *	    End if
 *	End init_uid
 *
 */


static VOID init_uid (cp)
char *cp;
{
    struct bstat bsbuf;
    int intuid;

    DBUG_ENTER ("init_uid");
    DBUG_PRINT ("uid", ("convert %s", cp));
    if (file_access (cp, A_EXISTS, FALSE)) {
	if (bstat (cp, &bsbuf, TRUE)) {
	    uid = bsbuf.bst_uid;
	} else {
	    bru_message (MSG_GUID, cp);
	}
    } else {
	intuid = ur_guid (cp);
	if (intuid != -1) {
	    uid = intuid;
	} else {
	    if (s_isdigit (*cp)) {
		uid = (unsigned) s_atoi (cp);
	    } else {
		bru_message (MSG_GUID, cp);
	    }
	}
    }
    DBUG_PRINT ("uid", ("got uid %u", uid));
    if (flags.vflag > 1) {
	voutput ("select files owned by %s (uid %d)", ur_gname (uid), uid);
	vflush ();
    }
    DBUG_VOID_RETURN;
}


/*
 *	Check to ensure that the current buffer size is no larger
 *	than the maximum size of a shared memory segment.  Also
 *	guard against running on a system where shared memory support
 *	is turned off in the kernel and the size comes back as zero.
 *	Adjust the buffer size downwards if necessary.
 *
 *	We can be called more than once, so only issue double buffering
 *	warnings once.
 *
 *	If no double buffering support is compiled in, reset the
 *	Dflag to avoid side-effects such as forcing the archive device
 *	to be non-seekable.
 *
 */

VOID shmcheck ()
{
#if HAVE_SHM
    int shmsize;
#endif
    static BOOLEAN warned = FALSE;

    DBUG_ENTER ("shmcheck");
    if (flags.Dflag) {
#if HAVE_SHM
	shmsize = maxshmsize ();
	if (shmsize == 0) {
	    flags.Dflag = FALSE;
	    if ((flags.vflag > 3) && !warned) {
		bru_message (MSG_DBLBUFOFF);
		warned = TRUE;
	    }
	} else if (shmsize < bufsize) {
	    bru_message (MSG_SHMSIZE, bufsize / 1024, shmsize / 1024);
	    bufsize = BLOCKS (shmsize) * BLKSIZE;
	    bru_message (MSG_BUFADJ, bufsize / 1024);
	}
#else
	flags.Dflag = FALSE;
	if ((flags.vflag > 3) && !warned) {
	    bru_message (MSG_NODBLBUF);
	    warned = TRUE;
	}
#endif
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	process_opts    miscellaneous processing of command line options
 *
 *  SYNOPSIS
 *
 *	static VOID process_ops (argv)
 *	char *argv[];
 *
 *  DESCRIPTION
 *
 *	Certain processing required by some command line options is best
 *	delayed slightly from the time the option itself is recognized.
 *	This also helps to reduce the size of the usually large switch
 *	statement for detecting various options.  This post processing
 *	of arguments is done here.
 *
 *	Note that -uf and -ur are synonymous.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin process_opts
 *	    If not creating inspecting or listing archive then
 *		If not finding differences, extracting, or getting help then
 *		    Tell user to specify a mode
 *		    Clean up and exit
 *		End if
 *	    End if
 *	    If reading file list from standard input then
 *		If reading archive from standard input then
 *		    Notify user of conflict in stdin usage
 *		    Clean up and exit
 *		End if
 *	    End if
 *	    If selecting only files for given user then
 *		Initialize the user id
 *	    End if
 *	    If selecting only files past given date then
 *		Initialize the date
 *	    End if
 *	    If unconditional selection specified then
 *		Remember if block special files are to be selected
 *		Remember if character special files are to be selected
 *		Remember if directories are to be selected
 *		Remember if symbolic links are to be selected
 *		Remember if fifos are to be selected
 *		Remember if regular files are to be selected
 *	    End if
 *	    If archive device was specified then
 *		Look it up in device table
 *	    Else
 *		Use first entry in device table
 *		Initialize the file name in file information structure
 *	    End if
 *	    Initialize the device pointer
 *	    If buffer size specified then
 *		Convert argument to internal format
 *		Round up to nearest archive block boundry
 *	    Else if device found and default buffer size for device then
 *		Round up default size to nearest block boundry and use it
 *	    End if
 *	    If size of media given then
 *		Convert size and save it
 *	    Else
 *		Get size from device table
 *	    End if
 *	    If media size unknown then
 *		If a media usage estimate was requested then
 *		    Warn user can't do it
 *		    Reset the media estimate flag to ignore request
 *		End if
 *	    Else
 *		Compute number of block groups per media
 *		If no groups on media then 
 *		    Notify user block media size too small
 *		    Clean up and exit
 *		Else
 *		    Make media size even multiple of buffer size
 *		End if
 *	    End if
 *	End process_opts
 *
 */


static VOID process_opts (argv)
char *argv[];
{
    LBA grps;		/* Number of block groups */
    char *scan;

    DBUG_ENTER ("process_opts");
    if (!flags.cflag && !flags.iflag && !flags.tflag && !flags.eflag
    	&& !flags.gflag && flags.dflag == 0 && !flags.xflag && !flags.hflag) {
	    bru_message (MSG_MODE);
	    done ();
    }
    if (argv[optind] != NULL && STRSAME (argv[optind], "-")) {
	DBUG_PRINT ("inlist", ("read pathname list from stdin"));
	if (STRSAME (afile.fname, "-") && !flags.cflag) {
	    bru_message (MSG_STDIN);
	    done ();
	}
	DBUG_PRINT ("Pflag", ("turn off expansion of dirs by default"));
	flags.PEflag = FALSE;
	DBUG_PRINT ("Pflag", ("turn on filter mode by default"));
	flags.PFflag = TRUE;
	DBUG_PRINT ("Pflag", ("turn off auto-parent dirs by default"));
	flags.PPflag = FALSE;
    } else {
	DBUG_PRINT ("Pflag", ("turn on expansion of dirs by default"));
	flags.PEflag = TRUE;
	DBUG_PRINT ("Pflag", ("turn off filter mode by default"));
	flags.PFflag = FALSE;
	DBUG_PRINT ("Pflag", ("turn on auto-parent dirs by default"));
	flags.PPflag = TRUE;
    }
    if (flags.oflag) {
	init_uid (o_arg);
    }
    if (flags.nflag) {
	init_time (n_arg);
    }
    if (flags.uflag) {
	flags.uaflag = (BOOLEAN) (s_strchr (u_arg, 'a') != NULL);
	flags.ubflag = (BOOLEAN) (s_strchr (u_arg, 'b') != NULL);
	flags.ucflag = (BOOLEAN) (s_strchr (u_arg, 'c') != NULL);
	flags.udflag = (BOOLEAN) (s_strchr (u_arg, 'd') != NULL);
	flags.ulflag = (BOOLEAN) (s_strchr (u_arg, 'l') != NULL);
	flags.upflag = (BOOLEAN) (s_strchr (u_arg, 'p') != NULL);
	flags.urflag = (BOOLEAN) (s_strchr (u_arg, 'r') != NULL);
	if (!flags.urflag) {
	    flags.urflag = (BOOLEAN) (s_strchr (u_arg, 'f') != NULL);
	}
    }
    if (flags.Aflag) {
	flags.Acflag = (BOOLEAN) (s_strchr (A_arg, 'c') != NULL);
	flags.Afflag = (BOOLEAN) (s_strchr (A_arg, 'f') != NULL);
	flags.Aiflag = (BOOLEAN) (s_strchr (A_arg, 'i') != NULL);
	flags.Arflag = (BOOLEAN) (s_strchr (A_arg, 'r') != NULL);
	flags.Asflag = (BOOLEAN) (s_strchr (A_arg, 's') != NULL);
	if (flags.Afflag) {
	    (VOID) ipc_init ();
	}
    }
    if (flags.Pflag) {
	scan = P_arg;
	while (scan != NULL && *scan != EOS) {
	    switch (*scan++) {
		case 'e':
		    flags.PEflag = FALSE;
		    DBUG_PRINT ("Pflag", ("turn off expansion of dirs"));
		    break;
		case 'E':
		    flags.PEflag = TRUE;
		    DBUG_PRINT ("Pflag", ("turn on expansion of dirs"));
		    break;
		case 'f':
		    flags.PFflag = FALSE;
		    DBUG_PRINT ("Pflag", ("turn off filter mode"));
		    break;
		case 'F':
		    DBUG_PRINT ("Pflag", ("turn on filter mode"));
		    break;
		case 'p':
		    flags.PPflag = FALSE;
		    DBUG_PRINT ("Pflag", ("turn off auto-parent dirs"));
		    break;
		case 'P':
		    flags.PPflag = TRUE;
		    DBUG_PRINT ("Pflag", ("turn on auto-parent dirs"));
		    break;
	    }
	}
    }
    if (flags.fflag > 0) {
	ardp = get_ardp (afile.fname);
	if (ardp == NULL) {
	    if (!nameconfirm (afile.fname)) {
		done ();
	    }
	}
    } else {
	ardp = get_ardp ((char *) NULL);
	if (ardp == NULL) {
	    bru_message (MSG_DEFDEV);
	    done ();
	} else {
	    savedev (ardp -> dv_dev);
	    copyname (afile.fname, ardp -> dv_dev);
	}
    }
    if (flags.bflag) {
	bufsize = (UINT) getsize (b_arg);
	bufsize = BLOCKS (bufsize) * BLKSIZE;
    } else if (ardp != NULL && ardp -> dv_bsize != 0) {
	bufsize = BLOCKS (ardp -> dv_bsize) * BLKSIZE;
    }
    if (ardp != NULL && ardp -> dv_maxbsize != 0) {
	if (bufsize > ardp -> dv_maxbsize) {
	    bru_message (MSG_MAXBUFSZ, bufsize / 1024,
		       ardp -> dv_maxbsize / 1024);
	    if (!flags.bflag) {
		bufsize = BLOCKS (ardp -> dv_maxbsize) * BLKSIZE;
		bru_message (MSG_BUFADJ, bufsize / 1024);
	    }
	}
    }
    DBUG_PRINT ("bufsize", ("buffer size %uk bytes", (UINT) bufsize/1024));
    if (flags.sflag) {
	msize = (ULONG) getsize (s_arg);
    } else if (ardp != NULL) {
	msize = ardp -> dv_msize;
    }
    DBUG_PRINT ("opts", ("msize %lu  bufsize %u", msize, bufsize));
    if (msize != 0) {
	DBUG_PRINT ("opts", ("msize %lu  bufsize %u", msize, bufsize));
	grps = msize / bufsize;
	if (grps == 0L) {
	    bru_message (MSG_BUFSZ);
	    done ();
	} else {
	    msize = grps * bufsize;
	}
	DBUG_PRINT ("opts", ("grps %ld  msize %lu", grps, msize));
    }
    if (flags.Nflag) {
	nzbits = (UINT) getsize (N_arg);
    }
    if (flags.Sflag) {
	sfthreshold = getsize (S_arg);
    }
    DBUG_PRINT ("opts", ("sparse file threshold size %ld", sfthreshold));
    shmcheck ();
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	init_info    initialize the invocation information structure
 *
 *  SYNOPSIS
 *
 *	static VOID init_info (argv)
 *	char *argv[];
 *
 *  DESCRIPTION
 *
 *	Initialize the invocation information structure.
 *
 */

static VOID init_info (argv)
char *argv[];
{
    DBUG_ENTER ("init_info");
    info.bru_ulimit = s_ulimit (1, 0L);
    DBUG_PRINT ("ulimit", ("max file size %ld (blks)", info.bru_ulimit));
    info.bru_uid = s_getuid ();
    info.bru_gid = s_getgid ();
    if (info.bru_ttyout == NULL) {
	info.bru_ttyout = TERMINAL;
    }
    if (info.bru_ttyin == NULL) {
	info.bru_ttyin = TERMINAL;
    }
    info.bru_time = s_time ((long *) 0);
    info.bru_name = s_strrchr (argv[0], '/');
    if (info.bru_name == NULL) {
	info.bru_name = argv[0];
    } else {
	info.bru_name++;
    }
    if ((info.bru_tmpdir = s_getenv ("BRUTMPDIR")) == NULL) {
	info.bru_tmpdir = BRUTMPDIR;
    }
    DBUG_PRINT ("brutmpdir", ("prefered tmp dir is '%s'", info.bru_tmpdir));
    if ((info.bru_brutab = s_getenv ("BRUTAB")) == NULL) {
	info.bru_brutab = BRUTAB;
    }
    DBUG_PRINT ("brutab", ("prefered brutab is '%s'", info.bru_brutab));
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	setiopt    process the argument to a interaction option
 *
 *  SYNOPSIS
 *
 *	static VOID setiopt (option)
 *	char *option;
 *
 *  DESCRIPTION
 *
 *	Given the argument from an interaction option (-I command
 *	line argument), process the argument.
 *
 */

static VOID setiopt (option)
char *option;
{
    char *scan;
    int fildes;

    DBUG_ENTER ("setiopt");
    DBUG_PRINT ("iopt", ("process -I option '%s'", option));
    scan = option;
    switch (*scan++) {
	case 'l':
	    if (*scan++ != ',') {
		bru_message (MSG_IOPT, option);
	    } else {
		if ((fildes = pcreat (scan, 0666)) == -1) {
		    logfp = NULL;
		} else {
		    (VOID) s_close (fildes);
		    logfp = s_fopen (scan, "w");
		}
		if (logfp == NULL) {
		    bru_message (MSG_OPEN, scan);
		    logfp = stdout;
		}
	    }
	    break;
	case 'q':
	    if (*scan++ != ',') {
		bru_message (MSG_IOPT, option);
	    } else {
		info.bru_ttyout = s_strdup (scan);
	    }
	    break;
	case 'r':
	    if (*scan++ != ',') {
		bru_message (MSG_IOPT, option);
	    } else {
		info.bru_ttyin = s_strdup (scan);
	    }
	    break;
	default:
	    bru_message (MSG_IOPT, option);
	    break;
    }
    DBUG_VOID_RETURN;
}

/*
 *	Do some simple consistency checks to ensure that we are
 *	as compatible as possible with all other versions of bru.
 *	If something is grossly wrong, with the size of various
 *	structures that define the archive format for example,
 *	it is better to catch it here and die immediately rather
 *	than going on to produce lots of incompatible archives.
 *	This way the problem gets noticed before any archives
 *	are produced or processed.
 *
 *	Note the hardwired constants for the sizes of various
 *	structures.  This is deliberate, since these sizes are
 *	specified by the portable bru format.  If a customized
 *	format is specified, either by design or accident, these
 *	comparisons should help to draw attention to that fact.
 */

static VOID selfcheck ()
{
    int error = 0;

    DBUG_ENTER ("selfcheck");
    DBUG_PRINT ("check", ("struct blkhdr %d", sizeof (struct blkhdr)));
    DBUG_PRINT ("check", ("struct ar_hdr %d", sizeof (struct ar_hdr)));
    DBUG_PRINT ("check", ("struct file_hdr %d", sizeof (struct file_hdr)));
    DBUG_PRINT ("check", ("union  blkdata %d", sizeof (union blkdata)));
    DBUG_PRINT ("check", ("struct sblock %d", sizeof (struct sblock)));
    DBUG_PRINT ("check", ("union  blk %d", sizeof (union blk)));
    error |= sizeof (struct blkhdr) != 256;
    error |= sizeof (struct ar_hdr) != (2048 - 256);
    error |= sizeof (struct file_hdr) != (2048 - 256);
    error |= sizeof (union blkdata) != (2048 - 256);
    error |= sizeof (struct sblock) != 2048;
    error |= sizeof (union blk) != 2048;
    if (error) {
	bru_message (MSG_SELFCHK);
	done ();
    }
    DBUG_VOID_RETURN;
}
