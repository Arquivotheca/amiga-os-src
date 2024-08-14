/*
 * The routines in this file
 * read and write ASCII files from the
 * disk. All of the knowledge about files
 * are here. A better message writing
 * scheme should be used.
 */
#include	"ed.h"

#if	AMIGA
#define EOF (-256)
ULONG	ffp;
int	filePointer;
int	wfilePointer;
int	ferror; /* file error flag */
extern UBYTE *fileBuffer;
#else
FILE	*ffp;				/* File pointer, all functions.	*/
#endif


/* returns 1 if file exists, -1 if directory exists, 0 otherwise */
int filexists(fn)
UBYTE	*fn;
{
#if	AMIGA

ULONG	alock;
struct FileInfoBlock __aligned fb;
int type;

alock=Lock(fn, ACCESS_READ);
if(alock) {
    if(Examine(alock,&fb))type = fb.fib_DirEntryType;
    UnLock(alock);
    if(type > 0)return(-1);
    return(1);
}
return(0);
#else
return(TRUE);
#endif

}

ULONG getprot(fn)
UBYTE *fn;
{
ULONG alock;
ULONG protection=0;
#if	AMIGA

	alock = Lock(fn,ACCESS_READ); 	/* get old protection bits */
	if (alock) {
	    if (Examine(alock,finfo)) {
		protection = finfo->fib_Protection;
	    }
	    UnLock(alock);
	}
	else protection = curbp->b_protection;
#endif

return(protection);
}


void setprot(fn,protection)
UBYTE *fn;
ULONG protection;
{
#if	AMIGA
	protection &= ~FIBF_ARCHIVE;	/* leave this bit alone */
	SetProtection(fn,protection);
#endif
}

/*
 * Open a file for reading.
 */
int ffropen(fn)
UBYTE *fn;
{
#if	AMIGA
	if ((ffp = Open(fn,MODE_OLDFILE)) == 0)return (FIOFNF);
	ferror=1; /* setup for the initial read */
	filePointer=ferror;
	wfilePointer=0;
	return(FIOSUC);
#else
	if ((ffp=fopen(fn, "r")) == NULL)
		return (FIOFNF);
	return (FIOSUC);
#endif
}

/*
 * Open a file for writing.
 * Return TRUE if all is well, and
 * FALSE on error (cannot create).
 */

int ffwopen(fn)
UBYTE *fn;
{
int filestate;

#if	AMIGA
	UBYTE nbuffer[40];

	ffp=0;
	if((filestate=filexists(fn)) < 0) {
		mlwrite("Cannot write: directory exists");
		return (FIOERR);
	}
	/* if the file exists */
	if(filestate) {
		/* they have picked the rename option */
	    if (BackupBefore < 0) {
		strncpy(nbuffer,fn,27);
		strcat(nbuffer,".BAK");
		/* if a .BAK exists and it is a file */
		if( filexists(nbuffer)==1 )DeleteFile(nbuffer);
		if(Rename (fn, nbuffer) == FALSE) {
			mlwrite("Cannot rename file for backup");
			return(FIOERR);
		}
	    }
	    else if(BackupBefore == 0) {
		/* they picked the no overwrite option */
		mlwrite("Cannot write: file exists");
		return (FIOERR);
	    }
#if 0
	    else (BackupBefore > 0)DeleteFile(fn); /* unneeded */
#endif
	}

	wfilePointer=0;
	filePointer=0;
	ferror=1;
	if((ffp=Open(fn,MODE_NEWFILE)) != 0) {
		mlwrite("[Writing file]");
		return(FIOSUC);
	}
	else {
#elif VMS
	int fd;
	if ((fd=creat(fn, 0666, "rfm=var", "rat=cr")) < 0
	|| (ffp=fdopen(fd, "w")) == NULL) {
#else
	int fd;
	if ((ffp=fopen(fn, "w")) == NULL) {
#endif
		mlwrite("Cannot open file for writing");
		return (FIOERR);
	}
	mlwrite("[Writing file]");
	return (FIOSUC);
}

/*
 * Close a file.
 * Should look at the status in all systems.
 */
int ffclose()
{
#if	AMIGA
	if(ffp) {
	   if(wfilePointer) { /* write last buffer */
    	    if(ferror >= 0)Write(ffp,fileBuffer,wfilePointer);
	   }
           filePointer=0;
	   wfilePointer=0;
	   Close(ffp);
	}
	return(FIOSUC);
#elif	V7
	if (fclose(ffp) != FALSE) {
		mlwrite("Error closing file");
		return(FIOERR);
	}
	return(FIOSUC);
#else

	fclose(ffp);
	return (FIOSUC);
#endif
}


/*
 * Write a line to the already
 * opened file. The "buf" points to the
 * buffer, and the "nbuf" is its length, less
 * the free newline. Return the status.
 * Check only at the newline.
 */
int ffputline(buf, nbuf)
    UBYTE buf[];
    int nbuf;
{
	 int	i;

	for (i=0; i<nbuf && (ferror >=0) ; ++i)aputc(buf[i]&0xFF, ffp);
	if(ferror >= 0)aputc('\n', ffp);

#if	AMIGA
	if (ferror == (-1)) {
#else
	if (ferror(ffp) != FALSE) {
#endif
		mlwrite("Write I/O error");
		return (FIOERR);
	}
	return (FIOSUC);
}

/*
 * Read a line from a file,
 * and store the bytes in the supplied
 * buffer. The "nbuf" is the length of the
 * buffer. Complain about long lines and lines
 * at the end of the file that don't have a
 * newline present. Check for I/O errors
 * too. Return status.
 */

int ffgetline(buf, nbuf, length)
    UBYTE buf[];
    int nbuf;
    int *length;
{
	 int	c;
	 int	i=0;

	while ((c=agetc(ffp))!=EOF && c!='\n') {
		buf[i++] = c;
		if (i >= nbuf-1) {
			buf[i]=0;
			mlwrite("File has long line");
			*length=i;
			return (FIOSUC);
		}

	}
	if (c == EOF) {
#if	AMIGA
		if (ferror == (-1)) {
#else
		if (ferror(ffp) != FALSE) {
#endif
			mlwrite("File read error");
			*length=i;
			return (FIOERR);
		}
		if (i == 0) {
			*length=0;
			return (FIOEOF);
		}
		else {
		    *length=i;
		    return(FIOSUC);
		}
	}
	*length=i;
	return (FIOSUC);
}

int agetc(ffp)
#if AMIGA
ULONG ffp;
#else
FILE	*ffp;				/* File pointer, all functions.	*/
#endif
{

#if	AMIGA
if(filePointer >= ferror) { /* time for more characters */
    if((ferror=Read(ffp,fileBuffer,MAXFILEBUF)) <= 0)return(EOF);
    filePointer=0;
}
return((int)fileBuffer[filePointer++]);

#else
return(getc(ffp));
#endif

}

int aputc(ch,ffp)
UBYTE ch;
#if AMIGA
ULONG ffp;
#else 
FILE *ffp;
#endif
{

#if	AMIGA
    if(wfilePointer >= MAXFILEBUF) { /* time to write characters */
    	if(ferror= (Write(ffp,fileBuffer,wfilePointer) != wfilePointer)) {
		ferror = -1;
		return(EOF);
    	}
	wfilePointer=0;
    }
    fileBuffer[wfilePointer++]=ch;
#else
    putc(ch,ffp);
#endif
}
