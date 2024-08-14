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
 *	create.c    functions for creating archive
 *
 *  SCCS
 *
 *	@(#)create.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains functions concerned primarily with creating new
 *	archives.
 *
 */

#include "globals.h"

/*
 *	These macros are used to determine whether or not compression
 *	of the file to be archived will be attempted.
 */

#define FSIZE(fip)  (fip -> bstatp -> bst_size)
#define AUTOZ(fip)  ((flags.Zflag & Z_AUTO) && (FSIZE (fip) > sfthreshold))
#define TRYZ(fip)   ((flags.Zflag & Z_ON) || AUTOZ (fip))

#define ZEROBLK(blkp) (VOID)s_memset((blkp)->bytes,0,ELEMENTS((blkp)->bytes))


/*
 *	Local function declarations.
 */

static VOID wfile PROTO((struct finfo *fip));
static int preparefile PROTO((struct finfo *fip));
static VOID do_write PROTO((struct finfo *fip));
static VOID wheader PROTO((struct finfo *fip));
static VOID wcontents PROTO((struct finfo *fip));
static VOID bldhdr PROTO((union blk *blkp, struct finfo *fip));
static VOID winfo PROTO((struct finfo *afip));
static VOID markend PROTO((struct finfo *afip));
static VOID check_intr PROTO((void));
static BOOLEAN changed PROTO((struct finfo *fip));


/*
 *  NAME
 *
 *	create    main entry point for creating new archives
 *
 *  SYNOPSIS
 *
 *	VOID create ()
 *
 *  DESCRIPTION
 *
 *	This is the main entry point for creating new archives.
 *	It is called once all the command line options have been
 *	processed and common initialization has been performed.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin create
 *	    Set current mode to create
 *	    Open the archive file
 *	    Save state of signal handling
 *	    Catch signals
 *	    Write the archive header block
 *	    Walk directory tree, archiving each file
 *	    Mark end of archive
 *	    Restore previous signal handling state
 *	    If not suppressing link warnings
 *		Complain about unresolved links
 *	    End if
 *	    Free the list of unresolved links
 *	    Close the archive file
 *	End create
 *
 */

VOID create ()
{
    SIGTYPE prevINT;
    SIGTYPE prevQUIT;
    
    DBUG_ENTER ("create");
    mode = 'c';
    reload ("archive creation");
    ar_open ();
    sig_push (&prevINT, &prevQUIT);
    sig_catch ();
    winfo (&afile);
    if (flags.PFflag) {
	filter (wfile);
    } else {
	tree_walk (wfile);
    }
    markend (&afile);
    sig_pop (&prevINT, &prevQUIT);
    if (!flags.lflag) {
	unresolved ();
    }
    free_list ();
    ar_close ();
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	wfile    write a file to archive
 *
 *  SYNOPSIS
 *
 *	static VOID wfile (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Given pathname to a file, writes that file to the archive.
 *	Each regular file written in the archive has a header block 
 *	containing information about the file, and zero or more data
 *	blocks containing the actual file contents.
 *
 *	Note that directories, block special files, character special
 *	files, and other special files only have a header block written.
 *	This is so those nodes can be restored with the proper attributes.
 *	This is a particular failing of the Unix "tar" utility.
 *
 *	Note that the file must be opened with the O_NDELAY flag
 *	set or else character special files associated with communication
 *	lines will cause the open to block until the line has carrier
 *	present.  See open(2) in the Unix Sys V User's Manual.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin wfile
 *	    Check for interrupts
 *	    If there is a file to write and is not "." or ".." then
 *		If this file is selected then
 *		    If archiving confirmed then
 *			Committed, so write it to archive
 *		    End if
 *		End if
 *	    End if
 *	End wfile
 *
 */

static VOID wfile (fip)
struct finfo *fip;
{
    DBUG_ENTER ("wfile");
    check_intr ();
    DBUG_PRINT ("path", ("write \"%s\" to archive", fip -> fname));
    if (*fip -> fname != EOS && !DOTFILE (fip -> fname)) {
	if (selected (fip)) {
	    if (confirmed ("c", fip)) {
		do_write (fip);
	    }
	}
    }
    DBUG_VOID_RETURN;
}

/*
 *  PSEUDO CODE (obsolete)
 *
 *	Begin preparefile
 *	    Do verbosity processing
 *	    Set current file block to zero
 *	    If is special file or linked to another file then
 *		Write header block only
 *	    Else
 *		If file is accessible for read then
 *		    Open the file for read
 *		    If open failed then
 *			Report open error to user
 *		    Else
 *			Write the header block to archive
 *			Write the file contents to archive
 *			If file close failed then
 *			    Report close error to user
 *			End if
 *			If should mark file as archived then
 *			    Set file archived bit
 *			Else if should mark file unarchived then
 *			    Reset file archived bit
 *			Endif
 *		    End if
 *		End if
 *	    End if
 *	End preparefile
 */

static int preparefile (fip)
struct finfo *fip;
{
    int fileok = FALSE;

    DBUG_ENTER ("preparefile");
    if (IS_REG (fip -> bstatp -> bst_mode) && fip -> lname == NULL) {
	if (file_access (fip -> fname, A_READ, TRUE)) {
	    GETPROT (fip);
	    FULLACCESS (fip);
	    fip -> fildes = s_open (fip -> fname, O_RDONLY|O_NDELAY, 0);
	    if (fip -> fildes == SYS_ERROR) {
		bru_message (MSG_OPEN, fip -> fname);
	    } else {
		fileok = TRUE;
		if (TRYZ (fip)) {
		    if (!compressfip (fip)) {
			bru_message (MSG_ZFAILED, fip -> fname);
			discard_zfile (fip);
		    } else {
			if (fip -> zsize > fip -> bstatp -> bst_size) {
			    if (flags.vflag > 3 && !(flags.Zflag & Z_AUTO)) {
				bru_message (MSG_NOSHRINK, fip -> fname);
			    }
			    discard_zfile (fip);
			}
		    }
		}
	    }
	}
    }
    DBUG_RETURN (fileok);
}


/*
 *  FUNCTION
 *
 *	do_write    committed to doing write so do it
 *
 *  SYNOPSIS
 *
 *	static VOID do_write (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Once it has been decided to write a file to the archive,
 *	after applying all tests, this function is called to do
 *	the actual write.
 *
 *	If the command line option -Z to store files in a compressed
 *	form has been given, the input file is compressed, and the
 *	resulting temporary compressed file is saved in the archive.
 *	For backwards compatibility with versions of bru that don't
 *	understand compressed files, the saved file name has a ".Z"
 *	extension appended to it, the standard name for compressed
 *	versions of files.  The original file size is saved in an
 *	area that won't be seen by older versions of bru, and the
 *	compressed file size is stored in the normal size location.
 *	Thus, if an archive built by a new version of bru is read
 *	by an old version of bru, it will appear that all the files
 *	were compressed before archiving, and the files will be
 *	extracted as compressed files.
 *
 *	This routine has been reorganized considerably in order to
 *	get file compression implemented, because the input file 
 *	needs to be compressed BEFORE verbosity() is called.  Plus
 *	verbosity() should not be called if we can't access or open
 *	the file for some reason, and the file is a file we will
 *	need to access and read.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin do_write
 *	    If not a symbolic link then
 *		Set linked file name to NULL.
 *	    End if
 *	    If not directory and more than one link then
 *		Add link to list of linked files
 *	    End if
 *	End do_write
 *
 */

static VOID do_write (fip)
struct finfo *fip;
{
    BOOLEAN fileok = FALSE;
    static long fseq = 0;

    DBUG_ENTER ("do_write");
    if (!IS_FLNK (fip -> bstatp -> bst_mode)) {
	fip -> lname = NULL;
    }
    if (!IS_DIR (fip -> bstatp -> bst_mode) && fip -> bstatp -> bst_nlink > 1) {
	fip -> lname = add_link (fip);
    }
    if (namesfit (fip)) {
	fileok = preparefile (fip);
	if (!IS_REG (fip -> bstatp -> bst_mode) || fip -> lname != NULL) {
	    verbosity (fip);
	    fip -> flba = 0L;
	    fip -> fseq = ++fseq;
	    wheader (fip);
	} else if (fileok) {
	    verbosity (fip);
	    fip -> flba = 0L;
	    fip -> fseq = ++fseq;
	    wheader (fip);
	    wcontents (fip);
	    if (s_close (fip -> fildes) == SYS_ERROR) {
		bru_message (MSG_CLOSE, fip -> fname);
	    }
	    SETOLDPROT (fip);
	    discard_zfile (fip);
	    if (flags.Asflag) {
		mark_archived (fip);
	    } else if (flags.Acflag) {
		clear_archived (fip);
	    }
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	wheader    build and write header block to archive
 *
 *  SYNOPSIS
 *
 *	static VOID wheader (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Builds a header block and writes it to the archive.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin wheader
 *	    Get next archive block
 *	    Zero the block
 *	    Copy the file name to block header
 *	    If linked to another file then
 *		Copy the linked name to file header
 *	    End if
 *	    Build the rest of the file header
 *	    Write the header block to archive
 *	End wheader
 *
 */

static VOID wheader (fip)
struct finfo *fip;
{
    union blk *blkp;

    DBUG_ENTER ("wheader");
    blkp = ar_next ();
    ZEROBLK (blkp);
    bldhdr (blkp, fip);
    ar_write (fip, H_MAGIC);
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	wcontents    write contents of normal file to archive
 *
 *  SYNOPSIS
 *
 *	static VOID wcontents (fip)
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	This routine is responsible for writing the contents of a normal
 *	file to the archive.
 *
 *	Note that each block is zeroed to assure that no junk is left
 *	laying around in it from it's last use.  This is necessary because
 *	the data section is only part of the block (otherwise only the
 *	last block would require zeroing).  The header is initialized
 *	by ar_write, which may not clear unused fields.
 *
 *  BUGS
 *
 *	In previous versions of bru, if the read failed or the file was
 *	truncated, the number of blocks in the archive did not match the
 *	number expected from the file size stored in the file header
 *	block.  This caused problems when the archive was read because
 *	the header block of the following file was never found.
 *
 *	The solution was to always write the correct number of blocks
 *	to the archive, regardless of whether or not their contents
 *	are correct.  This fixes the previous problem and insures
 *	that as much as possible of the file is preserved.  Note
 *	however, that the only error observed is when the archive
 *	is created.  Once the archive is created, as far as it is
 *	concerned, all the internal data is correct.
 *
 *	Another problem is what to do about files that grow while
 *	archiving.  The solution implemented currently is simply to warn
 *	the user and discard any file additions.  The file scanning
 *	routines could be made smarter about finding a data block when
 *	expecting a file header.  This would allow the additional data
 *	to be archived.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin wcontents
 *	    Initialize the file truncated flag to FALSE
 *	    Get the number of bytes in the file to be archived
 *	    If the file has anything in it at all then
 *		For each block of data to be archived
 *		    Increment the file block number
 *		    Get the next archive block
 *		    Zero the block
 *		    If only a partial block left to read then
 *			Read only the correct number of bytes
 *		    End if
 *		    Read data from file into block
 *		    Write the block to the archive
 *		    If read was not correct then
 *			Set the file has been truncated flag
 *		    End if
 *		End for
 *		If last read returned with error condition then
 *		    Warn user about read error
 *		End if
 *		If the file was truncated then
 *		    Warn user about truncation
 *		Else if more data can be read from input file then
 *		    Warn user that the file grew while archiving
 *		Else if file contents changed (size didn't) then
 *		    Warn user that file contents changed
 *		End if
 *	    End if
 *	End wcontents
 *
 */

static VOID wcontents (fip)
struct finfo *fip;
{
    off_t bytes;
    int iobytes;
    UINT rbytes;
    union blk *blkp;
    char testbuf[DATASIZE];
    BOOLEAN truncated;
    int fildes;
    char *fname;

    DBUG_ENTER ("wcontents");
    truncated = FALSE;
    if (IS_COMPRESSED (fip)) {
	bytes = fip -> zsize;
	fildes = fip -> zfildes;
	fname = fip -> zfname;
    } else {
	bytes = fip -> bstatp -> bst_size;
	fildes = fip -> fildes;
	fname = fip -> fname;
    }
    DBUG_PRINT ("rbytes", ("read and save %ld bytes from '%s'", bytes, fname));
    if (bytes > 0) {
	for (rbytes = DATASIZE; bytes > 0; bytes -= rbytes) {
	    fip -> flba++;
	    blkp = ar_next ();
	    ZEROBLK (blkp);
	    if (bytes < DATASIZE) {
		rbytes = bytes;
	    }
	    iobytes = s_read (fildes, blkp -> FD, rbytes);
	    ar_write (fip, D_MAGIC);
	    if (iobytes != rbytes) {
		truncated = TRUE;
	    }
	}
	if (iobytes == SYS_ERROR) {
	    bru_message (MSG_READ, fname);
	}
	if (truncated) {
	    bru_message (MSG_FTRUNC, fname);
	} else if (s_read (fildes, testbuf, DATASIZE) > 0) {
	    bru_message (MSG_FGREW, fname);
	} else if (changed (fip)) {
	    bru_message (MSG_FDATACHG, fip -> fname);
	}
    }
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	bldhdr    do grunge work for building a file header
 *
 *  SYNOPSIS
 *
 *	static VOID bldhdr (blkp, fip)
 *	union blk *blkp;
 *	struct finfo *fip;
 *
 *  DESCRIPTION
 *
 *	Does the actual work of building each field in the file
 *	header block.
 *
 *	Note that if the file is a directory, the size in the file
 *	header block is set to zero, since we don't actually archive
 *	any of the contents of directory files.
 *
 */

static VOID bldhdr (blkp, fip)
union blk *blkp;
struct finfo *fip;
{
    DBUG_ENTER ("bldhdr");
    if (fip -> bname2) {
	(VOID) s_strcpy (blkp -> FH.f_lname, fip -> bname2);
    }
    if (fip -> fi_flags & FI_XFNAME || fip -> fi_flags & FI_XLNAME) {
	(VOID) s_memcpy (blkp -> FH.f_xname, fip -> bname3, XNAMESIZE);
    }
    TOHEX (blkp -> HD.h_magic, H_MAGIC);
    TOHEX (blkp -> FH.f_mode, fip -> bstatp -> bst_mode);
    TOHEX (blkp -> FH.f_ino, fip -> bstatp -> bst_ino);
    TOHEX (blkp -> FH.f_dev, fip -> bstatp -> bst_dev);
    TOHEX (blkp -> FH.f_rdev, fip -> bstatp -> bst_rdev);
    TOHEX (blkp -> FH.f_nlink, fip -> bstatp -> bst_nlink);
    TOHEX (blkp -> FH.f_uid, fip -> bstatp -> bst_uid);
    TOHEX (blkp -> FH.f_gid, fip -> bstatp -> bst_gid);
    if (IS_REG (fip -> bstatp -> bst_mode)) {
	if (IS_COMPRESSED (fip)) {
	    TOHEX (blkp -> FH.f_size, fip -> zsize);
	    TOHEX (blkp -> FH.f_xsize, fip -> bstatp -> bst_size);
	    TOHEX (blkp -> FH.f_nzbits, nzbits);
	} else {
	    TOHEX (blkp -> FH.f_size, fip -> bstatp -> bst_size);
	}
    } else {
	TOHEX (blkp -> FH.f_size, 0L);
    }
    TOHEX (blkp -> FH.f_atime, fip -> bstatp -> bst_atime);
    TOHEX (blkp -> FH.f_mtime, fip -> bstatp -> bst_mtime);
    TOHEX (blkp -> FH.f_ctime, fip -> bstatp -> bst_ctime);
    TOHEX (blkp -> FH.f_flags, fip -> fi_flags);
    TOHEX (blkp -> FH.f_fibprot, fip -> fib_Protection);
    (VOID) s_strcpy (blkp -> FH.f_fibcomm, fip -> fib_Comment);
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	winfo    write the archive header block
 *
 *  SYNOPSIS
 *
 *	static VOID winfo (afip)
 *	struct finfo *afip;
 *
 *  DESCRIPTION
 *
 *	Builds and writes the archive header block, which is
 *	the first block in the archive.  This block contains
 *	useful information about the archive.
 *
 *	Previous versions just either picked up a copy of utsname from
 *	the <sys/utsname.h> file, or the faked version.  In both cases
 *	the structure was identical, and of the same size.  Then along
 *	came xenix release 3.0, with a slightly different layout for
 *	the utsname structure, and I realised that it was a bad idea
 *	to blindly use the utsname structure directly in the declarations
 *	to describe archive blocks.  This is really an internal data
 *	structure containing system information, that simply happens
 *	to have a one-to-one mapping with a system structure on some
 *	systems, but not all.
 *
 */

static VOID winfo (afip)
struct finfo *afip;
{
    union blk *blkp;

    DBUG_ENTER ("winfo");
    blkp = ar_seek (0L, 0);
    ZEROBLK (blkp);
    (VOID) namesfit (afip);
    TOHEX (blkp -> AH.a_uid, info.bru_uid);
    TOHEX (blkp -> AH.a_gid, info.bru_gid);
    (VOID) copy (blkp -> AH.a_name, afile.fname, sizeof (blkp -> AH.a_name));
    if (flags.Lflag) {
	if (!copy (blkp -> AH.a_label, label, BLABELSIZE)) {
	    bru_message (MSG_LABEL, label);
	}
    }
    (VOID) s_strcpy (blkp -> AH.a_id, id);
    if (s_uname (&utsname) == SYS_ERROR) {
	bru_message (MSG_UNAME);
    } else {
	(VOID) s_strncpy (blkp -> AH.a_sysname, utsname.sysname, UTSELSZ - 1);
	(VOID) s_strncpy (blkp -> AH.a_nodename, utsname.nodename, UTSELSZ-1);
	(VOID) s_strncpy (blkp -> AH.a_release, utsname.release, UTSELSZ - 1);
	(VOID) s_strncpy (blkp -> AH.a_version, utsname.version, UTSELSZ - 1);
#if HAVE_UTSNAME_MACHINE
	(VOID) s_strncpy (blkp -> AH.a_machine, utsname.machine, UTSELSZ - 1);
#else
	(VOID) s_strncpy (blkp -> AH.a_machine, "unknown", UTSELSZ - 1);
#endif
    }
    ar_write (afip, A_MAGIC);
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	markend    mark the end of the archive with a terminator block
 *
 *  SYNOPSIS
 *
 *	static VOID markend (afip)
 *	struct finfo *afip;
 *
 *  DESCRIPTION
 *
 *	Marks the end of the archive by writing a terminator block.
 *
 */

static VOID markend (afip)
struct finfo *afip;
{
    union blk *blkp;

    DBUG_ENTER ("markend");
    blkp = ar_next ();
    ZEROBLK (blkp);
    ar_write (afip, T_MAGIC);
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	check_intr    check for an interrupt received
 *
 *  SYNOPSIS
 *
 *	static VOID check_intr ()
 *
 *  DESCRIPTION
 *
 *	Checks to see if an interrupt was received while signals
 *	were being caught.  If so, marks the end of the archive
 *	and cleans up and exits.
 *
 */

static VOID check_intr ()
{
    if (interrupt) {
	DBUG_PRINT ("sig", ("interrupt recognized"));
	einfo.warnings++;
	markend (&afile);
	ar_close ();
	done ();
    }
}


/*
 *	Do a stat on the file and compare the original st_mtime
 *	value against the current st_mtime value.  Return FALSE
 *	if they match, TRUE if they don't match.  We save the old
 *	time by copying it to a member of a temporary stat structure,
 *	thus we don't need to know what the type of st_mtime is.
 *
 *	Note that if less than one second elapses since bru does
 *	the initial stat and then the final stat, or someone
 *	writes on the file in the first second it is open, it is
 *	possible that bru will miss the change because of the one
 *	second granularity of the times.
 *
 */

static BOOLEAN changed (fip)
struct finfo *fip;
{
    BOOLEAN result;
    struct bstat ostat;

    DBUG_ENTER ("changed");
    result = FALSE;
    ostat.bst_mtime = fip -> bstatp -> bst_mtime;
    if (fipstat (fip, TRUE)) {
	if (ostat.bst_mtime != fip -> bstatp -> bst_mtime) {
	    result = TRUE;
	}
    }
    DBUG_RETURN (result);
}
