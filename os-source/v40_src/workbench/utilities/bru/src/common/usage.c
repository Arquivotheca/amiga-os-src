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
 *	usage.c    internal help stuff
 *
 *  SCCS
 *
 *	@(#)usage.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines for printing internal help or usage
 *	information.
 *
 */

#include "globals.h"


/*
 *	The following is the internal documentation which gets printed
 *	when invoked with the -h flag.
 */

#if !CRAMPED

static char *documentation[] = {
    "",
    "NAME",
    "",
    "\tbru -- backup and restore utility",
    "",
    "SYNOPSIS",
    "",
    "\tbru  mode  [control options]  [selection options]  [files]",
    "",
    "MODE",
    "",
    "\t-c         create a new archive with specified files",
    "\t-d         find differences between archived files and current files",
    "\t-e         estimate media requirements for create mode",
    "\t-g         give only information from archive header",
    "\t-h         print this help information",
    "\t-i         inspect archive for consistency and data integrity",
    "\t-t         list archive table of contents for files",
    "\t-x         extract named files from archive",
    "",
    "CONTROL OPTIONS",
    "",
    "\tSizes are specified in bytes.  The scale factors 'M' or 'm',",
    "\t'K' or 'k', or 'B' or 'b' can be appended to the size to indicate",
    "\tmegabytes, kilobytes, or blocks (512 bytes) respectively.",
    "",
    "\t-# str     use debugging control string str",
    "\t-a         do not reset file access times after reads (now default)",
    "\t-A flags   Commodore Amiga specific flags:",
    "\t           c   clear file archived bit after processing",
    "\t           f   during filter mode reroute interactions to ipc port",
    "\t           i   ignore file archived bit for selecting files",
    "\t           r   reject files that have archived bit set",
    "\t           s   set file archived bit after processing",
    "\t-b N       set archive buffer size to N bytes (scalable)",
    "\t-B         background mode, no interaction with operator",
    "\t-C         always chown extracted files to the user's uid/gid",
    "\t-D         on some systems, provides speedup via double buffering",
    "\t-f file    use specified file as archive ('-' for stdin/stdout)",
    "\t-F         fast mode, no checksum computations or checking",
    "\t-I option  an interaction option:",
    "\t           l,pathname  write verbosity info to pathname",         
    "\t           q,fifo      write interaction queries to fifo",         
    "\t           r,fifo      read interaction replies from fifo",         
    "\t-L str     label archive with given string (63 char max)",
    "\t-l         suppress warnings about unresolved links",
    "\t-m         limit directory expansions to same mounted filesystem",
    "\t-N nbits   use nbits for LZW compression (default 12); see -Z",
    "\t-p         pass over archive files by reading rather than seeking",
    "\t-P opts    special options for pathname handling and expansions",
    "\t           e    turn off expansion of directories",
    "\t           E    turn on expansion of directories",
    "\t           f    turn off filter mode (build internal file tree)",
    "\t           F    turn on filter mode (do not build internal tree)",
    "\t           p    turn off auto archiving of parent directory nodes",
    "\t           P    turn on auto archiving of parent directory nodes",
    "\t-R         exclude remotely mounted files for NFS/RFS systems",
    "\t-s N       specify size of archive media in bytes (scalable)",
    "\t-S N       turn on options to handle sparse files intelligently",
    "\t           and set sparse file size threshold in bytes (scalable)",
    "\t-v         enable verbose mode (-vv and -vvv for more verbosity)",
    "\t-w         display action to be taken and wait for confirmation",
    "\t-Z         use LZW compression on archived files; see -N",
    "",
    "FILE SELECTION OPTIONS",
    "",
    "\t-n date    select files modified since date",
    "\t           EX: 14-Apr-84,15:24:00",
    "\t-o user    select files owned by user, where user may be a symbolic",
    "\t           user name, numeric user id, or file owned by user",
    "\t-u abcdf   use selected files in given class regardless of",
    "\t   lpr     modification dates, where class is one or more of:",
    "\t           a    use any file, same as giving all other args",
    "\t           b    use block special files",
    "\t           c    use character special files",
    "\t           d    use directories",
    "\t           f    use regular files (same as 'r')",
    "\t           l    use symbolic links",
    "\t           p    use fifos (named pipes)",
    "\t           r    use regular files (same as 'f')",
    "",
    "ENVIRONMENT VARIABLES",
    "",
    "\tBRUTAB     full pathname of device table (EX: /etc/brutab)",
    "\tBRUTMPDIR  full pathname of preferred temporary directory (EX: /tmp)",
    "\tSHELL      preferred shell (EX: /bin/sh",
    "",
    NULL
};

#endif   /* !CRAMPED */

static char *quickdoc[] = {
    "usage:  bru -cdeghitx [-#aAbBCfFILlmnNopPRsSuvwZ] file(s)...",
    "bru -h for help",
    NULL
};

static char *fmt1 = "\t%-16s";		/* Multi-use format string */


/*
 *	Copyright for evaluation copies.
 */

static char *cpyright [] = {
#if COPYRIGHT
    "******************************************",
    "*                                        *",
    "*           Copyright (c) 1988           *",
    "*  Enhanced Software Technologies, Inc.  *",
    "*           All Rights Reserved          *",
    "*                                        *",
    "*             Evaluation copy.           *",
    "*          For in-house use only.        *",
    "*                                        *",
    "*      Not to be distributed without     *",
    "*      explicit written permission.      *",
    "*                                        *",
    "******************************************",
#endif
    NULL,
};


/*
 *  FUNCTION
 *
 *	usage    give usage information
 *
 *  SYNOPSIS
 *
 *	VOID usage (type)
 *	int type;
 *
 *  DESCRIPTION
 *
 *	Gives specified type of usage information.  Type is
 *	usually either "BRIEF" or "VERBOSE".
 *
 */

VOID usage (type)
int type;
{
    char **dp;

    DBUG_ENTER ("usage");
    mode = 'h';
    if (type == VERBOSE) {
#if CRAMPED
	dp = quickdoc;
#else
	dp = documentation;
#endif
    } else {
	dp = quickdoc;
    }
    while (*dp) {
	voutput ("%s", *dp++);
	vflush ();
    }
    if (type == VERBOSE) {
	voutput ("OTHER INFO");
	vflush ();
	vflush ();
	voutput (fmt1, "release:");
	voutput ("%d.%d", release, level);
	vflush ();
	voutput (fmt1, "variant:");
	voutput ("%d", variant);
	vflush ();
	voutput (fmt1, "bru id:");
	voutput ("%s", id);
	vflush ();
	voutput (fmt1, "config:");
	voutput ("%s", CONFIG_DATE);
	vflush ();
	voutput (fmt1, "archive:");
	voutput ("%s", afile.fname);
	vflush ();
	voutput (fmt1, "media size:");
	if (msize == 0) {
	    voutput ("<unknown>");
	} else {
	    voutput ("%luk bytes usable", (ULONG) B2KB (msize));
	}
	vflush ();
	voutput (fmt1, "buffer size:");
	voutput ("%uk bytes", (UINT) bufsize/1024);
	vflush ();
	voutput (fmt1, "queries to:");
	voutput ("%s", info.bru_ttyout);
	vflush ();
	voutput (fmt1, "replies from:");
	voutput ("%s", info.bru_ttyin);
	vflush ();
	voutput (fmt1, "temporaries:");
	voutput ("%s", info.bru_tmpdir);
	vflush ();
	voutput (fmt1, "device table:");
	voutput ("%s", info.bru_brutab);
	vflush ();
	voutput (fmt1, "compression:");
	voutput ("%u bits", nzbits);
	vflush ();
	voutput (fmt1, "copyright:");
	voutput ("%s", copyright);
	vflush ();
	dp = cpyright;
	vflush ();
	vflush ();
	while (*dp) {
	    voutput ("\t\t\t%s",*dp++);
	    vflush ();
	}
    }
    vflush ();
    DBUG_VOID_RETURN;
}
