#ifndef STREAM_H
#define STREAM_H

#ifndef LIBRARIES_DOS_H
#include <libraries/dos.h>	/* just for BPTR definition */
#endif LIBRARIES_DOS_H

/* default buffer size for file streams */
#define S_BUFFSIZE 1024

/* typedefs for prototyping the stream functions */
typedef int  (*__SREAD)( struct Stream *);
typedef int  (*__SREADS)( struct Stream *, uchar *, int);
typedef int  (*__SWRITE)( struct Stream *, uchar);
typedef int  (*__SWRITES)( struct Stream *, uchar *, int);
typedef int  (*__SSEEK)(struct Stream *, int);
typedef int  (*__SFLUSH)( struct Stream *);
typedef int  (*__SCLOSE)( struct Stream *);

/* Define a stream structure.  Normally accessed as a file object internally */
typedef struct Stream {
	uchar	  *buffer;	/* temp file buffer or start of a string */
	uchar	  *current;	/* current pointer into buffer or a string */
	int	  remain;	/* data left to read or space left for write */
	int	  bufflen;	/* max len of string or buffer (for seeking) */
	int	  filepos;	/* current seek position in a file stream */
	uchar	  flags;	/* see below */
	int 	  pchar;	/* character pushed back into stream */
	uchar	  pad1;		/* pad out the structure to longword length */
    short 	  pad2;
	BPTR	  file;		/* filehandle when reading from files */
	__SREAD   rproc;	/* read character from a stream */
	__SWRITE  wproc;	/* write a character to a stream */
	__SREADS  rsproc;	/* read a string from a stream */
	__SWRITES wsproc;	/* write a string to a stream */
	__SFLUSH  fproc;	/* flush a stream */
	__SCLOSE  cproc;	/* flush, free buff mem and close file */
	__SSEEK   sproc;	/* seek, set file position if seekable */
} stream;

/* flag to determine if a character has been pushed back into a stream */
#define S_PUSHED	0x01
/* flag to determine if this stream can be seeked (always true) */
#define S_SEEKABLE	0x02
/* flag to determine if this file can be read from (not write only) */
#define S_READABLE	0x04
/* flag to determine if this file can be written to (not read only) */
#define S_WRITEABLE	0x08
/* when a file has been closed this flag is cleared */
#define S_OPEN		0x10
/* when data has been written to a stream this flag gets set */
#define S_WRITTEN	0x20
/* sometimes closing will require a buffer to be freed too */
#define S_BUFFERED	0x40
/* scanner has to know about String Streams */
#define S_STRING	0x80

/* macros to access streams in an object oriented manner */
#define sgetc(s) (s)->rproc(s)
#define sputc(s,c) (s)->wproc((s),(c))
#define sflush(s) (s)->fproc((s))
#define sclose(s) (s)->cproc((s))
#define sseek(s,p) (s)->sproc((s),(p))
#define sgets(s,b,n) (s)->rsproc((s),(b),(n))
#define sputs(s,b,n) (s)->wsproc((s),(b),(n))
#define sungetc(s,c) { (s)->pchar=(c); (s)->flags|=S_PUSHED; }

/* prototypes for public functions in stream.c */
int MakeStringStream(stream *, uchar *, int, uchar);
int OpenStringStream(stream *, int, uchar);
int MakeFileStream(stream *, BPTR, uchar);
int OpenFileStream(stream *, char *, uchar);
#endif STREAM_H
