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
 *	utime.c    emulation of Unix utime function for AmigaDOS
 *
 *  SCCS
 *
 *	@(#)utime.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Emulation of the Unix utime(2) call.  Based on program touch.c
 *	by Phil Lindsay and Andy Finkel, of Commodore-Amiga, Inc.
 *	Original copyright info follows:
 * 
 *		touch.c by Phil Lindsay and Andy Finkel
 *		(c) 1986 Commodore-Amiga, Inc.
 *		Permission to use in any way granted, as long as
 *		the copyright notice stays intact
 *
 *		This is a simple command to set the date of a file to now.
 *		It was compiled using Greenhills C.  You might have to change
 *		it a little for use with Lattice or Manx.  The program
 *		compiles with just amigalib.  (that's why those string
 *		functions are tacked on the end of the file)
 *
 *	Note that the original touch.c code contained a bug where trying
 *	to set the date of any file except those in the current directory
 *	would fail.  The fix is to strip off all leading pathname
 *	components before inserting the filename in the packet structure.
 *
 */

#include "globals.h"

#ifndef ACTION_SET_DATE		/* Early header files didn't have this... */
#define ACTION_SET_DATE (34)	/* This is the correct value, we hope... */
#endif


/*
 *  FUNCTION
 *
 *	utime    emulation of Unix utime function for AmigaDOS
 *
 *  SYNOPSIS
 *
 *	int utime (path, times)
 *	char *path;
 *	struct utimbuf *times;
 *
 *  DESCRIPTION
 *
 *	Given a pathname, and a pointer to a utimbuf with the desired
 *	time, change the timestamp on the specified file.
 *
 */

int utime (path, times)
char *path;
struct utimbuf *times;
{
    struct MsgPort *task;
    LONG arg[4];
    LONG rc;
    struct DateStamp dateStamp;
    ULONG lock;
    ULONG plock;
    UBYTE *pointer;
    int result = -1;
    int strsize;
    char *fname;

    DBUG_ENTER ("utime");
    if((task = (struct MsgPort *) DeviceProc (path)) != NULL) {
	DBUG_PRINT ("utime", ("found task"));
	if ((lock = (ULONG) Lock (path, SHARED_LOCK)) != 0) {
	    DBUG_PRINT ("utime", ("got SHARED_LOCK lock"));
	    plock = (ULONG) ParentDir (lock);
	    (VOID) UnLock (lock);
	    if ((fname = s_strrchr (path, '/')) == NULL) {
		fname = path;
	    } else {
		fname++;
	    }
	    DBUG_PRINT ("utime", ("path relative to parent = '%s'", fname));
	    strsize = strlen (fname);
	    pointer = (UBYTE *) AllocMem (strsize + 2, MEMF_PUBLIC);
	    if (pointer != NULL) {
		DBUG_PRINT ("utime", ("got %d bytes of mem", strsize + 2));
		(VOID) strcpy ((pointer + 1), fname);
		*pointer = strsize;
		arg[0] = NULL;
		arg[1] = plock;
		arg[2] = (ULONG) &pointer[0] >> 2;	/* BSTR of filename */
		unix2dos (times -> mtime, &dateStamp);
		arg[3] = (ULONG) &dateStamp;
		rc = sendpkt (task, ACTION_SET_DATE, arg, 4);
		DBUG_PRINT ("utime", ("sendpkt returned %d", rc));
		if (rc) {
		    result = 0;
		}
		(VOID) FreeMem (pointer, strsize + 2);
	    }
	    (VOID) UnLock (plock);
	}
    }
    DBUG_RETURN (result);
}


LONG sendpkt (id, type, args, nargs)
struct MsgPort *id;	/* process indentifier ... (handlers message port ) */
LONG type;		/* packet type ... (what you want handler to do )   */
LONG args[];		/* a pointer to a argument list */
LONG nargs;		/* number of arguments in list  */
{
    struct MsgPort *replyport;
    struct StandardPacket *packet;
    LONG count;
    LONG *pargs;
    LONG res1 = NULL; 

    DBUG_ENTER ("sendpkt");
    if ((replyport = (struct MsgPort *) CreatePort (NULL, NULL)) != NULL) {
	packet = (struct StandardPacket *)
		AllocMem ((LONG) sizeof (*packet), MEMF_PUBLIC | MEMF_CLEAR);
	if (packet) {
	    /*link packet */
	    packet -> sp_Msg.mn_Node.ln_Name = (char *) &(packet -> sp_Pkt);
	    /* to message    */
	    packet -> sp_Pkt.dp_Link = &(packet -> sp_Msg);
	    /* set-up reply port   */
	    packet -> sp_Pkt.dp_Port = replyport;
	    /* what to do... */
	    packet -> sp_Pkt.dp_Type = type;
	    /* move all the arguments to the packet */
	    pargs = &(packet -> sp_Pkt.dp_Arg1); /* address of first argument */
	    for (count = 0; (count < nargs) && (count < 7); count++) {
	        pargs[count] = args[count];
	    }
	    PutMsg (id, (struct Message *) packet);	/* send packet */
	    WaitPort (replyport);			/* wait for rtn */
	    GetMsg (replyport);				/* pull message */
	    res1 = packet -> sp_Pkt.dp_Res1;		/* get result */
	    FreeMem (packet, (LONG) sizeof (*packet)); 
	}
	DeletePort (replyport); 
    }
    DBUG_RETURN (res1);   
}



