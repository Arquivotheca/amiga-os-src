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
 *	mkdir.c    emulation of unix mkdir library routine
 *
 *  SCCS
 *
 *	@(#)mkdir.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	This is an emulation of the unix mkdir() library function for
 *	AmigaDOS.  Lattice has a mkdir() routine but it only has the
 *	"path" argument.
 *
 *  AUTHOR
 *
 *	Fred Fish
 *	Tempe, Arizona
 *
 */

#include "globals.h"

int s_mkdir  (path, mode)
char *path;
int mode;
{
    int rtnval = -1;

    DBUG_ENTER ("s_mkdir");
    if ((path != NULL) && (mkdir (path) == 0)) {
	if (s_chmod (path, mode) == 0) {
	    rtnval = 0;
	}
    }
    DBUG_RETURN (rtnval);
}
