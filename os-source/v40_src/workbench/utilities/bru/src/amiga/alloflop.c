/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
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
 *	alloflop.c    allocate floppy device for exclusive use of bru
 *
 *  SCCS
 *
 *	@(#)alloflop.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Disable DOS from playing with our archive device.  Based on
 *	program touch.c by Phil Lindsay and Andy Finkel, of
 *	Commodore-Amiga, Inc.  Original copyright info follows:
 * 
 *		touch.c by Phil Lindsay and Andy Finkel
 *		(c) 1986 Commodore-Amiga, Inc.
 *		Permission to use in any way granted, as long as
 *		the copyright notice stays intact
 *
 *  AUTHOR
 *
 *	Based on touch.c by Phil Lindsay and Andy Finkel
 *	Heavily modified by Fred Fish for use in bru
 *
 */

#include "globals.h"


/*
 *  FUNCTION
 *
 *	DisableDevice    disable dos from mucking with our archive device
 *
 *  SYNOPSIS
 *
 *	int DisableDevice (name)
 *	char *name;
 *
 *  DESCRIPTION
 *
 *	Tells dos to leave our archive device alone until further notice.
 *	Returns non-zero on success, zero on failure.
 *
 */

int DisableDevice (name)
char *name;
{
    LONG arg[1];
    int result = 0;
    struct MsgPort *task;

    DBUG_ENTER ("DisableDevice");
    DBUG_PRINT ("dev", ("disable device %s", name));
    if((task = (struct MsgPort *) DeviceProc (name)) != NULL) {
	DBUG_PRINT ("dev", ("found task"));
	arg[0] = 1;
	result = sendpkt (task, ACTION_INHIBIT, arg, 1);
	DBUG_PRINT ("dev", ("sendpkt returned %d", result));
    }
    DBUG_RETURN (result);
}


/*
 *  FUNCTION
 *
 *	EnableDevice    tell dos it can have our archive device back now
 *
 *  SYNOPSIS
 *
 *	int EnableDevice (name)
 *	char *name;
 *
 *  DESCRIPTION
 *
 *	Tells dos it can have our archive device back now.
 *	Returns non-zero on success, zero on failure.
 *
 */

int EnableDevice (name)
char *name;
{
    LONG arg[1];
    int result = 0;
    struct MsgPort *task;

    DBUG_ENTER ("EnableDevice");
    DBUG_PRINT ("dev", ("enable device %s", name));
    if((task = (struct MsgPort *) DeviceProc (name)) != NULL) {
	DBUG_PRINT ("dev", ("found task"));
	arg[0] = 0;
	result = sendpkt (task, ACTION_INHIBIT, arg, 1);
	DBUG_PRINT ("utime", ("sendpkt returned %d", result));
    }
    DBUG_RETURN (result);
}
