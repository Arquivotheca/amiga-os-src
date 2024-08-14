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
 *	fmode.c    routines to handle conversion of file modes
 *
 *  SCCS
 *
 *	@(#)fmode.c	12.8	11 Feb 1991
 *
 */

 
/*
 *  NAME
 *
 *	filemode    convert file mode word to printable version
 *
 *  SYNOPSIS
 *
 *	VOID filemode (buf, mode)
 *	char buf[];
 *	INT mode;
 *
 *  DESCRIPTION
 *
 *	Takes a file mode as returned by the "stat" system call,
 *	and decodes it to produce a printable string of the
 *	form "drwxrwxrwx" as produced by the "ls" command.
 *
 */

#include "globals.h"


/*
 *	Each character in mode string has a related table.  The
 *	table contains the appropriate characters, in descending
 *	precedence, for bits set in the mode word.  When a matching
 *	bit is found to be set, the corresponding char is transfered
 *	to the mode string and the next table is examined.
 */

struct modetbl {
    MODE bmask;			/* Bit to test */
    char ch;			/* Corresponding character for mode string */
};

static struct modetbl ch1[] = {BS_IREAD, 'r', 0, EOS};
static struct modetbl ch2[] = {BS_IWRITE, 'w', 0, EOS};
static struct modetbl ch3[] = {BS_ISUID, 's', BS_IEXEC, 'x', 0, EOS};
static struct modetbl ch4[] = {BS_IREAD >> 3, 'r', 0, EOS};
static struct modetbl ch5[] = {BS_IWRITE >> 3, 'w', 0, EOS};
static struct modetbl ch6[] = {BS_ISGID, 's', BS_IEXEC >> 3, 'x', 0, EOS};
static struct modetbl ch7[] = {BS_IREAD >> 6, 'r', 0, EOS};
static struct modetbl ch8[] = {BS_IWRITE >> 6, 'w', 0, EOS};
static struct modetbl ch9[] = {BS_ISVTX, 't', BS_IEXEC >> 6, 'x', 0, EOS};

static VOID type PROTO((char buf[], MODE mode));
static VOID faccess PROTO((char buf[], MODE mode));
static VOID setmode PROTO((MODE mode, struct modetbl *tp, char *cp));


VOID filemode (buf, mode)
char buf[];
MODE mode;
{
    DBUG_ENTER ("filemode");
    DBUG_PRINT ("dir", ("mode is %o", mode));
    (VOID) s_strcpy (buf, "----------");
    type (buf, mode);
    faccess (buf, mode);
    DBUG_PRINT ("dir", ("decoded mode is %s", buf));
    DBUG_VOID_RETURN;
}


/*
 *  NAME
 *
 *	type    determine type of the file
 *
 *  SYNOPSIS
 *
 *	type (buf, mode)
 *	char buf[];
 *	MODE mode;
 *
 *  DESCRIPTION
 *
 *	Determines the file type and initializes the first
 *	character of the mode string appropriately.
 *
 */

static VOID type (buf, mode)
char buf[];
MODE mode;
{
    char mchar;

    DBUG_ENTER ("type");
    switch (mode & BS_IFMT) {
	case BS_IFIFO:
	    mchar = 'p';
	    break;
	case BS_IFCHR:
	    mchar = 'c';
	    break;
	case BS_IFDIR:
	    mchar = 'd';
	    break;
	case BS_IFBLK:
	    mchar = 'b';
	    break;
	case BS_IFLNK:
	    mchar = 'l';
	    break;
	case BS_IFCTG:
	    mchar = 'C';
	    break;
	case BS_IFSOCK:
	    mchar = '|';	/* "is a pipe", never happens? */
	    break;
	default:
	    mchar = '-';
	    break;
    }
    buf[0] = mchar;
    DBUG_VOID_RETURN;
}


/*
 *  NAME
 *
 *	faccess    decode the access permissions
 *
 *  SYNOPSIS
 *
 *	static VOID faccess (buf, mode)
 *	char buf[];
 *	MODE mode;
 *
 *  DESCRIPTION
 *
 *	Decodes the access permissions for the file.  Note
 *	that the tables take into account special bits such
 *	as the "sticky bit", which take precedence over access
 *	permissions in the mode string.
 *
 */

static VOID faccess (buf, mode)
char buf[];
MODE mode;
{
    char *cp;

    DBUG_ENTER ("faccess");
    cp = &buf[1];
    setmode (mode, ch1, cp++);
    setmode (mode, ch2, cp++);
    setmode (mode, ch3, cp++);
    setmode (mode, ch4, cp++);
    setmode (mode, ch5, cp++);
    setmode (mode, ch6, cp++);
    setmode (mode, ch7, cp++);
    setmode (mode, ch8, cp++);
    setmode (mode, ch9, cp++);
    DBUG_VOID_RETURN;
}


/*
 *  NAME
 *
 *	setmode    set a particular char in mode string
 *
 *  SYNOPSIS
 *
 *	static VOID setmode (mode, tp, cp)
 *	MODE mode;
 *	struct modetbl *tp;
 *	char *cp;
 *
 *  DESCRIPTION
 *
 *	Given the file mode word, a pointer to a decode table,
 *	and a pointer to the location to store the decoded
 *	mode, decodes bits according to masks in the table,
 *	until a set bit is found or end of table is reached.
 *	If a matching bit is found, stores the corresponding
 *	mode character from the table in the location pointed
 *	to by "cp".
 *
 */

static VOID setmode (mode, tp, cp)
MODE mode;
struct modetbl *tp;
char *cp;
{
    DBUG_ENTER ("setmode");
    while (tp -> bmask != 0) {
	if (tp -> bmask & mode) {
	    *cp = tp -> ch;
	    break;
	} else {
	    tp++;
	}
    }
    DBUG_VOID_RETURN;
}
