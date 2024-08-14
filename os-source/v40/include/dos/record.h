#ifndef DOS_RECORD_H
#define DOS_RECORD_H
/*
**
**	$Id: record.h,v 36.5 90/07/12 03:26:22 jesup Exp $
**
**	include file for record locking
**
**	(C) Copyright 1989,1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
*/

#ifndef DOS_DOS_H
#include "dos/dos.h"
#endif

/* Modes for LockRecord/LockRecords() */
#define REC_EXCLUSIVE		0
#define REC_EXCLUSIVE_IMMED	1
#define REC_SHARED		2
#define REC_SHARED_IMMED	3

/* struct to be passed to LockRecords()/UnLockRecords() */

struct RecordLock {
	BPTR	rec_FH;		/* filehandle */
	ULONG	rec_Offset;	/* offset in file */
	ULONG	rec_Length;	/* length of file to be locked */
	ULONG	rec_Mode;	/* Type of lock */
};

#endif /* DOS_RECORD_H */
