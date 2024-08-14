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
 *	expandargs.c    filename argument expansion routines for the Amiga
 *
 *  SCCS
 *
 *	@(#)expandargs.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Each explicit filename argument on the command line is added to
 *	the tree of files to archive.  The tree_add function is
 *	responsible for adding each name to the tree.  On systems where
 *	the shell expands wildcard characters in filenames (Unix for example),
 *	the tree_add routine can be called directly.  Others, such as the
 *	Amiga, must have the arguments expanded before calling tree_add, so
 *	on these machines we first call an appropriate expansion routine, 
 *	which calls tree_add with each expanded filename.  This module
 *	implements the expansion for the Commodore Amiga.
 *
 */

#include "globals.h"

static VOID expand PROTO((char *arg));
static VOID expandparent PROTO((char *arg, char *parent, char *basename));

#define FIB struct FileInfoBlock


/*
 *  FUNCTION
 *
 *	amiga_tree_add    expand pathnames and add to tree
 *
 *  SYNOPSIS
 *
 *	VOID amiga_tree_add (arg)
 *	char *arg;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a command line pathname argument, expand
 *	it using Unix style expansion, and add each expansion to
 *	the tree of files to process.
 *
 *	There are two special cases that are handled first:
 *
 *		"."	Expanded as "*"
 *		"XXX:"	Expanded as "XXX:*"
 *
 *	Note that we do expansion in the Unix shell tradition:
 *
 *		*	=>	Zero or more occurances of any char
 *		?	=>	Any char
 *		[a-k]	=>	Any char in class {a,b,c,...i,j,k}
 *		[^a-k]	=>	Any char not in class {a,b,c,...i,j,k}
 *
 */

VOID amiga_tree_add (arg)
char *arg;
{
    BPTR lock;
    int length;
    char *temp;

    DBUG_ENTER ("amiga_tree_add");
    DBUG_PRINT ("xpand", ("expand arg '%s' and add to tree", arg));
    length = strlen (arg);
    if (arg[length-1] == ':' && arg[length] == '\000') {
	DBUG_PRINT ("xpand", ("handle logical device '%s'", arg));
	temp = malloc (length + 2);
	strcpy (temp, arg);
	temp[length] = '*';
	temp[length + 1] = '\000';
	expand (temp);
	free (temp);
    } else if (strcmp (arg, ".") == 0) {
	DBUG_PRINT ("xpand", ("arg '%s' special case of current dir", arg));
	expand ("*");
    } else if ((lock = Lock (arg, (long) ACCESS_READ)) != NULL) {
	DBUG_PRINT ("xpand", ("arg '%s' is existing file/device", arg));
	UnLock (lock);
	tree_add (arg);
    } else if (strchr (arg, '*') || strchr (arg, '?')) {
	DBUG_PRINT ("xpand", ("arg '%s' contains wildcards", arg));
	expand (arg);
    } else if (strchr (arg, '[') || strchr (arg, ']')) {
	DBUG_PRINT ("xpand", ("arg '%s' contains char classes", arg));
	expand (arg);
    } else {
	DBUG_PRINT ("xpand", ("arg '%s' unknown, passed on as is", arg));
	tree_add (arg);
    }
    DBUG_VOID_RETURN;
}


static VOID expand (arg)
char *arg;
{
    char *scan;
    char *parent;

    DBUG_ENTER ("expand");
    DBUG_PRINT ("xpand", ("expand arg '%s'", arg));
    scan = arg + strlen (arg);
    do {
	scan--;
    } while (*scan != ':' && *scan != '/' && scan > arg);
    if (*scan == ':' || *scan == '/') {
	scan++;
	parent = malloc (scan - arg + 1);
	strncpy (parent, arg, scan - arg);
	parent[scan - arg] = '\000';
	expandparent (arg, parent, scan);
	free (parent);
    } else if (scan == arg) {
	expandparent (arg, "", arg);
    }
    DBUG_VOID_RETURN;
}


static VOID expandparent (arg, parent, basename)
char *arg;
char *parent;
char *basename;
{
    BPTR lock;
    FIB *f_info;
    char *temp;
    int length;

    DBUG_ENTER ("expandparent");
    DBUG_PRINT ("xpand", ("parent = '%s' basename = '%s'", parent, basename));
    lock = Lock (parent, (long) ACCESS_READ);
    if (lock == NULL) {
	tree_add (arg);
    } else {
	f_info = (FIB *) AllocMem ((long) sizeof (FIB),
		(long) (MEMF_CLEAR | MEMF_CHIP));
	if (f_info == NULL) {
	    bru_message (MSG_MALLOC, (long) sizeof (FIB));
	} else {
	    if (Examine (lock, f_info) == 0) {
		bru_message (MSG_OPEN, arg);
	    } else {
		if (f_info -> fib_DirEntryType > 0) {
		    while (ExNext (lock, f_info) != 0) {
			if (wild (f_info -> fib_FileName, basename)) {
			    length = strlen (parent);
			    length += strlen (f_info -> fib_FileName);
			    temp = malloc (length + 1);
			    strcpy (temp, parent);
			    strcat (temp, f_info -> fib_FileName);
			    DBUG_PRINT ("xpand", ("add expansion '%s'", temp));
			    tree_add (temp);
			    free (temp);
			}
		    }
		} else {
		    DBUG_PRINT ("xpand", ("add '%s'", arg));
		    tree_add (arg);
		}
	    }
	    FreeMem (f_info, (long) sizeof (FIB));
	}
	UnLock (lock);
    }
    DBUG_VOID_RETURN;
}

