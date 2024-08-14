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
 *	devices.h    structures and defines for manipulating device info
 *
 *  SCCS
 *
 *	@(#)devices.h	12.8	11 Feb 1991
 *
 *  SYNOPIS
 *
 *	#include "devices.h"
 *
 *  DESCRIPTION
 *
 *	The first thing bru does when it encounters a read/write
 *	error on an archive is consult an internal device table
 *	for guidance on how to proceed.  For each environment,
 *	this table should contain recovery strategy information
 *	for each common archive device.
 *
 *	If the archive device is not in the table then default
 *	builtin strategies apply.
 */

#if unix || xenix
#ifndef EPERM
#include <errno.h>		/* May need to be <sys/errno.h> for BSD 4.2 */
				/* That won't work for xenix */
#endif
#endif

#define D_TRIES		4	/* Max R/W retries at each error */
#define D_RECUR		3	/* Max recursive error recovery limit */


/*
 *	Bits for the capabilities flags word.
 *
 *	D_ADVANCE means that even when reads or writes fail due to bad
 *	spots, the media advances so that the bad spot is eventually
 *	skipped over by successive read/writes.  This also implies that
 *	the equation <current position> = <previous position> + <bytes in/out>
 *	is not always valid in the presence of media errors.
 *
 *	D_IGNORECLOSE means to ignore the result of the close system call
 *	for the archive device.  Some systems return an error on the close
 *	if the close is issued during tape rewind.  This bug has been seen
 *	in SunOS (errno set to EPERM), on Altos machines running Xenix 3.2
 *	(errno set to EIO), and in AMIX 3.2 for the Commodore Amiga.
 *
 *	D_REBLOCKS means that the device does reblocking of the data, I.E.
 *	that the physical record size is independent of the I/O buffer
 *	size.  This means that we can write or read the device with
 *	any size buffer, providing we don't bump into the end of it
 *	before finding or writing the trailer block.
 */

#define D_REOPEN	(1<<0)	/* Close and reopen when at end of device */
#define D_ISTAPE	(1<<1)	/* Device is a tape drive */
#define D_RAWTAPE	(1<<2)	/* Device is a raw magtape drive */
#define D_NOREWIND	(1<<3)	/* No automatic rewind on close of file */
#define D_ADVANCE	(1<<4)  /* Read/write always advances media */
#define D_NOREOPEN	(1<<5)	/* Explicit no reopen, D_REOPEN obsolete */
#define D_QFWRITE	(1<<6)	/* Query before first write to vol */
#define D_EJECT		(1<<7)	/* Eject media after use (Mac'sh machines) */
#define D_FORMAT	(1<<8)	/* Format the media if necessary */
#define D_SHMCOPY	(1<<9)	/* Cannot do I/O directly from shared memory */
#define D_RAWFLOPPY	(1<<10)	/* Device is a raw floppy drive */
#define D_IGNORECLOSE	(1<<11)	/* Ignore result of close on device */
#define D_REBLOCKS	(1<<12) /* Device ignores I/O buffer size, reblocks */

/*
 *	A partial read/write is one that returns less than nbytes
 *	A zero read/write is one that returns -1
 */

struct device {			/* Known archive devices */
    char *dv_dev;		/* Named system device */
    char *dv_handler;		/* Handler for device (Amiga only) */
    int dv_flags;		/* Flags for various capabilities */
    ULONG dv_msize;		/* Media size if known */
    long dv_shmmax;		/* Limit on size of a shared mem segment */
    long dv_shmall;		/* Limit on total amount of shared memory */
    UINT dv_bsize;		/* Default buffer size for this device */
    UINT dv_maxbsize;		/* Maximum buffer size for this device */
    UINT dv_blocksize;		/* Physical blocksize for this device */
    UINT dv_shmseg;		/* Limit on number of shared mem segments */
    int dv_seek;		/* Minimum seek resolution, 0 if no seeks */
    int dv_prerr;		/* Errno for partial reads at end of device */
    int dv_pwerr;		/* Errno for partial writes at end of device */
    int dv_zrerr;		/* Errno for zero reads at end of device */
    int dv_zwerr;		/* Errno for zero writes at end of device */
    int dv_frerr;		/* Errno for unformatted media read attempt */
    int dv_fwerr;		/* Errno for unformatted media write attempt */
    int dv_wperr;		/* Errno for write protected */
    int dv_ederr;		/* Errno for end of device reached */
    int dv_unit;		/* SCSI unit number (Amiga only) */
};

extern int errno;		/* Error number */
