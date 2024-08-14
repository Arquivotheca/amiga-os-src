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
 *	info.c   routines to print only info from archive header
 *
 *  SCCS
 *
 *	@(#)info.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 * 	Opens archive, reading and dumping the header block.  Reads
 *	no other archive blocks, because scan() ain't called.
 *
 */

#include "globals.h"


/*
 *  FUNCTION
 *
 *	info_only  main routine to display header info
 *
 *  SYNOPSIS
 *
 *	VOID info_only ()
 *
 *  DESCRIPTION
 *
 *	Mainline for the header-info mode.  Called after all initialization
 *	is complete.  Simply read archive header, prints info, then quits.
 *
 */

VOID info_only ()
{
    DBUG_ENTER ("info_only");
    mode = 'g';
    reload ("giving header info");
    ar_open ();		/* header info given here */
			/* ... so no reason to scan anything in archive */
    ar_close ();
    DBUG_VOID_RETURN;
}
