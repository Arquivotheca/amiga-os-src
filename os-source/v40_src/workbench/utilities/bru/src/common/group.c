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
 *	group.c    /etc/group file interface routines
 *
 *  SCCS
 *
 *	@(#)group.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines for translating from numerical gid's
 *	to symbolic group names found in /etc/group and vice-versa.
 *
 *	This involves very little overhead and is well worth
 *	the trouble to avoid the use of numerical id's whenever
 *	possible.
 *
 *	Supported routines are:
 *
 *		gp_init		initialize translation list
 *		gp_gname	translate gid to name
 *
 *
 *	This file is a duplication of passwd.c 5.2, modified to use
 *	the getgrent suite of routines.  So even though it was done at
 *	Georgia Tech, it is appropriate to include the copyright notice.
 */

#include "globals.h"

/*
 *	A translation list of groups is kept in a linear linked list
 *	with each link one of the "bru_group" structures.  Search time
 *	to locate any link in the list is considered to be
 *	insignificant with respect to the total run time.
 *
 */

struct bru_group {			/* The group translation info */
    char *gp_name;			/* Pointer to name of group */
    USHORT gp_gid;			/* The group id */
    struct bru_group *gp_next;		/* Pointer to next link in list */
};

static struct bru_group *gp_list = NULL;	/* Pointer to head of list */
static struct bru_group *gp_last = NULL;	/* Pointer to last link in list */


/*
 *  FUNCTION
 *
 *	gp_init    read group file and initialize translation list
 *
 *  SYNOPSIS
 *
 *	VOID gp_init ();
 *
 *  DESCRIPTION
 *
 *	Reads the system group file and initializes the translation
 *	list.
 *
 *	The tradeoff involved here is time vs space (as usual).
 *	The space consideration is the memory required to hold
 *	the translation list, which allows very fast access to
 *	any given user structure.  The time consideration is
 *	the amount of time required to use the library routines
 *	"getgrgid" and "getgrnam" to do translation on the fly
 *	without a translation list.  Since a large number of
 *	translations is the normal situation, and since memory
 *	demands are minimal, I have opted to read the password
 *	file once and maintain a translation list.
 *
 *  NOTES
 *
 *	Any failure to allocate sufficient memory is immediately
 *	fatal.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin gp_init
 *	    While there is another entry in group file
 *		Allocate memory for a link
 *		Allocate memory for copy of name
 *		Copy name to private storage
 *		Remember the group id
 *		If first link then
 *		    Make this link head of list
 *		Else
 *		    Connect this link to last link
 *		End if
 *		Make new link the last in list
 *	    End while
 *	    Close the group file
 *	End gp_init
 */

VOID gp_init ()
{
    struct group *group_p;
    struct bru_group *bgrp;
    UINT size;

    DBUG_ENTER ("gp_init");
    while ((group_p = s_getgrent ()) != NULL) {
	size = (sizeof (struct bru_group));
	bgrp = (struct bru_group *) get_memory (size, TRUE);
	size = s_strlen (group_p -> gr_name) + 1;
	bgrp -> gp_name = (char *) get_memory (size, TRUE);
	(VOID) s_strcpy (bgrp -> gp_name, group_p -> gr_name);
	bgrp -> gp_gid = group_p -> gr_gid;
	bgrp -> gp_next = NULL;
	if (gp_list == NULL) {
	    gp_list = bgrp;
	} else {
	    gp_last -> gp_next = bgrp;
	}
	gp_last = bgrp;
    }
    s_endgrent ();
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	gp_gname    translate a numerical group id number
 *
 *  SYNOPSIS
 *
 *	char *gp_gname (gid)
 *	UINT gid;
 *
 *  DESCRIPTION
 *
 *	Translate from a numerical group id number to a name.
 *	Always returns a valid pointer, either to the name string
 *	or a string containing the numeric value of the gid.
 *	Note that the string must be copied between calls if
 *	it is to be saved, since numeric strings are overwritten
 *	at the next call that returns a pointer to a numeric string.
 *
 *  NOTE
 *
 *	This routine does not use the DBUG_ENTER, LEAVE, and DBUG_n
 *	macros because their usage screws up the table of contents
 *	with the verbose option (when trace or debug is enabled).
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin gp_gname
 *	    Default return value is NULL
 *	    For each translation link in list
 *		If group id numbers match then
 *		    Remember name
 *		    Break translation scan loop
 *		End if
 *	    End for
 *	    If no translation found then
 *		Build numeric string instead
 *		Will return pointer to this string
 *	    Endif
 *	    Return pointer
 *	End gp_gname
 *
 */


char *gp_gname (gid)
UINT gid;
{
    struct bru_group *bgrp;
    char *name;
    static char gidstr[16];

    name = NULL;
    for (bgrp = gp_list; bgrp != NULL; bgrp = bgrp -> gp_next) {
	if (bgrp -> gp_gid == gid) {
	    name = bgrp -> gp_name;
	    break;
	}
    }
    if (name == NULL) {
	(VOID) s_sprintf (gidstr, "%u", gid);
	name = gidstr;
    }
    return (name);
}
