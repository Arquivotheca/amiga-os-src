/*
 * Standard stream package for Amiga Display PostScript.  Allows us to read
 * and write files and strings without having to know which type they are.
 * A stream can be opened for read or write but never both at the same time.
 *
 * No PostScript errors are generated (ie invalidaccess) these are checked
 * by the interpreter before posting the stream requests.  However, the
 * stream ops are checked for validity before performing the request. 
 */

#include "exec/types.h"

#include <exec/memory.h> /* **NOTE** needed for AllocMem/FreeMem calls probably change to internal allocator */

#include "pstypes.h"
#include "stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <proto/dos.h>
#include <proto/exec.h>

/* prototypes for functions that handle string streams (must match stream.h) */
int StringRead(stream *);		  /* matches typedef __SREAD   */
int StringReadS(stream *, uchar *, int);  /* matches typedef __SREADS  */
int StringWrite(stream *,uchar);	  /* matches typedef __SWRITE  */
int StringWriteS(stream *, uchar *, int); /* matches typedef __SWRITES */
int StringSeek(stream *, int);		  /* matches typedef __SSEEK   */
int StringFlush(stream *);		  /* matches typedef __SFLUSH  */
int StringClose(stream *);		  /* matches typedef __SCLOSE  */

/* prototypes for functions that handle file streams (must match stream.h) */
int FileRead(stream *);			  /* matches typedef __SREAD   */
int FileReadS(stream *,uchar *, int);	  /* matches typedef __SREADS  */
int FileWrite(stream *,uchar);		  /* matches typedef __SWRITE  */
int FileWriteS(stream *,uchar *,int);	  /* matches typedef __SWRITES */
int FileSeek(stream *, int);		  /* matches typedef __SSEEK   */
int FileFlush(stream *);		  /* matches typedef __SFLUSH */
int FileClose(stream *);		  /* matches typedef __SCLOSE */


/******************************************************************************
*
* character = StringRead( stream )
*    d0			    a0
*
* Returns the next character from a string stream or EOF if no characters are
* left or the file has been closed.
*
******************************************************************************/
int StringRead(stream *s) {
    if( !(s->flags&S_OPEN) || !(s->flags&S_READABLE) ) return EOF;
    if(s->flags&S_PUSHED) {s->flags&=(~S_PUSHED); return (int)s->pchar;}
    else if(s->remain) { --s->remain; return((int)(*(s->current++))); }
    else return(EOF);
}

/******************************************************************************
*
* count = StringReadS( stream, buffer, length )
*  d0			 a0	 a1	 d0
*
* Reads length characters from the given stream and stores them in the provided
* buffer.  Returns the actual number of characters read or EOF if the stream is
* closed already or the end of the stream has been reached (when no chars left)
*
******************************************************************************/
int StringReadS( stream *s, uchar *b, int len) {
int count,res=0;
    if( !(s->flags&S_OPEN) || !(s->flags&S_READABLE) ) return EOF;
    if(s->flags&S_PUSHED) {s->flags&=(~S_PUSHED); *b++=s->pchar; --len; res=1; }
    if(s->remain && len) {
	count=min(len,s->remain);
	/*memcpy(b,s->current,count);*/
	CopyMem((char *)(s->current),(char *)b,(long)count);
	s->current += count;
	s->remain -= count;
	res+=count;
    }
    return( res ? res : EOF );
}

/******************************************************************************
*
* count = StringWrite( stream, character )
*  d0			 a0	   d0
*
* Writes a character to a string stream and returns the number of characters
* written (simply 1 or 0).  EOF will be returned if the stream has been closed
* or there is no room left in the output string.
*
******************************************************************************/
int StringWrite( stream *s, uchar c) {
    if( !(s->flags&S_OPEN) || !(s->flags&S_WRITEABLE) ) return EOF;
    if(s->remain) { --s->remain; *(s->current++)=c; return(1); }
    else return(0);
}

/******************************************************************************
*
* count = StringWriteS( stream, buffer, length )
*  d0			  a0	  a1	  d0
*
* Writes length characters from buffer to stream and returns amount written. If
* the stream has been closed returns EOF. If there is no space left in the 
* string, returns 0.
*
******************************************************************************/
int StringWriteS( stream *s, uchar *b, int len) {
int count,res=0;
    if( !(s->flags&S_OPEN) || !(s->flags&S_WRITEABLE) ) return EOF;
    if(s->remain) {
	count=min(len,s->remain);
	/*memcpy(s->current,b,count);*/
	CopyMem((char *)b,(char *)(s->current),(long)count);
	s->current += count;
	s->remain -= count;
	res+=count;
    }
    return res;
}

/******************************************************************************
*
* position = StringSeek( stream, position )
*   d0			   a0	    d0
*
* Seeks to position in a stream.  Returns EOF if seek position invalid or seeks
* are not permitted for this stream or the stream has been closed already.
*
******************************************************************************/
int StringSeek( stream *s, int p) {
    if( !(s->flags&S_OPEN) || !(s->flags&S_SEEKABLE) ||
	(p<0) ||
	(p>=s->bufflen)) return(EOF);
    s->current = s->buffer+p;	/* reset buffer pointer */
    s->remain = s->bufflen-p; 	/* set data/space remaining in buffer */
    s->flags &= ~S_PUSHED;	/* invalidate pushed back character */
    return p;			/* return given position */
}

/******************************************************************************
*
* success = StringFlush( stream )
*   d0			   a0
*
* Flushes data in a string stream.  Flushing a read stream causes seek to EOF.
******************************************************************************/
int StringFlush(stream *s) { 
    if(!(s->flags & S_OPEN)) return FALSE;
    if(s->flags & S_READABLE) {	/* if it's a read stream ... */
	s->flags &= ~(S_PUSHED);/* flush pushed back character */
	s->remain = 0;		/* then flush read to EOF */
	s->current=s->buffer+s->bufflen;
    }
    return TRUE;
}

/******************************************************************************
*
* success = StringClose( stream )
*   d0			   a0
*
* Closes a string stream invalidating all further access to that stream.  If
* the stream is marked as buffered then the buffer memory will be freed too.
* Returns TRUE if the stream was closed, FALSE if already closed.
*******************************************************************************/
int StringClose(stream *s) {
    if(s->flags & S_OPEN) {
	if(s->flags & S_BUFFERED) FreeMem(s->buffer,s->bufflen);
	s->current = s->buffer = NULL;	/* no valid buffer */
	s->remain = s->bufflen = NULL;	/* so no valid data either */
	s->flags &= ~S_OPEN;		/* mark as not open */
	return TRUE;
    }
    else return FALSE;
}

/******************************************************************************
*
* success = MakeStringStream( stream, string, length, access )
*   d0				a0	a1	d0	d1
*
* Initialises a string stream for access.  Access is a mask made of S_READABLE
* or S_WRITEABLE and S_SEEKABLE.  All later accesses to that stream must observe
* the mask rules (though reading a write only stream will just return EOF).
*******************************************************************************/
int MakeStringStream( stream *s, uchar *string, int length, uchar access ) {
    s->current=s->buffer=string;	/* where the data will come from*/
    s->remain=s->bufflen=length;	/* amount of data/space left in buffer*/
    s->flags   = S_STRING|S_OPEN|access;		/* setup access permissions */
    s->rproc   = StringRead;
    s->rsproc  = StringReadS;
    s->wproc   = StringWrite;
    s->wsproc  = StringWriteS;
    s->sproc   = StringSeek;
    s->cproc   = StringClose;
    s->fproc   = StringFlush;
    return TRUE;
}

/******************************************************************************
*
* success = OpenStringStream( stream, length, access )
*   d0				a0	d0	d1
*
* Initialises a string stream for access and allocates a buffer for output. It
* is usual for an OpenStringStream() call to open for output (S_WRITEABLE) but
* it's OK to open a string stream as S_READABLE if another stream created later
* shares the same buffer and writes to it.
*******************************************************************************/
int OpenStringStream(stream *s, int size, uchar access) {
uchar *string;
    if(string=(uchar *)AllocMem(size,MEMF_PUBLIC|MEMF_CLEAR)) {
	MakeStringStream(s,string,size,access|S_BUFFERED);
	return TRUE;
    }
    else return FALSE;
}

/******************************************************************************
*
* character = FileRead( stream )
*    d0			  a0
*
* Returns the next character from a file stream or EOF if no characters are
* left or the file is readonly or the file has been closed.
*
******************************************************************************/
int FileRead(stream *s) {
    if( !(s->flags&S_OPEN) || !(s->flags&S_READABLE) ) return EOF;
    if(s->flags & S_PUSHED) { s->flags &= ~S_PUSHED; return (int)s->pchar; }
    if(s->remain == 0) {	/* no data remaining in the buffer */
	s->current=s->buffer;	/* reset buffer position */
	s->remain = Read(s->file,s->buffer,s->bufflen); /* and read some data */
	s->filepos += s->remain;	/* update file seek position */
    }
    if(s->remain) { --(s->remain); return((int)(*(s->current++))); }
    else return(EOF);
}

/******************************************************************************
*
* count = FileReadS( stream, buffer, length )
*  d0		       a0      a1      d0
*
* Reads length characters from the given stream and stores them in the provided
* buffer.  Returns the actual number of characters read or EOF if the stream is
* closed already or the end of the stream has been reached (when no chars left)
*
******************************************************************************/
int FileReadS( stream *s, uchar *b, int len) {
int count,res=0;
    if( !(s->flags&S_OPEN) || !(s->flags&S_READABLE) ) return EOF;
    if(s->flags&S_PUSHED) {s->flags&=(~S_PUSHED); *b++=s->pchar; --len; res=1; }

    while(len) {
	if(s->remain==0) { /* need to read from the file */
	    s->current=s->buffer;	/* reset buffer position */
	    s->remain = Read(s->file,s->buffer,s->bufflen); /* and read some data */
	    s->filepos += s->remain;	/* update file seek position */
	}
	if(s->remain) {
	    count=min(len,s->remain);
	    /*memcpy(b,s->current,count);*/
	    CopyMem((char *)(s->current),(char *)b,(long)count);
	    s->current += count;
	    s->remain -= count;
	    res+=count;
		len -=count;
	}
	else len=0;	/* ran out of characters */

    }
    return( res ? res : EOF );
}

/******************************************************************************
*
* count = FileWrite( stream, character )
*  d0		       a0	 d0
*
* Writes a character to a file stream and returns the number of characters
* written (simply 1 or 0).  EOF will be returned if the stream has been closed
* or is readonly or there is an error flushing the buffer.
*
******************************************************************************/
int FileWrite(stream *s, uchar c) {
int written;
    if( !(s->flags&S_OPEN) || !(s->flags&S_WRITEABLE) ) return EOF;
    if(!(s->remain)) { 		/* if buffer is full already */
	s->current=s->buffer;	/* reset buffer pointer */
	s->remain=s->bufflen;	/* reset space left in buffer */
	written=Write(s->file,s->buffer,s->bufflen);
	s->flags &= ~S_WRITTEN;
	s->filepos += written;	/* update file position */
	if(written != s->bufflen) return FALSE;
    }
    --(s->remain);
    *(s->current++)=c;
    s->flags |= S_WRITTEN;
    return TRUE;
}

/******************************************************************************
*
* count = FileWriteS( stream, buffer, length )
*  d0		        a0      a1      d0
*
* Writes length characters from buffer to the given stream.  Returns the actual
* number of characters written or EOF if the stream is closed already or the
* stream is readonly.
*
******************************************************************************/
int FileWriteS( stream *s, uchar *b, int len) {
int count,res=0;

    if( !(s->flags&S_OPEN) || !(s->flags&S_WRITEABLE) ) return EOF;
    while(len) {
	if(!(s->remain)) { 		/* need to flush to the file ? */
	    s->current=s->buffer;	/* yes, reset buffer position */
	    count = Write(s->file,s->buffer,s->bufflen); /* flush out the buffer */
	    s->flags &= ~S_WRITTEN;	/* buffer not dirty now */
	    s->filepos += count;	/* bump file position */
	    s->remain = s->bufflen;	/* whole buffer is free */
	    if(count != s->bufflen) len=0;	/* ran out of space ?? */
	}
	if(s->remain && len) {
	    count=min(len,s->remain);
	    /*memcpy(b,s->current,count);*/
	    CopyMem((char *)b,(char *)(s->current),(long)count);
	    s->current += count;
	    s->remain -= count;
	    res+=count;
	    len-=count;
	    s->flags |= S_WRITTEN;	/* buffer is dirty */
	}
    }
    return( res ? res : EOF );
}

/******************************************************************************
*
* success = FileFlush( stream )
*   d0			 a0
*
* Flushes data in a file stream.  Flushing a read stream causes seek to EOF.
******************************************************************************/
int FileFlush(stream *s) {
    if(!(s->flags&S_OPEN)) return FALSE;
    if(s->flags & S_WRITTEN) {		/* if something to write out */
	s->filepos += Write(s->file,s->buffer,s->bufflen-s->remain);
	s->flags &= ~S_WRITTEN;
    }
    if(s->flags & S_READABLE) { /* is it a read stream ? */
	s->filepos = Seek(s->file,0,OFFSET_END);
	s->remain = 0;
	s->flags &= ~S_PUSHED;	/* invalidate pushed character */
    }
    else s->remain = s->bufflen; /* no it's a write stream */
    s->current=s->buffer;
    return TRUE;
}

/******************************************************************************
*
* success = FileClose( stream )
*   d0			 a0
*
* Closes a file stream invalidating all further access to that stream.  If
* the stream is marked as buffered then the buffer memory will be freed too.
* Returns TRUE if the stream was closed, FALSE if already closed.
*******************************************************************************/
int FileClose(stream *s) {
    if(s->flags & S_OPEN) {
	if(s->flags & S_WRITEABLE) FileFlush(s);	/* flush and get returncode */
	Close(s->file);		/* and close the file */
	return(StringClose(s));	/* the rest is the same as strings */
    }
    else return FALSE;
}

/******************************************************************************
*
* position = FileSeek( stream, position )
*   d0			 a0	  d0
*
* Seeks to position in a stream.  Returns EOF if seek position invalid or seeks
* are not permitted for this stream or the stream has been closed already.
*
******************************************************************************/
int FileSeek( stream *s, int p) {
int ret;

    if( !(s->flags&S_OPEN) || !(s->flags&S_SEEKABLE) ) return EOF;

    FileFlush(s);	/* expensive !!! */
    ret = Seek(s->file,p,OFFSET_BEGINNING);
    if(ret != -1) { s->filepos = p; }
    else p = ret;
    return p;
}

/******************************************************************************
*
* success = MakeFileStream( stream, file, access )
*   d0			      a0     a1     d0
*
* Initialises a file stream for access.  The file should already been opened
* and access should reflect the mode in which that file was opened.  Memory
* will be allocated for buffering reads and writes to that file.
*******************************************************************************/
int MakeFileStream( stream *s, BPTR file, uchar access ) {
    if(s->current=s->buffer=(uchar *)AllocMem(S_BUFFSIZE,MEMF_PUBLIC)) {
	s->remain=s->bufflen=S_BUFFSIZE;/* amount of space left in buffer*/
	s->flags   = S_OPEN|S_BUFFERED|access;	/* setup access permissions */
	s->rproc   = FileRead;
	s->rsproc  = FileReadS;
	s->wproc   = FileWrite;
	s->wsproc  = FileWriteS;
	s->sproc   = FileSeek;
	s->cproc   = FileClose;
	s->fproc   = FileFlush;
	s->filepos = 0;
	if(access & S_READABLE) s->remain = 0; /* no data read into buffer yet */
	s->file    = file;
	return TRUE;
    }
    else return FALSE;
}

/******************************************************************************
*
* success = OpenFileStream( stream, name, access )
*   d0			      a0     a1     d0
*
* Attempts to open the named file for read or write (depending on access) and
* if successful constructs a stream with the filehandle returned from Open().
*******************************************************************************/
int OpenFileStream( stream *s, char *name, uchar access) {
BPTR file;
int mode;

    if(access & S_READABLE) mode = MODE_OLDFILE; else mode = MODE_NEWFILE;
    if(file=Open(name,mode)) {
	if(MakeFileStream(s,file,access)) return TRUE;
	else Close(file);
    }
    return FALSE;
}

