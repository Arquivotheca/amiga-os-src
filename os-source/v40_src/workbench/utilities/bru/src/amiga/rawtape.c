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
 *	rawtape.c    routines to handle "raw" tape devices
 *
 *  SCCS
 *
 *	@(#)rawtape.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	These routines implement the "raw" interface for tape
 *	devices.  Unfortunately, AmigaDOS is much less friendly
 *	than Unix about allowing "raw" access to devices.
 *
 *  NOTES
 *
 *	The raw floppy support and the raw tape support we added to
 *	bru at different times, and without a lot of thought to making
 *	them general.  Both modules should be merged into one, most
 *	likely by sucking the raw floppy support into this one, which
 *	is more general, but not by much...
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

static int Name2rfd PROTO((char *name));

/*
 *	Raw devices are managed by a table that is kept internal to
 *	this module, indexed by the raw file descriptor which is
 *	passed to and from external routines.  To be recognized, the
 *	device must first be "registered" by calling AddRawDevice().
 *	It can then be accessed by calling the other routines.
 *
 *	The raw device table consists of an array of pointers to
 *	structures of the following type.
 */

struct rawdevice {
    char *rd_dev;		/* Device name from the brutab entry */
    char *rd_handler;		/* Device handler from the brutab entry */
    int rd_unit;		/* Unit number from the brutab entry */
    int rd_flags;		/* Device flags word */
    int rd_fd;			/* File descriptor, if open */
    int rd_oflag;		/* Open mode of raw device */
    int rd_blocksize;		/* Physical block size of device */
};

/*
 *	File descriptors for raw tape devices start at FD_BASE and go up.
 *	The file descriptor is FD_BASE plus an integral offset into an
 *	table of pointers to structures containing information about the
 *	raw device.  This is somewhat of a kludge (see the comments in
 *	common/sys.c) but does allow us to coexist peaceably with the C
 *	runtime environment.
 *
 */

#define RFD_BASE	512		/* Raw device fd base */
#define RFD_COUNT	64		/* Up to 64 raw devices */

#define RFI(rfd)	((rfd) - RFD_BASE)
#define RFD(rfi)	((rfi) + RFD_BASE)
#define VALIDRFD(rfd)	((rfd >= RFD_BASE) && (rfd < (RFD_BASE + RFD_COUNT)))
#define RDP(rfd)	(VALIDRFD(rfd) ? rdt[RFI(rfd)] : NULL)
#define RAWTAPE(rdp)	((rdp) -> rd_flags & D_RAWTAPE)

static struct rawdevice *rdt[RFD_COUNT];

/*
 *	Other defined constants.
 */

#define BLOCKSIZE 512		/* Physical blocksize if no brutab value */

/*
 *	Local structures, variables, etc.
 */

static struct IOStdReq SCSIReq;
static struct SCSICmd Cmd;
static struct MsgPort Port;
static UBYTE Sense[18];			/* Auto sense data goes here */

/*
 *	SCSI command templates.
 */

static UBYTE  TestReady[] = { 0x00,0,0,0,0,0 };
static UBYTE  Rewind[] = { 0x01,0,0,0,0,0 };
static UBYTE  WriteBlocks[] = { 0x0a,1,0,0,1,0 };
static UBYTE  ReadBlocks[] = { 0x08,1,0,0,1,0 };
static UBYTE  WriteFM[] = { 0x10,0,0,0,1,0 };
static UBYTE  Space[] = { 0x11,1,0,0,1,0 };	/* Skip one filemark */

/*
 *	Set the errno to something meaningful for perror.
 */

void SetErrno ()
{
    extern int errno;

    DBUG_ENTER ("SetErrno");
    if (Sense[0] == 0x70 && Sense[2] == 7) {
	errno = EROFS;
    } else {
	errno = EIO;
    }
    DBUG_VOID_RETURN;
}

/*
 *	Given the name of a device, look through the known raw device
 *	table to see if it has been registered.  If so, return it's
 *	raw file descriptor.  Otherwise, return -1, which is never
 *	a valid raw file descriptor.
 */

static int Name2rfd (name)
char *name;
{
    int rdi;
    int rfd = -1;
    struct rawdevice *rdp;

    DBUG_ENTER ("Name2rfd");
    for (rdi = 0; rdi < RFD_COUNT; rdi++) {
	if (((rdp = rdt[rdi]) != NULL) && (STRSAME (rdp -> rd_dev, name))) {
	    rfd = RFD (rdi);
	    DBUG_PRINT ("rfd", ("found rfd %d for %s", rfd, name));
	    break;
	}
    }
    DBUG_RETURN (rfd);
}

/*
 *	Given a pointer to a device structure as built from a brutab entry,
 *	add that device to the table of known devices.  First try to see if
 *	a device with that name is already registered.  If so, just return
 *	the rfd associated with the already registered device (thus all names
 *	must be unique).  If no matching device with that name is found, 
 *	attempt to add it to the table and return its rfd.  If the table is
 *	full, return -1.
 *
 *	Note that we leave the device's rd_fd (a copy of the raw file
 *	descriptor) set to zero to flag the fact that the device is
 *	not yet open.  When it is opened, the rd_fd is set to match
 *	the rfd for that device.
 */

int AddRawDevice (dp)
struct device *dp;
{
    struct rawdevice *rdp;
    int rdi;
    int rfd;

    DBUG_ENTER ("AddRawDevice");
    DBUG_PRINT ("rdev", ("add raw device '%s'", dp -> dv_dev));
    if ((rfd = Name2rfd (dp -> dv_dev)) == -1) {
	DBUG_PRINT ("rdev", ("not found in table, add it"));
	for (rdi = 0; rdi < RFD_COUNT; rdi++) {
	    if (rdt[rdi] == NULL) {
		DBUG_PRINT ("rdt", ("found empty slot at rdi %d", rdi));
		rdp = (struct rawdevice *) s_malloc (sizeof (*rdp));
		if (rdp != NULL) {
		    rdp -> rd_dev = s_strdup (dp -> dv_dev);
		    rdp -> rd_handler = s_strdup (dp -> dv_handler);
		    rdp -> rd_unit = dp -> dv_unit;
		    rdp -> rd_flags = dp -> dv_flags;
		    rdp -> rd_fd = 0;
		    rdp -> rd_blocksize = dp -> dv_blocksize;
		    if (rdp -> rd_blocksize == 0) {
			rdp -> rd_blocksize = BLOCKSIZE;
		    }
		    rdt[rdi] = rdp;
		    rfd = RFD (rdi);
		}
		break;
	    }
	}
    }
    DBUG_PRINT ("rfd", ("got rfd %d for %s", rfd, dp -> dv_dev));
    DBUG_RETURN (rfd);
}

/*
 *	All seeks fail for now.
 */

long LseekRawTape (fd, offset, origin)
int fd;
long offset;
int origin;
{
    DBUG_ENTER ("LseekRawTape");
    DBUG_PRINT ("rdseek", ("all raw device seeks fail for now"));
    DBUG_RETURN (-1);
}

/*
 *	A way for external routines to query whether or not a file descriptor
 *	is for an open raw tape device.  The rfd must be valid, must translate
 *	to a valid raw device pointer, which must be for a tape device, which
 *	must be open.  Returns 1 if it is an open raw tape device, 0 otherwise.
 */

int IsRawTapeFd (rfd)
int rfd;
{
    struct rawdevice *rdp;

    return (((rdp=RDP(rfd)) != NULL) && RAWTAPE(rdp) && (rdp->rd_fd != 0));
}

/*
 *	Check to see if a filename matches the name of a registered
 *	raw device, and that device is a tape device.  If so,then return
 *	zero for success, -1 otherwise.  Note that the "mode" argument
 *	is provided for symmetry with the UNIX access() routine, and is
 *	unused here since raw devices have no protection.
 */

int AccessRawTape (filename, mode)
char *filename;
int mode;
{
    int rtnval = -1;
    int rfd;

    DBUG_ENTER ("AccessRawTape");
    DBUG_PRINT ("rtape", ("check '%s' for amiga raw tape", filename));
    if ((rfd = Name2rfd (filename)) != -1) {
	if (RAWTAPE (RDP (rfd))) {
	    DBUG_PRINT ("rtape", ("match found, is a pseudo tape name"));
	    rtnval = 0;
	}
    }
    DBUG_RETURN (rtnval);
}

/*
 *	When opening the raw tape device, we do a testready to see
 *	if the device is ready yet.  If this command gets an error,
 *	we ignore the error, wait 5 seconds, and try again.  If
 *	the second try also fails, then our open is unsuccessful.
 *
 *	Note that we set the rd_fd field of the raw device structure
 *	to match the rfd which indexes the raw device structure in
 *	the table.  This flags the device as open and ready to be
 *	read/written/closed.
 *
 *	As with the other raw device routines, the "mode" parameter is
 *	provided for symmetry with the UNIX equivalent, and is not
 *	used since all raw devices have no protection.
 */

int OpenRawTape (name, oflag, mode)
char *name;
int oflag;
int mode;
{
    struct rawdevice *rdp;
    int rfd;
    int rtnval = -1;

    DBUG_ENTER ("OpenRawTape");
    DBUG_PRINT ("tape", ("open raw tape '%s', oflag 0%o", name, oflag));
    rfd = Name2rfd (name);
    if ((rfd != -1) && ((rdp = RDP (rfd)) != NULL) && RAWTAPE (rdp)) {
	DBUG_PRINT ("rtape", ("pseudo device '%s'", rdp -> rd_dev));
	DBUG_PRINT ("rtape", ("handler '%s'", rdp -> rd_handler));
	DBUG_PRINT ("rtape", ("unit %d", rdp -> rd_unit));
	Port.mp_Node.ln_Pri = 0;
	Port.mp_SigBit = AllocSignal (-1L);
	Port.mp_SigTask = (struct Task *) FindTask (0L);
	NewList (&(Port.mp_MsgList));
	SCSIReq.io_Message.mn_ReplyPort = &Port;
	
	/* unless .device was specified, or if the signal alloc failed... */
	/* error out */
	if(!Port.mp_SigBit || ((strlen(rdp->rd_handler) < 8 ) || 
	stricmp(&rdp->rd_handler[strlen(rdp->rd_handler)-7],".device"))) {
	        if(Port.mp_SigBit)FreeSignal(Port.mp_SigBit);
		SetErrno();
	}
	else if (OpenDevice (rdp -> rd_handler, rdp -> rd_unit, &SCSIReq, 0) == 0)  {
	    SCSIReq.io_Length = 0;
	    SCSIReq.io_Data = (APTR) &Cmd;
	    SCSIReq.io_Command = HD_SCSICMD;
	    Cmd.scsi_Data = (UWORD *) 0;
	    Cmd.scsi_Length = 0;
	    Cmd.scsi_CmdLength = 6;
	    Cmd.scsi_Flags = SCSIF_AUTOSENSE;
	    Cmd.scsi_SenseData = Sense;
	    Cmd.scsi_SenseLength = 18;
	    Cmd.scsi_SenseActual = 0;
	    Cmd.scsi_Command = TestReady;
	    DoIO (&SCSIReq);
	    if (Cmd.scsi_Status != 0) {
		DBUG_PRINT ("rtopen", ("test ready failed, try again"));
		Delay (50 * 5);
		DoIO (&SCSIReq);
	    }
	    if (Cmd.scsi_Status != 0) {
		SetErrno ();
		CloseDevice (&SCSIReq);
	        if(Port.mp_SigBit)FreeSignal(Port.mp_SigBit);
	    } 
	    else {
		DBUG_PRINT ("rtopen", ("test ready succeeded"));
		rdp -> rd_fd = rfd;
		rdp -> rd_oflag = oflag;
		rtnval = rfd;
	    }
	}
    }
    DBUG_PRINT ("rfd", ("rfd %d for tape %s", rtnval, name));
    DBUG_RETURN (rtnval);
}

/*
 *	Close a raw tape device.  If open for write, then a filemark
 *	is written at the current tape location.  If the device is
 *	the rewinding device, a SCSI rewind command is issued.
 */

int CloseRawTape (rfd)
int rfd;
{
    int status = -1;
    struct rawdevice *rdp;
    int wrt;

    DBUG_ENTER ("CloseRawTape");
    if (((rdp = RDP (rfd)) != NULL) && RAWTAPE (rdp) && (rdp -> rd_fd > 0)) {
	status = 0;
	wrt = (rdp -> rd_oflag & O_RDWR) || (rdp -> rd_oflag & O_WRONLY);
	if (wrt) {
	    DBUG_PRINT ("crtape", ("write a filemark"));
	    Cmd.scsi_Command = WriteFM;
	    DoIO (&SCSIReq);
	    if (Cmd.scsi_Status != 0) {
		SetErrno ();
		status = -1;
	    }
	}
	if (!(rdp -> rd_flags & D_NOREWIND)) {
	    DBUG_PRINT ("crtape", ("rewind tape"));
	    Cmd.scsi_Command = Rewind;
	    DoIO (&SCSIReq);
	    if (Cmd.scsi_Status != 0) {
		SetErrno ();
		status = -1;
	    }
	} else if (!wrt) {
#if 0	/* Does not seem to work like expected.  Needs further checking */
	    DBUG_PRINT ("crtape", ("skip over filemark"));
	    Cmd.scsi_Command = Space;
	    DoIO (&SCSIReq);
	    if (Cmd.scsi_Status != 0) {
		SetErrno ();
		status = -1;
	    }
#endif
	}
	CloseDevice (&SCSIReq);
	rdp -> rd_fd = 0;
	if(Port.mp_SigBit)FreeSignal(Port.mp_SigBit);
    }
    DBUG_RETURN (status);
}

/*
 *	A write request.  Looks just like the standard UNIX write() system
 *	call.  Will fail if either fd or buf is not valid.  Will
 *	also fail if bufsize is not a valid multiple of a block size.
 */

int WriteRawTape (rfd, buf, bufsize)
int rfd;
char *buf;
int bufsize;
{
    int status = -1;
    int nblks;
    struct rawdevice *rdp;

    DBUG_ENTER ("WriteRawTape");
    DBUG_PRINT ("wraw", ("rfd %d, bufsize %d", rfd, bufsize));
    if (((rdp = RDP (rfd)) != NULL) && RAWTAPE (rdp) && (rdp -> rd_fd > 0)) {
	DBUG_PRINT ("blksize", ("phys blocksize %d", rdp -> rd_blocksize));
	if ((buf != NULL) && ((bufsize % (rdp -> rd_blocksize)) == 0)) {
	    nblks = bufsize / (rdp -> rd_blocksize);
	    DBUG_PRINT ("nblks", ("write %d physical blocks", nblks));
	    WriteBlocks[2] = (nblks >> 16) & 0xFF;
	    WriteBlocks[3] = (nblks >>  8) & 0xFF;
	    WriteBlocks[4] = (nblks >>  0) & 0xFF;
	    Cmd.scsi_Command = WriteBlocks;
	    Cmd.scsi_Length = bufsize;
	    Cmd.scsi_Data = (UWORD *) buf;
	    DoIO (&SCSIReq);
	    if (Cmd.scsi_Status != 0) {
		SetErrno ();
	    } else {	
		status = bufsize;
	    }
	}
    }
    DBUG_RETURN (status);
}

/*
 *	A read request.  Looks just like the standard UNIX read() system
 *	call.  Will fail if either fd or buf is not valid.  Will
 *	also fail if bufsize is not a valid multiple of a block size.
 */

int ReadRawTape (rfd, buf, bufsize)
int rfd;
char *buf;
int bufsize;
{
    int status = -1;
    int nblks;
    struct rawdevice *rdp;

    DBUG_ENTER ("ReadRawTape");
    DBUG_PRINT ("rraw", ("rfd %d, bufsize %d", rfd, bufsize));
    if (((rdp = RDP (rfd)) != NULL) && RAWTAPE (rdp) && (rdp -> rd_fd > 0)) {
	DBUG_PRINT ("blksize", ("phys blocksize %d", rdp -> rd_blocksize));
	if ((buf != NULL) && ((bufsize % (rdp -> rd_blocksize)) == 0)) {
	    nblks = bufsize / (rdp -> rd_blocksize);
	    DBUG_PRINT ("nblks", ("read %d physical blocks", nblks));
	    ReadBlocks[2] = (nblks >> 16) & 0xFF;
	    ReadBlocks[3] = (nblks >>  8) & 0xFF;
	    ReadBlocks[4] = (nblks >>  0) & 0xFF;
	    Cmd.scsi_Command = ReadBlocks;
	    Cmd.scsi_Length = bufsize;
	    Cmd.scsi_Data = (UWORD *) buf;
	    DoIO (&SCSIReq);
	    if (Cmd.scsi_Status != 0) {
		SetErrno ();
	    } else {	
		status = bufsize;
	    }
	}
    }
    DBUG_RETURN (status);
}

