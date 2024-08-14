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
 *	rawdev.c    routines to handle "raw" devices
 *
 *  SCCS
 *
 *	@(#)rawdev.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	These routines implement the "raw" interface for floppy
 *	devices.  Unfortunately, AmigaDOS is much less friendly
 *	than Unix about allowing "raw" access to devices.
 *
 *	Note that there are LOTS of hardwired dependencies here,
 *	that really should not be hardwired into the executable.
 *	Sigh...
 *
 *  AUTHOR
 *
 *	Fred Fish
 *	Tempe, Arizona
 *
 */

#include "globals.h"

static struct raw_device *FindRaw PROTO((char *name));
static VOID CloseStuff PROTO((void));
static int OpenDisk PROTO((int unit));
static int BufferAvailable PROTO((int bufsize));

#ifdef MANX
#define EIO EINVAL
#define EROFS EINVAL
#endif

#ifndef NUMCYLS
#define NUMCYLS 80L
#endif

#ifndef NUMHEADS
#define NUMHEADS 2L
#endif

#ifndef NUMSECS
#define NUMSECS 11L
#endif

#define TRACKSIZE	(NUMSECS * TD_SECTOR)
#define MAXOFFSET	(NUMCYLS * NUMHEADS * NUMSECS * TD_SECTOR)
#define VALIDBUFSIZE(s)	((s) >= 0 && ((s) % TD_SECTOR) == 0)
#define TRACKALIGNED(s) (((s) % (TRACKSIZE)) == 0)

/*
 *  Note that the way raw file descriptors are handled is a hack.
 *  For manx, file descriptors returned by the open() call are normally
 *  very small integers (0, 1, 2, ...) like unix.  For Lattice, file
 *  descriptors are actually amiga file handles, which are typically
 *  very large integers (I.E. 784560).  We use a range of 256 to 259, 
 *  which should be above those returned by manx and below those
 *  returned by Lattice, while still meeting the requirements of
 *  being non-negative integers and falling withing the range of
 *  a manx 16-bit int.  Sigh...
 */

#define VALIDFD(fd)	((fd) >= 256 && (fd) <= 259)
#define FD2UNIT(fd)	((fd) - 256)
#define UNIT2FD(unit)	((unit) + 256)

static struct MsgPort *diskport;
static struct IOExtTD *diskreq;

static ULONG *diskbuffer;
static ULONG diskbuffersize;
static ULONG diskchangecount;
static ULONG CurrentOffset = 0;

struct raw_device {
    char *df_name;
    int df_unit;
};

static struct raw_device raw_devices[] = {
    "df0:", 0,
    "df1:", 1,
    "df2:", 2,
    "df3:", 3,
    "DF0:", 0,
    "DF1:", 1,
    "DF2:", 2,
    "DF3:", 3,
    NULL, 0
};

static struct raw_device *FindRaw (name)
char *name;
{
    struct raw_device *dev;

    DBUG_ENTER ("FindRaw");
    for (dev = raw_devices; dev -> df_name != NULL; dev++) {
	if (strcmp (name, dev -> df_name) == 0) {
	    DBUG_PRINT ("raw", ("found raw device name '%s'", name));
	    break;
	}
    }
    if (dev -> df_name == NULL) {
	dev = NULL;
    }
    DBUG_RETURN (dev);
}

/*
 *  This can be called externally during cleanup, even if motor
 *  was never turned on...
 */

int MotorOff ()
{
    int rtnval = 0;
    long err;

    DBUG_ENTER ("MotorOff");
    if (diskreq != NULL) {
	diskreq -> iotd_Req.io_Length = 0;    
	/* says that motor is to be turned off */
	diskreq -> iotd_Req.io_Command = TD_MOTOR;    
	/* do something with the motor */
	if ((err = DoIO ((struct IORequest *) diskreq)) == 0) {
	    rtnval = 1;
	} else {
	    DBUG_PRINT ("err", ("DoIO returns %d", err));
	}
    }
    DBUG_RETURN (rtnval);
}

static VOID CloseStuff ()
{
    DBUG_ENTER ("CloseStuff");
    if (diskreq != NULL) {
	CloseDevice ((struct IORequest *) diskreq);
	DeleteExtIO ((struct IORequest *) diskreq);
	diskreq = NULL;
    }
    if (diskbuffer != NULL && diskbuffersize > 0) {
	FreeMem (diskbuffer, diskbuffersize);
	diskbuffer = NULL;
	diskbuffersize = 0;
    }
    if (diskport != NULL) {
	DeletePort (diskport);
	diskport = NULL;
    }
    DBUG_VOID_RETURN;
}

/*
 *  Returns 0 if disk cannot be opened, 1 if success.
 */

static int OpenDisk (unit)
int unit;
{
    int rtnval = 0;
    long err;

    DBUG_ENTER ("OpenDisk");
    DBUG_PRINT ("raw", ("open unit %d", unit));
    if ((diskport = CreatePort (NULL, NULL)) != NULL) {
	diskreq = (struct IOExtTD *) 
		CreateExtIO (diskport, (long) sizeof (*diskreq));
	if (diskreq != NULL) {
	    DBUG_PRINT ("dreq", ("diskreq = %x", diskreq));
	    err = OpenDevice (TD_NAME, (long) unit,
			      (struct IORequest *) diskreq, (long) NULL);
	    if (err != 0) {
		DBUG_PRINT ("raw", ("OpenDevice() returned %ld", err));
		DeleteExtIO ((struct IORequest *) diskreq);
		diskreq = NULL;
	    } else {
		DBUG_PRINT ("dopen", ("OpenDevice ok, now DoIO"));
		diskreq -> iotd_Req.io_Command = TD_CHANGENUM;
		if ((err = DoIO ((struct IORequest *) diskreq)) == 0) {
#if 0		    /* temp hack, ignore count til we figure it out */
		    diskchangecount = diskreq -> iotd_Req.io_Actual;
		    DBUG_PRINT ("dcnt", ("change count %d", diskchangecount));
#else
		    diskchangecount = 0xFFFFFFFF;
#endif
		    rtnval = 1;
		    DBUG_PRINT ("raw", ("disk is now open"));
		} else {
		    DBUG_PRINT ("err", ("DoIO returns %d", err));
		}
	    }
	}
    }
    DBUG_RETURN (rtnval);
}

#define ALLOCFLAGS (long) (MEMF_CLEAR | MEMF_CHIP)

static int BufferAvailable (bufsize)
int bufsize;
{
    int rtnval = 0;

    DBUG_ENTER ("BufferAvailable");
    if (bufsize > 0) {
	if (bufsize > diskbuffersize) {
	    if (diskbuffer != NULL && diskbuffersize != 0) {
		FreeMem (diskbuffer, diskbuffersize);
	    }
	    diskbuffersize = bufsize;
	    diskbuffer = (ULONG *) AllocMem (diskbuffersize, ALLOCFLAGS);
	}
	rtnval = (diskbuffer != NULL);
    }
    DBUG_RETURN (rtnval);
}

int OpenRawFloppy (name, oflag, mode)
char *name;
int oflag;
int mode;
{
    struct raw_device *dev;
    int fd = -1;
    int rtnval = -1;
    extern int DisableDevice ();

    DBUG_ENTER ("OpenRawFloppy");
    DBUG_PRINT ("raw", ("try to open raw device '%s'", name));
    CurrentOffset = 0;
    if ((dev = FindRaw (name)) != NULL) {
	fd = UNIT2FD (dev -> df_unit);
	DBUG_PRINT ("raw", ("try raw stream %d", fd));
    }
    if (VALIDFD (fd)) {
	if (DisableDevice (name)) {
	    if (OpenDisk (FD2UNIT (fd))) {
	        rtnval = fd;
	        DBUG_PRINT ("raw", ("raw device open now"));
	    }
	}
    }
    DBUG_RETURN (rtnval);
}

int ReadRawFloppy (fd, buf, bufsize)
int fd;
char *buf;
int bufsize;
{
    int rtnval = -1;
    long err;

    DBUG_ENTER ("ReadRawFloppy");
    if ((CurrentOffset + bufsize) > MAXOFFSET) {
	bufsize = MAXOFFSET - CurrentOffset;
    }
    if (VALIDFD (fd) && buf != NULL && VALIDBUFSIZE (bufsize)) {
	if (BufferAvailable (bufsize)) {
	    diskreq -> iotd_Req.io_Length = bufsize;
	    diskreq -> iotd_Req.io_Data = (APTR) diskbuffer;	
	    /* show where to put the data when read */
	    diskreq -> iotd_Req.io_Command = ETD_READ;
	    /* check that disk not changed before reading */
	    diskreq -> iotd_Count = diskchangecount;
	    diskreq -> iotd_Req.io_Offset = CurrentOffset;
	    if ((err = DoIO ((struct IORequest *) diskreq)) == 0) {
		CurrentOffset += bufsize;
		movmem ((char *) diskbuffer, buf, bufsize);
		rtnval = bufsize;
	    } else {
		DBUG_PRINT ("err", ("DoIO returns %d", err));
		errno = EIO;
	    }
	}
    }
    DBUG_RETURN (rtnval);
}

/*
 *	Write out raw data.  If the buffersize is a multiple of the
 *	track size, and the current offset is on a track boundry, then
 *	we can format the track at the same time.
 */
 
int WriteRawFloppy (fd, buf, bufsize)
int fd;
char *buf;
int bufsize;
{
    long err;
    int rtnval = -1;

    DBUG_ENTER ("WriteRawFloppy");
    if ((CurrentOffset + bufsize) > MAXOFFSET) {
	bufsize = MAXOFFSET - CurrentOffset;
    }
    if (VALIDFD (fd) && buf != NULL && VALIDBUFSIZE (bufsize)) {
	if (BufferAvailable (bufsize)) {
	    diskreq -> iotd_Req.io_Length = bufsize;
	    diskreq -> iotd_Req.io_Data = (APTR) diskbuffer;	
	    /* show where to put the data when read */
	    if (TRACKALIGNED (bufsize) && TRACKALIGNED (CurrentOffset)) {
		DBUG_PRINT ("fmt", ("format and write tracks"));
		diskreq -> iotd_Req.io_Command = ETD_FORMAT;
	    } else {
		DBUG_PRINT ("fmt", ("can't format, not aligned"));
		diskreq -> iotd_Req.io_Command = ETD_WRITE;
	    }
	    /* check that disk not changed before writing */
	    diskreq -> iotd_Count = diskchangecount;
	    diskreq -> iotd_Req.io_Offset = CurrentOffset;
	    movmem (buf, (char *) diskbuffer, bufsize);
	    if ((err = DoIO ((struct IORequest *) diskreq)) == 0) {
		CurrentOffset += bufsize;
		rtnval = bufsize;
	    } else {
		DBUG_PRINT ("err", ("DoIO returns %d", err));
		if ((err == TDERR_WriteProt) || (err == TDERR_DiskChanged)) {
		    errno = EROFS;
		} else {
		    errno = EIO;
		}
	    }
	}
    }
    DBUG_RETURN (rtnval);
}

/*
 *  Note that in keeping with the Unix semantics of lseek(), all the seek
 *  does is reposition the current file offset.  Nothing about readability
 *  or writability of the location is assumed...
 */

long LseekRawFloppy (fd, offset, origin)
int fd;
long offset;
int origin;
{
    DBUG_ENTER ("LseekRawFloppy");
    switch (origin) {
	case 0:
	    CurrentOffset = offset;
	    break;
	case 1:
	    CurrentOffset += offset;
	    break;
	case 2:				/* should not happen in bru */
	    break;
    }
    DBUG_RETURN ((long) CurrentOffset);
}

int FlushRawFloppy ()
{
    int rtnval = 0;
    long err;

    DBUG_ENTER ("FlushRawFloppy");
    if (diskreq != NULL) {
	diskreq -> iotd_Req.io_Length = 1;
	diskreq -> iotd_Req.io_Command = ETD_UPDATE;
	diskreq -> iotd_Count = diskchangecount;
	if ((err = DoIO ((struct IORequest *) diskreq)) == 0) {
	    rtnval = 1;
	} else {
	    DBUG_PRINT ("err", ("DoIO returns %d", err));
	}
    }
    DBUG_RETURN (rtnval);
}

int CloseRawFloppy (fd)
int fd;
{
    int rtnval = -1;
    extern int EnableDevice ();

    DBUG_ENTER ("CloseRawFloppy");
    if (VALIDFD (fd)) {
	if (FlushRawFloppy ()) {
	    (VOID) MotorOff ();
	    (VOID) EnableDevice (raw_devices[FD2UNIT (fd)].df_name);
	    if (diskbuffer != NULL) {
	 	FreeMem (diskbuffer, diskbuffersize);
		diskbuffer = NULL;
		diskbuffersize = 0;
	    }
	    CloseStuff ();
	    rtnval = 0;
	}
    }
    DBUG_RETURN (rtnval);
}

int AccessRawFloppy (filename, mode)
char *filename;
int mode;
{
    int rtnval = -1;

    DBUG_ENTER ("AccessRawFloppy");
    if (FindRaw (filename) != NULL) {
	rtnval = 0;
    }
    DBUG_RETURN (rtnval);
}

/*
 *  A way for external routines to query whether or not a file descriptor
 *  is for a raw device.
 */

int IsRawFloppyFd (fd)
int fd;
{
	return (VALIDFD (fd));
}


