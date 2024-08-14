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
 *	mktemp.c    emulation of Unix mktemp library routine
 *
 *  SCCS
 *
 *	@(#)mktemp.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Simple minded implementation of Unix mktemp call.
 *	Works fine for us, but is not general enough to
 *	replace a real Unix mktemp.  The name must have exactly
 *	six trailing 'X' characters.  Each time we are called
 *	we will change the six 'X's to a character between
 *	'a'-'z' followed by the number "00000".  Bru only calls
 *	this function once anyway...
 *
 */

#include "globals.h"

char *mktemp (template)
char *template;
{
    static char tnamebuf[128];
    static char ch = 'a';
    char *scan;

    DBUG_ENTER ("mktemp");
    DBUG_PRINT ("mtmp", ("use template '%s'", template));
    for (scan = tnamebuf; (*scan++ = *template++) != '\000'; ) {;}
    scan--;
    while (*--scan == 'X') {;}
    scan++;
    s_sprintf (scan, "%c00000", ch++);
    if (ch == 'z') {
	ch = 'a';
    }
    DBUG_PRINT ("mtmp", ("returns '%s'", tnamebuf));
    DBUG_RETURN (tnamebuf);
}
