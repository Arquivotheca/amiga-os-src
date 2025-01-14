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
 *	blocks.c    routines to handle block buffers for bru
 *
 *  SCCS
 *
 *	@(#)blocks.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains routines to manipulate the block buffers for
 *	the bru utility.  Provides facilities to read, write,
 *	and seek on the archive file.  Buffers blocks internally
 *	and only does I/O in increments of the blocking factor
 *	chosen.
 *
 *	The following routines are provided:
 *
 *		ar_open:	open archive stream
 *		ar_read:	read next logical block
 *		ar_write:	write current logical block
 *		ar_seek:	seek to specified logical block
 *		ar_tell:	return current logical block number
 *		ar_close:	close archive stream
 *		ar_next:	locate next logical block
 *
 *  NOTES
 *
 *	Originally, the archive device was assumed to be seekable unless
 *	the user explicitly requested no seeks via the -p option,
 *	a brutab entry indicated that the device was NOT seekable, or
 *	the archive device was a pipe.   This turned out to cause lots
 *	of confusion when devices are not specified in the brutab file
 *	and were in fact not seekable (such as a raw mag tape unit
 *	used over a network).   So... the safest thing is to make the
 *	default to be NOT seekable and only use seeks when device is
 *	known to be seekable.  Better slow than broken!
 *
 *  BUGS
 *
 *	Currently does not support mixed reading/writing
 *	of archive file.
 *
 */

#include "globals.h"

static int inerr_recover PROTO((int nb, int err));
static int outerr_recover PROTO((int nb, int err));
static VOID dupdate PROTO((long nvolume));
static VOID setsize PROTO((long nsize));
static VOID open_std PROTO((void));
static union blk *lba_seek PROTO((LBA nlba));
static VOID rgrp PROTO((void));
static VOID rpipe PROTO((void));
static VOID rfile PROTO((void));
static VOID wgrp PROTO((void));
static VOID wpipe PROTO((void));
static VOID wfile PROTO((void));
static VOID ar_sync PROTO((void));
static VOID allocate PROTO((void));
static VOID r_phys_seek PROTO((LBA npba));
static BOOLEAN recover PROTO((int iobytes, int ioerr, BOOLEAN readflag));
static BOOLEAN do_open PROTO((void));
static VOID infer_end PROTO((void));
static VOID check_vol PROTO((void));
static VOID bad_vol PROTO((int vol));
static VOID new_bufsize PROTO((int size));
static VOID clean_buffer PROTO((int iobytes));
static VOID harderror PROTO((LBA errorpba, BOOLEAN readflag, int iobytes));
static VOID qfwrite PROTO((void));
static VOID eject_media PROTO((void));

/*
 *	In cases of extreme bugs, we can define DUMPBLK, which enables
 *	dumping of the contents of each archive block using the DBUG
 *	macro mechanism.  We don't normally want this, as it results
 *	in overwhelming output...
 */

#ifdef DUMPBLK
#  define DUMP_BLK(a,b) dump_blk(a,b)	/* call support routine */
#else
#  define DUMP_BLK(a,b)			/* define the calls away */
#endif


static BOOLEAN reading;			/* Reading from archive */
static BOOLEAN bswap = FALSE;		/* Swap bytes in buffer */
static BOOLEAN sswap = FALSE;		/* Swap shorts in buffer */
static union blk *blocks = NULL;	/* Pointer to block buffer */
static BOOLEAN seekflag = FALSE;	/* Use seeks instead of reads */
static int lbi = 0;			/* Index into blocks[] */
static LBA gsba = 0;			/* Group start block address */
static LBA vsba = 0;			/* Volume start block address */
static LBA pba = 0;			/* Physical block addr in archive */
static LBA grp = 0;			/* Current block group in buffer */
static int volume = 0;			/* Current volume number */
static int lastvolume = 0;		/* Last volume of previous run */
static int vol_estimate = 0;		/* Estimation volume number */
static LBA lba_estimate = 0;		/* Estimation logical block address */
static BOOLEAN data = FALSE;		/* Flag for buffer has data */
static BOOLEAN dirty = FALSE;		/* Buffer has been changed */
static BOOLEAN first = TRUE;		/* First read/write of media */
static BOOLEAN checkswap = TRUE;	/* Only swap check once per media */
static BOOLEAN pipe_io = FALSE;		/* Doing I/O to/from a pipe */
static int tries;			/* Current number of retries */
static int firsterrno = 0;		/* Original errno from original err */
static int recursions = 0;		/* Error recovery recursion count */
static long fseqnum = 0;		/* File sequence number */

#if HAVE_SHM

#define	DBL_READING	0		/* reading from device */
#define DBL_WRITING	1		/* writing to device */
#define DBL_DOBUF	1		/* perform double buffering */
#define DBL_NOBUF	0		/* be transparent */

static int trans_dbl;			/* if in transparent mode */
static int wdblinit;			/* set if write init done */

#define READ(fd,buf,count)	dbl_read(buf,count)
#define WRITE(fd,buf,count)	dbl_write(buf,count)

#else	/* !HAVE_SHM */

#define READ(fd,buf,count)	s_read(fd,buf,count)
#define WRITE(fd,buf,count)	s_write(fd,buf,count)

#endif	/* HAVE_SHM */


#if HAVE_SHM

/*
 * The following two routines are called by the double buffering module
 * when an input or output error occurs.
 */

static int inerr_recover (nb, err)
int nb;
int err;
{
    int status;

    DBUG_ENTER ("inerr_recover");
    tries++;
    status = recover (nb, err, 1);
    DBUG_RETURN (status);
}

static int outerr_recover (nb, err)
int nb;
int err;
{
    int status;

    DBUG_ENTER ("outerr_recover");
    tries++;
    status = recover (nb, err, 0);
    DBUG_RETURN (status);
}

static VOID dupdate (nvolume)
long nvolume;
{
    switch_media ((int) nvolume);
}

static VOID setsize (nsize)
long nsize;
{
    msize = nsize;
}

#endif	/* HAVE_SHM */


/*
 *  FUNCTION
 *
 *	ar_open   open the archive stream
 *
 *  SYNOPSIS
 *
 *	VOID ar_open ();
 *	
 *  DESCRIPTION
 *
 *	Note that when the archive is opened for the first time,
 *	space is automatically allocated for the block buffer.
 *
 */

VOID ar_open ()
{
    DBUG_ENTER ("ar_open");
    fseqnum = 0;
    pba = 0;
    gsba = 0;
    DBUG_PRINT ("gsba", ("group start block addr %ld", gsba));
    vsba = 0;
    DBUG_PRINT ("vsba", ("new volume start block addr %ld", vsba));
    grp = 0;
    DBUG_PRINT ("grp", ("block group %ld", grp));
    volume = 0;
    einfo.zmin = 0;
    einfo.zmax = 0;
    einfo.filebytes = 0;
    einfo.zfilebytes = 0;
#if HAVE_SHM
    wdblinit = 0;
#endif
    lbi = -1;				/* CAUTION ! */
    dirty = FALSE;
    data = FALSE;
    if (!do_open ()) {
	switch_media (0);
    }
    if (mode == 't' && !flags.bflag && seekable (afile.fname, BLKSIZE)) {
	DBUG_PRINT ("bufsize", ("bufsize auto-adjusted for min reads"));
	bufsize = BLKSIZE;
	flags.bflag = TRUE;
    }
    allocate ();
    if (reading) {
	read_info ();
#if HAVE_SHM
	if (msize != 0) {
	    if (trans_dbl == DBL_DOBUF) {
		(VOID) dbl_rpcdown (setsize, (long) msize);
	    }
	}
#endif
    } else {
	qfwrite ();
    }
    sanity ();
    DBUG_VOID_RETURN;
}


/*
 *  INTERNAL FUNCTION
 *
 *	open_std    open stdin/stdout as archive
 *
 *  SYNOPSIS
 *
 *	static VOID open_std ()
 *
 *  DESCRIPTION
 *
 *	Opens either the standard input or standard output as
 *	the archive, depending upon whether an archive is being
 *	read or written respectively.
 *
 */

static VOID open_std ()
{
    DBUG_ENTER ("open_std");
    pipe_io = TRUE;
    seekflag = FALSE;
    if (mode == 'c') {
	afile.fildes = s_dup (1);
    } else {
	afile.fildes = s_dup (0);
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	ar_seek   seek to a logical block
 *
 *  SYNOPSIS
 *
 *	union blk *ar_seek (offset, whence)
 *	LBA offset;
 *	int whence;
 *
 *  DESCRIPTION
 *
 *	Repositions archive read/write pointer to the first byte
 *	of the logical block specified by the combination of the
 *	current read/write pointer (implicitly) and the offset and
 *	whence values (explicitly).
 *
 *	Sets the file pointer as follows:
 *
 *	    If whence is 0, the pointer is set to offset blocks.
 *
 *	    If whence is 1, the pointer is set to its current
 *		block number plus offset blocks.
 *
 *	Returns a pointer to the first byte of the logical block.
 *
 *	Note that this simply provides a place to either read
 *	data from (after a call to ar_read) or to write data
 *	to (followed by a call to ar_write).  There is no
 *	meaningful I/O implied, although physical I/O may
 *	take place if physical seeks are not allowed.
 *
 *
 */


union blk *ar_seek (offset, whence)
LBA offset;
int whence;
{
    union blk *blkp;
    LBA nlba;
    
    DBUG_ENTER ("ar_seek");
    if (whence == 0) {
	nlba = offset;
    } else {
	nlba = offset + gsba + lbi;
    }
    blkp = lba_seek (nlba);
    DBUG_RETURN (blkp);
}


/*
 *  Check the final derived lbi to make sure it is in the range of
 *  the buffer.  If not (probably because gsba took an unexpected
 *  jump due to error recovery), then force it back to zero and
 *  ignore the descrepancy.  This used to generate an error message
 *  if a volume was abandoned because of an unrecoverable problem
 *  with it.
 */

static union blk *lba_seek (nlba)
LBA nlba;
{
    LBA ngrp;
    int nvolume;
    LBA npba;
    int bufblocks;
    LBA mblocks;

    DBUG_ENTER("lba_seek");
    DBUG_PRINT ("seek", ("logical seek to archive block %ld", nlba));
    bufblocks = BLOCKS (bufsize);
    ngrp = nlba / bufblocks;
    DBUG_PRINT ("ngrp", ("new block in group %ld", ngrp));
    if (ngrp != grp) {
	ar_sync ();
	data = FALSE;
	grp = ngrp;
	DBUG_PRINT ("grp", ("block group %ld", grp));
	gsba = grp * bufblocks;
	DBUG_PRINT ("gsba", ("group start block addr %ld", gsba));
	npba = gsba - vsba;
	DBUG_PRINT ("npba", ("new phys block addr %ld", npba));
	if (msize != 0) {
	    mblocks = BLOCKS (msize);
	    nvolume = (int) (nlba / mblocks);
	    if (nvolume != volume) {
#if HAVE_SHM
		if (trans_dbl == DBL_DOBUF) {
		    if (!reading) {
			SIGTYPE prevINT;
			SIGTYPE prevQUIT;

			/*
			 * In case user pops the interrupt while the
			 * child is doing the next reel, we need to
			 * recover correctly as well.
			 */
			sig_push (&prevINT, &prevQUIT);
			(VOID) s_signal (SIGINT, SIG_DFL);
			(VOID) dbl_rpcdown (dupdate, (long) nvolume);
			sig_pop (&prevINT, &prevQUIT);

			/*
			 * After remote call completes, clean up by
			 * doing some of the switch_media stuff that
			 * needs to be done locally.
			 */
			volume = nvolume;
			pba = 0;
			first = TRUE;
			checkswap = TRUE;
			sswap = FALSE;
			bswap = FALSE;
		    }
		} else {
		    switch_media (nvolume);
		}
#else
		switch_media (nvolume);
#endif
		vsba = volume * mblocks;
		DBUG_PRINT ("vsba", ("new volume start block addr %ld", vsba));
		npba = gsba - vsba;
		DBUG_PRINT ("npba", ("new phys block addr %ld", npba));
	    }
	}
	phys_seek (npba);
    }
    lbi = (int) (nlba - gsba);
    if (lbi < 0 || lbi >= bufblocks) {
	DBUG_PRINT ("badlbi", ("bad lbi %d, forced to 0", lbi));
	lbi = 0;
	/* bru_message (MSG_BUG, "lba_seek"); */
    }
    DBUG_PRINT ("lba_seek", ("new logical block index %d", lbi));
    DBUG_PRINT ("grp", ("grp %ld  gsba %ld  pba %ld", grp, gsba, pba));
    DBUG_RETURN (&blocks[lbi]);
}


/*
 *	Note that the seek is done even if the npba equals
 *	the pba, since the pba may not be correct if error
 *	recovery is being attempted.
 *
 *	If we are attempting to seek by reading, it is
 *	possible that we may encounter an unrecoverable
 *	problem with the initial volume, causing it to
 *	be abandoned, and switching to the next volume.
 *	We watch for this situation by monitoring the
 *	current volume number, since it makes no sense
 *	to do a physical seek across volume boundries.
 *	If the volume changes, we abort the reads and
 *	simply return at whatever point we currently
 *	are in the archive.  This used to be a fatal
 *	condition.  It still is fatal for true seeks,
 *	which may be a malfeature...
 * 
 */

VOID phys_seek (npba)
LBA npba;
{
    S32BIT offset;
    int startvol;

    DBUG_ENTER ("phys_seek");
    DBUG_PRINT ("pba", ("seek from %ld to %ld", pba, npba));
    if (!seekflag) {
	startvol = volume;
	while (pba < npba && startvol == volume) {
	    rgrp ();
	}
	data = FALSE;
    } else {
	offset = npba * BLKSIZE;
	DBUG_PRINT ("ar_io", ("physical seek to block %ld", npba));
	if (s_lseek (afile.fildes, offset, 0) != offset) {
	    bru_message (MSG_ARSEEK);
	    done ();
	}
	pba = npba;
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	ar_read   read next logical block
 *
 *  SYNOPSIS
 *
 *	VOID ar_read (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Performs logical read of next logical block.
 *
 */

VOID ar_read (fip)
struct finfo *fip;
{
    LBA bfound;

    DBUG_ENTER ("ar_read");
    DBUG_PRINT ("flba", ("read file block %ld", fip -> flba));
    if (!data) {
	rgrp ();
    }
    DUMP_BLK (&blocks[lbi], (LBA) (gsba + lbi));
    if (fip == NULL) {
	bru_message (MSG_BUG, "ar_read");
    } else {
	if (!chksum_ok (&blocks[lbi])) {
	    fip -> fi_flags |= FI_CHKSUM;
	    einfo.chkerr++;
	    fip -> chkerrs++;
	    fip -> flba++;
	} else {
	    DBUG_PRINT ("alba", ("archive blk %ld", FROMHEX (blocks[lbi].HD.h_alba)));
	    bfound = FROMHEX (blocks[lbi].HD.h_flba);
	    DBUG_PRINT ("lba", ("file blk %ld", bfound));
	    if (fip -> flba != bfound) {
		DBUG_PRINT ("bseq", ("wrong file block (%ld) found", bfound));
		fip -> flba = bfound + 1;
		fip -> fi_flags |= FI_BSEQ;
	    } else {
		fip -> flba++;
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	ar_write   logical write of current logical block
 *
 *  SYNOPSIS
 *
 *	VOID ar_write (fip, magic)
 *	struct finfo *fip;
 *	int magic;
 *
 *  DESCRIPTION
 *
 *	Performs logical write of current logical block.
 *	This is also where several things in the header
 *	get initialized.
 *
 *	Note that in particular, the current volume number and
 *	block checksum ARE NOT computed at this time.  This operation
 *	is delayed until just before the write to the archive, because
 *	the buffer may have to be written to a different volume than
 *	the current volume.  When it comes time to write out the buffer,
 *	all the blocks are patched with the volume number at the time
 *	of the write, and their checksums are computed after the volume
 *	number patch, just before the write.
 *
 */

VOID ar_write (fip, magic)
struct finfo *fip;
int magic;
{
    DBUG_ENTER ("ar_write");
    data = TRUE;
    dirty = TRUE;
    (VOID) s_strcpy (blocks[lbi].HD.h_name, fip -> bname1);
    TOHEX (blocks[lbi].HD.h_alba, gsba + lbi);
    TOHEX (blocks[lbi].HD.h_flba, fip -> flba);
    TOHEX (blocks[lbi].HD.h_time, info.bru_time);
    TOHEX (blocks[lbi].HD.h_bufsize, bufsize);
    TOHEX (blocks[lbi].HD.h_msize, msize);
    TOHEX (blocks[lbi].HD.h_magic, magic);
    TOHEX (blocks[lbi].HD.h_release, release);
    TOHEX (blocks[lbi].HD.h_level, level);
    TOHEX (blocks[lbi].HD.h_variant, variant);
    TOHEX (blocks[lbi].HD.h_fseq, fip -> fseq);
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	ar_close   close the virtual disk file
 *
 *  SYNOPSIS
 *
 *	VOID ar_close()
 *
 *  DESCRIPTION
 *
 *	If the virtual disk file is open, then closes it.
 *
 *	If the device is unknown, assume that closing it "rewinds" it.
 *	If the device is known and the D_NOREWIND flag is NOT set then
 *	closing it rewinds it.  Setting the pba to 0 is sufficient
 *	(I believe) but initializing the rest of the buffer control
 *	variables doesn't hurt.  This code is starting to get really
 *	twisted!!  Better safe than sorry...
 *
 */

VOID ar_close ()
{
    DBUG_ENTER ("ar_close");
    ar_sync();
#if HAVE_SHM
    (VOID) dbl_flush ();
    volume = dbl_getvol ();
    eject_media ();
    if (trans_dbl != DBL_DOBUF) {
	af_close ();
    }
#else
    eject_media ();
    af_close ();
#endif
    lastvolume = volume;
    DBUG_PRINT ("lastvol", ("lastvolume = %d", lastvolume));
    if ((ardp == NULL) || (!(ardp -> dv_flags & D_NOREWIND))) {
	pba = 0;
	gsba = 0;
	DBUG_PRINT ("gsba", ("group start block addr %ld", gsba));
	vsba = 0;
	DBUG_PRINT ("vsba", ("new volume start block addr %ld", vsba));
	grp = 0;
	DBUG_PRINT ("grp", ("block group %ld", grp));
	volume = 0;
#if HAVE_SHM
	dbl_setvol (volume);
#endif
	lbi = -1;				/* CAUTION ! */
	dirty = FALSE;
	data = FALSE;
    }
    DBUG_PRINT ("ar_io", ("virtual disk file closed"));
    DBUG_VOID_RETURN;
}



/*
 *  FUNCTION
 *
 *	rgrp    fill blocks buffer from archive stream
 *
 *  SYNOPSIS
 *
 *	rgrp ()
 *
 *  DESCRIPTION
 *
 *	Fills the blocks buffer from the archive stream at the
 *	current read/write point.
 *
 *	Also note that if the input is being read from a pipe
 *	then there is no guarantee that the desired number of
 *	bytes will be available in the pipe.  Only the number
 *	of bytes available will be returned.  Thus we continue
 *	to request additional bytes until the buffer is full or
 *	a read error occurs.
 */

static VOID rgrp ()
{
    DBUG_ENTER ("rgrp");
    if (pipe_io) {
	rpipe ();
    } else {
	rfile ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	rpipe    file archive buffer from a pipe
 *
 *  SYNOPSIS
 *
 *	static VOID rpipe ()
 *
 *  DESCRIPTION
 *
 *	Used to fill archive buffer from a pipe.  Since pipe
 *	I/O does not block until specified number of bytes are
 *	ready, but rather returns the bytes immediately available,
 *	we simply hang in a loop until the buffer is full or no
 *	bytes are returned (signifying end of file).
 *
 *	If the buffer only gets partially filled (such as when
 *	the blocking factor of the writer is different than the
 *	buffer size, no error occurs as long as the last block
 *	placed in the buffer is the end of archive marker.  This
 *	is because rpipe will never be called again since the
 *	upper level routines will find the end of the archive and
 *	things will terminate normally.  If however, the end of
 *	archive is not found in the data read during a partial
 *	buffer fill, the upper level routines will note that the
 *	remaining (unfilled) data space contains bogus data, another
 *	call to rpipe will eventually occur, it will get no data at
 *	all, a read error error will be issued, and bru will terminate
 *	via a direct call to done.
 *
 */

static VOID rpipe ()
{
    char *destination;
    int request;
    int iobytes;

    DBUG_ENTER ("rpipe");
    destination = blocks[0].bytes;
    request = bufsize;
    DBUG_PRINT ("rpipe", ("get %u bytes at pba %ld", request, pba));
    do {
	iobytes = s_read (afile.fildes, destination, (UINT) request);
	request -= iobytes;
	destination += iobytes;
	DBUG_PRINT ("pipe_in", ("got %d bytes from pipe", iobytes));
    } while (iobytes > 0 && request > 0);
    einfo.breads += BLOCKS (bufsize - request);
    pba += BLOCKS (bufsize - request);
    if (request == bufsize) {
	bru_message (MSG_ARREAD, pba);
	einfo.rhard++;
	done ();
    }
    data = TRUE;
    dirty = FALSE;
    if (bswap || sswap) {
	do_swap ();
    }
    if (first) {
	check_vol ();
	first = FALSE;
    }
    DBUG_VOID_RETURN;
}


/*
 *	Note that we only check for the proper volume if the
 *	read gets at least one block of actual data.
 *
 *	One problem here with error recovery.  When the retry limit
 *	is reached, we attempt to continue by just ignoring the problem
 *	and going for a new block.  Since the file pointer still points
 *	to the current bad spot, we want to be able to skip over it by
 *	doing a phys_seek to the new location.  This works fine with seekable
 *	devices, like floppies, but for non-seekable devices phys_seek 
 *	attempts to reposition the file pointer by reading to the new
 *	block, which won't work because that's how we got into this mess
 *	in the first place.  The bottom line is, if the device is not
 *	seekable, we haven't gotten past the bad spot by retries, and
 *	read/writes do not advance the media (D_ADVANCE in brutab), we
 *	are probably stuck there forever and should just give up rather
 *	than hanging in an infinite loop.
 */
 
static VOID rfile ()
{
    int iobytes;

    DBUG_ENTER ("rfile");
    tries = 0;
    iobytes = 0;
    while (iobytes != bufsize) {
	DBUG_PRINT ("rfile", ("read %u bytes at pba %ld", bufsize, pba));
	iobytes = READ (afile.fildes, blocks[0].bytes, bufsize);
	tries++;
	DBUG_PRINT ("tries", ("tries = %d", tries));
	einfo.breads += BLOCKS (bufsize);
	if (iobytes == bufsize) {
	    pba += BLOCKS (bufsize);
	} else {
	    DBUG_PRINT ("errno", ("errno = %d", errno));
	    einfo.rsoft++;
	    if (!recover (iobytes, errno, TRUE)) {
		break;
	    }
	}
    }
    data = TRUE;
    dirty = FALSE;
    if (bswap || sswap) {
	do_swap ();
    }
    if (first) {
	check_vol ();
	first = FALSE;
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	wgrp    write blocks buffer to archive stream
 *
 *  SYNOPSIS
 *
 *	static VOID wgrp ()
 *
 *  DESCRIPTION
 *
 *	Writes the blocks buffer to the archive stream at the
 *	current read/write point.
 *
 *	If the output is going to a pipe, there is a limit on how
 *	much data can be written in a single write command.  Thus
 *	there is a separate routine used for writing to pipes that
 *	continues to write until the buffer is exhausted or a write
 *	error occurs.
 *
 */

static VOID wgrp ()
{
    DBUG_ENTER ("wgrp");
    if (pipe_io) {
	wpipe ();
    } else {
	wfile ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	wpipe    write buffer to a pipe
 *
 *  SYNOPSIS
 *
 *	static VOID wpipe ()
 *
 *  DESCRIPTION
 *
 *	Used to write the archive buffer out to a pipe.  Since there
 *	is a limit on how many bytes can be written to a pipe, we
 *	write out the buffer one archive block at a time.
 *
 *	Note that we specifically always write out a full buffer worth
 *	of data, even when the last buffer written contains only a few
 *	new blocks.  Although we know the last block used (in lbi), we
 *	write out the full buffer because this is the same behavior
 *	that occurs when the output is stream is not a pipe.  Thus
 *
 *		bru -cf - >/usr/me/myfile
 *		bru -cf - | dd of=/usr/me/myfile
 *		bru -cf /usr/me/myfile
 *
 *	will all produce output files of the same size (exact multiple
 *	of bufsize).  If this was not true, a subsequent read of the archive
 *	file would have a problem with the last buffer since it would be
 *	only a partial buffer.  Naturally, this scheme results in some
 *	left over blocks from the last write being used to pad out the
 *	buffer to a full bufsize bytes.  These normally cause no problem
 *	except in the very rare case where the terminator block is somehow
 *	missed during a read.
 *
 */

static VOID wpipe ()
{
    int index;
    int request;
    int iobytes;
    int bufblocks;
    char *source;

    DBUG_ENTER ("wpipe");
    patch_buffer (blocks, BLOCKS (bufsize));
    bufblocks = BLOCKS (bufsize);
    DBUG_PRINT ("wpipe", ("write out %d blocks from buffer", bufblocks));
    for (index = 0; index < bufblocks; index++) {
	source = blocks[index].bytes;
	request = BLKSIZE;
	DBUG_PRINT ("pipe_out", ("write %u bytes from block %d", request, index));
	iobytes = s_write (afile.fildes, source, (UINT) request);
	if (iobytes != request) {
	    bru_message (MSG_ARWRITE, pba);
	    einfo.whard++;
	    done ();
	}
	pba++;
	einfo.bwrites++;
    }
    data = TRUE;
    dirty = FALSE;
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	wfile    write buffer to a non-pipe output file
 *
 *  SYNOPSIS
 *
 *	wfile ();
 *
 *  DESCRIPTION
 *
 *	Used to write output to a non-pipe archive.  Writes are done
 *	in BUFSIZE increments, with errors being retried up to the
 *	specified limit.
 *
 *	For complications due to error recovery algorthms, see comments
 *	in description of analogous function rfile() for reads.
 *
 */

static VOID wfile ()
{
    int iobytes;

    DBUG_ENTER ("wfile");
    tries = 0;
    iobytes = 0;
    while (iobytes != bufsize) {
	patch_buffer (blocks, BLOCKS (bufsize));
	DBUG_PRINT ("wfile", ("write %u bytes at pba %ld", bufsize, pba));
	iobytes = WRITE (afile.fildes, blocks[0].bytes, bufsize);
	tries++;
	DBUG_PRINT ("tries", ("tries = %d", tries));
	einfo.bwrites += BLOCKS (bufsize);
	if (iobytes == bufsize) {
	    pba += BLOCKS (bufsize);
	    dirty = FALSE;
	} else {
	    DBUG_PRINT ("errno", ("errno = %d", errno));
#if HAVE_SHM
	    if (trans_dbl == DBL_NOBUF) {
		einfo.wsoft++;
		if (!recover (iobytes, errno, FALSE)) {
		    break;
		}
	    }
#else
	    einfo.wsoft++;
	    if (!recover (iobytes, errno, FALSE)) {
		break;
	    }
#endif
	}
    }
    first = FALSE;
    if (bswap || sswap) {
	do_swap ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	ar_sync   write out sector buffer if "dirty"
 *
 *  SYNOPSIS
 *
 *	static VOID ar_sync()
 *
 *  DESCRIPTION
 *
 *	Writes the contents of block buffer if necessary.
 *
 */

static VOID ar_sync()
{

    DBUG_ENTER("ar_sync");
    if (dirty) {
	DBUG_PRINT ("ar_io", ("flushing dirty buffer"));
	wgrp ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	allocate    allocate space for the block buffer
 *
 *  SYNOPSIS
 *
 *	static VOID allocate ()
 *
 *  DESCRIPTION
 *
 *	This function is called when the archive file is first
 *	opened, to allocate space for the block buffers.
 *
 *	Any error is immediately fatal.
 *
 */

static VOID allocate ()
{
    DBUG_ENTER ("allocate");
    if (blocks == NULL) {
	blocks = (union blk *) get_memory (bufsize, FALSE);
	if (blocks == NULL) {
	    bru_message (MSG_BALLOC, bufsize/1024);
	    done ();
	}
    }
    DBUG_VOID_RETURN;
}


VOID deallocate ()
{
    DBUG_ENTER ("deallocate");
    if (blocks != NULL) {
	s_free ((char *) blocks);
	blocks = NULL;
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	ar_tell    return current logical block address
 *
 *  SYNOPSIS
 *
 *	LBA ar_tell ()
 *
 *  DESCRIPTION
 *
 *	Returns logical block address of current block in archive.
 *	The current block is the block which was the last argument of
 *	an lba_seek command or -1 by default.
 *
 *  NOTES
 *
 *	Does not use DBUG_ENTER, LEAVE, or DBUG_n macros because this
 *	function is called in the middle of printing some verbose
 *	output.
 *
 */

LBA ar_tell ()
{
    LBA rtnval;

    if (mode == 'e') {
	rtnval = lba_estimate;
    } else {
	rtnval = gsba + lbi;
    }
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	ar_next    locate next logical block in archive
 *
 *  SYNOPSIS
 *
 *	union *blk ar_next ()
 *
 *  DESCRIPTION
 *
 *	Locates next logical block in archive and returns pointer
 *	to usuable data buffer.  Similar to lba_seek except that
 *	the logical block number defaults to the current logical
 *	block number + 1.
 *
 *	Note that no I/O is implied by this operation, it simply
 *	locates a buffer and sets up a new current logical block
 *	number.  The ar_read function must still be called if
 *	the buffer is expected to contain valid data.
 *
 */

union blk *ar_next ()
{
    union blk *new;

    DBUG_ENTER ("ar_next");
    new = ar_seek (1L, 1);
    DBUG_RETURN (new);
}


/*
 *  NOTES
 *
 *	It is assumed that switching media will cause the
 *	next read to start at physical block 0.  This may
 *	not always be true if the archive file is not closed.
 *	One would expect it to be true for magnetic tapes but
 *	not necessarily random access devices like removable
 *	disks.  Thus it may be necessary to use the D_REOPEN
 *	flag in brutab for the specific device, to force this
 *	condition.  In some systems, such as the Convergent
 *	Miniframe, removing a floppy without closing and
 *	reopening the file (by D_REOPEN) will cause a read
 *	error anyway.
 *
 *	It is now the default to close the archive device file
 *	and reopen it on a media switch.  The reason for this is
 *	that for some devices, a reopen is a must, otherwise we
 *	will never be able to access the device again.  If this
 *	situation occurs for a device not in the brutab file, bru
 *	will simply go into an infinite prompting loop, asking for
 *	the next volume.  Thus the D_REOPEN flag is now obsolete.
 *	The D_NOREOPEN flag is used to suppress the close and reopen
 *	for known devices that can continue without the reopen.
 *
 *	If the user did not specify an explicit buffer size, and the
 *	new media's entry in the brutab file does specify a default
 *	buffer size that is not identical to the current buffer size,
 *	a warning message is printed.  The buffer size MUST remain
 *	constant across all volumes of a multiple volume archive.
 *
 */

VOID switch_media (nvolume)
int nvolume;
{
    BOOLEAN new;
    UINT newbufsize;
    BOOLEAN reopen = FALSE;
    
    DBUG_ENTER ("switch_media");
    eject_media ();
    if (flags.fflag > 1 || ardp == NULL || !(ardp -> dv_flags & D_NOREOPEN)) {
	DBUG_PRINT ("reopen", ("close archive prior to media switch"));
	af_close ();
	reopen = TRUE;
    }
    DBUG_PRINT ("vol", ("new volume = %d", nvolume));
    new = new_arfile (nvolume);
    if (new || reopen) {
	if (ardp != NULL && !flags.bflag) {
	    newbufsize = BLOCKS (ardp -> dv_bsize) * BLKSIZE;
	    if (newbufsize != bufsize && !forcebuffer ()) {
		bru_message (MSG_BSZCHG, bufsize, newbufsize);
	    }
	}
	if (ardp != NULL && ardp -> dv_maxbsize != 0) {
	    if (bufsize > ardp -> dv_maxbsize) {
		bru_message (MSG_MAXBUFSZ, bufsize / 1024,
			   ardp -> dv_maxbsize / 1024);
		if (!flags.bflag) {
		    newbufsize = BLOCKS (ardp -> dv_maxbsize) * BLKSIZE;
		    if (newbufsize != bufsize && !forcebuffer ()) {
			bru_message (MSG_BSZCHG, bufsize, newbufsize);
		    }
		}
	    }
	}
	DBUG_PRINT ("bufsize", ("buffer size %uk bytes", (UINT) bufsize/1024));
	af_close ();
	if (!do_open ()) {
	    switch_media (nvolume);
	}
    }
    if (new) {
	qfwrite ();
    }
    volume = nvolume;
#if HAVE_SHM
    dbl_setvol (volume);
#endif
    pba = 0;
    first = TRUE;
    checkswap = TRUE;
    sswap = FALSE;
    bswap = FALSE;
    DBUG_PRINT ("swap", ("checkswap = first = TRUE, sswap = bswap = FALSE"));
    if (reading) {
	data = FALSE;
    }
    if (msize != 0) {
	vsba = volume * BLOCKS (msize);
	DBUG_PRINT ("vsba", ("new volume start block addr %ld", vsba));
    } else {
	vsba = gsba;
	DBUG_PRINT ("vsba", ("new volume start block addr %ld", vsba));
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	r_phys_seek    recovery phys_seek with recursion detection
 *
 *  SYNOPSIS
 *
 *	static VOID r_phys_seek (npba)
 *	LBA npba;
 *
 *  DESCRIPTION
 *
 *	During error recovery, we may want to call phys_seek to seek
 *	to a specific physical location on the media.  For nonseekable
 *	devices, this may fail because we may attempt to seek to a given
 *	location by reading, which can itself get another error, thus
 *	getting into a recursive situation.  This routine simply
 *	increments a recursion counter for each call to the real
 *	phys_seek, and decrements it at each return.  Calls to phys_seek
 *	which never return cause the counter to increment and when
 *	it reaches the preset limit, this volume is abandoned.
 *
 */

static VOID r_phys_seek (npba)
LBA npba;
{
    DBUG_ENTER ("r_phys_seek");
    DBUG_PRINT ("recursions", ("recursions = %d", recursions));
    if (recursions < D_RECUR) {
	recursions++;
	phys_seek (npba);
	recursions--;
    } else {
	recursions = 0;
	if (firsterrno > 0) {
	    errno = firsterrno;
	}
	bru_message (MSG_PEOV, volume + 1);
	infer_end ();
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	recover    recover from a read/write error
 *
 *  SYNOPSIS
 *
 *	static BOOLEAN recover (iobytes, ioerr, readflag)
 *	int iobytes;
 *	int ioerr;
 *	BOOLEAN readflag;
 *
 *  DESCRIPTION
 *
 *	When a read or write error on the archive media is detected,
 *	this routine gains control and attempts to set things up
 *	so that the next read or write will be successful.
 *	The recovery algorithms are complicated by the fact that
 *	the behavour due to various errors (such as insertion of
 *	unformatted media, end of device, media I/O errors, etc)
 *	is very implementation and device dependent.
 *
 *	The basic strategy is to switch to next media if the
 *	error was definitely due to an attempt to do I/O past
 *	the physical bounds, otherwise do at least one retry.
 *	Then attempt to infer either the end of the device or
 *	insertion of unformatted media.  Finally, simply retry
 *	the I/O up to the retry limit specified in the configuration
 *	file.
 *
 *	Because the error recovery algorithms can cause the original
 *	errno, from the original error, to be destroyed, we save
 *	errno for the first error, then restore it if all attempts
 *	to recover fail and we need to report an error.  This prevents
 *	perror, or its equivalent, from reporting mysterious reasons for
 *	failures.
 *
 */

static BOOLEAN recover (iobytes, ioerr, readflag)
int iobytes;
int ioerr;
BOOLEAN readflag;
{
    LBA npba;
    int nvolume;
    BOOLEAN unfmt;
    BOOLEAN wprot;
    BOOLEAN poed;		/* Possible end of device */
    BOOLEAN koed;		/* Known end of device */
    BOOLEAN rawtape;
    LBA errorpba;
    BOOLEAN recoverable;
    BOOLEAN advanced;

    DBUG_ENTER ("recover");
    DBUG_PRINT ("fault", ("io of %d bytes", iobytes));
    DBUG_PRINT ("fault", ("ioerr %d", ioerr));
    DBUG_PRINT ("tries", ("tries = %d", tries));
    if (tries == 1 && ioerr > 0) {
	firsterrno = ioerr;
	DBUG_PRINT ("errno", ("errno from original error = %d", firsterrno));
    }
    errorpba = pba;
    advanced = (ardp != NULL && ardp -> dv_flags & D_ADVANCE);
    if (advanced) {
	pba += BLOCKS (bufsize);
    }
    DBUG_PRINT ("fault", ("error pba %ld, current pba %ld", errorpba, pba));
    DBUG_PRINT ("fault", ("gsba %ld grp %ld", gsba, grp));
    DBUG_PRINT ("fault", ("vsba %ld", vsba));
    s_sleep ((UINT) 3);
    unfmt = unformatted (iobytes, ioerr, readflag);
    wprot = write_protect (iobytes, ioerr, readflag);
    poed = possible_end (iobytes, ioerr, readflag);
    koed = known_end (iobytes, ioerr);
    rawtape = raw_tape ();
    recoverable = TRUE;
    if (msize != 0 && errorpba >= BLOCKS (msize)) {
	DBUG_PRINT ("fault", ("found end of vol %d (known size)", volume + 1));
	nvolume = (int) (volume + (errorpba / BLOCKS (msize)));
	switch_media (nvolume);
	npba = gsba - vsba;
	DBUG_PRINT ("npba", ("new phys block addr %ld", npba));
	r_phys_seek (npba);
    } else if (ardp != NULL && !first && koed) {
	DBUG_PRINT ("fault", ("found known end of device"));
	if (msize != 0) {
	    msize = KBYTES (errorpba);
	    bru_message (MSG_ADJMSIZE, msize);
	    msize *= 1024;
	}
	infer_end ();
    } else if (ardp != NULL && !first && poed && msize == 0) {
	DBUG_PRINT ("fault", ("end of known device of unknown size"));
	infer_end ();
    } else if (ardp != NULL && first && unfmt && !wprot) {
	DBUG_PRINT ("fault", ("first io on unformatted known device"));
	if (firsterrno > 0) {
	    errno = firsterrno;
	}	
	if (!(ardp -> dv_flags & D_FORMAT) || !(s_format (afile.fildes))) {
	    bru_message (MSG_UNFMT);
	    switch_media (volume);
	}
	r_phys_seek (errorpba);
    } else if (ardp != NULL && first && !unfmt && wprot && !readflag) {
	DBUG_PRINT ("fault", ("first write to write protected known device"));
	if (firsterrno > 0) {
	    errno = firsterrno;
	}	
	bru_message (MSG_WRTPROT);
	switch_media (volume);
	r_phys_seek (errorpba);
    } else if (ardp != NULL && first && iobytes > 0 && rawtape && readflag) {
	DBUG_PRINT ("fault", ("read of known raw tape with other blocking factor"));
#if HAVE_SHM
	if (trans_dbl == DBL_NOBUF) {
	    new_bufsize (iobytes);
	    r_phys_seek (errorpba);
	}
#else
	new_bufsize (iobytes);
	r_phys_seek (errorpba);
#endif
    } else if (ardp != NULL && first && unfmt && wprot) {
	DBUG_PRINT ("fault", ("first io to unformatted or write protected device"));
	if (firsterrno > 0) {
	    errno = firsterrno;
	}	
	bru_message (MSG_UFORWP);
	switch_media (volume);
	r_phys_seek (errorpba);
    } else if (!seekflag && advanced) {
	DBUG_PRINT ("fault", ("advanced on media, and can't back up"));
	if (msize == 0) {
	    DBUG_PRINT ("fault", ("media size unknown, infer end"));
	    if (firsterrno > 0) {
		errno = firsterrno;
	    }	
	    bru_message (MSG_AEOV, volume + 1);
	    infer_end ();
	} else {
	    DBUG_PRINT ("fault", ("not at end, so unrecoverable"));
	    DBUG_PRINT ("fault", ("assume advanced over bad spot"));
	    recoverable = FALSE;
	    harderror (errorpba, readflag, iobytes);
	}
    } else if (tries >= D_TRIES) {
	DBUG_PRINT ("fault", ("retry limit (%d) hit", D_TRIES));
	if (first) {
	    DBUG_PRINT ("fault", ("first read/write of media, just reload"));
	    if (firsterrno > 0) {
		errno = firsterrno;
	    }	
	    bru_message (MSG_FBIOERR);
	    switch_media (volume);
	    r_phys_seek (errorpba);
	} else if (msize == 0) {
	    DBUG_PRINT ("fault", ("not first block, size unknown, infer end"));
	    if (firsterrno > 0) {
		errno = firsterrno;
	    }	
	    bru_message (MSG_AEOV, volume + 1);
	    infer_end ();
	} else {
	    DBUG_PRINT ("fault", ("not first block, media size known"));
	    recoverable = FALSE;
	    harderror (errorpba, readflag, iobytes);
	    if (seekflag) {
		DBUG_PRINT ("fault", ("seek past bad spot"));
		r_phys_seek ((LBA) (errorpba + BLOCKS (bufsize)));
	    } else {
		recursions = 0;
		if (firsterrno > 0) {
		    errno = firsterrno;
		}	
		bru_message (MSG_PEOV, volume + 1);
		infer_end ();
	    }
	}
    } else {
	DBUG_PRINT ("fault", ("retry %d at %ld", tries, errorpba));
	r_phys_seek (errorpba);
    }
    DBUG_PRINT ("fault", ("volume %d  vsba %ld", volume, vsba));
    DBUG_PRINT ("fault", ("gsba %ld  pba %ld", gsba, pba));
    DBUG_RETURN (recoverable);
}


/*
 *	Note that we force nonseekable operation if double buffering
 *	is requested.  This is because the double buffering code does
 *	not support seeking.  The parent process ends up trying to
 *	do the seek, AFTER it has closed the file descriptor, while
 *	it is the child process that is actually handling the read/write
 *	of the archive device.
 *
 *	If double buffering setup fails, then fall back to normal 
 *	buffering.  Note we have to call dbl_setup again, to get
 *	transparent mode.  This second call can't fail unless there is
 *	a serious bug in the code, or it changes radically.
 *
 */

static BOOLEAN do_open ()
{
    int open_flag;
    int open_mode;
    BOOLEAN rtnval;
    BOOLEAN accessible;
    BOOLEAN exists;
#if HAVE_SHM
    long shmmax;
    long shmall;
    UINT shmseg;
    int direction;
#endif

    DBUG_ENTER ("do_open");
#if HAVE_SHM
    if (!flags.Dflag || pipe_io || seekflag) {
	trans_dbl = DBL_NOBUF;
    } else {
	trans_dbl = DBL_DOBUF;
    }
    dbl_errp (inerr_recover, outerr_recover);
    if (ardp != NULL) {
	shmmax = ardp -> dv_shmmax;
	shmseg = ardp -> dv_shmseg;
	shmall = ardp -> dv_shmall;
    } else {
	shmmax = B_SHMMAX;
	shmseg = B_SHMSEG;
	shmall = B_SHMALL;
    }
    dbl_parms (shmseg, shmmax, shmall, (long) bufsize);
    dbl_iop (s_read, s_write);
    if (mode == 'c') {
	direction = DBL_WRITING;
    } else {
	direction = DBL_READING;
    }
#endif
    if (mode == 'c') {
	reading = FALSE;
    } else {
	reading = TRUE;
    }
    if (STRSAME (afile.fname, "-")) {
	open_std ();
	rtnval = TRUE;
    } else {
	rtnval = FALSE;
	exists = file_access (afile.fname, A_EXISTS, FALSE);
	if (mode == 'c') {
	    open_flag = O_WRONLY | O_CREAT;
	    open_mode = 0666;
	    accessible = FALSE;
	    if (exists) {
		if (file_access (afile.fname, A_WRITE, TRUE)) {
		    accessible = TRUE;
		}
	    } else {
		if (dir_access (afile.fname, A_WRITE, TRUE)) {
		    accessible = TRUE;
		}
	    }
	} else {
	    open_flag = O_RDONLY;
	    open_mode = 0;
	    accessible = FALSE;
	    if (exists) {
		if (file_access (afile.fname, A_READ, TRUE)) {
		    accessible = TRUE;
		}
	    }
	}
	if (!accessible) {
	    rtnval = FALSE;
	    bru_message (MSG_AROPEN, afile.fname);
	} else {
	    afile.fildes = s_open (afile.fname, open_flag, open_mode);
	    if (afile.fildes < 0) {
		rtnval = FALSE;
		eject_media ();
		bru_message (MSG_AROPEN, afile.fname);
	    } else {
		rtnval = TRUE;
		if (flags.pflag || flags.Dflag) {
		    seekflag = FALSE;
		} else {
		    seekflag = seekable (afile.fname, BLKSIZE);
		} 
		if (!exists) {
		    file_chown (afile.fname, info.bru_uid, info.bru_gid);
		}
#if HAVE_SHM
		if (!wdblinit) {
		    if (!reading && trans_dbl == DBL_DOBUF) {
			wdblinit++;
		    }
		    if (dbl_setup (trans_dbl, direction)) {
			bru_message (MSG_DBLSUP);
			trans_dbl = DBL_NOBUF;
			flags.Dflag = FALSE;
			(VOID) dbl_setup (trans_dbl, direction);
		    }
		    dbl_setvol (volume);
		}
#endif
	    }
	}
    }
    DBUG_PRINT ("arf", ("%s open on %d", afile.fname, afile.fildes));
    DBUG_RETURN (rtnval);
}


VOID do_swap ()
{
    int maxindex;
    int index;

    DBUG_ENTER ("do_swap");
    maxindex = BLOCKS (bufsize);
    if (bswap) {
	for (index = 0; index < maxindex; index++) {
	    DBUG_PRINT ("swap", ("swap bytes for block %ld", (long) (gsba+index)));
	    swapbytes (&blocks[index]);
	}
    }
    if (sswap) {
	for (index = 0; index < maxindex; index++) {
	    DBUG_PRINT ("swap", ("swap shorts for block %ld", (long) (gsba+index)));
	    swapshorts (&blocks[index]);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *	Check to see if each buffer needs to have any byte or
 *	short swapping done.  The checkswap flag insures that this
 *	is only done once per media, even when things are called
 *	recursively.  Otherwise it is possible to get confused over
 *	whether or not swapping is necessary.
 */
 
BOOLEAN need_swap (blkp)
union blk *blkp;
{
    DBUG_ENTER ("need_swap");
    DBUG_PRINT ("swap", ("checkswap=%d", checkswap));
    DBUG_PRINT ("swap", ("sswap=%d bswap=%d", sswap, bswap));
    if (checkswap) {
	checkswap = FALSE;
	bswap = FALSE;
	sswap = FALSE;
	swapbytes (blkp);
	DUMP_BLK (blkp, 0L);
	if (chksum_ok (blkp) && magic_ok (blkp)) {
	    DBUG_PRINT ("swap", ("swap bytes"));
	    bswap = TRUE;
	}
	swapshorts (blkp);
	DUMP_BLK (blkp, 0L);
	if (chksum_ok (blkp) && magic_ok (blkp)) {
	    DBUG_PRINT ("swap", ("swap bytes and shorts"));
	    sswap = TRUE;
	}
	swapbytes (blkp);
	DUMP_BLK (blkp, 0L);
	if (chksum_ok (blkp) && magic_ok (blkp)) {
	    DBUG_PRINT ("swap", ("swap shorts"));
	    sswap = TRUE;
	    bswap = FALSE;
	}
	swapshorts (blkp);
	DUMP_BLK (blkp, 0L);
    }
    DBUG_PRINT ("swap", ("checkswap=%d", checkswap));
    DBUG_PRINT ("swap", ("sswap=%d bswap=%d", sswap, bswap));
    DBUG_RETURN (sswap || bswap);
}


/*
 *  After switching volumes, reset the group start block address
 *  and the group number to force starting with the first block.
 *  This applies even when we know the size of the volume and are
 *  giving up on the current volume because of a bad spot that we
 *  cannot get past.  In fact, if we don't do this, the next group
 *  will trigger a seek back to the old volume, that we gave up on.
 */

static VOID infer_end ()
{
    DBUG_ENTER ("infer_end");
    DBUG_PRINT ("fault", ("inferred end of volume %d", volume + 1));
    switch_media (volume + 1);
    gsba = vsba;
    DBUG_PRINT ("gsba", ("group start block addr %ld", gsba));
    grp = gsba / BLOCKS (bufsize);
    DBUG_PRINT ("grp", ("block group %ld", grp));
    phys_seek (0L);
    tries = 0;
    DBUG_PRINT ("tries", ("tries = %d", tries));
    recursions = 0;
    DBUG_VOID_RETURN;
}


/*
 *	Note that we must check for a proper checksum because it
 *	is not guaranteed at this point that the buffer data is
 *	good (a bad read for example).  We don't want to be
 *	generating messages to load volume 67,892 if the first
 *	block of a media is unreadable for some reason!
 *
 *	Also note that since this is at a lower level than readinfo,
 *	we also need to insure that the swap flags are up to date
 *	by calling need_swap().
 *
 */
 
static VOID check_vol ()
{
    int vol;

    DBUG_ENTER ("check_vol");
    if (need_swap (&blocks[0])) {
	do_swap ();
    }
    if (chksum_ok (&blocks[0])) {
	vol = (int) FROMHEX (blocks[0].HD.h_vol);
	if (vol != volume) {
	    bru_message (MSG_WRONGVOL, vol + 1, volume + 1);
	    bad_vol (vol);
	}
	if (artime != (time_t)0 && artime != FROMHEX (blocks[0].HD.h_time)) {
	    bru_message (MSG_WRONGTIME, s_ctime ((long *) &artime));
	    bad_vol (vol);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *	Note that for the reload case we must save the current pba, since
 *	switch_media() currently forces it back to zero.  We need to be
 *	able to seek to the same relative point in the new volume.
 */
 
static VOID bad_vol (vol)
int vol;
{
    char key;
    LBA mblocks;
    LBA oldpba;

    DBUG_ENTER ("bad_vol");
    key = response ("c => continue  q => quit  r => reload  s => shell",
   	'q');
    switch (key) {
	case 'q':
	case 'Q':
	    done ();
	    break;
	case 'r':
	case 'R':
	    oldpba = pba;
	    switch_media (volume);
	    phys_seek ((LBA) (oldpba - BLOCKS (bufsize)));
	    rgrp ();
	    break;
	case 'c':
	case 'C':
	    readsizes (&blocks[0]);
	    mblocks = BLOCKS (msize);
	    gsba += (vol - volume) * mblocks;
	    DBUG_PRINT ("gsba", ("group start block addr %ld", gsba));
	    vsba = vol * mblocks;
	    DBUG_PRINT ("vsba", ("new volume start block addr %ld", vsba));
	    grp = gsba / BLOCKS (bufsize);
	    DBUG_PRINT ("grp", ("block group %ld", grp));
	    volume = vol;
#if HAVE_SHM
	    dbl_setvol (volume);
#endif
	    DBUG_PRINT ("cont", ("grp %ld gsba %ld", grp, gsba));
	    DBUG_PRINT ("cont", ("volume %d vsba %ld", volume, vsba));
	    break;
	case 'S':
	case 's':
	    fork_shell ();
	    bad_vol (vol);
	    break;
    }
    DBUG_VOID_RETURN;
}


BOOLEAN ar_ispipe ()
{
    return (pipe_io);
}


/*
 *  FUNCTION
 *
 *	new_arfile    issue prompt for new media and save file name
 *
 *  SYNOPSIS
 *
 *	BOOLEAN new_arfile (vol)
 *	int vol;
 *
 *  DESCRIPTION
 *
 *	Issue prompt for new volume and remember name if specified.
 *	Note that the new name, if any, overwrites the old name.
 *	Returns TRUE if new name was given, false otherwise.
 *
 */
 
BOOLEAN new_arfile (vol)
int vol;
{
    BOOLEAN rtnval;
    char newname[BRUPATHMAX];
    SIGTYPE prevINT;
    SIGTYPE prevQUIT;
    struct device *tmpardp;
    
    DBUG_ENTER ("new_arfile");
    DBUG_PRINT ("vol", ("new volume = %d", vol));
    rtnval = FALSE;
    sig_push (&prevINT, &prevQUIT);
    sig_done ();
#if amigados
    FlushRawFloppy ();
    MotorOff ();
#endif
    for (;;) {
	tty_newmedia (vol + 1, afile.fname, newname, sizeof (newname));
	if (newname[0] == EOS) {
	    break;
	} else {
	    if (namesane (newname)) {
		if ((tmpardp = get_ardp (newname)) != NULL) {
		    break;
		} else {
		    if (nameconfirm (newname)) {
			break;
		    }
		}
	    }
	}
    }
    sig_pop (&prevINT, &prevQUIT);
    if (newname[0] != EOS) {
	rtnval = TRUE;
	copyname (afile.fname, newname);
	ardp = tmpardp;
    }
    DBUG_RETURN (rtnval);
}


/*
 *  FUNCTION
 *
 *	ar_vol    return number of current volume
 *
 *  SYNOPSIS
 *
 *	int ar_vol ()
 *
 *  DESCRIPTION
 *
 *	Returns the index of the current volume [0,1,...].
 *	Note that if current mode is media estimation mode
 *	then the value returned is the estimated volume number
 *	since there is not really an archive open.
 *
 *  NOTES
 *
 *	This function does not contain DBUG package macros because
 *	it is called in the middle of printing normal output lines.
 *
 *	With double buffering, the volume number is whatever volume
 *	number the child is currently working with.
 *
 */
 
int ar_vol ()
{
    int rtnval;
    
    if (mode == 'e') {
	rtnval = vol_estimate;
    } else {
#if HAVE_SHM
	rtnval = dbl_getvol ();
#else
	rtnval = volume;
#endif
    }
    return (rtnval);
}


/*
 *  FUNCTION
 *
 *	last_vol    return number of last volume from previous run
 *
 *  SYNOPSIS
 *
 *	int last_vol ()
 *
 *  DESCRIPTION
 *
 *	Returns the index of the last volume [0, 1, ...] from an
 *	archive read or write, after the archive has been closed.
 *	This value is typically used to detect whether or not
 *	to request a volume switch between executing sequential
 *	major modes in the same run of bru.  I.E., doing an
 *	inspection after a creation requires switching back to
 *	the first volume if the archive spans more than a single
 *	volume.
 *
 *  NOTES
 *
 *	We could have simply exported the variable containing the
 *	the value of the last volume number, but in general it
 *	seems to be better to export function names than to 
 *	export simple variables (because the internal contents
 *	of functions can change transparently to the caller,
 *	if necessary).
 *
 */
 
int last_vol ()
{
    DBUG_ENTER ("last_vol");
    DBUG_PRINT ("lastvol", ("lastvolume = %d", lastvolume));
    DBUG_RETURN (lastvolume);
}


/*
 *	Note that the new buffer size will be smaller than the
 *	current buffer size, since we are reading from the raw
 *	magtape file and got less data than requested.  We
 *	cannot deallocate the larger buffer and allocate a
 *	smaller buffer at this level, since parent routines
 *	have been passed a pointer to the current buffer, and
 *	know nothing about the fact that we have had an "archive
 *	buffer fault".  Thus we will simply force the buffer size
 *	to be smaller but continue to use the same space allocated
 *	for the larger, previous buffer.
 * 
 *	Also note that if the old buffer size was bigger than the
 *	entire archive (say a buffer size of 200K, when the archive
 *	is 4 records of 20K each) some drivers may return all the
 *	archive data, rather than just the data in the first physical
 *	record.  The Sun 3/50 (/dev/rst8, OS release 3.0) is one such 
 *	system.  This may confuse us terribly...
 *
 *	I have commented out the flags.bflag line because it
 *	was suppressing checking of the recorded buffer size against
 *	the actual buffer size in routine readsizes() in readinfo.c.
 *	I am not sure why it was there to begin with...
 *	Fred Fish, 20-Nov-86.  
 */
 
static VOID new_bufsize (size)
int size;
{
    DBUG_ENTER ("new_bufsize");
    /* flags.bflag = TRUE; */
    bufsize = size;
    ar_close ();
    (VOID) do_open ();
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	ar_estimate    accumulate estimate blocks and compute volume
 *
 *  SYNOPSIS
 *
 *	VOID ar_estimate (size);
 *	LBA size;
 *
 *  DESCRIPTION
 *
 *	Given number of logical blocks to accumulate, adds it to the
 *	current total and computes the new volume number based on
 *	the known media size.  If the media size is unknown then
 *	the volume estimate is forced to one (note that vol_estimate
 *	is the volume index).
 *
 */

VOID ar_estimate (size)
LBA size;
{
    lba_estimate += size;
    if (msize != 0) {
	vol_estimate = (int) (KBYTES (lba_estimate) / B2KB (msize));
    } else {
	vol_estimate = 0;
    }
}


/*
 *  FUNCTION
 *
 *	patch_buffer    perform final operations on buffer
 *
 *  SYNOPSIS
 *
 *	VOID patch_buffer (blkp, count)
 *	union blk *blkp;
 *	UINT count;
 *
 *  DESCRIPTION
 *
 *	Given a pointer to a buffer full of archive blocks, and a count
 *	of the number of blocks in the buffer, patch each block with the
 *	correct volume number and checksum.
 *
 *	Because the volume number that a buffer is finally written
 *	to may not correspond to the current volume number at the
 *	time the buffer was filled, the buffer is patched with the
 *	volume number just prior to the write operation.  If the write
 *	generates a fault and the media is switched to a new volume,
 *	the buffer is automatically patched again with the updated
 *	volume number just prior to the write retry.  Since the checksums
 *	cannot be computed until the rest of the data is stable, the
 *	checksum computations are delayed until this time also.
 *
 */
 
VOID patch_buffer (blkp, count)
union blk *blkp;
UINT count;
{
    DBUG_ENTER ("patch_buffer");
    DBUG_PRINT ("ptchb", ("patch %d blocks in buffer", count));
    while (count-- != 0) {
	TOHEX (blkp -> HD.h_vol, volume);
	TOHEX (blkp -> HD.h_chk, chksum (blkp));
	blkp++;
    }
    DBUG_VOID_RETURN;
}

	

/*
 *  FUNCTION
 *
 *	clean_buffer    clear to end of io buffer
 *
 *  SYNOPSIS
 *
 *	static VOID clean_buffer (iobytes)
 *	int iobytes;
 *
 *  DESCRIPTION
 *
 *	Cleans up rest of buffer after a partial read, so that
 *	no junk from a previous read is left hanging around.
 *	This way, other routines don't get fooled by leftover blocks
 *	that have correct checksums and such.
 *
 */
 
static VOID clean_buffer (iobytes)
int iobytes;
{
    char *zap;
    int count;
    
    DBUG_ENTER ("clean_buffer");
    count = bufsize - iobytes;
    zap = blocks[0].bytes + iobytes;
    (VOID) s_memset (zap, 0, count);
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	harderror    report location of hard error
 *
 *  SYNOPSIS
 *
 *	static VOID harderror (errorpba, readflag, iobytes)
 *	LBA errorpba;
 *	BOOLEAN readflag;
 *	int iobytes;
 *
 *  DESCRIPTION
 *
 *	Report location of hard error and adjust soft/hard error
 *	counts appropriately.
 *
 *	The original errno from the original error that triggered
 *	recovery was stored in firsterrno, and is restored prior to
 *	calling the routine to report the error.
 *
 *	On writes, go through the I/O buffer looking at each block
 *	that was not written.  If that block's file sequence number
 *	is greater than the last file sequence number for which an
 *	unrecoverable write warning was reported, then report a
 *      warning for this file, and update the saved file sequence
 *	number.  Thus, on each hard error, we print an error
 *	message that pertains to the entire I/O buffer, and we
 *	print warning messages for any files that have unwritten
 *	data in the I/O buffer.
 *
 */

static VOID harderror (errorpba, readflag, iobytes)
LBA errorpba;
BOOLEAN readflag;
int iobytes;
{
    int index;
    long fseq;
    char *fname;

    DBUG_ENTER ("harderror");
    if (firsterrno > 0) {
	errno = firsterrno;
    }
    if (iobytes < 0) {
	iobytes = 0;
    }
    if (readflag) {
	bru_message (MSG_ARREAD, errorpba + BLOCKS (iobytes));
	einfo.rsoft -= tries;
	einfo.rhard++;
	clean_buffer (iobytes);
    } else {
	bru_message (MSG_ARWRITE, errorpba + BLOCKS (iobytes));
	einfo.wsoft -= tries;
	einfo.whard++;
	for (index = BLOCKS (iobytes); index < BLOCKS (bufsize); index++) {
	    fseq = FROMHEX (blocks[index].HD.h_fseq);
	    DBUG_PRINT ("fseq", ("error at index %d; file %ld", index, fseq));
	    if (fseq > fseqnum) {
		fname = blocks[index].HD.h_name;
		if (FROMHEX (blocks[index].HD.h_magic) == H_MAGIC) {
		    if (FROMHEX (blocks[index].FH.f_flags) & FI_XFNAME) {
			fname = blocks[index].FH.f_xname;
		    }
		}
		bru_message (MSG_UWERR, fname);
		fseqnum = fseq;
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *	Note that querying is suppressed if device cycling is
 *	in effect.  The cycling takes precedence over the
 *	query flag in the brutab file.
 */

static VOID qfwrite ()
{
    char key;

    DBUG_ENTER ("qfwrite");
    if (!reading && (flags.fflag < 2)) {
	if ((ardp != NULL) && (ardp -> dv_flags & D_QFWRITE)) {
	    bru_message (MSG_DATAOVW, afile.fname);
	    key = response ("c => continue  q => quit  r => reload  s => shell", 'r');
	    switch (key) {
		case 'q':
		case 'Q':
		    done ();
		    break;
		case 'r':
		case 'R':
		    switch_media (volume);
		    break;
		case 'c':
		case 'C':
		    break;
		case 'S':
		case 's':
		    fork_shell ();
		    qfwrite ();
		break;
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  Test for, and eject the current media, if supported.  Under
 *  A/UX, for example, an open for write on a write protected
 *  floppy will fail, so at this point we have no file descriptor
 *  on which to perform the ioctl.  Thus if no open file descriptor
 *  is available, attempt to open one for read, just to get a
 *  handle for the ioctl call.
 *
 *  Note that if the floppy is forced to be nonseekable, by hacking
 *  brutab for example, and double buffering is in effect, then
 *  the ejection fails for some currently unknown reason under A/UX.
 *  This may be some sort of kernel bug, or some bug in the double
 *  buffering scheme.  Mysterious...
 *
 */

static VOID eject_media ()
{
    int fildes;

    DBUG_ENTER ("eject_media");
    if (ejectable ()) {
	if ((fildes = afile.fildes) < 0) {
	    fildes = s_open (afile.fname, O_RDONLY, 0);
	}
	if (fildes >= 0) {
	    DBUG_PRINT ("eject", ("eject media in '%s'", afile.fname));
	    if (s_eject (fildes) == SYS_ERROR) {
		bru_message (MSG_EJECT, afile.fname);
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	af_close    close the archive file and mark it as closed
 *
 *  SYNOPSIS
 *
 *	VOID af_close ()
 *
 *  DESCRIPTION
 *
 *	Since we seem to close the archive file in lots of different
 *	places, including from the double buffering code, supply
 *	a function that does this (with error detection) and also
 *	flags the file as closed by setting afile.fildes to -1.
 *
 *  NOTES
 *
 *	Some systems (such as SunOS 4.0 for example) return spurious
 *	errors when the device is closed during tape rewind.  For these
 *	systems, we can set the ignoreclose flag in brutab to ignore
 *	the result of the close system call.
 *
 */

VOID af_close ()
{
    DBUG_ENTER ("af_close");
    if (afile.fildes >= 0) {
	DBUG_PRINT ("afile", ("close archive file '%s'", afile.fname));
	if (s_close (afile.fildes) == SYS_ERROR) {
	    if (ardp != NULL && !(ardp -> dv_flags & D_IGNORECLOSE)) {
		bru_message (MSG_ARCLOSE, afile.fname);
		bru_message (MSG_IGNCLOSE);
	    }
	}
	afile.fildes = -1;
    }
    DBUG_VOID_RETURN;
}
