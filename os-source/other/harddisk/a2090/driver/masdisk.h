From amiga!kodiak@cbmvax Wed Oct 28 18:51:10 1987
Received: from cbmvax. by skipper.skipper.uucp (4.24/3.14)
	id AA01322; Wed, 28 Oct 87 18:51:06 pst
Received: from tooter.amiga.uucp by amiga.uucp (1.1/3.14)
	id AA02730; Wed, 28 Oct 87 13:51:46 PST
Received: by tooter.amiga.uucp (1.1/SMI-3.0DEV3)
	id AA01572; Wed, 28 Oct 87 13:56:15 PST
Date: Wed, 28 Oct 87 13:56:15 PST
From: amiga!kodiak@cbmvax (Robert Burns)
Message-Id: <8710282156.AA01572@tooter.amiga.uucp>
To: amiga!andy@cbmvax, amiga!bart@cbmvax
Subject: masdisk.h
Cc: amiga!kodiak@cbmvax
Status: RO

Here's the way the MicroBotics MAS-Drives (and StarDrives) access SCSI.
Unfortunately, I know that some code also makes use of the MDERR result
codes (specifically, MDERR_SenseError).  I didn't design them to be
reasonable for everyone, and their use is very constrained (Chuck McMannis),
so we might be able to work something out there, too.

------------------------------------------------------------------------------
#ifndef	DEVICES_MASDISK_H
#define	DEVICES_MASDISK_H

#ifndef	EXEC_IO_H
#include	"exec/io.h"
#endif	!EXEC_IO_H
#ifndef	EXEC_DEVICES_H
#include	"exec/devices.h"
#endif	!EXEC_DEVICES_H

#define	MD_NAME		"MAS-Drive_20.device"
#define	MD_SCSICMD	(CMD_NONSTD+13)
#define	MD_FLAGSCMD	(CMD_NONSTD+14)

/* Hard Disk Unit Flags */
#define	UFB_WRITEVERIFY		0	/* write, then read and verify */
#define	UFB_WRITEPROTECT	1	/* do not allow write operations */
#define	UFB_RESERVED		2	/* was BADNEIGHBORS */
#define	UFB_ALLERRORS		3	/* report warnings as errors */
#define UFB_BLINDWRITE		4	/* assume disk is fastest */
#define	UFB_BLINDREAD		5

#define	UFF_WRITEVERIFY		1
#define	UFF_WRITEPROTECT	2
#define	UFF_RESERVED		4
#define	UFF_ALLERRORS		8
#define UFF_BLINDWRITE		16
#define	UFF_BLINDREAD		32

struct MDSCSIReq {
    struct IORequest scsi_IOR;
    UBYTE *	scsi_Command;
    ULONG	scsi_CmdSize;
    UBYTE *	scsi_Data;
    ULONG	scsi_DataSize;
};

/* error codes */
/*   general device codes returned by this device:
/*	IOERR_OPENFAIL	-1	-- only returned for parallel.device
/*	IOERR_ABORTED	-2	-- command has been aborted
/*	IOERR_NOCMD	-3	-- unsupported command attempted

/*   trackdisk.device codes also returned by this device:
/*	TDERR_WriteProt	28	-- attempt to write to a protected disk

/*   SCSI Interface Error Codes */
#define	MDERR_Hardware	1	/* Misc. hardware failure */
#define	MDERR_Software	2	/* Misc. software failure */
#define	MDERR_BusyLost	3	/* SCSI busy lost (bus dropped) */
#define	MDERR_Parity	4	/* SCSI parity error */
#define	MDERR_Reset	5	/* SCSI reset error */
#define	MDERR_BadPhase	6	/* invalid SCSI phase */
#define	MDERR_SelFail	7	/* SCSI bus is down */
#define	MDERR_SCSIDown	8	/* SCSI Bus strange (no-power?) */
#define	MDERR_Timeout	9	/* DMA transfer timeout */
#define	MDERR_BadMessage 10	/* message not Command Complete
/*	When MDERR_BadMessage is reported, the byte at IO_ACTUAL contains
/*	the offensive message. */

#define	MDERR_BadComplete 11	/* Command Completion Status != 0
/*	When MDERR_BadComplete is reported, the byte at IO_ACTUAL contains
/*	the Command Completion Status.  Note that if the Check Condition
/*	bit is set, the MDERR_SenseError will be reported instead... */

#define	MDERR_SenseError 12	/* Command Completion Status & Check
				   Condition, so Read Status was performed...
/*	When MDERR_SenseError is reported, IO_ACTUAL and IO_OFFSET will contain
/*	additional error information.
/*	     -- the byte at IO_ACTUAL contains the Command Completion Status
/*	     -- the byte at IO_ACTUAL+1 contains the Valid bit and the disk
/*		manufacturer specific Error Class & Error Code.  This is the
/*		first byte returned by the SCSI Read Sense Command.
/*	     -- the byte at IO_ACTUAL+2 contains the second byte returned by
/*		the sense.  This is used to hold different data depending on
/*		whether this is an extended sense or not.
/*	     -- the byte at IO_ACTUAL+3 contains the third byte returned by
/*		extended sense (or zero for non-extended sense).  When masked
/*		with 00FH, it contains the Sense Key, described below.
/*	if the most significant bit of the byte at IO_ACTUAL+1 is set (the
/*	Valid bit), IO_OFFSET will contain the Logical Block Address of the
/*	error: usually the same as the original offset divided by the block
/*	size (512 bytes). */

#define	MDERR_DataTimeout 13	/* DMA transfer timeout in data phase
/*	When MDERR_DataTimeout is reported, the command completion is
/*	otherwise successful, save that IO_ACTUAL contains a number
/*	of bytes transferred that is different than the number requested */

/*   SCSI extended sense keys */
#define	SK_NoExtension	0	/* extended sense not available */
#define	SK_Recoverable	1	/* (warning) recovery action taken */
#define	SK_NotReady	2
#define	SK_Media	3	/* non recoverable media flaw */
#define	SK_Hardware	4
#define	SK_IllegalReq	5
#define	SK_UnitAttn	6	/* (warning) reset occured previously */
#define	SK_DataProtect	7
#define	SK_Aborted	11
#define	SK_Equal	12	/* for use w/ SCSI "SEARCH DATA EQUAL" command */
#define	SK_VolOverflow	13

#endif	DEVICES_MASDISK_H

